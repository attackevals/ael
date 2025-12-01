package restapi_test

import (
    "encoding/json"
    "fmt"
    "io/ioutil"
    "net/http"
    "os"
    "path/filepath"
    "testing"

    "evals.mitre.org/control_server/handlers/handler_manager"
    "evals.mitre.org/control_server/restapi"
    "evals.mitre.org/control_server/restapi/restapi_test_util"
    "evals.mitre.org/control_server/restapi/restapi_util"
    "evals.mitre.org/control_server/sessions"
    "evals.mitre.org/control_server/tasks"
    "evals.mitre.org/control_server/util"
    "evals.mitre.org/control_server/util/http_util"
    "evals.mitre.org/control_server/util/test_util"
    "evals.mitre.org/control_server/util/test_util/assert_util"
)

func init() {
    cwd, _ := os.Getwd()
    os.Chdir("..")
    defer os.Chdir(cwd)
    util.SetRootDirectories()

    restapi_test_util.SetPayloadTestDirs()
}

const (
    TEST_SESSION_1_GUID = "test-session-1"
    TEST_SESSION_2_GUID = "test-session-2"
    TEST_TASK_GUID      = "test-task-guid"
    TEST_TASK_COMMAND   = "whoami > file.txt"
    TEST_TASK_OUTPUT    = "newly-set-output"
)

var TEST_SESSION_1 = sessions.Session{
    GUID:          TEST_SESSION_1_GUID,
    IPAddr:        "127.0.0.1",
    HostName:      "myHostName",
    User:          "myUserName",
    Dir:           "C:\\MyDir\\",
    PID:           "1234",
    PPID:          "4",
    SleepInterval: 60,
    Jitter:        1.5,
}

var TEST_SESSION_2 = sessions.Session{
    GUID:          TEST_SESSION_2_GUID,
    IPAddr:        "127.0.0.2",
    HostName:      "myHostName2",
    User:          "myUserName2",
    Dir:           "C:\\MyDir2\\",
    PID:           "3456",
    PPID:          "5",
    SleepInterval: 61,
    Jitter:        2,
}

var NEW_TEST_TASK = tasks.Task{
    GUID:     TEST_TASK_GUID,
    Command:  TEST_TASK_COMMAND,
    Output:   "",
    Status:   tasks.TASK_STATUS_NEW,
    ExitCode: -1,
}

var TEST_SESSION_WITH_TASK = sessions.Session{
    GUID:          TEST_SESSION_1_GUID,
    IPAddr:        "127.0.0.1",
    HostName:      "myHostName",
    User:          "myUserName",
    Dir:           "C:\\MyDir\\",
    PID:           "1234",
    PPID:          "4",
    SleepInterval: 60,
    Jitter:        1.5,
    Task:          &NEW_TEST_TASK,
}

var TEST_RESP_SESSION_LIST = []sessions.Session{
    TEST_SESSION_1,
    TEST_SESSION_2,
}

var TEST_FINISHED_TASK = tasks.Task{
    GUID:     TEST_TASK_GUID,
    Command:  TEST_TASK_COMMAND,
    Output:   TEST_TASK_OUTPUT,
    Status:   tasks.TASK_STATUS_FINISHED,
    ExitCode: -1,
}

func TestJsonMarshalIndentNoHtmlEncode(t *testing.T) {
    toEncode := restapi_util.ApiStringResponse{
        ResponseType: 0,
        Status:       0,
        Data:         "&<test encode>\ns",
    }
    want := `{
  "type": 0,
  "status": 0,
  "data": "&<test encode>\ns"
}
`

    encoded, err := restapi_util.JsonMarshalIndentNoHtmlEncode(toEncode)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, want, string(encoded), "")
}

func TestStartStopRESTapi(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", "", nil)
    defer restapi_test_util.StopRestApiServer()
    want := "http://10.2.3.4:8888/plugins/emu/beacons"
    assert_util.AssertEq(t, want, restapi.CalderaForwardingEndpoint, "")
}

