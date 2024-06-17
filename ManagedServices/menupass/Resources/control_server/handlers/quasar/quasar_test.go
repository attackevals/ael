package quasar

import (
    "bytes"
    "crypto/md5"
    "encoding/hex"
    "errors"
    "fmt"
    "io/ioutil"
    "net/http"
    "os"
    "reflect"
    "strings"
    "testing"
    "time"
    
    "attackevals.mitre-engenuity.org/control_server/restapi"
)

var mockHttpGetResp []byte
var mockHttpPostResp []byte

var mockSession1 = QuasarSession{
    Id: "TESTID1",
    Username: "USER1",
    Hostname: "HOST1",
    IpAddress: "10.0.1.1",
    Os: "Windows 10",
    AccountType: "Admin",
    Tag: "TAG1",
    CountryCode: "us",
    HardwareId: "HWID1",
}

var mockSession2 = QuasarSession{
    Id: "TESTID2",
    Username: "USER2",
    Hostname: "HOST2",
    IpAddress: "10.0.1.2",
    Os: "Windows 11",
    AccountType: "User",
    Tag: "TAG2",
    CountryCode: "nz",
    HardwareId: "HWID2",
}

const (
    restAPIlistenHost = "127.0.0.1:9994"
    restAPIBaseURL = "http://" + restAPIlistenHost + "/api/v1.0/"
    mockSessionsRespString = `[
    {
        "id": "TESTID1",
        "username": "USER1",
        "hostname": "HOST1",
        "ip_addr": "10.0.1.1",
        "os": "Windows 10",
        "account_type": "Admin",
        "tag": "TAG1",
        "country_code": "us",
        "hardware_id": "HWID1"
    },
    {
        "id": "TESTID2",
        "username": "USER2",
        "hostname": "HOST2",
        "ip_addr": "10.0.1.2",
        "os": "Windows 11",
        "account_type": "User",
        "tag": "TAG2",
        "country_code": "nz",
        "hardware_id": "HWID2"
    }
]`
    dummyProcTaskString = "{\"seq\": 1, \"type\": 1, \"proc_path\": \"whoami.exe\", \"proc_args\": \"/all\"}"
    dummyPortScanTaskString = "{\"seq\": 2, \"type\": 5, \"range\": \"10.1.2.0/24\", \"ports\": [80, 443, 8080]}"
    
    mockTaskExecProcErrorRespString = `{
    "task_id": "caedfd24-2078-42b6-9a7e-e8724173073a",
    "task_type": 1,
    "task_status": 1,
    "task_status_msg": "Dummy error",
    "proc_path": "whoami.exe",
    "proc_args": "/all",
    "download_url": "",
    "download_dst": "",
    "use_shell": false,
    "get_output": true,
    "no_window": true,
    "pid": -1,
    "exit_code": -1,
    "stdout": "",
    "stderr": "" 
}`

    mockTaskExecProcPendingRespString = `{
    "task_id": "caedfd24-2078-42b6-9a7e-e8724173073a",
    "task_type": 1,
    "task_status": 2,
    "task_status_msg": "",
    "proc_path": "whoami.exe",
    "proc_args": "/all",
    "download_url": "",
    "download_dst": "",
    "use_shell": false,
    "get_output": true,
    "no_window": true,
    "pid": -1,
    "exit_code": -1,
    "stdout": "",
    "stderr": ""
}`

    mockTaskExecProcCompleteRespString = `{
    "task_id": "97bfb33b-d16b-4c06-92dc-95086b618e4a",
    "task_type": 1,
    "task_status": 0,
    "task_status_msg": "",
    "proc_path": "whoami.exe",
    "proc_args": "/all",
    "download_url": "",
    "download_dst": "",
    "use_shell": false,
    "get_output": true,
    "no_window": true,
    "pid": 27356,
    "exit_code": 0,
    "stdout": "dummystdout",
    "stderr": "dummystderr"
}`

    mockTransferErrorRespString = `{
    "id": 356532246,
    "type": 1,
    "size": 0,
    "transferred_size": 0,
    "local_path": "test_src",
    "remote_path": "test_dst",
    "status_msg": "Dummy error",
    "status": 1
}`
    mockTransferError2RespString = `{
    "id": 109357389,
    "type": 1,
    "size": 0,
    "transferred_size": 0,
    "local_path": "test_src3",
    "remote_path": "test_dst3",
    "status_msg": "Dummy error",
    "status": 1
}`
    mockTransferCanceledRespString = `{
    "id": 356532246,
    "type": 1,
    "size": 0,
    "transferred_size": 0,
    "local_path": "test_src",
    "remote_path": "test_dst",
    "status_msg": "Canceled",
    "status": 3
}`
    mockTransferPendingRespString = `{
    "id": 356532246,
    "type": 1,
    "size": 0,
    "transferred_size": 0,
    "local_path": "test_src",
    "remote_path": "test_dst",
    "status_msg": "Pending...",
    "status": 2
}`
    mockTransferSuccessRespString = `{
    "id": 356532246,
    "type": 1,
    "size": 100,
    "transferred_size": 100,
    "local_path": "test_src",
    "remote_path": "test_dst",
    "status_msg": "Completed",
    "status": 0
}`

    mockTransferSuccess2RespString = `{
    "id": 358719857,
    "type": 1,
    "size": 100,
    "transferred_size": 100,
    "local_path": "test_src2",
    "remote_path": "test_dst2",
    "status_msg": "Completed",
    "status": 0
}`

    mockKeylogUploadPendingRespString = `{
    "task_id": "fb76942c-9f28-4c0d-ab5a-7b4b5858fdb9",
    "status": 2,
    "status_msg": "",
    "status_err_msg": "",
    "transfer_ids": []
}`

    mockKeylogUploadPending2RespString = `{
    "task_id": "126fbde6-3889-40de-9be6-01d8779bdbf6",
    "status": 2,
    "status_msg": "",
    "status_err_msg": "",
    "transfer_ids": []
}`

    mockKeylogUploadErrorRespString = `{
    "task_id": "fb76942c-9f28-4c0d-ab5a-7b4b5858fdb9",
    "status": 1,
    "status_msg": "",
    "status_err_msg": "Dummy error",
    "transfer_ids": [
        356532246
    ]
}`
    mockKeylogUploadPending3RespString = `{
    "task_id": "e7908310-2371-4abb-a16d-ac94314b3747",
    "status": 2,
    "status_msg": "",
    "status_err_msg": "",
    "transfer_ids": []
}`

    mockKeylogUploadSuccessNoLogsRespString = `{
    "task_id": "fb76942c-9f28-4c0d-ab5a-7b4b5858fdb9",
    "status": 0,
    "status_msg": "Successfully retrieved all logs",
    "status_err_msg": "",
    "transfer_ids": []
}`

    mockKeylogUploadSuccess2LogsRespString = `{
    "task_id": "126fbde6-3889-40de-9be6-01d8779bdbf6",
    "status": 0,
    "status_msg": "Successfully retrieved all logs",
    "status_err_msg": "",
    "transfer_ids": [
        356532246,
        358719857
    ]
}`
    mockKeylogUploadSuccess2Logs2RespString = `{
    "task_id": "e7908310-2371-4abb-a16d-ac94314b3747",
    "status": 0,
    "status_msg": "Successfully retrieved all logs",
    "status_err_msg": "",
    "transfer_ids": [
        356532246,
        109357389
    ]
}`

    mockKeylogUploadPendingPartialRespString = `{
    "task_id": "e6d7828a-2ab7-434b-9956-fd4d47b9cdf2",
    "status": 5,
    "status_msg": "",
    "status_err_msg": "",
    "transfer_ids": []
}`

    mockKeylogUploadPartialSuccessRespString = `{
    "task_id": "e6d7828a-2ab7-434b-9956-fd4d47b9cdf2",
    "status": 4,
    "status_msg": "Successfully retrieved some logs",
    "status_err_msg": "Dummy partial error",
    "transfer_ids": [
        356532246
    ]
}`

    mockKeylogUploadPendingPartial2RespString = `{
    "task_id": "fb5d9143-6e15-44d6-92b2-1f21e4bf95e2",
    "status": 5,
    "status_msg": "",
    "status_err_msg": "",
    "transfer_ids": []
}`

    mockKeylogUploadPartialSuccess2RespString = `{
    "task_id": "fb5d9143-6e15-44d6-92b2-1f21e4bf95e2",
    "status": 4,
    "status_msg": "Successfully retrieved some logs",
    "status_err_msg": "Dummy partial error",
    "transfer_ids": [
        356532246,
        109357389
    ]
}`

    mockTaskExecPortScanPendingRespString = `{
    "task_id": "5d7667e3-2a5c-45da-8fac-9256bca81c65",
    "task_type": 5,
    "task_status": 2,
    "task_status_msg": "",
    "range": "10.0.2.9/28",
    "ports": [
        8888,
        3389
    ],
    "result": {}
}`

    mockTaskExecPortScanCompleteRespString = `{
    "task_id": "5d7667e3-2a5c-45da-8fac-9256bca81c65",
    "task_type": 5,
    "task_status": 0,
    "task_status_msg": "",
    "range": "10.0.2.9/28",
    "ports": [
        8888,
        3389
    ],
    "result": {
        "10.0.2.4": [
            3389
        ],
        "10.0.2.5": [
            3389
        ]
    }
}`

    dummyFileContents = "dummyfilecontents"
    dummyFileContentsHash = "eb4174ca832389d274bb000cf97bbd46"
    
    dummyFileContents2 = "dummy file contents 2"
    dummyFileContents2Hash = "0b563ddd367af502bc4c43116d41972c"
)

