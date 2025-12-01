package handler_manager

import (
    "net/http"
    "evals.mitre.org/control_server/config"
)

// The Handler interface provides methods that all C2 handlers must implement.
type Handler interface {
    // Starts the handler given the rest API address string and configuration map.
    StartHandler(string, config.HandlerConfigEntry) error

    // Stops the given handler.
    StopHandler() error

    // Returns the handler name
    GetName() string
}

// Provides methods that all HTTP-based C2 handlers must implement
type HttpHandler interface {
    // Process GET request for tasks/beacons
    HandleGetTask(http.ResponseWriter, *http.Request)

    // Process POST requests containing task output
    HandleHandleTaskResponse(http.ResponseWriter, *http.Request)

    // Process GET requests for payloads
    HandlePayloadDownload(http.ResponseWriter, *http.Request)

    // Process POST requests for file uploads
    HandleFileUpload(http.ResponseWriter, *http.Request)
}

// Contains the available C2 handler implementations. These will be populated as from the init() functions in each handler subpackage.
var AvailableHandlers map[string]Handler = make(map[string]Handler)

// Contains running handler implementations.
var RunningHandlers map[string]Handler = make(map[string]Handler)

func RegisterAvailableHandler(handler Handler) {
    AvailableHandlers[handler.GetName()] = handler
}

// Returns true if specified handler is currently running, false if not, or if the handler isn't registered/available to begin with.
func HandlerCurrentlyRunning(name string) bool {
    _, ok := RunningHandlers[name]
    return ok
}
