package test_util

import (
    "context"
    "crypto/md5"
    "encoding/hex"
    "encoding/json"
    "fmt"
    "io/ioutil"
    "net/http"
    "os"
    "path/filepath"
    "strings"
    "testing"
    "time"

    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/handlers/handler_manager"
    "evals.mitre.org/control_server/logger"
    "evals.mitre.org/control_server/sessions"
    "evals.mitre.org/control_server/util"
    "evals.mitre.org/control_server/util/http_util"
    "evals.mitre.org/control_server/util/test_util/assert_util"

    "github.com/gorilla/mux"
)

var MockCalderaServer *http.Server

// Starts a given handler using the provided REST API bind address and handler configuration entry.
// Be sure to defer StopHandlerForUnitTest in the same unit test after calling this function.
func StartHandlerForUnitTest(t *testing.T, handler handler_manager.Handler, restApiBindAddr string, configEntry config.HandlerConfigEntry) {
    if err := handler.StartHandler(restApiBindAddr, configEntry); err != nil {
        t.Fatalf("Error when starting handler: %s", err.Error())
    }
    handler_manager.RunningHandlers[handler.GetName()] = handler
    time.Sleep(50 * time.Millisecond)
}

// Stops a given handler. Defer this function immediately after calling StartHandlerForUnitTest in any unit test
func StopHandlerForUnitTest(t *testing.T, handler handler_manager.Handler) {
    if err := handler.StopHandler(); err != nil {
        t.Fatalf("Error when stopping handler: %s", err.Error())
    }
    delete(handler_manager.RunningHandlers, handler.GetName())
    time.Sleep(50 * time.Millisecond)
}

// Sends an HTTP POST request to the given URL and checks for the expected response and status code
// Will terminate the calling test upon response/code mismatch or any errors
func SendPostAndCheckResponse(t *testing.T, url string, contentType string, extraHeaders map[string]string, data []byte, expectedResponse string, expectedStatusCode int) {
    resp, statusCode, err := http_util.PerformHttpPost(url, contentType, extraHeaders, data)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, statusCode, expectedStatusCode, "HTTP POST status code mismatch")
    assert_util.AssertEq(t, string(resp), expectedResponse, "HTTP POST response mismatch")
}

// Sends an HTTP PUT request to the given URL and checks for the expected response and status code
// Will terminate the calling test upon response/code mismatch or any errors
func SendPutAndCheckResponse(t *testing.T, url string, contentType string, extraHeaders map[string]string, data []byte, expectedResponse string, expectedStatusCode int) {
    resp, statusCode, err := http_util.PerformHttpPut(url, contentType, extraHeaders, data)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, statusCode, expectedStatusCode, "HTTP PUT status code mismatch")
    assert_util.AssertEq(t, string(resp), expectedResponse, "HTTP PUT response mismatch")
}

// Sends an HTTP GET request to the given URL and checks for the expected response and status code
// Will terminate the calling test upon response/code mismatch or any errors
func SendGetAndCheckResponse(t *testing.T, url string, extraHeaders map[string]string, expectedResponse string, expectedStatusCode int) {
    resp, statusCode, err := http_util.PerformHttpGet(url, extraHeaders)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, statusCode, expectedStatusCode, "HTTP GET status code mismatch")
    assert_util.AssertEq(t, string(resp), expectedResponse, "HTTP GET response mismatch")
}

// Sends an HTTP DELETE request to the given URL and checks for the expected response and status code
// Will terminate the calling test upon response/code mismatch or any errors
func SendDeleteAndCheckResponse(t *testing.T, url string, extraHeaders map[string]string, expectedResponse string, expectedStatusCode int) {
    resp, statusCode, err := http_util.PerformHttpDelete(url, extraHeaders)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, statusCode, expectedStatusCode, "HTTP DELETE status code mismatch")
    assert_util.AssertEq(t, string(resp), expectedResponse, "HTTP DELETE response mismatch")
}

// Calculates the MD5 hash for the provided buffer, compares it to the provided expected hash, and terminates the
// calling test upon hash mismatch
func CheckMD5Hash(t *testing.T, buf []byte, expectedHash string) {
    h := md5.Sum(buf)
    actualHash := hex.EncodeToString(h[:])
    assert_util.AssertEq(t, expectedHash, actualHash, "")
}

// Checks the hash of the specified uploaded file in the evalsC2Server/files directory
// Terminates the calling test upon hash mismatch or if the file is not found
func CheckUploadedFileHash(t *testing.T, fileName string, expectedHash string) {
    uploadedFilePath := filepath.Join(util.UploadDir, fileName)
    uploadedData, err := ioutil.ReadFile(uploadedFilePath)
    if err != nil {
        t.Fatal(err)
    }

    h := md5.Sum(uploadedData)
    actualHash := hex.EncodeToString(h[:])
    assert_util.AssertEq(t, expectedHash, actualHash, "")
}

// Deletes the specified file from the evalsC2Server/files uploads directory
func RemoveUploadedFile(t *testing.T, fileName string) {
    uploadedFilePath := filepath.Join(util.UploadDir, fileName)
    err := os.Remove(uploadedFilePath)
    if err != nil {
        t.Error(err)
    }
}

func HandleEmuBeacon(w http.ResponseWriter, r *http.Request) {
    req, err := ioutil.ReadAll(r.Body)
    if err != nil {
        fmt.Fprintf(w, "%v", err)
        return
    }

    // convert JSON data into a session struct
    var session sessions.Session
    err = json.Unmarshal(req, &session)
    if err != nil {
        fmt.Fprintf(w, "%v", err)
        return
    }
    fmt.Fprintf(w, "%v%v", "CALDERA server successfully received session: ", session.GUID)
}

func StartMockCalderaServer() {
    r := mux.NewRouter()
    r.HandleFunc("/plugins/emu/beacons", HandleEmuBeacon).Methods("POST")
    calderaAddress := "127.0.0.1:8888"
    MockCalderaServer = &http.Server{
        Addr:         calderaAddress,
        WriteTimeout: time.Second * 15,
        ReadTimeout:  time.Second * 15,
        IdleTimeout:  time.Second * 60,
        Handler:      r,
    }
    // start rest api in goroutine so it doesn't block
    go func() {
        err := MockCalderaServer.ListenAndServe()
        if err != nil && err.Error() != "http: Mock CALDERA Server closed" {

            if strings.Contains(err.Error(), fmt.Sprintf("Mock CALDERA listen tcp %s: bind: address already in use", calderaAddress)) {
                logger.Warning(err)
                return
            } else {
                logger.Error(err)
            }
        }
    }()
    time.Sleep(50 * time.Millisecond)
}

func StopMockCalderaServer() {
    time.Sleep(50 * time.Millisecond)
    emptyContext := context.Background()
    err := MockCalderaServer.Shutdown(emptyContext)
    if err != nil {
        logger.Error(fmt.Sprintf("Mock CALDERA server failed to shut down: %s", err.Error()))
    }
}