func MockPerformHttpGet(url string) ([]byte, error) {
    return mockHttpGetResp, nil
}

func MockPerformHttpPost(url string, contentType string, data []byte) ([]byte, error) {
    return mockHttpPostResp, nil
}

func MockPerformHttpGetSelective(url string) ([]byte, error) {
    if strings.HasSuffix(url, "/api/transfers/356532246") {
        return []byte(mockTransferSuccessRespString), nil
    } else if strings.HasSuffix(url, "/api/transfers/358719857") {
        return []byte(mockTransferSuccess2RespString), nil
    } else if strings.HasSuffix(url, "/api/transfers/109357389") {
        return []byte(mockTransferError2RespString), nil
    } else if strings.HasSuffix(url, "/api/uploadedfiles/356532246") {
        return []byte(dummyFileContents), nil
    } else if strings.HasSuffix(url, "/api/uploadedfiles/358719857") {
        return []byte(dummyFileContents2), nil
    } else if strings.HasSuffix(url, "/api/tasks/caedfd24-2078-42b6-9a7e-e8724173073a") {
        return []byte(mockTaskExecProcCompleteRespString), nil
    } else if strings.HasSuffix(url, "/api/tasks/5d7667e3-2a5c-45da-8fac-9256bca81c65") {
        return []byte(mockTaskExecPortScanCompleteRespString), nil
    } else if strings.HasSuffix(url, "/api/transfers/keylogger/fb76942c-9f28-4c0d-ab5a-7b4b5858fdb9") {
        return []byte(mockKeylogUploadSuccessNoLogsRespString), nil
    } else if strings.HasSuffix(url, "/api/transfers/keylogger/126fbde6-3889-40de-9be6-01d8779bdbf6") {
        return []byte(mockKeylogUploadSuccess2LogsRespString), nil
    } else if strings.HasSuffix(url, "/api/transfers/keylogger/e7908310-2371-4abb-a16d-ac94314b3747") {
        return []byte(mockKeylogUploadSuccess2Logs2RespString), nil
    } else if strings.HasSuffix(url, "/api/transfers/keylogger/e6d7828a-2ab7-434b-9956-fd4d47b9cdf2") {
        return []byte(mockKeylogUploadPartialSuccessRespString), nil
    } else if strings.HasSuffix(url, "/api/transfers/keylogger/fb5d9143-6e15-44d6-92b2-1f21e4bf95e2") {
        return []byte(mockKeylogUploadPartialSuccess2RespString), nil
    } else {
        return nil, errors.New("Unsupported url" + url)
    }
}