func TestGetVersion(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", "", nil)
    defer restapi_test_util.StopRestApiServer()

    getVersionURL := restapi_test_util.BASE_URL + "version"
    want := `{
  "type": 1,
  "status": 0,
  "data": "ATT&CK Evaluations Control Server 1.0"
}
`
    test_util.SendGetAndCheckResponse(t, getVersionURL, nil, want, http.StatusOK)
}

func TestGetConfig(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", "", nil)
    defer restapi_test_util.StopRestApiServer()

    getConfigURL := restapi_test_util.BASE_URL + "config"
    want := `{
  "type": 2,
  "status": 0,
  "data": {
    "Address": "127.0.0.1:9999",
    "DatabasePath": "",
    "CalderaForwardingAddress": "http://10.2.3.4:8888/plugins/emu",
    "EnableCalderaForwarding": true
  }
}
`
    test_util.SendGetAndCheckResponse(t, getConfigURL, nil, want, http.StatusOK)
}

func TestCreateSession(t *testing.T) {
    // setup test dependencies
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    // http://localhost:9999/api/v1.0/session
    createSessionURL := restapi_test_util.BASE_URL + "session"

    // convert testSession object into JSON
    testSessionJSON, err := json.Marshal(TEST_SESSION_1)
    if err != nil {
        t.Fatal(err)
    }
    want := `{
  "type": 0,
  "status": 0,
  "data": "Successfully added session."
}
`
    test_util.SendPostAndCheckResponse(t, createSessionURL, "application/json", nil, testSessionJSON, want, http.StatusOK)
}

func TestUpdateSession(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1)
    updateSession, err := json.Marshal(map[string]string{"user": "updatedUser", "hostName": "updatedHost", "pid": "9999"})
    if err != nil {
        t.Fatal(err)
    }

    url := restapi_test_util.BASE_URL + "session/update/" + TEST_SESSION_1_GUID
    want := `{
  "type": 0,
  "status": 0,
  "data": "Successfully updated session."
}
`
    test_util.SendPostAndCheckResponse(t, url, "application/json", nil, updateSession, want, http.StatusOK)
}

func TestGetSessions(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1)
    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_2)

    url := restapi_test_util.BASE_URL + "sessions"
    body, err := http_util.PerformHttpGetAssertSuccess(url, nil)
    if err != nil {
        t.Fatal(err)
    }

    var apiResponse restapi_util.ApiSessionsResponse
    err = json.Unmarshal(body, &apiResponse)
    if err != nil {
        t.Fatal(err)
    }
    want := restapi_util.ApiSessionsResponse{
        ResponseType: restapi_util.RESP_TYPE_SESSIONS,
        Status:       restapi_util.RESP_STATUS_SUCCESS,
        Data:         TEST_RESP_SESSION_LIST,
    }

    // ignore checkin times
    // have to do by index because range will copy values
    for i, _ := range apiResponse.Data {
        apiResponse.Data[i].FirstCheckIn = ""
        apiResponse.Data[i].LastCheckIn = ""
    }
    assert_util.AssertDeepEq(t, apiResponse, want, "")
}

func TestGetSessionByName(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1)
    url := restapi_test_util.BASE_URL + "session/" + TEST_SESSION_1_GUID
    body, err := http_util.PerformHttpGetAssertSuccess(url, nil)
    if err != nil {
        t.Fatal(err)
    }
    var apiResponse restapi_util.ApiSessionsResponse
    err = json.Unmarshal(body, &apiResponse)
    if err != nil {
        t.Fatal(err)
    }
    want := restapi_util.ApiSessionsResponse{
        ResponseType: restapi_util.RESP_TYPE_SESSIONS,
        Status:       restapi_util.RESP_STATUS_SUCCESS,
        Data: []sessions.Session{
            TEST_SESSION_1,
        },
    }

    // ignore checkin times
    for i, _ := range apiResponse.Data {
        apiResponse.Data[i].FirstCheckIn = ""
        apiResponse.Data[i].LastCheckIn = ""
    }
    assert_util.AssertDeepEq(t, apiResponse, want, "")
}

