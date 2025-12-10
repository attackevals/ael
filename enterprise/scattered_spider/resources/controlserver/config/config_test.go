package config

import (
	"encoding/json"
	"testing"

	"evals.mitre.org/control_server/util/test_util/assert_util"
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
		t.Fatal("expected an error, got nil")
	}
	restAPIConfigFile := "./test_restapi_config.yml"
	err = SetRestAPIConfig(restAPIConfigFile)
	if err != nil {
		t.Fatal(err)
	}

}

func TestGetRestAPIConfig(t *testing.T) {
	restAPIConfigFile := "./test_restapi_config.yml"
	err := SetRestAPIConfig(restAPIConfigFile)
	if err != nil {
		t.Fatal(err)
	}
	t.Logf("%v", RestAPIConfig)
	conf, err := GetRestAPIConfig()
	if err != nil {
		t.Fatal(err)
	}
	if len(conf) == 0 {
		t.Fatalf("configuration is nil %v", conf)
	}
	want := RestAPIConfigStruct{
		Address: "127.0.0.1:9999",
		CalderaForwardingAddress: "http://10.2.3.4:8888/plugins/emu",
		EnableCalderaForwarding: true,
	}
	var received RestAPIConfigStruct
	err = json.Unmarshal(conf, &received)
	if err != nil {
		t.Fatal(err)
	}
	assert_util.AssertDeepEq(t, received, want, "")
}

func TestHandlerConfigMockFile(t *testing.T) {
	handler := HandlerConfigGenerator(mockHandlerConfigFileReader)
	err := handler.SetHandlerConfig(HANDLER_CONFIG_FILE)
	if err != nil {
		t.Fatal(err)
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
	assert_util.AssertDeepEq(t, confMap, want, "")
	confJson, err := handler.GetHandlerConfigJSON()
	if err != nil {
		t.Fatal(err)
	}
	if len(confJson) == 0 {
		t.Fatalf("configuration is nil %v", confJson)
	}
}

func TestSetHandlerConfigActualFile(t *testing.T) {
	err := HandlerConfig.SetHandlerConfig(HANDLER_CONFIG_FILE)
	if err != nil {
		t.Fatal(err)
	}
	garbage := "this-is-garbage-input"
	err = HandlerConfig.SetHandlerConfig(garbage)
	if err == nil {
		t.Fatal("expected an error, got nil")
	}
}

func TestGetHandlerConfigJSONActualFile(t *testing.T) {
	conf, err := HandlerConfig.GetHandlerConfigJSON()
	if err != nil {
		t.Fatal(err)
	}
	if len(conf) == 0 {
		t.Fatalf("configuration is nil %v", conf)
	}
}

func TestGetHandlerConfigMapActualFile(t *testing.T) {
	conf := HandlerConfig.GetHandlerConfigMap()
	if len(conf) == 0 {
		t.Fatalf("configuration is nil %v", conf)
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
		t.Fatalf("Obtained error when getting host port string: %s", err.Error())
	}
	assert_util.AssertEq(t, result, want, "")
}

func TestGetHostPortStringMissingHost(t *testing.T) {
	configEntry := HandlerConfigEntry{
		"port": 8080,
	}
	want := "Config entry did not contain a host value. Expected key: host"
	result, err := GetHostPortString(configEntry)
	if err == nil {
		t.Fatal("Expected error but did not get one.")
	}
	assert_util.AssertEq(t, result, "", "Expected empty string")
	assert_util.AssertEq(t, err.Error(), want, "Error message mismatch")
}

func TestGetHostPortStringMissingPort(t *testing.T) {
	configEntry := HandlerConfigEntry{
		"host": "192.168.0.4",
	}
	want := "Config entry did not contain a port value. Expected key: port"
	result, err := GetHostPortString(configEntry)
	if err == nil {
		t.Fatal("Expected error but did not get one.")
	}
	assert_util.AssertEq(t, result, "", "Expected empty string")
	assert_util.AssertEq(t, err.Error(), want, "Error message mismatch")
}

func TestGetHostPortStringEmptyHost(t *testing.T) {
	configEntry := HandlerConfigEntry{
		"host": "",
		"port": "8080",
	}
	want := "Please provide a non-empty host value."
	result, err := GetHostPortString(configEntry)
	if err == nil {
		t.Fatal("Expected error but did not get one.")
	}
	assert_util.AssertEq(t, result, "", "Expected empty string")
	assert_util.AssertEq(t, err.Error(), want, "Error message mismatch")
}

func TestGetHostPortStringEmptyPort(t *testing.T) {
	configEntry := HandlerConfigEntry{
		"host": "192.168.0.4",
		"port": "",
	}
	want := "Port value must be an int."
	result, err := GetHostPortString(configEntry)
	if err == nil {
		t.Fatal("Expected error but did not get one.")
	}
	assert_util.AssertEq(t, result, "", "Expected empty string")
	assert_util.AssertEq(t, err.Error(), want, "Error message mismatch")
}
