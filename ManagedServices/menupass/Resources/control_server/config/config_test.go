package config

import (
	"encoding/json"
	"reflect"
	"testing"
)

const (
	HANDLER_CONFIG_FILE = "./sample_handler_config.yml"
)

func mockHandlerConfigFileReader(path string) ([]byte, error) {
	dataStr := `samplehandler1: 
  host: 192.168.0.4
  port: 8080
  enabled: true
samplehandler2: 
  host: 192.168.0.4
  port: 443
  cert_file: /dummy/https/cert
  key_file: /dummy/https/key
  enabled: true
samplehandler3: 
  host: 192.168.0.4
  port: 447
  enabled: false
  listvalue:
    - val1
    - val2
  listofmaps:
    - a: b
      c: d
    - e: f
samplehandler4: 
  host: 192.168.0.4
  port: 80
  enabled: false
samplehandler5: 
  host: 192.168.0.4
  port: 8443
  cert_file: /dummy/samplehandler5/cert
  key_file: /dummy/samplehandler5/key
  enabled: true`
	return []byte(dataStr), nil
}

func TestSetRestAPIConfig(t *testing.T) {
	garbage := "this-is-garbage-input"
	err := SetRestAPIConfig(garbage)
	if err == nil {
		t.Error("expected an error, got nil")
	}
	restAPIConfigFile := "./test_restapi_config.yml"
	err = SetRestAPIConfig(restAPIConfigFile)
	if err != nil {
		t.Error(err)
	}
	
}

func TestGetRestAPIConfig(t *testing.T) {
	restAPIConfigFile := "./test_restapi_config.yml"
	err := SetRestAPIConfig(restAPIConfigFile)
	if err != nil {
		t.Error(err)
	}
	t.Logf("%v", RestAPIConfig)
	conf, err := GetRestAPIConfig()
	if err != nil {
		t.Error(err)
	}
	if len(conf) == 0 {
		t.Errorf("configuration is nil %v", conf)
	}
	want := RestAPIConfigStruct{
		Address: "127.0.0.1:9999",
		CalderaForwardingAddress: "http://10.2.3.4:8888/plugins/emu",
		EnableCalderaForwarding: true,
	}
	var received RestAPIConfigStruct
	err = json.Unmarshal(conf, &received)
	if err != nil {
		t.Error(err)
	}
	if !reflect.DeepEqual(received, want) {
		t.Errorf("Expected \"%v\", got \"%v\"", want, received)
	}
}

func TestHandlerConfigMockFile(t *testing.T) {
	handler := HandlerConfigGenerator(mockHandlerConfigFileReader)
	err := handler.SetHandlerConfig(HANDLER_CONFIG_FILE)
	if err != nil {
		t.Error(err)
	}
	want := HandlerConfigMap{
		"samplehandler1": HandlerConfigEntry{
			"host": "192.168.0.4",
			"port": 8080,
			"enabled": true,
		},
		"samplehandler2": HandlerConfigEntry{
			"host": "192.168.0.4",
			"port": 443,
			"cert_file": "/dummy/https/cert",
			"key_file": "/dummy/https/key",
			"enabled": true,
		},
		"samplehandler3": HandlerConfigEntry{
			"host": "192.168.0.4",
			"port": 447,
			"enabled": false,
			"listvalue": []interface{}{"val1", "val2"},
			"listofmaps": []interface{}{
				map[string]interface{}{"a": "b", "c": "d"},
				map[string]interface{}{"e": "f"},
			},
		},
		"samplehandler4": HandlerConfigEntry{
			"host": "192.168.0.4",
			"port": 80,
			"enabled": false,
		},
		"samplehandler5": HandlerConfigEntry{
			"host": "192.168.0.4",
			"port": 8443,
			"cert_file": "/dummy/samplehandler5/cert",
			"key_file": "/dummy/samplehandler5/key",
			"enabled": true,
		},
	}
	confMap := handler.GetHandlerConfigMap()
	if !reflect.DeepEqual(want, confMap) {
		t.Errorf("Expected %#v, got %#v", want, confMap)
	}
	confJson, err := handler.GetHandlerConfigJSON()
	if err != nil {
		t.Error(err)
	}
	if len(confJson) == 0 {
		t.Errorf("configuration is nil %v", confJson)
	}
}

