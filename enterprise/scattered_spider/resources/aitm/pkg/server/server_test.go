package server

import (
    "fmt"
    "net/http"
    "path/filepath"
    "testing"

    "aitm/pkg/authentik_util"
    "aitm/pkg/test_util"
    "aitm/pkg/test_util/assert_util"
    "aitm/pkg/test_util/mock_authentik"
)

const (
    AITM_BIND_ADDR = "127.0.0.1:18080"
    AITM_BASE_URL = "http://" + AITM_BIND_ADDR
    LOGIN_URL = AITM_BASE_URL + authentik_util.MAIN_FLOW_URL
    AUTH_URL = AITM_BASE_URL + authentik_util.AUTH_API_URL
)

var (
    MOCK_LOGIN_REQ_IDENTIFICATION = &authentik_util.LoginRequest{
        Stage: authentik_util.AUTH_STAGE_IDENTIFICATION,
        Username: mock_authentik.MOCK_USERNAME,
    }
    RESOURCES_FILE_MAPPING = map[string]string{
        "/if/flow/default-authentication-flow/": filepath.Join("..", "..", "static", "auth.html"),
        "/if/flow/default-authentication-flow": filepath.Join("..", "..", "static", "auth.html"),
        "/flows/cancel/": filepath.Join("..", "..", "static", "auth.html"),
        "/flows/cancel": filepath.Join("..", "..", "static", "auth.html"),
        "/static/authentik.css": filepath.Join("..", "..", "static", "authentik.css"),
        "/static/index-2024.8.2.js": filepath.Join("..", "..", "static", "index-2024.8.2.js"),
        "/static/icons/icon_left_brand.svg": filepath.Join("..", "..", "static", "icons", "/icon_left_brand.svg"),
        "/static/icons/icon.png": filepath.Join("..", "..", "static", "icons", "/icon.png"),
        "/static/images/flow_background.jpg": filepath.Join("..", "..", "static", "images", "flow_background.jpg"),
    }
)

// generate mock AITM server
func generateMockAITMServer() *AITMServer {
    return AITMServerFactory(AITM_BIND_ADDR, filepath.Join("..", "..", "static"), "http://localhost:59999", "", "")
}

// generate mock HTTPS AITM server
func generateMockAITMServerHTTPS() *AITMServer {
    certFilesDir := filepath.Join("..", "..", "test_certs")
    return AITMServerFactory(
        AITM_BIND_ADDR,
        filepath.Join("..", "..", "static"),
        "http://localhost:59999",
        filepath.Join(certFilesDir, "server_test.crt"),
        filepath.Join(certFilesDir, "private_test.key"),
    )
}

func TestServeResources(t *testing.T) {
    a := generateMockAITMServer()
    test_util.StartServerForUnitTest(t, a)
    defer test_util.StopServerForUnitTest(t, a)

    for uri, filePath := range RESOURCES_FILE_MAPPING {
        test_util.RequestAndVerifyFile(t, fmt.Sprintf("http://%s%s", AITM_BIND_ADDR, uri), filePath)
    }
}

func TestServeResourcesHTTPS(t *testing.T) {
    a := generateMockAITMServerHTTPS()
    test_util.StartServerForUnitTest(t, a)
    defer test_util.StopServerForUnitTest(t, a)

    for uri, filePath := range RESOURCES_FILE_MAPPING {
        test_util.RequestAndVerifyFile(t, fmt.Sprintf("https://%s%s", AITM_BIND_ADDR, uri), filePath)
    }
}

func TestGetAuthentikCookies(t *testing.T) {
    aitm := generateMockAITMServer()
    authentik := mock_authentik.GenerateMockAuthentikServer()
    test_util.StartServerForUnitTest(t, aitm)
    defer test_util.StopServerForUnitTest(t, aitm)
    test_util.StartServerForUnitTest(t, authentik)
    defer test_util.StopServerForUnitTest(t, authentik)

    attemptCookie, err := authentik_util.GetAuthAttemptCookie(aitm.targetServerBaseUrl, MOCK_LOGIN_REQ_IDENTIFICATION.Username, nil)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, attemptCookie, mock_authentik.MOCK_AUTH_ATTEMPT_COOKIE_VAL, "")
}

