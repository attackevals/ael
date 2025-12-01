package simplefileserver

import (
    "context"
    "errors"
    "fmt"
    "io/ioutil"
    "net/http"
    "os"
    "path/filepath"
    "strings"
    "sync"

    "github.com/gorilla/mux"

    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/handlers/handler_util"
    "evals.mitre.org/control_server/handlers/handler_manager"
    "evals.mitre.org/control_server/logger"
)

const (
    serverErrMsg = "Internal server error\n"
    serverBadRequestMsg = "Bad request\n"
    defaultDirPath = "handlers/simplefileserver/files"
    HANDLER_NAME = "simplefileserver"
)

type SimpleFileServerHandler struct {
    restAPIaddress string
    server *http.Server
    prefixMappings []PrefixMappingEntry
    waitGroup *sync.WaitGroup
}

type noDirListingFileSystem struct {
    fs http.FileSystem
}

type ResponseWriterWithLogging struct {
    responseWriter http.ResponseWriter
    status int
    size int
    requestedFile string
}

type PrefixMappingEntry struct {
    Prefix string
    Dir string
}

// Factory method for creating a SimpleFileServerHandler handler
func simpleFileServerHandlerFactory(wg *sync.WaitGroup) *SimpleFileServerHandler {
    // restAPIaddress and other fields will be initialized when handler is started
    return &SimpleFileServerHandler{
        waitGroup: wg,
    }
}

// Creates and registers the SimpleFileServerHandler C2 handler as an available C2 handler.
func init() {
    handler_manager.RegisterAvailableHandler(simpleFileServerHandlerFactory(&sync.WaitGroup{}))
}

// Reference: https://www.alexedwards.net/blog/disable-http-fileserver-directory-listings
func (n noDirListingFileSystem) Open(path string) (http.File, error) {
    fileDesc, err := n.fs.Open(path)
    if err != nil {
        logger.Error(fmt.Sprintf("[Simple File Server] Requested file %s not found", path))
        return nil, err
    }

    statInfo, err := fileDesc.Stat()
    if statInfo.IsDir() {
        // If directory, attempt to serve index.html instead
        fileDesc.Close()
        indexPath := filepath.Join(path, "index.html")
        return n.fs.Open(indexPath)
    }

    return fileDesc, nil
}

func GetPrefixMappings(configEntry config.HandlerConfigEntry) ([]PrefixMappingEntry, error) {
    if mappingsEntry, ok := configEntry["mappings"]; ok {
        mappingsList, ok := mappingsEntry.([]interface{})
        if !ok {
            return nil, errors.New("Simple file server prefix mappings config must be a list of map[string]string")
        }
        /*mappings, ok := mappingsEntry.([]map[string]string)
        if !ok {
            return nil, errors.New("Simple file server prefix mappings config must be []map[string]string")
        }*/
        retMappings := make([]PrefixMappingEntry, 0)
        for _, mappingInterface := range mappingsList {
            mapping, ok := mappingInterface.(map[string]interface{})
            if !ok {
                return nil, errors.New("Prefix mapping entry must be a map of strings to strings.")
            }
            prefixInf, ok := mapping["prefix"]
            if !ok {
                return nil, errors.New("Prefix mapping is missing prefix path.")
            }
            prefix, ok := prefixInf.(string)
            if !ok {
                return nil, errors.New("Prefix must be a string")
            }
            dirInf, ok := mapping["dir"]
            if !ok {
                return nil, errors.New(fmt.Sprintf("Prefix mapping for path %s is missing directory.", prefix))
            }
            dir, ok := dirInf.(string)
            if !ok {
                return nil, errors.New("Dir must be a string")
            }
            dirPath, err := filepath.Abs(dir)
            if err != nil {
                return nil, errors.New(fmt.Sprintf("Failed to get absolute path for dir %s: %s", dir, err.Error()))
            }
            stat, err := os.Stat(dirPath)
            if err != nil {
                if os.IsNotExist(err) {
                    return nil, errors.New(fmt.Sprintf("Directory %s does not exist.", dirPath))
                }
                return nil, errors.New(fmt.Sprintf("Error when accessing %s: %s", dirPath, err.Error()))
            }
            if !stat.IsDir() {
                return nil, errors.New(fmt.Sprintf("%s is not a directory.", dirPath))
            }

            retMappings = append(retMappings, PrefixMappingEntry{ Prefix: prefix, Dir: dir })
        }
        return retMappings, nil
    }
    return []PrefixMappingEntry{PrefixMappingEntry{ Prefix: "/files", Dir: defaultDirPath }}, nil
}

func (s *SimpleFileServerHandler) GetName() string {
    return HANDLER_NAME
}

