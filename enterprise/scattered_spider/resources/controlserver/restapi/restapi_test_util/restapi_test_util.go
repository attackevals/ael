package restapi_test_util

import (
    "bytes"
    "encoding/json"
    "fmt"
    "io/ioutil"
    "net/http"
    "os"
    "path/filepath"
    "testing"
    "time"

    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/restapi"
    "evals.mitre.org/control_server/restapi/restapi_util"
    "evals.mitre.org/control_server/sessions"
    "evals.mitre.org/control_server/util"
)

const (
    BASE_URL           = "http://127.0.0.1:9999/api/v1.0/"
    REST_API_BIND_ADDR = "127.0.0.1:9999"
)

var (
    PAYLOAD_TEST_DIR string
    PAYLOAD_TEST_DIRS map[string]string
)

func init() {
    cwd, _ := os.Getwd()
    os.Chdir("../../")
    defer os.Chdir(cwd)

    util.SetRootDirectories()
    SetPayloadTestDirs()
}

func SetPayloadTestDirs() {
    PAYLOAD_TEST_DIR  = filepath.Join(util.ProjectRoot, "test_payloads")
    PAYLOAD_TEST_DIRS = map[string]string{"": PAYLOAD_TEST_DIR}
}

// Pass either a path to a configuration to use or a specific bind address. If a specific bind address is provided, the config path will not be used.
// If neither are provided (both empty strings), then a default config path of config/test_restapi_config.yml will be used
func StartRestApiServer(t *testing.T, testConfigPath string, bindAddr string, payloadDirectories map[string]string) {
    restAPIaddress := bindAddr
    if len(restAPIaddress) == 0 {
        configPath := filepath.Join(util.ProjectRoot, "config", "test_restapi_config.yml") // default config path if none specified
        if len(testConfigPath) > 0 {
            configPath = testConfigPath
        }
        err := config.SetRestAPIConfig(configPath)
        if err != nil {
            t.Fatal(err)
        }
        restAPIaddress = config.GetRestAPIListenAddress()
    }
    if payloadDirectories == nil {
        payloadDirectories = PAYLOAD_TEST_DIRS
    }
    restapi.Start(restAPIaddress, payloadDirectories, false)
    time.Sleep(50 * time.Millisecond)
}

func StopRestApiServer() {
    // Clear sessions list in between tests
    sessions.SessionList = nil

    time.Sleep(50 * time.Millisecond)
    restapi.Stop()
}

// Creates session on server for testing.
func CreateTestSession(t *testing.T, restApiAddr string, session sessions.Session) {
    // http://localhost:9999/api/v1.0/session
    createSessionURL := fmt.Sprintf("http://%s/api/v1.0/session", restApiAddr)

    // convert testSession object into JSON
    testSessionJSON, err := json.Marshal(session)
    if err != nil {
        t.Fatalf("Failed to marshal test session json: %s", err.Error())
    }

    // setup HTTP POST request
    req, err := http.NewRequest("POST", createSessionURL, bytes.NewBuffer(testSessionJSON))
    if err != nil {
        t.Fatal(err)
    }
    req.Header.Set("Content-Type", "application/json")

    // execute HTTP POST request and read response
    client := &http.Client{}
    response, err := client.Do(req)
    if err != nil {
        t.Fatal(err)
    }
    defer response.Body.Close()
    if response.StatusCode != 200 {
        t.Fatalf("Expected error code 200, got %v", response.StatusCode)
    }
}

func FetchSession(t *testing.T, restApiAddr string, sessionGuid string) *sessions.Session {
    // http://localhost:9999/api/v1.0/session/{guid}
    url := fmt.Sprintf("http://%s/api/v1.0/session/%s", restApiAddr, sessionGuid)

    // setup HTTP GET request
    req, err := http.NewRequest("GET", url, nil)
    req.Close = true
    if err != nil {
        t.Fatal(err)
    }

    // execute HTTP GET and read response
    client := &http.Client{}
    response, err := client.Do(req)
    if err != nil {
        t.Fatal(err)
    }
    defer response.Body.Close()
    if response.StatusCode != 200 {
        t.Fatalf("Expected error code 200, got %v", response.StatusCode)
    }

    body, err := ioutil.ReadAll(response.Body)
    if err != nil {
        t.Fatal(err)
    }

    var apiResponse restapi_util.ApiSessionsResponse
    err = json.Unmarshal(body, &apiResponse)
    if err != nil {
        t.Fatal(err)
    }
    if len(apiResponse.Data) < 1 {
        t.Fatalf("API query returned 0 sessions for ID %v", sessionGuid)
    }
    returnedSession := apiResponse.Data[0]
    return &returnedSession
}

// Creates new task on server associated with input sessionGuid.
// Can pass in empty string for taskGuid if you don't want to specify a task GUID.
func CreateTestTaskForSession(t *testing.T, restApiAddr string, sessionGuid string, taskGuid string, taskCommand string) {
    url := fmt.Sprintf("http://%s/api/v1.0/session/%s/task", restApiAddr, sessionGuid)

    // setup HTTP POST request
    req, err := http.NewRequest("POST", url, bytes.NewBufferString(taskCommand))
    if err != nil {
        t.Fatal(err)
    }
    if len(taskGuid) > 0 {
        req.Header.Set("X-Task-Guid", taskGuid)
    }
    req.Close = true

    // execute HTTP POST and read response
    client := &http.Client{}
    response, err := client.Do(req)
    if err != nil {
        t.Fatal(err)
    }
    defer response.Body.Close()

    if response.StatusCode != 200 {
        t.Fatalf("Expected error code 200, got %v", response.StatusCode)
    }
}
