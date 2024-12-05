package sdbbot

import (
    "encoding/base64"
    "encoding/json"
    "path/filepath"
    "testing"

    "attackevals.mitre-engenuity.org/control_server/config"
    "attackevals.mitre-engenuity.org/control_server/handlers/handler_util"
    "attackevals.mitre-engenuity.org/control_server/restapi/restapi_test_util"
    "attackevals.mitre-engenuity.org/control_server/sessions"
    "attackevals.mitre-engenuity.org/control_server/util"
    "attackevals.mitre-engenuity.org/control_server/util/test_util"
    "attackevals.mitre-engenuity.org/control_server/util/test_util/assert_util"
)

const (
    REST_API_LISTEN_HOST = "127.0.0.1:10008"
    TEST_TASK            = "{\"id\":\"execute\", \"arg\":\"whoami\"}"
    TEST_DOWNLOAD_TASK   = "{\"id\":\"download\", \"payload\":\"" + TEST_PAYLOAD + "\", \"arg\":\"C:\\\\Windows\\\\Temp\"}"
    TEST_TASK_RESPONSE   = "{\"header\":\"example-implant-123\",\"command\":\"2\",\"execute\":\"whoami\"}"
    TEST_PAYLOAD         = "hello_world.elf"
    TEST_PAYLOAD_HASH    = "fe7c47d38224529c7d8f9a11a62cdd7a"
    TEST_UPLOAD_FILENAME = "test_upload.txt"
    TEST_UPLOAD_CONTENTS = "test upload file contents"
    TEST_UPLOAD_HASH     = "09347adcaffed3c23290b3b5a07a8a36"
    TEST_BEACON_RESPONSE = "{\"header\":\"example-implant-123\"}"
)

var (
    PAYLOAD_TEST_DIR  = filepath.Join(util.ProjectRoot, "test_payloads")
    PAYLOAD_TEST_DIRS = map[string]string{HANDLER_NAME: PAYLOAD_TEST_DIR}

    TEST_UPLOAD_B64CONTENTS = base64.StdEncoding.EncodeToString([]byte(TEST_UPLOAD_CONTENTS))
    TEST_UPLOAD_PACKET      = map[string]interface{}{"header": mockSession.GUID, "command": readFile, "fileName": TEST_UPLOAD_FILENAME, "fileContents": TEST_UPLOAD_B64CONTENTS}
)

var mockSession = sessions.Session{
    GUID:     "example-implant-123",
    IPAddr:   "127.0.0.1",
    HostName: "myHostName",
    User:     "myUserName",
    Dir:      "C:\\MyDir\\",
    PID:      "1234",
    PPID:     "4",
    Task:     nil,
}

var testConfigEntry = config.HandlerConfigEntry{
    "host": "127.0.0.1",
    "port": 10006,
}

var mockImplantHeartbeat = ImplantResponse{
    Header:     "example-implant-123",
    Command:    "",
    FullPacket: "",
}

var mockImplantResponse = ImplantResponse{
    Header:     "example-implant-123",
    Command:    execute,
    FullPacket: "{\"header\":\"example-implant-123\",\"command\":\"2\"}",
}

// Generate example handler using the mock function wrappers
func generateMockSDBbotHandler() *SDBbotHandler {
    mockFuncHandles := &WrappedFuncHandles{
        randStringGetter: handler_util.MockGetRandomAlphanumericString,
    }
    return SDBbotHandlerFactory(mockFuncHandles)
}

// Starts the RestAPI, generates the handler, and returns teardown function
func setupTest(t *testing.T) (*SDBbotHandler, func(t *testing.T)) {
    // Set up REST API server
    restapi_test_util.StartRestApiServer(t, "", REST_API_LISTEN_HOST, PAYLOAD_TEST_DIRS)

    // create and start mock handler
    handler := generateMockSDBbotHandler()
    test_util.StartHandlerForUnitTest(t, handler, REST_API_LISTEN_HOST, testConfigEntry)

    return handler, func(t *testing.T) {
        restapi_test_util.StopRestApiServer()
        test_util.StopHandlerForUnitTest(t, handler)
    }
}

// Test getting tasks
func TestHandleGetTask(t *testing.T) {
    handler, teardownTest := setupTest(t)
    defer teardownTest(t)

    // Register session - must respond with beacon response
    resp, err := handler.HandleGetTask(mockImplantHeartbeat)
    if err != nil {
        t.Fatal(err.Error())
    }
    assert_util.AssertEq(t, resp, TEST_BEACON_RESPONSE, "")

    // Assign task
    _, err = handler_util.SetTask(REST_API_LISTEN_HOST, TEST_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }

    resp, err = handler.HandleGetTask(mockImplantHeartbeat)
    if err != nil {
        t.Fatal(err.Error())
    }
    assert_util.AssertEq(t, resp, TEST_TASK_RESPONSE, "")
}

// Test posting task output
func TestHandleTaskResponse(t *testing.T) {
    handler, teardownTest := setupTest(t)
    defer teardownTest(t)

    // Register session and assign a task so we can send output
    resp, err := handler.HandleGetTask(mockImplantHeartbeat)
    if err != nil {
        t.Fatal(err.Error())
    }
    assert_util.AssertEq(t, resp, TEST_BEACON_RESPONSE, "")

    // Assign task
    _, err = handler_util.SetTask(REST_API_LISTEN_HOST, TEST_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }

    _, err = handler.HandleGetTask(mockImplantHeartbeat)
    if err != nil {
        t.Fatal(err.Error())
    }

    // Send task output and check the response
    resp, err = handler.HandleTaskResponse(mockImplantResponse)
    if err != nil {
        t.Fatal(err.Error())
    }
    assert_util.AssertEq(t, resp, TEST_BEACON_RESPONSE, "")
}

func TestFileDownload(t *testing.T) {
    handler, teardownTest := setupTest(t)
    defer teardownTest(t)

    // Register session - must respond with beacon response
    resp, err := handler.HandleGetTask(mockImplantHeartbeat)
    if err != nil {
        t.Fatal(err.Error())
    }
    assert_util.AssertEq(t, resp, TEST_BEACON_RESPONSE, "")

    // Assign file download task
    _, err = handler_util.SetTask(REST_API_LISTEN_HOST, TEST_DOWNLOAD_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }

    resp, err = handler.HandleGetTask(mockImplantHeartbeat)
    if err != nil {
        t.Fatal(err.Error())
    }

    // Extract and decode file data
    var payloadData map[string]interface{}
    err = json.Unmarshal([]byte(resp), &payloadData)
    fileDataInt, ok := payloadData["fileContents"]
    if !ok {
        t.Fatal(ok)
    }
    fileDataStr, _ := fileDataInt.(string)
    fileData, _ := base64.StdEncoding.DecodeString(fileDataStr)

    // compare file hashes
    test_util.CheckMD5Hash(t, fileData, TEST_PAYLOAD_HASH)
}

func TestHandleFileUpload(t *testing.T) {
    handler, teardownTest := setupTest(t)
    defer teardownTest(t)

    // Upload a file
    err := handler.HandleFileUpload(TEST_UPLOAD_PACKET)
    if err != nil {
        t.Fatal(err)
    }

    //compare file hashes
    test_util.CheckUploadedFileHash(t, TEST_UPLOAD_FILENAME, TEST_UPLOAD_HASH)
}