func TestSetTaskBySessionId(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1)
    url := fmt.Sprintf("%ssession/%s/task", restapi_test_util.BASE_URL, TEST_SESSION_1_GUID)
    body, err := http_util.PerformHttpPostAssertSuccess(url, "", map[string]string{"X-Task-Guid": TEST_TASK_GUID}, []byte(TEST_TASK_COMMAND))
    if err != nil {
        t.Fatal(err)
    }

    var apiResponse restapi_util.ApiTaskResponse
    err = json.Unmarshal(body, &apiResponse)
    if err != nil {
        t.Fatal(err)
    }
    want := restapi_util.ApiTaskResponse{
        ResponseType: restapi_util.RESP_TYPE_TASK_INFO,
        Status:       restapi_util.RESP_STATUS_SUCCESS,
        Data:         NEW_TEST_TASK,
    }
    assert_util.AssertDeepEq(t, apiResponse, want, "")
}

func TestGetTaskCommandBySessionId(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    // Set up session and task
    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1)
    restapi_test_util.CreateTestTaskForSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1_GUID, TEST_TASK_GUID, TEST_TASK_COMMAND)

    url := fmt.Sprintf("%ssession/%s/task", restapi_test_util.BASE_URL, TEST_SESSION_1_GUID)
    test_util.SendGetAndCheckResponse(t, url, nil, TEST_TASK_COMMAND, http.StatusOK)

    // The second request should return empty string, as the Task has already been assigned
    test_util.SendGetAndCheckResponse(t, url, nil, "", http.StatusOK)
}

func TestBootstrapTask(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    // register dummy handler
    handlerName := "testhandler"
    handler_manager.RunningHandlers[handlerName] = nil
    defer delete(handler_manager.RunningHandlers, handlerName)

    // http://localhost:9999/api/v1.0/bootstraptask/handler
    url := restapi_test_util.BASE_URL + "bootstraptask" + "/" + handlerName
    task := "whoami"
    expectedOutput := `{
  "type": 0,
  "status": 0,
  "data": "Successfully set bootstrap task for handler testhandler"
}
`
    test_util.SendPostAndCheckResponse(t, url, "", nil, []byte(task), expectedOutput, http.StatusOK)
    test_util.SendGetAndCheckResponse(t, url, nil, task, http.StatusOK)

    expectedOutput = `{
  "type": 0,
  "status": 0,
  "data": "Successfully removed bootstrap task for handler testhandler"
}
`
    test_util.SendDeleteAndCheckResponse(t, url, nil, expectedOutput, http.StatusOK)
}

func TestBootstrapTaskNotRunningHandler(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    handlerName := "notrunning"
    url := restapi_test_util.BASE_URL + "bootstraptask" + "/" + handlerName
    task := "whoami"
    expectedOutput := `{
  "type": 0,
  "status": 1,
  "data": "SetBootstrapTask: handler notrunning is not currently running. Failed to set bootstrap task."
}
`
    test_util.SendPostAndCheckResponse(t, url, "", nil, []byte(task), expectedOutput, http.StatusInternalServerError)
    test_util.SendGetAndCheckResponse(t, url, nil, "", http.StatusOK)
    expectedOutput = `{
  "type": 0,
  "status": 1,
  "data": "RemoveBootstrapTask: handler notrunning is not currently running. Cannot manage bootstrap tasks."
}
`
    test_util.SendDeleteAndCheckResponse(t, url, nil, expectedOutput, http.StatusInternalServerError)
}

func TestSetTaskOutputBySessionId(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1)
    restapi_test_util.CreateTestTaskForSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1.GUID, TEST_FINISHED_TASK.GUID, TEST_FINISHED_TASK.Command)

    url := restapi_test_util.BASE_URL + "session/" + TEST_SESSION_1.GUID + "/task/output"
    body, err := http_util.PerformHttpPostAssertSuccess(url, "", nil, []byte(TEST_TASK_OUTPUT))
    if err != nil {
        t.Fatal(err)
    }
    var apiResponse restapi_util.ApiStringResponse
    err = json.Unmarshal(body, &apiResponse)
    if err != nil {
        t.Fatal(err)
    }
    expectedOutput := restapi_util.ApiStringResponse{
        ResponseType: restapi_util.RESP_TYPE_CTRL,
        Status:       0,
        Data:         "Successfully set task output.",
    }
    assert_util.AssertDeepEq(t, apiResponse, expectedOutput, "")

    returnedSession := restapi_test_util.FetchSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1.GUID)
    assert_util.AssertDeepEq(t, *(returnedSession.Task), TEST_FINISHED_TASK, "")
}

