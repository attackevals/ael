package handlers

import (
    "fmt"
    "strings"

    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/handlers/handler_manager"
    "evals.mitre.org/control_server/logger"

    _ "evals.mitre.org/control_server/handlers/simplefileserver"
)

// StartHandlers starts the C2 handlers
func StartHandlers() {
    restAPIAddr := config.GetRestAPIListenAddress()
    if len(restAPIAddr) == 0 {
        logger.Fatal("No REST API address provided. Please check your configuration.")
    }
    handlerConfigMap := config.HandlerConfig.GetHandlerConfigMapLowercase()
    if len(handlerConfigMap) == 0 {
        logger.Fatal("No handler configuration provided. Please check your configuration.")
    }
    if len(handler_manager.AvailableHandlers) == 0 {
        logger.Fatal("No handlers available to start.")
    }
    for handlerName, handler := range handler_manager.AvailableHandlers {
        configEntry, ok := handlerConfigMap[strings.ToLower(handlerName)]
        if !ok {
            logger.Info(fmt.Sprintf("No configuration entry found for %s. Skipping", handlerName))
            continue
        }
        enabled, err := config.ConfigEntryEnabled(configEntry)
        if err != nil {
            logger.Error(fmt.Sprintf("Failed to check if handler %s is enabled: %s", handlerName, err.Error()))
            continue
        }
        if enabled {
            if err := handler.StartHandler(restAPIAddr, configEntry); err != nil {
                logger.Error(fmt.Sprintf("Error starting handler %s: %s", handlerName, err.Error()))
            } else {
                logger.Success(fmt.Sprintf("Started handler %s", handlerName))
                handler_manager.RunningHandlers[handlerName] = handler
            }
        } else {
            logger.Info(fmt.Sprintf("Handler %s disabled. Skipping.", handlerName))
        }
    }
    if len(handler_manager.RunningHandlers) == 0 {
        logger.Fatal("Failed to start any C2 handler. Please make sure at least one handler is enabled in your configuration.")
    }
}

// StopHandlers stops all C2 handlers
func StopHandlers() {
    for handlerName, handler := range handler_manager.RunningHandlers {
        if err := handler.StopHandler(); err != nil {
            logger.Error(fmt.Sprintf("Error stopping handler %s: %s", handlerName, err.Error()))
        } else {
            logger.Success(fmt.Sprintf("Terminated handler %s", handlerName))
            delete(handler_manager.RunningHandlers, handlerName)
        }
    }
    // To do - each handler should send a signal to indicate it stopped gracefully
}
