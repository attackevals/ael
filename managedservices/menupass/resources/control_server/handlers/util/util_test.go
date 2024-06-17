package util_test

import (
    "bytes"
    "crypto/md5"
    "encoding/hex"
    "encoding/json"
    "fmt"
    "io/ioutil"
    "net/http"
    "os"
    "testing"
    "time"

    "attackevals.mitre-engenuity.org/control_server/handlers/util" // avoid import loop
    "attackevals.mitre-engenuity.org/control_server/logger"
    "attackevals.mitre-engenuity.org/control_server/restapi"
    "attackevals.mitre-engenuity.org/control_server/sessions"
    "attackevals.mitre-engenuity.org/control_server/test_utils"
)

const (
    REST_API_LISTEN_HOST  = "127.0.0.1:9990" // need to check on port
    REST_API_BASE_URL     = "http://" + REST_API_LISTEN_HOST + "/api/v1.0/"
    TEST_SESSION_GUID     = "new-implant-beacon"
    INVALID_DATA          = "invalid data"
    EXAMPLE_TASK          = "{\"id\":\"task-id\"}"
    HELLO_WORLD_FILE_NAME = "hello_world.pdf"
    HELLO_WORLD_FILE_HASH = "ff1665c2e7239b61f8d3f22248a4bff8"
)

func startRESTAPI(t *testing.T) {
    restapi.Start(REST_API_LISTEN_HOST, "./test_payloads")
    time.Sleep(50 * time.Millisecond)
    t.Log("Started REST API server")
}

func stopRESTAPI(t *testing.T) {
    restapi.Stop()
    time.Sleep(50 * time.Millisecond)
    t.Log("Stopped REST API server")
}

func createTestSession(uuid string) {
    // http://127.0.0.1:9999/api/v1.0/session
    createSessionURL := REST_API_BASE_URL + "session"

    // convert testSession object into JSON
    testSession := sessions.Session{
        GUID: TEST_SESSION_GUID,
    }

    testSessionJSON, err := json.Marshal(testSession)
    if err != nil {
        logger.Fatal(err)
    }

    // setup HTTP POST request
    req, err := http.NewRequest("POST", createSessionURL, bytes.NewBuffer(testSessionJSON))
    if err != nil {
        logger.Fatal(err)
    }
    req.Header.Set("Content-Type", "application/json")

    // execute HTTP POST request and read response
    client := &http.Client{}
    response, err := client.Do(req)
    if err != nil {
        logger.Fatal(err)
    }
    defer response.Body.Close()
    if response.StatusCode != 200 {
        logger.Fatal(fmt.Sprintf("Expected error code 200, got %v", response.StatusCode))
    }
}

func setTask(task string, uuid string) (string, error) {
    // http://127.0.0.1:9999/api/v1.0/session/<uuid>/task
    url := REST_API_BASE_URL + "session/" + uuid + "/task"

    // setup HTTP POST request
    request, err := http.NewRequest("POST", url, bytes.NewBufferString(task))
    if err != nil {
        return "", err
    }

    // execute HTTP POST and read response
    client := &http.Client{}
    response, err := client.Do(request)
    if err != nil {
        return "", err
    }

    defer response.Body.Close()
    if response.StatusCode != 200 {
        return "", fmt.Errorf("Expected error code 200, got %v", response.StatusCode)
    }
    body, err := ioutil.ReadAll(response.Body)
    return string(body[:]), err
}

func TestForwardImplantBeacon(t *testing.T) {
    startRESTAPI(t)
    test_utils.StartMockCalderaServer()
    restapi.CalderaForwardingEndpoint = "http://127.0.0.1:8888/plugins/emu/beacons"

    defer stopRESTAPI(t)
    defer test_utils.StopMockCalderaServer()
    createTestSession(TEST_SESSION_GUID)

    response, err := util.ForwardImplantBeacon(TEST_SESSION_GUID, REST_API_LISTEN_HOST)
    if err != nil {
        t.Error(err)
    }
    expectedOutput := fmt.Sprintf("Forwarded beacon for session: %s, received response: CALDERA server successfully received session: %s", TEST_SESSION_GUID, TEST_SESSION_GUID)
    if response != expectedOutput {
        t.Errorf("Expected message %s; got %s", expectedOutput, response)
    }
}

func TestForwardRegisterImplant(t *testing.T) {
    startRESTAPI(t)
    defer stopRESTAPI(t)

    // test that session can be registered
    expectedOutput := "Successfully added session."
    implantData := []byte("{\"guid\":\"test-implant-id\",\"hostName\":\"test-computerName\",\"pid\":1234,\"user\":\"test-user\"}")
    restResponse, err := util.ForwardRegisterImplant(REST_API_LISTEN_HOST, implantData)
    if err != nil {
        t.Error(err.Error())
    } else if restResponse != expectedOutput {
        t.Errorf("Status mismatch, got '%s' expected '%s'", restResponse, expectedOutput)
    }

    // test for correct error message if invalid data is sent
    implantData = []byte(INVALID_DATA)
    restResponse, err = util.ForwardRegisterImplant(REST_API_LISTEN_HOST, implantData)
    if err != nil {
        t.Error(err.Error())
    } else if restResponse == expectedOutput {
        t.Errorf("Got success message, error message expected.")
    }
}

func TestForwardGetTask(t *testing.T) {
    startRESTAPI(t)
    defer stopRESTAPI(t)

    // test that empty string is received when no task is set
    sessionData, err := util.ForwardGetTask(REST_API_LISTEN_HOST, TEST_SESSION_GUID)
    if err != nil {
        t.Error(err.Error())
    } else if sessionData != "" {
        t.Errorf("Status mismatch, got '%s' expected '%s'", sessionData, "")
    }

    // set a task
    _, err = setTask(EXAMPLE_TASK, TEST_SESSION_GUID)
    if err != nil {
        t.Error(err.Error())
    }

    // check that we can receive a task
    sessionData, err = util.ForwardGetTask(REST_API_LISTEN_HOST, TEST_SESSION_GUID)
    if err != nil {
        t.Error(err)
    } else if sessionData != EXAMPLE_TASK {
        t.Errorf("Expected %v, got %v", EXAMPLE_TASK, sessionData)
    }
}

func TestForwardGetFileFromServer(t *testing.T) {
    // set current working directory to main repo directory to access ./files
    cwd, _ := os.Getwd()
    os.Chdir("../../")
    defer os.Chdir(cwd) // restore cwd at end of test

    startRESTAPI(t)
    defer stopRESTAPI(t)

    // test downloading non-existent file
    expectedOutput := fmt.Sprintf("server did not return requested file: %s", INVALID_DATA)
    _, err := util.ForwardGetFileFromServer(REST_API_LISTEN_HOST, INVALID_DATA)
    if err != nil {
        if err.Error() != expectedOutput {
            t.Errorf("Expected error message: '%s'; got: '%s'", expectedOutput, err.Error())
        }
    } else {
        t.Error("Expected error message, none received.")
    }

    // test downloading a file
    fileData, err := util.ForwardGetFileFromServer(REST_API_LISTEN_HOST, HELLO_WORLD_FILE_NAME)
    if err != nil {
        t.Error(err.Error())
    }

    // compare file hashes
    hash := md5.Sum(fileData)
    expectedHash := hex.EncodeToString(hash[:])
    if HELLO_WORLD_FILE_HASH != expectedHash {
        t.Errorf("Expected %v, got %v", HELLO_WORLD_FILE_HASH, expectedHash)
    }
}
