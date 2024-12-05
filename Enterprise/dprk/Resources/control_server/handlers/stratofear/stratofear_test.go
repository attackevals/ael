package stratofear

import (
    "encoding/base64"
    "encoding/json"
    "net/http"
    "path/filepath"
    "testing"

    "attackevals.mitre-engenuity.org/control_server/config"
    "attackevals.mitre-engenuity.org/control_server/handlers/handler_util"
    "attackevals.mitre-engenuity.org/control_server/restapi/restapi_test_util"
    "attackevals.mitre-engenuity.org/control_server/sessions"
    "attackevals.mitre-engenuity.org/control_server/util"
    "attackevals.mitre-engenuity.org/control_server/util/http_util"
    "attackevals.mitre-engenuity.org/control_server/util/test_util"
)

const (
    REST_API_LISTEN_HOST      = "127.0.0.1:10009"
    BASE_REGISTER_URL         = "http://127.0.0.1:10007/"
    BASE_TASK_URL             = "http://127.0.0.1:10007/directory/v1/"
    TEST_PAYLOAD              = "hello_world.elf"
    TEST_PAYLOAD_HASH         = "fe7c47d38224529c7d8f9a11a62cdd7a"
    TEST_TASK                 = "{\"id\":\"0x07\"}"
    TEST_INVALID_TASK         = "Dummy invalid task string"
    TEST_INGRESS_TASK         = "{\"id\":\"" + loadModule + "\", \"args\":\"" + TEST_PAYLOAD + "\"}"
    TEST_INVALID_INGRESS_TASK = "{\"id\":\"" + loadModule + "\"}"
    TEST_EXFIL_TASK           = "{\"id\":\"" + moduleOutput + "\"}"
    TEST_UPLOAD_FILENAME      = "test_upload.txt"
    TEST_UPLOAD_CONTENTS      = "test upload file contents"
    TEST_UPLOAD_HASH          = "09347adcaffed3c23290b3b5a07a8a36"
)

var (
    PAYLOAD_TEST_DIR  = filepath.Join(util.ProjectRoot, "test_payloads")
    PAYLOAD_TEST_DIRS = map[string]string{HANDLER_NAME: PAYLOAD_TEST_DIR}

    ENCODED_GUID            = base64.StdEncoding.EncodeToString(xor([]byte(mockSession.GUID)))
    ENCODED_TEST_TASK       = base64.StdEncoding.EncodeToString(xor([]byte(TEST_TASK)))
    ENCODED_HEARTBEAT       = base64.StdEncoding.EncodeToString(xor([]byte(heartbeat)))
    ENCODED_UPLOAD_CONTENTS = base64.StdEncoding.EncodeToString([]byte(TEST_UPLOAD_CONTENTS))

    UPLOAD_PACKET, _      = json.Marshal(map[string]interface{}{"id": moduleOutput, "moduleName": TEST_UPLOAD_FILENAME, "moduleBytes": ENCODED_UPLOAD_CONTENTS})
    ENCODED_UPLOAD_PACKET = base64.StdEncoding.EncodeToString(xor(UPLOAD_PACKET))
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
    "host":      "127.0.0.1",
    "port":      10007,
    "cert_file": "",
    "key_file":  "",
    "https":     "false",
}

// Generate example handler using the mock function wrappers
func generateMockStratofearHandler() *stratofearHandler {
    mockFuncHandles := &WrappedFuncHandles{
        randStringGetter: handler_util.MockGetRandomAlphanumericString,
    }
    return stratofearHandlerFactory(mockFuncHandles)
}

// Starts the RestAPI, generates the handler, and returns teardown function
func setupTest(t *testing.T) (*stratofearHandler, func(t *testing.T)) {
    // Set up REST API server
    restapi_test_util.StartRestApiServer(t, "", REST_API_LISTEN_HOST, PAYLOAD_TEST_DIRS)

    // create and start mock handler
    handler := generateMockStratofearHandler()
    test_util.StartHandlerForUnitTest(t, handler, REST_API_LISTEN_HOST, testConfigEntry)

    return handler, func(t *testing.T) {
        restapi_test_util.StopRestApiServer()
        test_util.StopHandlerForUnitTest(t, handler)
    }
}