func MockSleep(d time.Duration) {
    return
}

func taskEqual(a, b QuasarEvalsTask) bool {
    return a.TaskNum == b.TaskNum &&
        a.TaskType == b.TaskType &&
        a.Timeout == b.Timeout &&
        a.FileTransferSource == b.FileTransferSource &&
        a.FileTransferDest == b.FileTransferDest &&
        a.ProcPath == b.ProcPath &&
        a.ProcArgs == b.ProcArgs &&
        a.DownloadUrl == b.DownloadUrl &&
        a.DownloadDst == b.DownloadDst &&
        (a.ShellExec == b.ShellExec || a.TaskType != TASK_TYPE_EXEC_PROC) &&
        (a.GetOutput == b.GetOutput || a.TaskType != TASK_TYPE_EXEC_PROC) &&
        (a.NoWindow == b.NoWindow || a.TaskType != TASK_TYPE_EXEC_PROC) &&
        a.TargetRange == b.TargetRange &&
        reflect.DeepEqual(a.TargetPorts, b.TargetPorts)
}

func generateMockedQuasarHandler() *QuasarHandler {
    mockFuncHandles := &QuasarWrappedFuncHandles {
        httpGetResponseGetter: MockPerformHttpGet,
        httpPostResponseGetter: MockPerformHttpPost,
        sleepFuncWrapper: MockSleep,
    }
    handler := quasarHandlerFactory(mockFuncHandles)
    handler.restAPIaddress = restAPIlistenHost
    return handler
}

func generatedMockedQuasarHandlerCustomFunc(httpGetRespGetter HttpGetRequestResponseGetter, httpPostRespGetter HttpPostRequestResponseGetter) *QuasarHandler {
    mockFuncHandles := &QuasarWrappedFuncHandles {
        httpGetResponseGetter: httpGetRespGetter,
        httpPostResponseGetter: httpPostRespGetter,
        sleepFuncWrapper: MockSleep,
    }
    handler := quasarHandlerFactory(mockFuncHandles)
    handler.restAPIaddress = restAPIlistenHost
    return handler
}

