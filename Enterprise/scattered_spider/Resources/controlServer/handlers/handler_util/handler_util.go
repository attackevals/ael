package handler_util

import (
    "bytes"
    "encoding/json"
    "errors"
    "fmt"
    "io/ioutil"
    "math/rand"
    "net/http"
    "strconv"
    "strings"
    "time"

    "evals.mitre.org/control_server/restapi/restapi_util"
    "evals.mitre.org/control_server/util/http_util"
)

// Function wrapper to get a random alphanumeric string of specified length
type RandAlphanumericStringGetter func(int) string

// For generating random strings
const ALPHANUMERIC_POOL = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

func init() {
    rand.Seed(time.Now().UnixNano())
}

// Sets the given task for the given implant. Returns the REST server response and/or error.
func SetTask(restApiBindAddr string, task string, implantId string) (string, error) {
    url := fmt.Sprintf("http://%s/api/v1.0/session/%s/task", restApiBindAddr, implantId)
    resp, err := http_util.PerformHttpPostAssertSuccess(url, "", nil, []byte(task))
    return string(resp), err
}

func ForwardTaskOutput(restAPIaddress string, uuid string, data []byte) (string, error) {
    url := "http://" + restAPIaddress + "/api/v1.0/session/" + uuid + "/task/output"

    // initialize HTTP request
    req, err := http.NewRequest("POST", url, bytes.NewBuffer(data))
    if err != nil {
        return "", err
    }
    req.Header.Set("Content-Type", "application/json")

    // execute HTTP POST request and read response
    client := &http.Client{}
    response, err := client.Do(req)
    if err != nil {
        return "", err
    }
    defer response.Body.Close()
    if response.StatusCode != 200 {
        return "", errors.New(fmt.Sprintf("Expected 200 HTTP response code, received %d", response.StatusCode))
    }
    return restapi_util.ExtractRestApiStringResponsedData(response)
}

// Forwards Implant Beacon to RESTAPI, which will forward session struct to CALDERA
func ForwardImplantBeacon(uuid string, restAPIaddress string) (string, error) {
    url := "http://" + restAPIaddress + "/api/v1.0/forwarder/session/" + uuid

    // initialize HTTP request
    req, err := http.NewRequest("POST", url, bytes.NewBufferString(""))
    if err != nil {
        return "", err
    }
    req.Header.Set("Content-Type", "application/json")

    // execute HTTP POST request and read response
    client := &http.Client{}
    response, err := client.Do(req)
    if err != nil {
        return "", err
    }
    defer response.Body.Close()
    if response.StatusCode != 200 {
        return "", errors.New(fmt.Sprintf("Received non-200 HTTP status code when forwarding implant beacon with ID %s: %d", uuid, response.StatusCode))
    }
    return restapi_util.ExtractRestApiStringResponsedData(response)
}

// Forward the request to the evals REST API server to register our implant
func ForwardRegisterImplant(restAPIaddress string, implantData []byte) (string, error) {
    url := "http://" + restAPIaddress + "/api/v1.0/session"

    // initialize HTTP request
    req, err := http.NewRequest("POST", url, bytes.NewBuffer(implantData))
    if err != nil {
        return "", err
    }
    req.Header.Set("Content-Type", "application/json")

    // execute HTTP POST request and read response
    client := &http.Client{}
    response, err := client.Do(req)
    if err != nil {
        return "", err
    }
    defer response.Body.Close()
    if response.StatusCode != 200 {
        return "", err
    }
    return restapi_util.ExtractRestApiStringResponsedData(response)
}

// Query the evals c2 server REST API for tasks for the implant with the specified GUID.
func ForwardGetTask(restAPIaddress string, guid string) (string, error) {
    url := "http://" + restAPIaddress + "/api/v1.0/session/" + guid + "/task"
    resp, err := http.Get(url)
    if err != nil {
        return "", err
    }
    defer resp.Body.Close()
    sessionData, err := ioutil.ReadAll(resp.Body)
    if err != nil {
        return "", err
    }
    return string(sessionData), err
}