func TestSuccessLoginFlow(t *testing.T) {
    aitm := generateMockAITMServer()
    authentik := mock_authentik.GenerateMockAuthentikServer()
    test_util.StartServerForUnitTest(t, aitm)
    defer test_util.StopServerForUnitTest(t, aitm)
    test_util.StartServerForUnitTest(t, authentik)
    defer test_util.StopServerForUnitTest(t, authentik)

    identificationData := []byte(`{"component": "ak-stage-identification", "uid_field": "dummyuser"}`)
    expectedResp := fmt.Sprintf(mock_authentik.AUTH_STAGE_ID_RESP_TEMPLATE, mock_authentik.MOCK_USERNAME)
    test_util.SendPostAndCheckResponse(t, AUTH_URL, "application/json", "", nil, identificationData, expectedResp, http.StatusOK)

    passwordData := []byte(`{"component": "ak-stage-password", "uid_field": "dummyuser", "password": "dummypassword"}`)
    expectedResp = fmt.Sprintf(mock_authentik.AUTH_STAGE_PASSWORD_RESP_SUCCESS_TEMPLATE, mock_authentik.MOCK_USERNAME)
    test_util.SendPostAndCheckResponse(t, AUTH_URL, "application/json", "", nil, passwordData, expectedResp, http.StatusOK)

    tokenData := []byte(`{"component": "ak-stage-authenticator-validate", "uid_field": "dummyuser", "code": "123456"}`)
    test_util.SendPostAndCheckResponse(t, AUTH_URL, "application/json", "", nil, tokenData, mock_authentik.AUTH_STAGE_MFA_RESP_SUCCESS, http.StatusOK)
}

func TestLoginFlowBadPassword(t *testing.T) {
    aitm := generateMockAITMServer()
    authentik := mock_authentik.GenerateMockAuthentikServer()
    test_util.StartServerForUnitTest(t, aitm)
    defer test_util.StopServerForUnitTest(t, aitm)
    test_util.StartServerForUnitTest(t, authentik)
    defer test_util.StopServerForUnitTest(t, authentik)

    identificationData := []byte(`{"component": "ak-stage-identification", "uid_field": "dummyuser"}`)
    expectedResp := fmt.Sprintf(mock_authentik.AUTH_STAGE_ID_RESP_TEMPLATE, mock_authentik.MOCK_USERNAME)
    test_util.SendPostAndCheckResponse(t, AUTH_URL, "application/json", "", nil, identificationData, expectedResp, http.StatusOK)

    passwordData := []byte(`{"component": "ak-stage-password", "uid_field": "dummyuser", "password": "badpassword"}`)
    expectedResp = fmt.Sprintf(mock_authentik.AUTH_STAGE_PASSWORD_RESP_FAILURE_TEMPLATE, mock_authentik.MOCK_USERNAME)
    test_util.SendPostAndCheckResponse(t, AUTH_URL, "application/json", "", nil, passwordData, expectedResp, http.StatusUnauthorized)
}

func TestLoginFlowBadToken(t *testing.T) {
    aitm := generateMockAITMServer()
    authentik := mock_authentik.GenerateMockAuthentikServer()
    test_util.StartServerForUnitTest(t, aitm)
    defer test_util.StopServerForUnitTest(t, aitm)
    test_util.StartServerForUnitTest(t, authentik)
    defer test_util.StopServerForUnitTest(t, authentik)

    identificationData := []byte(`{"component": "ak-stage-identification", "uid_field": "dummyuser"}`)
    expectedResp := fmt.Sprintf(mock_authentik.AUTH_STAGE_ID_RESP_TEMPLATE, mock_authentik.MOCK_USERNAME)
    test_util.SendPostAndCheckResponse(t, AUTH_URL, "application/json", "", nil, identificationData, expectedResp, http.StatusOK)

    passwordData := []byte(`{"component": "ak-stage-password", "uid_field": "dummyuser", "password": "dummypassword"}`)
    expectedResp = fmt.Sprintf(mock_authentik.AUTH_STAGE_PASSWORD_RESP_SUCCESS_TEMPLATE, mock_authentik.MOCK_USERNAME)
    test_util.SendPostAndCheckResponse(t, AUTH_URL, "application/json", "", nil, passwordData, expectedResp, http.StatusOK)

    tokenData := []byte(`{"component": "ak-stage-authenticator-validate", "uid_field": "dummyuser", "code": "99999"}`)
    expectedResp = fmt.Sprintf(mock_authentik.AUTH_STAGE_MFA_RESP_FAILURE_TEMPLATE, mock_authentik.MOCK_USERNAME)
    test_util.SendPostAndCheckResponse(t, AUTH_URL, "application/json", "", nil, tokenData, expectedResp, http.StatusUnauthorized)
}