func startRESTAPI(t *testing.T) {
    restapi.Start(restAPIlistenHost, "./test_payloads")
    time.Sleep(50 * time.Millisecond)
    t.Log("Started REST API server")
}

func stopRESTAPI(t *testing.T) {
    restapi.Stop()
    time.Sleep(50 * time.Millisecond)
    t.Log("Stopped REST API server")
}

func TestGetActiveSessions(t *testing.T) {
    mockHttpGetResp = []byte(mockSessionsRespString)
    handler := generateMockedQuasarHandler()
    sessions, err := handler.getActiveSessions()
    if err != nil {
        t.Error(err)
    }
    sessionsWant := []QuasarSession{
        mockSession1,
        mockSession2,
    }
    if !reflect.DeepEqual(sessionsWant, sessions) {
        t.Errorf("Expected %v, got %v", sessionsWant, sessions)
    }
}

func TestGetActiveSessionsEmpty(t *testing.T) {
    mockHttpGetResp = []byte("[]")
    handler := generateMockedQuasarHandler()
    sessions, err := handler.getActiveSessions()
    if err != nil {
        t.Error(err)
    }
    if len(sessions) != 0 {
        t.Errorf("Expected empty sessions list, got %d", len(sessions))
    }
}

func TestUpdateSessionsConnectedStatusEmpty(t *testing.T) {
    handler := generateMockedQuasarHandler()
    sessions := []QuasarSession{}
    if len(handler.sessionConnectedMap) > 0 {
        t.Errorf("Expected empty sessionsActiveMap, got %v", handler.sessionConnectedMap)
    }
    handler.updateSessionsConnectedStatus(sessions)
    if len(handler.sessionConnectedMap) > 0 {
        t.Errorf("Expected empty sessionsActiveMap, got %v", handler.sessionConnectedMap)
    }
}

func TestUpdateSessionsConnectedStatus(t *testing.T) {
    handler := generateMockedQuasarHandler()
    sessions := []QuasarSession{mockSession1, mockSession2}
    handler.updateSessionsConnectedStatus(sessions)
    wantMap := map[string]bool{mockSession1.Id: true, mockSession2.Id: true}
    if !reflect.DeepEqual(handler.sessionConnectedMap, wantMap) {
        t.Errorf("Expected %v, got %v", wantMap, handler.sessionConnectedMap)
    }
    
    // Test 1 beacon disconnect
    sessions = []QuasarSession{mockSession1}
    handler.updateSessionsConnectedStatus(sessions)
    wantMap = map[string]bool{mockSession1.Id: true, mockSession2.Id: false}
    if !reflect.DeepEqual(handler.sessionConnectedMap, wantMap) {
        t.Errorf("Expected %v, got %v", wantMap, handler.sessionConnectedMap)
    }
    
    // Test beacon reconnect
    sessions = []QuasarSession{mockSession1, mockSession2}
    handler.updateSessionsConnectedStatus(sessions)
    wantMap = map[string]bool{mockSession1.Id: true, mockSession2.Id: true}
    if !reflect.DeepEqual(handler.sessionConnectedMap, wantMap) {
        t.Errorf("Expected %v, got %v", wantMap, handler.sessionConnectedMap)
    }
}

func TestGetUploadedFileName(t *testing.T) {
    got := getUploadedFileName("C:\\path\\test.txt")
    want := "test.txt"
    if got != want {
        t.Errorf("Expected %v, got %v", want, got)
    }
    got = getUploadedFileName("test.txt")
    if got != want {
        t.Errorf("Expected %v, got %v", want, got)
    }
}

func TestExtractEvalsTaskStructFileTransfer(t *testing.T) {
    got, err := extractEvalsTaskStruct("{\"seq\": 0, \"type\": 2, \"transfer_src\": \"test_src\"}")
    if err != nil {
        t.Error(err)
    }
    want := QuasarEvalsTask{
        TaskNum: 0,
        TaskType: 2,
        Timeout: 0,
        FileTransferSource: "test_src",
        FileTransferDest: "",
    }
    if !taskEqual(want, *got) {
        t.Errorf("Expected %v, got %v", want, *got)
    }
    got, err = extractEvalsTaskStruct("{\"seq\": 1, \"timeout\": 120, \"type\": 3, \"transfer_src\": \"test_src\", \"transfer_dst\": \"test_dst\"}")
    if err != nil {
        t.Error(err)
    }
    want = QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 3,
        Timeout: 120,
        FileTransferSource: "test_src",
        FileTransferDest: "test_dst",
    }
    if !taskEqual(want, *got) {
        t.Errorf("Expected %v, got %v", want, *got)
    }
}