func TestGetTaskOutputBySessionId(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1)
    restapi_test_util.CreateTestTaskForSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1.GUID, TEST_FINISHED_TASK.GUID, TEST_FINISHED_TASK.Command)

    // set task output
    url := restapi_test_util.BASE_URL + "session/" + TEST_SESSION_1.GUID + "/task/output"
    _, err := http_util.PerformHttpPostAssertSuccess(url, "", nil, []byte(TEST_TASK_OUTPUT))
    if err != nil {
        t.Fatal(err)
    }

    // get task output
    preformatted := `{
  "type": 5,
  "status": 0,
  "data": "%s"
}
`
    want := fmt.Sprintf(preformatted, TEST_TASK_OUTPUT)
    test_util.SendGetAndCheckResponse(t, url, nil, want, http.StatusOK)
}

func TestRemoveTaskOutputBySessionId(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_WITH_TASK)
    url := restapi_test_util.BASE_URL + "session/" + TEST_SESSION_WITH_TASK.GUID + "/task/output"

    preformatted := `{
  "type": 0,
  "status": 0,
  "data": "Successfully deleted task output for session: %s"
}
`
    expectedOutput := fmt.Sprintf(preformatted, TEST_SESSION_WITH_TASK.GUID)
    test_util.SendDeleteAndCheckResponse(t, url, nil, expectedOutput, http.StatusOK)
}

func TestRemoveSession(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1)
    url := restapi_test_util.BASE_URL + "session/delete/" + TEST_SESSION_1.GUID
    preformatted := `{
  "type": 0,
  "status": 0,
  "data": "Successfully removed session: %s"
}
`
    expectedOutput := fmt.Sprintf(preformatted, TEST_SESSION_1.GUID)
    test_util.SendDeleteAndCheckResponse(t, url, nil, expectedOutput, http.StatusOK)
}

func TestGetFile(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    url := restapi_test_util.BASE_URL + "files/hello_world.elf"
    testFile, err := http_util.PerformHttpGetAssertSuccess(url, nil)
    if err != nil {
        t.Fatal(err)
    }
    test_util.CheckMD5Hash(t, testFile, "fe7c47d38224529c7d8f9a11a62cdd7a")

    // test non-existent file
    url = restapi_test_util.BASE_URL + "files/does_not_exist.txt"
    test_util.SendGetAndCheckResponse(t, url, nil, "404 page not found\n", http.StatusNotFound)
}

func TestUploadFile(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()

    // read file
    fileData, err := ioutil.ReadFile(filepath.Join(util.ProjectRoot, "test_payloads", "hello_world.elf"))
    if err != nil {
        t.Fatal(err)
    }

    // upload file via HTTP POST
    url := restapi_test_util.BASE_URL + "upload/test_file.elf"
    contentType := http.DetectContentType(fileData)
    expectedOutputTemplate := `{
  "type": 0,
  "status": 0,
  "data": "Successfully uploaded file to control server at '%s/files/test_file.elf'"
}
`
    expectedOutput := fmt.Sprintf(expectedOutputTemplate, util.ProjectRoot)
    test_util.SendPostAndCheckResponse(t, url, contentType, nil, fileData, expectedOutput, http.StatusOK)

    // get MD5 hash from uploaded file
    defer test_util.RemoveUploadedFile(t, "test_file.elf")
    test_util.CheckUploadedFileHash(t, "test_file.elf", "fe7c47d38224529c7d8f9a11a62cdd7a")
}

