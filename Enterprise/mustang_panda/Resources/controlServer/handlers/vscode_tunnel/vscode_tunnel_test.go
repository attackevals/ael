package vscode_tunnel

import (
    "net/http"
    "testing"

    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/restapi/restapi_test_util"
    "evals.mitre.org/control_server/util/test_util"
    "evals.mitre.org/control_server/util/test_util/assert_util"
)

const (
    REST_API_LISTEN_HOST = "127.0.0.1:10010"
    BASE_POST_URL        = "http://127.0.0.1:10011/code"
    TEST_SUBMISSION      = `*
* Visual Studio Code Server
*
* By using the software, you agree to
* the Visual Studio Code Server License Terms (https://aka.ms/vscode-server-license) and
* the Microsoft Privacy Statement (https://privacy.microsoft.com/en-US/privacystatement).
*
[2025-04-14 19:44:32] info Using Github for authentication, run ` + "`code tunnel user login --provider <provider>`" + ` option to change this.
To grant access to the server, please log into https://github.com/login/device and use code 80B6-1539`
    BAD_SUBMISSION       = `*
* Visual Studio Code Server
*
* By using the software, you agree to
* the Visual Studio Code Server License Terms (https://aka.ms/vscode-server-license) and
* the Microsoft Privacy Statement (https://privacy.microsoft.com/en-US/privacystatement).
*
[2025-04-14 19:44:32] info Using Github for authentication, run ` + "`code tunnel user login --provider <provider>`" + ` option to change this.
To grant access to the server, please log into https://github.com/login/device and use code XXXX`
)

var testConfigEntry = config.HandlerConfigEntry{
    "host": "127.0.0.1",
    "port": 10011,
}

// Starts the RestAPI, generates the handler, and returns teardown function
func setupTest(t *testing.T) (*VSCodeTunnelHandler, func(t *testing.T)) {
    // Set up REST API server
    restapi_test_util.StartRestApiServer(t, "", REST_API_LISTEN_HOST, nil)

    // create and start mock handler
    handler := vSCodeTunnelHandlerFactory()
    test_util.StartHandlerForUnitTest(t, handler, REST_API_LISTEN_HOST, testConfigEntry)

    return handler, func(t *testing.T) {
        restapi_test_util.StopRestApiServer()
        test_util.StopHandlerForUnitTest(t, handler)
    }
}

// Test parsing the string
func TestExtractCode(t *testing.T) {
    assert_util.AssertEq(t, extractCode(TEST_SUBMISSION), "80B6-1539", "")
    assert_util.AssertEq(t, extractCode(BAD_SUBMISSION), "", "")
}

// Test posting code
func TestHandleCodeSubmission(t *testing.T) {
    _, teardownTest := setupTest(t)
    defer teardownTest(t)

    want := "OK"
    test_util.SendPostAndCheckResponse(t, BASE_POST_URL, "", nil, []byte(TEST_SUBMISSION), want, http.StatusOK)

    want = "Internal server error\n"
    test_util.SendPostAndCheckResponse(t, BASE_POST_URL, "", nil, []byte(BAD_SUBMISSION), want, http.StatusInternalServerError)
}