func TestExtractEvalsTaskStructProc(t *testing.T) {
    // Default values
    got, err := extractEvalsTaskStruct("{\"seq\": 0, \"type\": 1, \"proc_path\": \"whoami.exe\"}")
    if err != nil {
        t.Error(err)
    }
    want := QuasarEvalsTask{
        TaskNum: 0,
        TaskType: 1,
        Timeout: 0,
        ProcPath: "whoami.exe",
        ProcArgs: "",
        DownloadUrl: "",
        DownloadDst: "",
        ShellExec: false,
        GetOutput: true,
        NoWindow: true,
    }
    if !taskEqual(want, *got) {
        t.Errorf("Expected %v, got %v", want, *got)
    }
    
    // Test specific values
    got, err = extractEvalsTaskStruct("{\"seq\": 1, \"timeout\": 180, \"type\": 1, \"proc_path\": \"test.exe\", \"proc_args\": \"test args\", \"no_window\": false, \"use_shell\": true, \"get_output\": false}")
    if err != nil {
        t.Error(err)
    }
    want = QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 1,
        Timeout: 180,
        ProcPath: "test.exe",
        ProcArgs: "test args",
        DownloadUrl: "",
        DownloadDst: "",
        ShellExec: true,
        GetOutput: false,
        NoWindow: false,
    }
    if !taskEqual(want, *got) {
        t.Errorf("Expected %v, got %v", want, *got)
    }
}

func TestExtractEvalsTaskStructPortScan(t *testing.T) {
    got, err := extractEvalsTaskStruct("{\"seq\": 2, \"type\": 5, \"range\": \"10.1.2.0/24\", \"ports\": [80, 443, 8080]}")
    if err != nil {
        t.Error(err)
    }
    want := QuasarEvalsTask{
        TaskNum: 2,
        TaskType: 5,
        TargetRange: "10.1.2.0/24",
        TargetPorts: []int{80, 443, 8080},
    }
    if !taskEqual(want, *got) {
        t.Errorf("Expected %v, got %v", want, *got)
    }
}

func TestTransferStruct(t *testing.T) {
    transferJsonStr := `{
    "id": 1477877996,
    "type": 0,
    "size": 4821,
    "transferred_size": 4821,
    "local_path": "C:\\Users\\testuser\\myfile.text",
    "remote_path": "C:\\Users\\Public\\myfile.text",
    "status_msg": "Completed",
    "status": 0
}`
    got, err := extractTransferStruct([]byte(transferJsonStr))
    if err != nil {
        t.Error(err)
    }
    want := QuasarFileTransfer{
        Id: 1477877996,
        Type: 0,
        Size: 4821,
        TransferredSize: 4821,
        LocalPath: "C:\\Users\\testuser\\myfile.text",
        RemotePath: "C:\\Users\\Public\\myfile.text",
        StatusMsg: "Completed",
        StatusCode: 0,
    }
    if !reflect.DeepEqual(want, *got) {
        t.Errorf("Expected %v, got %v", want, *got)
    }
}

func TestPerformTaskUnsupportedTaskType(t *testing.T) {
    badTask := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 20,
        FileTransferSource: "test_src",
        FileTransferDest: "test_dst",
    }
    handler := generateMockedQuasarHandler()
    err := handler.performTask("testid", &badTask)
    errWant := "Unsupported task type 20"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
}

func TestPerformTaskExecProcError(t *testing.T) {
    task, err := extractEvalsTaskStruct(dummyProcTaskString)
    if err != nil {
        t.Error(err)
    }
    handler := generateMockedQuasarHandler()
    mockHttpPostResp = []byte(mockTaskExecProcPendingRespString)
    mockHttpGetResp = []byte(mockTaskExecProcErrorRespString)
    err = handler.performTask("testid", task)
    errWant := "Task caedfd24-2078-42b6-9a7e-e8724173073a terminated due to an error: Dummy error"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
}

func TestPerformTaskExecProcTimedOut(t *testing.T) {
    task, err := extractEvalsTaskStruct(dummyProcTaskString)
    if err != nil {
        t.Error(err)
    }
    handler := generateMockedQuasarHandler()
    mockHttpPostResp = []byte(mockTaskExecProcPendingRespString)
    mockHttpGetResp = []byte(mockTaskExecProcPendingRespString)
    err = handler.performTask("testid", task)
    errWant := "Failed to wait for task completion for task ID caedfd24-2078-42b6-9a7e-e8724173073a: Timed out while waiting for task caedfd24-2078-42b6-9a7e-e8724173073a to complete"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
}

