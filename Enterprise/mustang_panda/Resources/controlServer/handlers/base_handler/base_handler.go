package base_handler

import (
    "encoding/json"
    "fmt"
    "strings"

    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/handlers/handler_util"
    "evals.mitre.org/control_server/logger"
)

// Provides common functionality across handlers
type BaseHandler interface {
    SetBindAddr(config.HandlerConfigEntry) error
    SetRestApiAddr(string)
    HasImplantSession(string) bool
    RegisterNewImplant(string, map[string]string) error
    GetImplantTask(string) (string, error)
    RegisterTaskOutput(string, []byte) (string, error)
    GetFileFromRestServer(string) ([]byte, error)
    SaveUploadedFile(string, []byte) (string, error)
    UpdateImplantSession(string, map[string]string) error

    // Logging functions that will prepend the handler name to the log message
    HandlerLogDebug(format string, a ...any)
    HandlerLogInfo(format string, a ...any)
    HandlerLogError(format string, a ...any)
    HandlerLogSuccess(format string, a ...any)
    HandlerLogTask(format string, a ...any)
    HandlerLogWarning(format string, a ...any)
    HandlerLogData(format string, a ...any)
}

// Will implement the Basehandler interface
type BaseHandlerUtil struct {
    Name        string
    BindAddr    string
    RestApiAddr string
}

// SETUP FUNCTIONS

func BaseHandlerFactory(name string) *BaseHandlerUtil {
    return &BaseHandlerUtil{
        Name: name,
    }
}

func (b *BaseHandlerUtil) SetBindAddr(configEntry config.HandlerConfigEntry) error {
    bindAddr, err := config.GetHostPortString(configEntry)
    if err != nil {
        return err
    }
    b.BindAddr = bindAddr
    return nil
}

func (b *BaseHandlerUtil) SetRestApiAddr(addr string) {
    b.RestApiAddr = addr
}

// IMPLANT SESSION MANAGEMENT FUNCTIONS

func (b *BaseHandlerUtil) HasImplantSession(implantId string) bool {
    sessionExists, err := handler_util.ForwardSessionExists(b.RestApiAddr, implantId)
    if err != nil {
        b.HandlerLogError("Failed to check for existing session for implant %s: %s", implantId, err.Error())
        return false
    }
    return sessionExists
}

func (b *BaseHandlerUtil) RegisterNewImplant(implantId string, sessionData map[string]string) error {
    //add handler name to sessionData
    sessionData["handler"] = b.Name

    jsonStr, err := json.Marshal(sessionData)
    if err != nil {
        return fmt.Errorf("Failed to create JSON info for session ID %s: %s", implantId, err.Error())
    }

    restResponse, err := handler_util.ForwardRegisterImplant(b.RestApiAddr, []byte(jsonStr))
    if err != nil {
        return err
    }

    b.HandlerLogInfo(restResponse)
    b.HandlerLogSuccess("Successfully created session for implant %s.", implantId)
    return nil
}

func (b *BaseHandlerUtil) GetImplantTask(implantId string) (string, error) {
    return handler_util.ForwardGetTask(b.RestApiAddr, implantId)
}

func (b *BaseHandlerUtil) RegisterTaskOutput(implantId string, data []byte) (string, error) {
    return handler_util.ForwardTaskOutput(b.RestApiAddr, implantId, data)
}

func (b *BaseHandlerUtil) GetFileFromRestServer(filename string) ([]byte, error) {
    return handler_util.ForwardGetFileFromServer(b.RestApiAddr, b.Name, filename)
}

func (b *BaseHandlerUtil) SaveUploadedFile(filename string, data []byte) (string, error) {
    return handler_util.ForwardFileUpload(b.RestApiAddr, filename, data)
}

func (b *BaseHandlerUtil) UpdateImplantSession(uuid string, updates map[string]string) error {
    sessionData, err := json.Marshal(updates)
    if err != nil {
        b.HandlerLogError("Failed to create JSON info for session for UUID %s: %s", uuid, err.Error())
        return nil
    }
    restResponse, err := handler_util.ForwardUpdateSession(b.RestApiAddr, uuid, sessionData)
    if err != nil {
        return err
    }
    b.HandlerLogInfo(restResponse)
    b.HandlerLogSuccess("Successfully updated session for implant %s.", uuid)

    return nil
}

// LOGGING FUNCTIONS

func (b *BaseHandlerUtil) HandlerLogDebug(format string, a ...any) {
    logger.Debug(b.getHandlerLogMsg(format, a...))
}

func (b *BaseHandlerUtil) HandlerLogInfo(format string, a ...any) {
    logger.Info(b.getHandlerLogMsg(format, a...))
}

func (b *BaseHandlerUtil) HandlerLogSuccess(format string, a ...any) {
    logger.Success(b.getHandlerLogMsg(format, a...))
}

func (b *BaseHandlerUtil) HandlerLogTask(format string, a ...any) {
    logger.Task(b.getHandlerLogMsg(format, a...))
}

func (b *BaseHandlerUtil) HandlerLogWarning(format string, a ...any) {
    logger.Warning(b.getHandlerLogMsg(format, a...))
}

func (b *BaseHandlerUtil) HandlerLogError(format string, a ...any) {
    logger.Error(b.getHandlerLogMsg(format, a...))
}

func (b *BaseHandlerUtil) HandlerLogData(format string, a ...any) {
    logger.Data(b.getHandlerLogMsg(format, a...))
}

func (b *BaseHandlerUtil) getHandlerLogMsg(format string, a ...any) string {
    prefix := fmt.Sprintf("[%s] ", strings.ToUpper(b.Name))
    return fmt.Sprintf(prefix+format, a...)
}