// Test getting tasks
func TestHandleGetTask(t *testing.T) {
    _, teardownTest := setupTest(t)
    defer teardownTest(t)

    // Register session - must respond with heartbeat response
    test_util.SendPostAndCheckResponse(t, BASE_REGISTER_URL, "", nil, []byte(ENCODED_GUID), ENCODED_HEARTBEAT, http.StatusOK)

    // Check response when no task available
    test_util.SendGetAndCheckResponse(t, BASE_TASK_URL+mockSession.GUID, nil, ENCODED_HEARTBEAT, http.StatusOK)

    // Check response when invalid task string is sent
    _, err := handler_util.SetTask(REST_API_LISTEN_HOST, TEST_INVALID_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }
    test_util.SendGetAndCheckResponse(t, BASE_TASK_URL+mockSession.GUID, nil, ENCODED_SERVER_ERR_MSG, http.StatusInternalServerError)

    // Assign task
    _, err = handler_util.SetTask(REST_API_LISTEN_HOST, TEST_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }

    test_util.SendGetAndCheckResponse(t, BASE_TASK_URL+mockSession.GUID, nil, ENCODED_TEST_TASK, http.StatusOK)
}

// Test posting task output
func TestHandleTaskResponse(t *testing.T) {
    _, teardownTest := setupTest(t)
    defer teardownTest(t)

    // Register session and assign a task so we can send output
    _, err := http_util.PerformHttpPostAssertSuccess(BASE_REGISTER_URL, "", nil, []byte(ENCODED_GUID))
    if err != nil {
        t.Fatal(err.Error())
    }
    _, err = handler_util.SetTask(REST_API_LISTEN_HOST, TEST_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }

    _, err = http_util.PerformHttpGetAssertSuccess(BASE_TASK_URL+mockSession.GUID, nil)
    if err != nil {
        t.Fatal(err.Error())
    }

    // Send task output and check the response
    test_util.SendPostAndCheckResponse(t, BASE_TASK_URL+mockSession.GUID, "", nil, []byte(base64.StdEncoding.EncodeToString(xor([]byte("dummy output")))), ENCODED_HEARTBEAT, http.StatusOK)
}

// Test fetching payloads
func TestHandlePayloadDownload(t *testing.T) {
    _, teardownTest := setupTest(t)
    defer teardownTest(t)

    // Register session - must respond with heartbeat response
    test_util.SendPostAndCheckResponse(t, BASE_REGISTER_URL, "", nil, []byte(ENCODED_GUID), ENCODED_HEARTBEAT, http.StatusOK)

    // Check response for invalid payload download task
    _, err := handler_util.SetTask(REST_API_LISTEN_HOST, TEST_INVALID_INGRESS_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }
    test_util.SendGetAndCheckResponse(t, BASE_TASK_URL+mockSession.GUID, nil, ENCODED_SERVER_ERR_MSG, http.StatusInternalServerError)

    // Assign payload download task
    _, err = handler_util.SetTask(REST_API_LISTEN_HOST, TEST_INGRESS_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }

    // Request the test payload
    b64Response, err := http_util.PerformHttpGetAssertSuccess(BASE_TASK_URL+mockSession.GUID, nil)
    if err != nil {
        t.Fatal(err)
    }

    // Decode response and extract payload data
    xorResponse, _ := base64.StdEncoding.DecodeString(string(b64Response))
    response := xor(xorResponse)

    var payloadData map[string]interface{}
    err = json.Unmarshal(response, &payloadData)
    fileDataInt, ok := payloadData["moduleBytes"]
    if !ok {
        t.Fatal(ok)
    }
    fileDataStr, _ := fileDataInt.(string)
    fileData, _ := base64.StdEncoding.DecodeString(fileDataStr)

    // compare file hashes
    test_util.CheckMD5Hash(t, fileData, TEST_PAYLOAD_HASH)
}

// Test uploading files
func TestHandleFileUpload(t *testing.T) {
    _, teardownTest := setupTest(t)
    defer teardownTest(t)

    // Register session - must respond with heartbeat response
    test_util.SendPostAndCheckResponse(t, BASE_REGISTER_URL, "", nil, []byte(ENCODED_GUID), ENCODED_HEARTBEAT, http.StatusOK)

    // Assign payload upload task
    _, err := handler_util.SetTask(REST_API_LISTEN_HOST, TEST_EXFIL_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }

    // Upload a file
    test_util.SendPostAndCheckResponse(t, BASE_TASK_URL+mockSession.GUID, "", nil, []byte(ENCODED_UPLOAD_PACKET), ENCODED_HEARTBEAT, http.StatusOK)

    // compare file hashes
    test_util.CheckUploadedFileHash(t, TEST_UPLOAD_FILENAME, TEST_UPLOAD_HASH)
}