// Forward the request to the evals REST API server to remove the implant
func ForwardRemoveImplant(restAPIaddress string, uuid string) error {
    url := "http://" + restAPIaddress + "/api/v1.0/session/delete/" + uuid

    // initialize HTTP(S) request
    req, err := http.NewRequest("DELETE", url, bytes.NewBuffer([]byte{}))
    if err != nil {
        return err
    }

    // execute HTTP(S) DELETE request
    client := &http.Client{}
    _, err = client.Do(req)
    if err != nil {
        return err
    }
    return nil
}

// Fetches the file from the control server. Returns the file bytes or an error if the file could not be retrieved or does not exist.
func ForwardGetFileFromServer(restAPIaddress, handlerName, fileName string) ([]byte, error) {
    url := "http://" + restAPIaddress + "/api/v1.0/files/" + handlerName + "/" + fileName
    resp, err := http.Get(url)
    var filedata []byte
    if err != nil {
        return filedata, err
    }
    if resp.StatusCode != 200 {
        return filedata, errors.New("server did not return requested file: " + fileName)
    }
    filedata, err = ioutil.ReadAll(resp.Body)
    if err != nil {
        return filedata, err
    }
    return filedata, nil
}

// Forward the request to the evals REST API server to update the implant session
func ForwardUpdateSession(restAPIaddress string, guid string, sessionData []byte) (string, error) {
    url := "http://" + restAPIaddress + "/api/v1.0/session/update/" + guid

    // initialize HTTP request
    req, err := http.NewRequest("POST", url, bytes.NewBuffer(sessionData))
    if err != nil {
        return "", err
    }
    req.Header.Set("Content-Type", "application/json")

    // execute HTTP POST request and read response
    client := &http.Client{}
    response, err := client.Do(req)
    if err != nil {
        return "", err
    }
    defer response.Body.Close()
    if response.StatusCode != 200 {
        return "", err
    }
    return restapi_util.ExtractRestApiStringResponsedData(response)
}

// Forward the request to the evals REST API server to check if the session exists
func ForwardSessionExists(restAPIaddress string, guid string) (bool, error) {
    url := "http://" + restAPIaddress + "/api/v1.0/session/exists/" + guid
    resp, err := http.Get(url)
    if err != nil {
        return false, err
    }
    defer resp.Body.Close()
    respBody, err := ioutil.ReadAll(resp.Body)
    if err != nil {
        return false, err
    }
    var data map[string]interface{}
    err = json.Unmarshal([]byte(respBody), &data)
    if err != nil {
        return false, err
    }
    sessionExists, err := strconv.ParseBool(data["data"].(string))
    if err != nil {
        return false, err
    }
    return sessionExists, nil
}

// Process file upload and forward it to the REST API server.
func ForwardFileUpload(restAPIaddress string, fileName string, data []byte) (string, error) {
    url := "http://" + restAPIaddress + "/api/v1.0/upload/" + fileName

    resp, err := http.Post(url, "application/octet-stream", bytes.NewBuffer(data))
    if err != nil {
        return "", err
    }
    defer resp.Body.Close()
    return restapi_util.ExtractRestApiStringResponsedData(resp)
}

// PRODUCTION IMPLEMENTATION OF WRAPPED FUNCTIONS

func GetRandomAlphanumericString(length int) string {
    buffer := make([]byte, length)
    for i := range buffer {
        buffer[i] = ALPHANUMERIC_POOL[rand.Intn(len(ALPHANUMERIC_POOL))]
    }
    return string(buffer)
}

// MOCK IMPLEMENTATION OF WRAPPED FUNCTIONS

// Return the letter "a" repeated length times
func MockGetRandomAlphanumericString(length int) string {
    return strings.Repeat("a", length)
}
