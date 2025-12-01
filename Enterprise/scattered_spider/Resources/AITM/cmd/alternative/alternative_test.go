package main

import (
    "testing"

    "aitm/pkg/test_util"
    "aitm/pkg/test_util/assert_util"
    "aitm/pkg/test_util/mock_authentik"
)

const (
    MOCK_AUTH_BASE_ADDR = "http://" + mock_authentik.MOCK_AUTHENTIK_BIND_ADDR
)

func TestObtainSessionCookie(t *testing.T) {
    authentik := mock_authentik.GenerateMockAuthentikServer()
    test_util.StartServerForUnitTest(t, authentik)
    defer test_util.StopServerForUnitTest(t, authentik)

    err := ObtainSessionCookie(MOCK_AUTH_BASE_ADDR, mock_authentik.MOCK_USERNAME, mock_authentik.MOCK_PASSWORD, mock_authentik.MOCK_TOKEN)
    if err != nil {
        t.Fatal(err)
    }
}

func TestObtainSessionCookieBadCreds(t *testing.T) {
    authentik := mock_authentik.GenerateMockAuthentikServer()
    test_util.StartServerForUnitTest(t, authentik)
    defer test_util.StopServerForUnitTest(t, authentik)

    err := ObtainSessionCookie(MOCK_AUTH_BASE_ADDR, mock_authentik.MOCK_USERNAME, "bad password", mock_authentik.MOCK_TOKEN)
    if err == nil {
        t.Fatal("Expected error, got none.")
    } else {
        assert_util.AssertEq(t, err.Error(), "Received non-200 HTTP status code when submitting password: 401", "")
    }
}

func TestObtainSessionCookieBadToken(t *testing.T) {
    authentik := mock_authentik.GenerateMockAuthentikServer()
    test_util.StartServerForUnitTest(t, authentik)
    defer test_util.StopServerForUnitTest(t, authentik)

    err := ObtainSessionCookie(MOCK_AUTH_BASE_ADDR, mock_authentik.MOCK_USERNAME, mock_authentik.MOCK_PASSWORD, "invalidtoken")
    if err == nil {
        t.Fatal("Expected error, got none.")
    } else {
        assert_util.AssertEq(t, err.Error(), "Received non-200 HTTP status code when submitting MFA token: 401", "")
    }
}