func TestPerformTaskExecProcSuccess(t *testing.T) {
    startRESTAPI(t)
    defer stopRESTAPI(t)
    handler := generateMockedQuasarHandler()
    err := handler.registerNewImplant(&mockSession1)
    if err != nil {
        t.Error(err)
    }
    
    _, err = setTask(dummyProcTaskString, mockSession1.Id)
    if err != nil {
        t.Error(err)
    }
    
    task, err := extractEvalsTaskStruct(dummyProcTaskString)
    if err != nil {
        t.Error(err)
    }
    
    mockHttpPostResp = []byte(mockTaskExecProcPendingRespString)
    mockHttpGetResp = []byte(mockTaskExecProcCompleteRespString)
    err = handler.performTask(mockSession1.Id, task)
    if err != nil {
        t.Error(err)
    }
}

func TestPerformTaskPortScanSuccess(t *testing.T) {
    startRESTAPI(t)
    defer stopRESTAPI(t)
    handler := generateMockedQuasarHandler()
    err := handler.registerNewImplant(&mockSession1)
    if err != nil {
        t.Error(err)
    }
    
    _, err = setTask(dummyPortScanTaskString, mockSession1.Id)
    if err != nil {
        t.Error(err)
    }
    
    task, err := extractEvalsTaskStruct(dummyPortScanTaskString)
    if err != nil {
        t.Error(err)
    }
    
    mockHttpPostResp = []byte(mockTaskExecPortScanPendingRespString)
    mockHttpGetResp = []byte(mockTaskExecPortScanCompleteRespString)
    err = handler.performTask(mockSession1.Id, task)
    if err != nil {
        t.Error(err)
    }
}


func TestPerformTaskFileTransferError(t *testing.T) {
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 2,
        FileTransferSource: "test_src",
        FileTransferDest: "test_dst",
    }
    handler := generateMockedQuasarHandler()
    mockHttpPostResp = []byte(mockTransferErrorRespString)
    mockHttpGetResp = []byte(mockTransferErrorRespString)
    err := handler.performTask("testid", &task)
    errWant := "File transfer 356532246 terminated due to an error: Dummy error"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
}

func TestPerformTaskFileTransferCanceled(t *testing.T) {
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 3,
        FileTransferSource: "test_src",
        FileTransferDest: "test_dst",
    }
    handler := generateMockedQuasarHandler()
    mockHttpPostResp = []byte(mockTransferCanceledRespString)
    mockHttpGetResp = []byte(mockTransferCanceledRespString)
    err := handler.performTask("testid", &task)
    errWant := "File transfer 356532246 was canceled"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
}

func TestPerformTaskFileTransferTimedOut(t *testing.T) {
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 3,
        FileTransferSource: "test_src",
        FileTransferDest: "test_dst",
    }
    handler := generateMockedQuasarHandler()
    mockHttpPostResp = []byte(mockTransferPendingRespString)
    mockHttpGetResp = []byte(mockTransferPendingRespString)
    err := handler.performTask("testid", &task)
    errWant := "Timed out while waiting for file transfer 356532246 to complete"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
}

func TestPerformTaskFileDownloadNoSrc(t *testing.T) {
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 2,
        FileTransferDest: "test_dst",
    }
    handler := generateMockedQuasarHandler()
    err := handler.performTask("testid", &task)
    errWant := "File download task requires a source file. None provided"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
}

func TestPerformTaskFileUploadNoSrc(t *testing.T) {
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 3,
        FileTransferDest: "test_dst",
    }
    handler := generateMockedQuasarHandler()
    err := handler.performTask("testid", &task)
    errWant := "File upload task requires a source file. None provided"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
}

func TestPerformTaskFileDownloadSuccess(t *testing.T) {
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 2,
        FileTransferSource: "test_src",
        FileTransferDest: "test_dst",
    }
    handler := generateMockedQuasarHandler()
    mockHttpPostResp = []byte(mockTransferPendingRespString)
    mockHttpGetResp = []byte(mockTransferSuccessRespString)
    err := handler.performTask("testid", &task)
    if err != nil {
        t.Error(err)
    }
}

func TestPerformTaskFileUploadSuccess(t *testing.T) {
    // set current working directory to main repo directory to access ./files
    cwd, _ := os.Getwd()
    os.Chdir("../../")
    defer os.Chdir(cwd) // restore cwd at end of test
    
    startRESTAPI(t)
    defer stopRESTAPI(t)

    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 3,
        FileTransferSource: "test_src",
        FileTransferDest: "test_dst",
    }
    handler := generatedMockedQuasarHandlerCustomFunc(MockPerformHttpGetSelective, MockPerformHttpPost)
    mockHttpPostResp = []byte(mockTransferPendingRespString)
    err := handler.performTask("testid", &task)
    if err != nil {
        t.Error(err)
    }
    
    // Check for uploaded file. local path is server-side path for uploads.
    compareUploadedFile(t, "test_src", dummyFileContentsHash)
}