// StartHandler starts the SimpleFileServerHandler
func (s *SimpleFileServerHandler) StartHandler(restAddress string, configEntry config.HandlerConfigEntry) error {
    // config entry must contain bind address and port
    bindEndpoint, err := config.GetHostPortString(configEntry)
    if err != nil {
        return err
    }
    s.prefixMappings, err = GetPrefixMappings(configEntry)
    if err != nil {
        return err
    }

    // make sure we know the REST API address
    s.restAPIaddress = restAddress

    mux := mux.NewRouter()
    mux.HandleFunc("/uploads", s.handleFileUpload).Methods("POST")
    for _, mapping := range s.prefixMappings {
        fs := http.FileServer(noDirListingFileSystem{http.Dir(mapping.Dir)})
        mux.Handle(mapping.Prefix, http.NotFoundHandler())
        mux.PathPrefix(mapping.Prefix + "/").Handler(HttpHandlerWithLogging(http.StripPrefix(mapping.Prefix, fs)))
        logger.Info(fmt.Sprintf("[Simple File Server] Serving files from %s at URL prefix %s", mapping.Dir, mapping.Prefix))
    }

    s.server = &http.Server{Addr: bindEndpoint, Handler: mux}
    logger.Info(fmt.Sprintf("[Simple File Server] Starting Simple File Server on endpoint %s", bindEndpoint))
    s.waitGroup.Add(1)
    go func() {
        defer s.waitGroup.Done()
        if err := s.server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
            logger.Error(fmt.Sprintf("[SimpleFileServer] ListenAndServe(): %s", err.Error()))
        }
    }()

    return nil
}

// StopHandler stops the SimpleFileServerHandler
func (s *SimpleFileServerHandler) StopHandler() error {
    logger.Info("[Simple File Server] Stopping Simple File Server")

    if err := s.server.Shutdown(context.Background()); err != nil {
        return errors.New(fmt.Sprintf("SimpleFileServer Shutdown error: %s", err.Error()))
    }
    s.waitGroup.Wait()
    logger.Info("[Simple File Server] Stopped Simple File Server")
    return nil
}

func (s *SimpleFileServerHandler) handleFileUpload(w http.ResponseWriter, r *http.Request) {
    logger.Debug(fmt.Sprintf("[Simple File Server] Handling file upload POST request from %s", r.RemoteAddr))

	fileName := r.Header.Get("filename")
	if len(fileName) == 0 {
	    logger.Error("[Simple File Server] Upload POST request did not contain filename header")
	    w.WriteHeader(http.StatusBadRequest)
	    w.Write([]byte(serverBadRequestMsg))
	    return
	}

	fileData, err := ioutil.ReadAll(r.Body)
	if err != nil {
        logger.Error(fmt.Sprintf("[Simple File Server] Failed to read upload data: %s", err.Error()))
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(serverBadRequestMsg))
        return
	}
	response, err := handler_util.ForwardFileUpload(s.restAPIaddress, fileName, fileData)
    if err != nil {
        logger.Error(fmt.Sprintf("[Simple File Server] Failed to process and forward upload: %s", err.Error()))
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(serverErrMsg))
        return
    }
    logger.Success(response)
    w.WriteHeader(http.StatusOK)
    w.Write([]byte("OK"))
    return
}

// Wrapper for http.Handler with added logging
func HttpHandlerWithLogging(h http.Handler) http.Handler {
    logFunc := func(w http.ResponseWriter, r *http.Request) {
        logger.Debug(fmt.Sprintf("[Simple File Server] %s %s from %s", r.Method, r.RequestURI, r.RemoteAddr))

        _, fileName := filepath.Split(r.RequestURI)

        loggingRw := &ResponseWriterWithLogging{
            responseWriter: w,
            status: 0,
            size: 0,
            requestedFile: fileName,
        }

        h.ServeHTTP(loggingRw, r)

        logger.Debug(fmt.Sprintf("[Simple File Server] Response size of %d, response code %d", loggingRw.size, loggingRw.status))
        if loggingRw.status == http.StatusOK {
            logger.Success(fmt.Sprintf("[Simple File Server] File %s downloaded by %s", fileName, r.RemoteAddr))
        }
    }
    return http.HandlerFunc(logFunc)
}

func (r *ResponseWriterWithLogging) Header() http.Header {
    respHeaders := r.responseWriter.Header()
    if (strings.ToLower(filepath.Ext(r.requestedFile)) != ".html") {
        // have recipient browser always download non-HTML files
        respHeaders.Set("Content-Disposition", "attachment")
    }
    return respHeaders
}

func (r *ResponseWriterWithLogging) Write(input []byte) (int, error) {
    size, err := r.responseWriter.Write(input)
    if err == nil {
        r.size = size
    } else {
        logger.Error(fmt.Sprintf("[Simple File Server] Error when writing response: %s", err.Error()))
        r.size = 0
    }
    return size, err
}

func (r *ResponseWriterWithLogging) WriteHeader(statusCode int) {
    r.responseWriter.WriteHeader(statusCode)
    r.status = statusCode // save status code
}
