package plugx

import (
    "net/http"
    "path/filepath"
    "testing"
    "encoding/base64"
    "bytes"
    "encoding/binary"
    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/handlers/handler_util"
    "evals.mitre.org/control_server/restapi/restapi_test_util"
    "evals.mitre.org/control_server/sessions"
    "evals.mitre.org/control_server/util"
    "evals.mitre.org/control_server/util/test_util"
)

const (
    REST_API_LISTEN_HOST      = "127.0.0.1:10009"
    BASE_URL                  = "http://127.0.0.1:10007/"
    TEST_TASK                 = "{\"id\":\"0x1001\"}"
    TEST_INVALID_TASK         = "Dummy invalid task string"
    REGISTRATION_PLACEHOLDER  = "success"
)

var (
    PAYLOAD_TEST_DIR  = filepath.Join(util.ProjectRoot, "test_payloads")
    PAYLOAD_TEST_DIRS = map[string]string{HANDLER_NAME: PAYLOAD_TEST_DIR}
    GUID            = mockSession.GUID
    HEADERS = map[string]string{
        "Sec-Dest": "example-implant-123",
        "Sec-Host": "cool data",
    }

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
func generateMockplugxHandler() *plugxHandler {
    mockFuncHandles := &WrappedFuncHandles{
        randStringGetter: handler_util.MockGetRandomAlphanumericString,
    }
    return plugxHandlerFactory(mockFuncHandles)
}

// Starts the RestAPI, generates the handler, and returns teardown function
func setupTest(t *testing.T) (*plugxHandler, func(t *testing.T)) {
    // Set up REST API server
    restapi_test_util.StartRestApiServer(t, "", REST_API_LISTEN_HOST, PAYLOAD_TEST_DIRS)

    // create and start mock handler
    handler := generateMockplugxHandler()
    test_util.StartHandlerForUnitTest(t, handler, REST_API_LISTEN_HOST, testConfigEntry)

    return handler, func(t *testing.T) {
        restapi_test_util.StopRestApiServer()
        test_util.StopHandlerForUnitTest(t, handler)
    }
}

// Test Beacon and registration
func TestHandleBeaconRegistration(t *testing.T) {
    _, teardownTest := setupTest(t)
    defer teardownTest(t)

    // Check response registration/beacon
    test_util.SendGetAndCheckResponse(t, BASE_URL, HEADERS, REGISTRATION_PLACEHOLDER, http.StatusOK)
}

// Test getting tasks
func TestHandleGetTask(t *testing.T) {
    _, teardownTest := setupTest(t)
    defer teardownTest(t)

    // Register session - must respond with heartbeat response
    test_util.SendGetAndCheckResponse(t, BASE_URL, HEADERS, REGISTRATION_PLACEHOLDER, http.StatusOK)

    // Check response when no task available
    respPacket := []byte{
        0x00, 0x00, 0x00, 0x00, // ID
        0x00, 0x00, 0x00, 0x00, // arg length
        0x12, 0x00, 0x00, 0x00, // content length

        // content
        0x42, 0x61, 0x73, 0x65, 0x20, 0x6e, 0x75, 0x6c, 0x6c, 0x20, 0x72, 0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65,
    }
    ciphertext, _ := CryptRC4(respPacket)
    encodedCiphertext := base64.StdEncoding.EncodeToString(ciphertext)
    test_util.SendGetAndCheckResponse(t, BASE_URL, HEADERS, encodedCiphertext, http.StatusOK)


    // Check response when invalid task string is sent
    _, err := handler_util.SetTask(REST_API_LISTEN_HOST, TEST_INVALID_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }

    test_util.SendGetAndCheckResponse(t, BASE_URL, HEADERS, encodedCiphertext, http.StatusOK)


    // Assign task
    _, err = handler_util.SetTask(REST_API_LISTEN_HOST, TEST_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }
}

// Test posting task output
func TestHandleTaskResponse(t *testing.T) {
    _, teardownTest := setupTest(t)
    defer teardownTest(t)


    // Register session and assign a task so we can send output
    test_util.SendGetAndCheckResponse(t, BASE_URL, HEADERS, REGISTRATION_PLACEHOLDER, http.StatusOK)

    // Check response when no task available
    respPacket := []byte{
        0x00, 0x00, 0x00, 0x00, // ID
        0x00, 0x00, 0x00, 0x00, // arg length
        0x12, 0x00, 0x00, 0x00, // content length

        // content
        0x42, 0x61, 0x73, 0x65, 0x20, 0x6e, 0x75, 0x6c, 0x6c, 0x20, 0x72, 0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65,
    }
    ciphertext, _ := CryptRC4(respPacket)
    encodedCiphertext := base64.StdEncoding.EncodeToString(ciphertext)
    test_util.SendGetAndCheckResponse(t, BASE_URL, HEADERS, encodedCiphertext, http.StatusOK)


    // Check response when invalid task string is sent
    _, err := handler_util.SetTask(REST_API_LISTEN_HOST, TEST_TASK, mockSession.GUID)
    if err != nil {
        t.Fatal(err.Error())
    }

    // Send task output and check the response
    test_util.SendGetAndCheckResponse(t, BASE_URL, nil, "", http.StatusOK)

}


// Test Beacon and registration
func TestBuildPacket(t *testing.T) {
    _, teardownTest := setupTest(t)
    defer teardownTest(t)

    id := uint32(123)
    args := "testarg"
    content := []byte("data")

    packet, err := BuildPacket(id, args, content)
    if err != nil {
        t.Errorf("Expected valid contruction, got %v", err)
    }
    if len(packet) == 0 {
        t.Error("Expected non-empty packet")
    }
    buf := bytes.NewReader(packet)
    var idOut uint32
    _ = binary.Read(buf, binary.LittleEndian, &idOut)
    if idOut != id {
        t.Errorf("Expected ID %d, got %d", id, idOut)
    }
}
