package fullhouse

import (
    "encoding/base64"
    "encoding/json"
    "fmt"
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
    REST_API_LISTEN_HOST = "127.0.0.1:10002"
    BASE_TASK_URL        = "http://127.0.0.1:10005/request/"
    BASE_RESPONSE_URL    = "http://127.0.0.1:10005/form/"
    BASE_UPLOAD_URL      = "http://127.0.0.1:10005/submit"
    BASE_FILES_URL       = "http://127.0.0.1:10005/files"
    TEST_TASK            = "Dummy command string"
    TEST_PAYLOAD         = "hello_world.elf"
    TEST_PAYLOAD_HASH    = "fe7c47d38224529c7d8f9a11a62cdd7a"
    TEST_UPLOAD_FILENAME = "test_upload.txt"
    TEST_UPLOAD_CONTENTS = "test upload file contents"
    TEST_UPLOAD_HASH     = "09347adcaffed3c23290b3b5a07a8a36"
    DISCOVERY_DATA       = "myUserName\r\nmyHostName\r\n1234"
)

var (
    PAYLOAD_TEST_DIR  = filepath.Join(util.ProjectRoot, "test_payloads")
    PAYLOAD_TEST_DIRS = map[string]string{HANDLER_NAME: PAYLOAD_TEST_DIR}

    ENCODED_DISCOVERY_DATA  = base64.StdEncoding.EncodeToString(xor([]byte(DISCOVERY_DATA)))
    ENCODED_TEST_TASK       = base64.StdEncoding.EncodeToString(xor([]byte(TEST_TASK)))
    ENCODED_HEARTBEAT       = base64.StdEncoding.EncodeToString(xor([]byte(heartbeatResp)))
    B64_UPLOAD_CONTENTS     = base64.StdEncoding.EncodeToString([]byte(TEST_UPLOAD_CONTENTS))
    ENCODED_UPLOAD_CONTENTS = base64.StdEncoding.EncodeToString(xor([]byte(B64_UPLOAD_CONTENTS)))
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
    "port": 10005,
}

// Generate example handler using the mock function wrappers
func generateMockFullhouseHandler() *fullhouseHandler {
    mockFuncHandles := &WrappedFuncHandles{
        randStringGetter: handler_util.MockGetRandomAlphanumericString,
    }
    return fullhouseHandlerFactory(mockFuncHandles)
}

// Starts the RestAPI, generates the handler, and returns teardown function
func setupTest(t *testing.T) (*fullhouseHandler, func(t *testing.T)) {
    // Set up REST API server
    restapi_test_util.StartRestApiServer(t, "", REST_API_LISTEN_HOST, PAYLOAD_TEST_DIRS)

    // create and start mock handler
    handler := generateMockFullhouseHandler()
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

    // Register session - must respond with empty task
    test_util.SendPostAndCheckResponse(t, BASE_TASK_URL+mockSession.GUID, "", nil, []byte(ENCODED_DISCOVERY_DATA), ENCODED_HEARTBEAT, http.StatusOK)

    // Assign task
    _, err := handler_util.SetTask(REST_API_LISTEN_HOST, TEST_TASK, mockSession.GUID)
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
    _, err := http_util.PerformHttpPostAssertSuccess(BASE_TASK_URL+mockSession.GUID, "", nil, []byte(ENCODED_DISCOVERY_DATA))
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
    test_util.SendPostAndCheckResponse(t, BASE_RESPONSE_URL+mockSession.GUID, "", nil, []byte(base64.StdEncoding.EncodeToString(xor([]byte("dummy output")))), ENCODED_HEARTBEAT, http.StatusOK)
}

// Test fetching payloads
func TestHandlePayloadDownload(t *testing.T) {
    _, teardownTest := setupTest(t)
    defer teardownTest(t)

    // Request the test payload
    url := fmt.Sprintf("%s/%s", BASE_FILES_URL, TEST_PAYLOAD)
    b64Response, err := http_util.PerformHttpGetAssertSuccess(url, nil)
    if err != nil {
        t.Fatal(err)
    }

    // Decode response
    xorResponse, _ := base64.StdEncoding.DecodeString(string(b64Response))
    response := xor(xorResponse)

    var payloadData map[string]interface{}
    _ = json.Unmarshal(response, &payloadData)
    fileDataInt, ok := payloadData["file_bytes"]
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

    // Upload a file
    _, err := http_util.PerformHttpPostAssertSuccess(BASE_UPLOAD_URL, "", map[string]string{"filename": TEST_UPLOAD_FILENAME}, []byte(ENCODED_UPLOAD_CONTENTS))
    if err != nil {
        t.Fatal(err)
    }

    // compare file hashes
    test_util.CheckUploadedFileHash(t, TEST_UPLOAD_FILENAME, TEST_UPLOAD_HASH)
}