func TestPerformKeylogUploadError(t *testing.T) {
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 4,
    }
    handler := generateMockedQuasarHandler()
    mockHttpPostResp = []byte(mockKeylogUploadPendingRespString)
    mockHttpGetResp = []byte(mockKeylogUploadErrorRespString)
    err := handler.performTask("testid", &task)
    errWant := "Keystroke log upload task fb76942c-9f28-4c0d-ab5a-7b4b5858fdb9 terminated due to an error: Dummy error"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
}

func TestPerformKeylogUploadTimedOut(t *testing.T) {
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 4,
    }
    handler := generateMockedQuasarHandler()
    mockHttpPostResp = []byte(mockKeylogUploadPendingRespString)
    mockHttpGetResp = []byte(mockKeylogUploadPendingRespString)
    err := handler.performTask("testid", &task)
    errWant := "Failed to wait for keystroke log upload task completion for task ID fb76942c-9f28-4c0d-ab5a-7b4b5858fdb9: Timed out while waiting for keystroke log upload task fb76942c-9f28-4c0d-ab5a-7b4b5858fdb9 to complete"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
}

func TestKeylogUploadSuccessNoLogs(t *testing.T) {
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 4,
    }
    handler := generatedMockedQuasarHandlerCustomFunc(MockPerformHttpGetSelective, MockPerformHttpPost)
    mockHttpPostResp = []byte(mockKeylogUploadSuccessNoLogsRespString)
    err := handler.performTask("testid", &task)
    if err != nil {
        t.Error(err)
    }
}

func TestKeylogUploadSuccess2Logs(t *testing.T) {
    // set current working directory to main repo directory to access ./files
    cwd, _ := os.Getwd()
    os.Chdir("../../")
    defer os.Chdir(cwd) // restore cwd at end of test
    
    startRESTAPI(t)
    defer stopRESTAPI(t)
    
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 4,
    }
    handler := generatedMockedQuasarHandlerCustomFunc(MockPerformHttpGetSelective, MockPerformHttpPost)
    mockHttpPostResp = []byte(mockKeylogUploadPending2RespString)
    err := handler.performTask("testid", &task)
    if err != nil {
        t.Error(err)
    }
    
    // Check for uploaded files
    compareUploadedFile(t, "testid_test_src", dummyFileContentsHash)
    compareUploadedFile(t, "testid_test_src2", dummyFileContents2Hash)
}

func TestKeylogUploadSuccess2LogsFailRetrieve1(t *testing.T) {
    // set current working directory to main repo directory to access ./files
    cwd, _ := os.Getwd()
    os.Chdir("../../")
    defer os.Chdir(cwd) // restore cwd at end of test
    
    startRESTAPI(t)
    defer stopRESTAPI(t)
    
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 4,
    }
    handler := generatedMockedQuasarHandlerCustomFunc(MockPerformHttpGetSelective, MockPerformHttpPost)
    mockHttpPostResp = []byte(mockKeylogUploadPending3RespString)
    err := handler.performTask("testid", &task)
    errWant := "Keystroke log upload transfer ID 109357389 not actually complete - cannot fetch uploaded file.\n"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
    
    // Check for uploaded files
    compareUploadedFile(t, "testid_test_src", dummyFileContentsHash)
}

func TestKeylogUploadPartialSuccess(t *testing.T) {
    // set current working directory to main repo directory to access ./files
    cwd, _ := os.Getwd()
    os.Chdir("../../")
    defer os.Chdir(cwd) // restore cwd at end of test
    
    startRESTAPI(t)
    defer stopRESTAPI(t)
    
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 4,
    }
    handler := generatedMockedQuasarHandlerCustomFunc(MockPerformHttpGetSelective, MockPerformHttpPost)
    mockHttpPostResp = []byte(mockKeylogUploadPendingPartialRespString)
    err := handler.performTask("testid", &task)
    errWant := "Keystroke log upload task e6d7828a-2ab7-434b-9956-fd4d47b9cdf2 partially succeeded: Dummy partial error"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
    
    // Check for uploaded files
    compareUploadedFile(t, "testid_test_src", dummyFileContentsHash)
}

func TestKeylogUploadPartialSuccessWithErr(t *testing.T) {
    // set current working directory to main repo directory to access ./files
    cwd, _ := os.Getwd()
    os.Chdir("../../")
    defer os.Chdir(cwd) // restore cwd at end of test
    
    startRESTAPI(t)
    defer stopRESTAPI(t)
    
    task := QuasarEvalsTask{
        TaskNum: 1,
        TaskType: 4,
    }
    handler := generatedMockedQuasarHandlerCustomFunc(MockPerformHttpGetSelective, MockPerformHttpPost)
    mockHttpPostResp = []byte(mockKeylogUploadPendingPartial2RespString)
    err := handler.performTask("testid", &task)
    errWant := "Keystroke log upload task fb5d9143-6e15-44d6-92b2-1f21e4bf95e2 partially succeeded: Dummy partial error\n\tAdditional errors: Keystroke log upload transfer ID 109357389 not actually complete - cannot fetch uploaded file.\n"
    if err == nil {
        t.Errorf("Expected error, got nil")
    } else {
        if err.Error() != errWant {
            t.Errorf("Expected %v, got %v", errWant, err.Error())
        }
    }
    
    // Check for uploaded files
    compareUploadedFile(t, "testid_test_src", dummyFileContentsHash)
}

