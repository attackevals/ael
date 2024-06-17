package handlers

import (
  "os"
  "testing"
  "time"

  "attackevals.mitre-engenuity.org/control_server/config"
  "attackevals.mitre-engenuity.org/control_server/handlers/util"
)

func mockHandlerConfigFileReaderAllEnabled(path string) ([]byte, error) {
  dataStr := `quasar:
  host: 127.0.0.1
  port: 60010
  enabled: true
simplefileserver:
  host: 127.0.0.1
  port: 60012
  enabled: true
sodamaster:
  host: 127.0.0.1
  port: 60011
  enabled: true`
  return []byte(dataStr), nil
}

func mockHandlerConfigFileReaderSomeEnabled(path string) ([]byte, error) {
  dataStr := `quasar:
  host: 127.0.0.1
  port: 60009
  enabled: true
simplefileserver:
  host: 127.0.0.1
  port: 60012
  enabled: false
sodamaster:
  host: 127.0.0.1
  port: 60022
  enabled: false`
  return []byte(dataStr), nil
}

func TestStartStopHandlers(t *testing.T) {
  // set current working directory to main repo directory
  // this is needed so that the handlers can reference correct file structure
  cwd, _ := os.Getwd()
  os.Chdir("../")
  defer os.Chdir(cwd) // restore cwd at end of test

  wantAvailable := 3
  wantRunning := 3
  config.SetRestAPIConfig("config/restAPI_config.yml")
  config.HandlerConfig = config.HandlerConfigGenerator(mockHandlerConfigFileReaderAllEnabled)
  config.HandlerConfig.SetHandlerConfig("config/sample_handler_config.yml")
  if len(util.AvailableHandlers) != wantAvailable {
    t.Errorf("Expected %d available handlers, got %d", wantAvailable, len(util.AvailableHandlers))
  }
  StartHandlers()
  time.Sleep(100 * time.Millisecond)
  if len(util.RunningHandlers) != wantRunning {
    t.Errorf("Expected %d running handlers, got %d", wantRunning, len(util.RunningHandlers))
  }
  StopHandlers()
  if len(util.RunningHandlers) != 0 {
    t.Errorf("Expected to stop all running handlers, got %d remaining", len(util.RunningHandlers))
  }
}

func TestStartStopHandlersSomeEnabled(t *testing.T) {
  // set current working directory to main repo directory
  // this is needed so that the handlers can reference correct file structure
  cwd, _ := os.Getwd()
  os.Chdir("../")
  defer os.Chdir(cwd) // restore cwd at end of test

  wantAvailable := 3
  wantRunning := 1
  config.SetRestAPIConfig("config/restAPI_config.yml")
  config.HandlerConfig = config.HandlerConfigGenerator(mockHandlerConfigFileReaderSomeEnabled)
  config.HandlerConfig.SetHandlerConfig("config/sample_handler_config.yml")
  if len(util.AvailableHandlers) != wantAvailable {
    t.Errorf("Expected %d available handlers, got %d", wantAvailable, len(util.AvailableHandlers))
  }
  StartHandlers()
  time.Sleep(100 * time.Millisecond)
  if len(util.RunningHandlers) != wantRunning {
    t.Errorf("Expected %d running handlers, got %d", wantRunning, len(util.RunningHandlers))
  }
  if _, ok := util.RunningHandlers["quasar"]; !ok {
    t.Error("Expected simplehttp to be enabled and running")
  }
  if _, ok := util.RunningHandlers["sodamaster"]; ok {
    t.Error("Expected trickbot to be disabled")
  }
  StopHandlers()
  if len(util.RunningHandlers) != 0 {
    t.Errorf("Expected to stop all running handlers, got %d remaining", len(util.RunningHandlers))
  }
}
