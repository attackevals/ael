package quasar

import (
    "encoding/json"
    "strings"
)

const (
    TASK_TYPE_EXEC_PROC             = 1
    TASK_TYPE_FILE_DOWNLOAD         = 2
    TASK_TYPE_FILE_UPLOAD           = 3
    TASK_TYPE_KEYSTROKE_LOG_UPLOAD  = 4
    TASK_TYPE_PORT_SCAN             = 5
)

type QuasarSession struct {
    Id          string  `json:"id"`
    Username    string  `json:"username"`
    Hostname    string  `json:"hostname"`
    IpAddress   string  `json:"ip_addr"`
    Os          string  `json:"os"`
    AccountType string  `json:"account_type"`
    Tag         string  `json:"tag"`
    CountryCode string  `json:"country_code"`
    HardwareId  string  `json:"hardware_id"`
}

// Evals c2 server representation of quasar task
type QuasarEvalsTask struct {
    TaskNum             int     `json:"seq"`
    TaskType            int     `json:"type"`
    Timeout             int     `json:"timeout"`
    
    // For file transfers
    FileTransferSource  string  `json:"transfer_src"`
    FileTransferDest    string  `json:"transfer_dst"`
    
    // For process execution
    ProcPath            string  `json:"proc_path"`
    ProcArgs            string  `json:"proc_args"`
    DownloadUrl         string  `json:"download_url"`
    DownloadDst         string  `json:"download_dst"`
    ShellExec           bool    `json:"use_shell"`
    GetOutput           bool    `json:"get_output"`
    NoWindow            bool    `json:"no_window"`
    
    // For port scan
    TargetRange         string  `json:"range"`
    TargetPorts         []int   `json:"ports"`
}

// Returns bytes representing JSON dict containing Quasar session information
func createNewSessionDataBytes(session *QuasarSession) ([]byte, error) {
    sessionData := make(map[string]string)
    sessionData["guid"] = session.Id
    sessionData["ipAddr"] = session.IpAddress
    sessionData["hostName"] = session.Hostname
    sessionData["user"] = session.Username
    
    jsonStr, err := json.Marshal(sessionData)
    if err != nil {
        return nil, err
    }
    return []byte(jsonStr), nil
}

func (q *QuasarSession) IsElevated() bool {
    return strings.ToLower(q.AccountType) == "admin"
}

func (q* QuasarHandler) setSessionTaskingStatusFree(guid string) {
    q.sessionBusyStatusMutex.Lock()
    defer q.sessionBusyStatusMutex.Unlock()
    q.sessionBusyStatusMap[guid] = false
}

func (q* QuasarHandler) setSessionTaskingStatusBusy(guid string) {
    q.sessionBusyStatusMutex.Lock()
    defer q.sessionBusyStatusMutex.Unlock()
    q.sessionBusyStatusMap[guid] = true
}

func (q* QuasarHandler) isSessionBusy(guid string) bool {
    q.sessionBusyStatusMutex.Lock()
    defer q.sessionBusyStatusMutex.Unlock()
    ok, busy := q.sessionBusyStatusMap[guid]
    if !ok {
        return false
    }
    return busy
}

func extractEvalsTaskStruct(taskStr string) (*QuasarEvalsTask, error) {
    extracted := new(QuasarEvalsTask)
    // Set default values
    extracted.ShellExec = false
    extracted.GetOutput = true
    extracted.NoWindow = true
    
    err := json.Unmarshal([]byte(taskStr), extracted)
    if err != nil {
        return nil, err
    }
    return extracted, nil
}
