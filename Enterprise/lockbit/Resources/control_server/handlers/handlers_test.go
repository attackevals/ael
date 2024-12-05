package handlers

import (
    "path/filepath"
    "testing"
    "time"

    "attackevals.mitre-engenuity.org/control_server/config"
    "attackevals.mitre-engenuity.org/control_server/handlers/handler_manager"
    "attackevals.mitre-engenuity.org/control_server/util"
)

func mockHandlerConfigFileReaderAllEnabled(path string) ([]byte, error) {
    dataStr := `simplefileserver:
  host: 127.0.0.1
  port: 60012
  enabled: true
examplehttp:
  host: 127.0.0.1
  port: 60013
  enabled: true
STRATOFEAR: 
  host: 127.0.0.1
  port: 60015
  enabled: true
  https: true
  cert_file: ""
  key_file: ""
sdbbot:
  host: 127.0.0.1
  port: 60014
  enabled: true
fullhouse:
  host: 127.0.0.1
  port: 60016
  enabled: true
stealbit:
  host: 127.0.0.1
  port: 60017
  enabled: true
`
    return []byte(dataStr), nil
}

func mockHandlerConfigFileReaderSomeEnabled(path string) ([]byte, error) {
    dataStr := `simplefileserver:
  host: 127.0.0.1
  port: 60012
  enabled: true
examplehttp:
  host: 127.0.0.1
  port: 60013
  enabled: false
STRATOFEAR: 
  host: 127.0.0.1
  port: 60015
  enabled: false
  https: true
  cert_file: ""
  key_file: ""
sdbbot:
  host: 127.0.0.1
  port: 60014
  enabled: false
fullhouse:
  host: 127.0.0.1
  port: 60016
  enabled: false
stealbit:
  host: 127.0.0.1
  port: 60017
  enabled: true
`
    return []byte(dataStr), nil
}

func TestStartStopHandlers(t *testing.T) {
    restApiConfigPath := filepath.Join(util.ProjectRoot, "config", "restAPI_config.yml")
    handlerConfigPath := filepath.Join(util.ProjectRoot, "config", "sample_handler_config.yml")
    wantAvailable := 2
    wantRunning := 2
    config.SetRestAPIConfig(restApiConfigPath)
    config.HandlerConfig = config.HandlerConfigGenerator(mockHandlerConfigFileReaderAllEnabled)
    config.HandlerConfig.SetHandlerConfig(handlerConfigPath)
    if len(handler_manager.AvailableHandlers) != wantAvailable {
        t.Errorf("Expected %d available handlers, got %d", wantAvailable, len(handler_manager.AvailableHandlers))
    }
    StartHandlers()
    time.Sleep(100 * time.Millisecond)
    if len(handler_manager.RunningHandlers) != wantRunning {
        t.Errorf("Expected %d running handlers, got %d", wantRunning, len(handler_manager.RunningHandlers))
    }
    StopHandlers()
    if len(handler_manager.RunningHandlers) != 0 {
        t.Errorf("Expected to stop all running handlers, got %d remaining", len(handler_manager.RunningHandlers))
    }
}

func TestStartStopHandlersSomeEnabled(t *testing.T) {
    restApiConfigPath := filepath.Join(util.ProjectRoot, "config", "restAPI_config.yml")
    handlerConfigPath := filepath.Join(util.ProjectRoot, "config", "sample_handler_config.yml")

    wantAvailable := 2
    wantRunning := 2
    config.SetRestAPIConfig(restApiConfigPath)
    config.HandlerConfig = config.HandlerConfigGenerator(mockHandlerConfigFileReaderSomeEnabled)
    config.HandlerConfig.SetHandlerConfig(handlerConfigPath)
    if len(handler_manager.AvailableHandlers) != wantAvailable {
        t.Errorf("Expected %d available handlers, got %d", wantAvailable, len(handler_manager.AvailableHandlers))
    }
    StartHandlers()
    time.Sleep(100 * time.Millisecond)
    if len(handler_manager.RunningHandlers) != wantRunning {
        t.Errorf("Expected %d running handlers, got %d", wantRunning, len(handler_manager.RunningHandlers))
    }
    if _, ok := handler_manager.RunningHandlers["simplefileserver"]; !ok {
        t.Error("Expected simplefileserver to be enabled and running")
    }
    StopHandlers()
    if len(handler_manager.RunningHandlers) != 0 {
        t.Errorf("Expected to stop all running handlers, got %d remaining", len(handler_manager.RunningHandlers))
    }
}