func TestGetTask(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()
    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1)
    restapi_test_util.CreateTestTaskForSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1.GUID, NEW_TEST_TASK.GUID, NEW_TEST_TASK.Command)

    // http://localhost:9999/api/v1.0/task/output/{guid}
    url := fmt.Sprintf("%stask/%s", restapi_test_util.BASE_URL, NEW_TEST_TASK.GUID)
    body, err := http_util.PerformHttpGetAssertSuccess(url, nil)

    var apiResponse restapi_util.ApiTaskResponse
    err = json.Unmarshal(body, &apiResponse)
    if err != nil {
        t.Fatal(err)
    }

    want := restapi_util.ApiTaskResponse{
        ResponseType: restapi_util.RESP_TYPE_TASK_INFO,
        Status:       restapi_util.RESP_STATUS_SUCCESS,
        Data:         NEW_TEST_TASK,
    }

    assert_util.AssertDeepEq(t, apiResponse, want, "")
}

func TestSetGetAndRemoveTaskOutput(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", restapi_test_util.REST_API_BIND_ADDR, nil)
    defer restapi_test_util.StopRestApiServer()
    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1)
    restapi_test_util.CreateTestTaskForSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1.GUID, TEST_FINISHED_TASK.GUID, TEST_FINISHED_TASK.Command)

    // http://localhost:9999/api/v1.0/task/output/{guid}
    url := fmt.Sprintf("%stask/output/%s", restapi_test_util.BASE_URL, TEST_FINISHED_TASK.GUID)
    preformatted := `{
  "type": 0,
  "status": 0,
  "data": "Successfully set task output for task: %s"
}
`
    expectedOutput := fmt.Sprintf(preformatted, TEST_FINISHED_TASK.GUID)
    test_util.SendPostAndCheckResponse(t, url, "", nil, []byte(TEST_TASK_OUTPUT), expectedOutput, http.StatusOK)

    returnedSession := restapi_test_util.FetchSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1.GUID)
    assert_util.AssertDeepEq(t, *(returnedSession.Task), TEST_FINISHED_TASK, "")

    // http://localhost:9999/api/v1.0/task/output/{guid}
    url = fmt.Sprintf("%stask/output/%s", restapi_test_util.BASE_URL, TEST_FINISHED_TASK.GUID)
    preformatted = `{
  "type": 5,
  "status": 0,
  "data": "%s"
}
`
    expectedOutput = fmt.Sprintf(preformatted, TEST_TASK_OUTPUT)
    test_util.SendGetAndCheckResponse(t, url, nil, expectedOutput, http.StatusOK)

    // Remove task output
    url = fmt.Sprintf("%stask/output/%s", restapi_test_util.BASE_URL, TEST_FINISHED_TASK.GUID)
    preformatted = `{
  "type": 0,
  "status": 0,
  "data": "Successfully removed task output for task: %s"
}
`
    expectedOutput = fmt.Sprintf(preformatted, TEST_FINISHED_TASK.GUID)
    test_util.SendDeleteAndCheckResponse(t, url, nil, expectedOutput, http.StatusOK)
}

func TestForwardSessionBeacon(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", "", nil)
    defer restapi_test_util.StopRestApiServer()
    test_util.StartMockCalderaServer()
    restapi.CalderaForwardingEndpoint = "http://127.0.0.1:8888/plugins/emu/beacons"
    defer test_util.StopMockCalderaServer()

    restapi_test_util.CreateTestSession(t, restapi_test_util.REST_API_BIND_ADDR, TEST_SESSION_1)

    // http://localhost:9999/api/v1.0/forwarder/session/{guid}
    url := fmt.Sprintf("%sforwarder/session/%s", restapi_test_util.BASE_URL, TEST_SESSION_1.GUID)
    body, err := http_util.PerformHttpPostAssertSuccess(url, "", nil, []byte(""))
    if err != nil {
        t.Fatal(err)
    }
    var apiResponse restapi_util.ApiStringResponse
    err = json.Unmarshal(body, &apiResponse)
    if err != nil {
        t.Fatal(err)
    }
    want := restapi_util.ApiStringResponse{
        ResponseType: restapi_util.RESP_TYPE_CTRL,
        Status:       restapi_util.RESP_STATUS_SUCCESS,
        Data:         fmt.Sprintf("Forwarded beacon for session: %s, received response: CALDERA server successfully received session: %s", TEST_SESSION_1.GUID, TEST_SESSION_1.GUID),
    }
    assert_util.AssertDeepEq(t, want, apiResponse, "")
}