func TestGetTaskPollCount(t *testing.T) {
    got := getTaskPollCount(-1)
    want := 12
    if got != want {
        t.Errorf("Expected %d, got %d", want, got)
    }
    
    got = getTaskPollCount(0)
    want = 12
    if got != want {
        t.Errorf("Expected %d, got %d", want, got)
    }
    
    got = getTaskPollCount(4)
    want = 1
    if got != want {
        t.Errorf("Expected %d, got %d", want, got)
    }
    
    got = getTaskPollCount(150)
    want = 15
    if got != want {
        t.Errorf("Expected %d, got %d", want, got)
    }
    
    got = getTaskPollCount(1999)
    want = 200
    if got != want {
        t.Errorf("Expected %d, got %d", want, got)
    }
}

func TestValidateCidr(t *testing.T) {
    err := validateCidr("invalidinput")
    want := "Invalid format: invalidinput"
    if err == nil {
        t.Error("Expected error, got none")
    } else {
        if err.Error() != want {
            t.Errorf("Expected error message %s, got %s", want, err.Error())
        }
    }
    
    // too many octects
    err = validateCidr("1.2.3.4.5")
    want = "Invalid format: 1.2.3.4.5"
    if err == nil {
        t.Error("Expected error, got none")
    } else {
        if err.Error() != want {
            t.Errorf("Expected error message %s, got %s", want, err.Error())
        }
    }
    
    // no mask
    err = validateCidr("1.2.3.4")
    want = "Invalid format: 1.2.3.4"
    if err == nil {
        t.Error("Expected error, got none")
    } else {
        if err.Error() != want {
            t.Errorf("Expected error message %s, got %s", want, err.Error())
        }
    }
    
    // bad octets
    err = validateCidr("a.1.2.3/24")
    want = "Invalid format: a.1.2.3/24"
    if err == nil {
        t.Error("Expected error, got none")
    } else {
        if err.Error() != want {
            t.Errorf("Expected error message %s, got %s", want, err.Error())
        }
    }
    
    // octet too large
    err = validateCidr("300.1.2.3/24")
    want = "IP address octet too large: 300"
    if err == nil {
        t.Error("Expected error, got none")
    } else {
        if err.Error() != want {
            t.Errorf("Expected error message %s, got %s", want, err.Error())
        }
    }
    
    err = validateCidr("1.1.2.256/24")
    want = "IP address octet too large: 256"
    if err == nil {
        t.Error("Expected error, got none")
    } else {
        if err.Error() != want {
            t.Errorf("Expected error message %s, got %s", want, err.Error())
        }
    }
    
    // mask too small
    err = validateCidr("10.1.2.0/0")
    want = "Invalid CIDR mask: 0"
    if err == nil {
        t.Error("Expected error, got none")
    } else {
        if err.Error() != want {
            t.Errorf("Expected error message %s, got %s", want, err.Error())
        }
    }
    
    // mask too big
    err = validateCidr("10.1.2.0/33")
    want = "Invalid CIDR mask: 33"
    if err == nil {
        t.Error("Expected error, got none")
    } else {
        if err.Error() != want {
            t.Errorf("Expected error message %s, got %s", want, err.Error())
        }
    }
    
    // good
    err = validateCidr("10.1.2.0/24")
    if err != nil {
        t.Error(err)
    }
}

func compareUploadedFile(t *testing.T, targetName string, targetHash string) {
    uploadedFile := "./files/" + targetName
    defer cleanupFile(t, uploadedFile)
    uploadedData, err := ioutil.ReadFile(uploadedFile)
    if err != nil {
        t.Error(err)
    }
    
    h := md5.Sum(uploadedData)
    actualHash := hex.EncodeToString(h[:])
    if targetHash != actualHash {
        t.Errorf("Expected %v, got %v", targetHash, actualHash)
    }
}

func cleanupFile(t *testing.T, uploadedFile string) {
    // clean up test file
    err := os.Remove(uploadedFile)
    if err != nil {
        t.Error(err)
    }
}

func setTask(task string, guid string) (string, error) {
    url := restAPIBaseURL + "session/" + guid + "/task"
    
    // setup HTTP POST request
    req, err := http.NewRequest("POST", url, bytes.NewBufferString(task))
    if err != nil {
        return "", err
    }

    // execute HTTP POST and read response
    client := &http.Client{}
    response, err := client.Do(req)
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