func TestSetHandlerConfigActualFile(t *testing.T) {
	err := HandlerConfig.SetHandlerConfig(HANDLER_CONFIG_FILE)
	if err != nil {
		t.Error(err)
	}
	garbage := "this-is-garbage-input"
	err = HandlerConfig.SetHandlerConfig(garbage)
	if err == nil {
		t.Error("expected an error, got nil")
	}
}

func TestGetHandlerConfigJSONActualFile(t *testing.T) {
	conf, err := HandlerConfig.GetHandlerConfigJSON()
	if err != nil {
		t.Error(err)
	}
	if len(conf) == 0 {
		t.Errorf("configuration is nil %v", conf)
	}
}

func TestGetHandlerConfigMapActualFile(t *testing.T) {
	conf := HandlerConfig.GetHandlerConfigMap()
	if len(conf) == 0 {
		t.Errorf("configuration is nil %v", conf)
	}
}

func TestGetHostPortString(t *testing.T) {
	configEntry := HandlerConfigEntry{
		"host": "192.168.0.4",
		"port": 8080,
		"enabled": "true",
	}
	want := "192.168.0.4:8080"
	result, err := GetHostPortString(configEntry)
	if err != nil {
		t.Errorf("Obtained error when getting host port string: %s", err.Error())
	}
	if result != want {
		t.Errorf("Expected %s, got %s", want, result)
	}
}

func TestGetHostPortStringMissingHost(t *testing.T) {
	configEntry := HandlerConfigEntry{
		"port": 8080,
	}
	want := "Config entry did not contain a host value. Expected key: host"
	result, err := GetHostPortString(configEntry)
	if err == nil {
		t.Error("Expected error but did not get one.")
	}
	if len(result) != 0 {
		t.Errorf("Expected empty string, got: %s", result)
	}
	if err.Error() != want {
		t.Errorf("Expected error message: %s; got: %s", want, err.Error())
	}
}

func TestGetHostPortStringMissingPort(t *testing.T) {
	configEntry := HandlerConfigEntry{
		"host": "192.168.0.4",
	}
	want := "Config entry did not contain a port value. Expected key: port"
	result, err := GetHostPortString(configEntry)
	if err == nil {
		t.Error("Expected error but did not get one.")
	}
	if len(result) != 0 {
		t.Errorf("Expected empty string, got: %s", result)
	}
	if err.Error() != want {
		t.Errorf("Expected error message: %s; got: %s", want, err.Error())
	}
}

func TestGetHostPortStringEmptyHost(t *testing.T) {
	configEntry := HandlerConfigEntry{
		"host": "",
		"port": "8080",
	}
	want := "Please provide a non-empty host value."
	result, err := GetHostPortString(configEntry)
	if err == nil {
		t.Error("Expected error but did not get one.")
	}
	if len(result) != 0 {
		t.Errorf("Expected empty string, got: %s", result)
	}
	if err.Error() != want {
		t.Errorf("Expected error message: %s; got: %s", want, err.Error())
	}
}

func TestGetHostPortStringEmptyPort(t *testing.T) {
	configEntry := HandlerConfigEntry{
		"host": "192.168.0.4",
		"port": "",
	}
	want := "Port value must be an int."
	result, err := GetHostPortString(configEntry)
	if err == nil {
		t.Error("Expected error but did not get one.")
	}
	if len(result) != 0 {
		t.Errorf("Expected empty string, got: %s", result)
	}
	if err.Error() != want {
		t.Errorf("Expected error message: %s; got: %s", want, err.Error())
	}
}
