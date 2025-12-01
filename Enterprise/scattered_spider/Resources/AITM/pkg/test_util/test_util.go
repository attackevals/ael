package test_util

import (
    "crypto/md5"
    "encoding/hex"
    "fmt"
    "net/http"
    "os"
    "testing"
    "time"

    "aitm/pkg/http_util"
    "aitm/pkg/test_util/assert_util"
)

func StartServerForUnitTest(t *testing.T, a http_util.IServer) {
    if err := a.StartServer(); err != nil {
        t.Fatalf("Error when starting server: %s", err.Error())
    }
    time.Sleep(50 * time.Millisecond)
}

func StopServerForUnitTest(t *testing.T, a http_util.IServer) {
    if err := a.StopServer(); err != nil {
        t.Fatalf("Error when stopping server: %s", err.Error())
    }
    time.Sleep(50 * time.Millisecond)
}

// Sends an HTTP POST request to the given URL and checks for the expected response and status code
// Will terminate the calling test upon response/code mismatch or any errors
func SendPostAndCheckResponse(t *testing.T, url string, contentType string, cookieStr string, extraHeaders map[string]string, data []byte, expectedResponse string, expectedStatusCode int) {
    resp, statusCode, _, err := http_util.PerformHttpPost(url, contentType, cookieStr, extraHeaders, data)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, statusCode, expectedStatusCode, "HTTP POST status code mismatch")
    assert_util.AssertEq(t, string(resp), expectedResponse, "HTTP POST response mismatch")
}

// Sends an HTTP GET request to the given URL and checks for the expected response and status code
// Will terminate the calling test upon response/code mismatch or any errors
func SendGetAndCheckResponse(t *testing.T, url string, cookieStr string, extraHeaders map[string]string, expectedResponse string, expectedStatusCode int) {
    resp, statusCode, _, err := http_util.PerformHttpGet(url, cookieStr, extraHeaders)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, statusCode, expectedStatusCode, "HTTP GET status code mismatch")
    assert_util.AssertEq(t, string(resp), expectedResponse, "HTTP GET response mismatch")
}

// Calculates the MD5 hash for the provided buffer, compares it to the provided expected hash, and terminates the
// calling test upon hash mismatch
func CheckMD5Hash(t *testing.T, buf []byte, expectedHash string) {
    h := md5.Sum(buf)
    actualHash := hex.EncodeToString(h[:])
    assert_util.AssertEq(t, expectedHash, actualHash, "")
}

// Requests the specified file from the URL and performs a hash comparison against the specified local file
func RequestAndVerifyFile(t *testing.T, url string, comparisonFilePath string) {
    // Get hash of comparison file
    fileData, err := os.ReadFile(comparisonFilePath)
    if err != nil {
        t.Fatal(err)
    }
    h := md5.Sum(fileData)
    expectedHash := hex.EncodeToString(h[:])

    // Download file and compare
    resp, statusCode, _, err := http_util.PerformHttpGet(url, "", nil)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, statusCode, http.StatusOK, fmt.Sprintf("Non-200 HTTP status code from GET request for %s", url))
    CheckMD5Hash(t, resp, expectedHash)
}
