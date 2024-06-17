package util

import (
    "bytes"
    "encoding/json"
    "errors"
    "fmt"
    "io/ioutil"
    "net/http"

    "attackevals.mitre-engenuity.org/control_server/config"
    restapi_util "attackevals.mitre-engenuity.org/control_server/restapi/util"
)

// The Handler interface provides methods that all C2 handlers must implement.
type Handler interface {
    // Starts the handler given the rest API address string and configuration map.
    StartHandler(string, config.HandlerConfigEntry) error

    // Stops the given handler.
    StopHandler() error
}

// Contains the available C2 handler implementations. These will be populated as from the init() functions in each handler subpackage.
var AvailableHandlers map[string]Handler = make(map[string]Handler)

// Contains running handler implementations.
var RunningHandlers map[string]Handler = make(map[string]Handler)

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
    return ExtractRestApiStringResponsedData(response)
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
    return ExtractRestApiStringResponsedData(response)
}

// Extracts string response data from REST API string response. Assumes caller will close response body
func ExtractRestApiStringResponsedData(resp *http.Response) (string, error) {
    body, err := ioutil.ReadAll(resp.Body)
    if err != nil {
        return "", err
    }
    // parse out message from REST API
    return ExtractRestApiStrRespDataFromBytes(body)
}

func ExtractRestApiStrRespDataFromBytes(respBytes []byte) (string, error) {
    var apiResponse restapi_util.ApiStringResponse
    err := json.Unmarshal(respBytes, &apiResponse)
    if err != nil {
        return "", err
    }
    return apiResponse.Data, nil
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
    return ExtractRestApiStringResponsedData(response)
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
func ForwardGetFileFromServer(restAPIaddress string, fileName string) ([]byte, error) {
    url := "http://" + restAPIaddress + "/api/v1.0/files/" + fileName
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
    return ExtractRestApiStringResponsedData(response)
}

// Process file upload and forward it to the REST API server.
func ForwardFileUpload(restAPIaddress string, fileName string, data []byte) (string, error) {
    url := "http://" + restAPIaddress + "/api/v1.0/upload/" + fileName

    resp, err := http.Post(url, "application/octet-stream", bytes.NewBuffer(data))
    if err != nil {
        return "", err
    }
    defer resp.Body.Close()
    return ExtractRestApiStringResponsedData(resp)
}
