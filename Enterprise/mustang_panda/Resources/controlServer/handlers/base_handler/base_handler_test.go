package base_handler

import (
    "testing"

    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/restapi/restapi_test_util"
    "evals.mitre.org/control_server/sessions"
    "evals.mitre.org/control_server/util/test_util/assert_util"
)

const (
    TEST_HANDLER_NAME    = "testbase"
    TEST_SESSION_ID      = "implant1"
    REST_API_LISTEN_HOST = "127.0.0.1:10001"
)

var testConfigEntry = config.HandlerConfigEntry{
    "host": "127.0.0.1",
    "port": 18085,
}

var testSession = sessions.Session{
    GUID: TEST_SESSION_ID,
}

func TestSetBindAddr(t *testing.T) {
    handler := BaseHandlerFactory(TEST_HANDLER_NAME)
    err := handler.SetBindAddr(testConfigEntry)
    if err != nil {
        t.Fatal(err.Error())
    }
    want := "127.0.0.1:18085"
    assert_util.AssertEq(t, handler.BindAddr, want, "")
}

func TestHasImplantSession(t *testing.T) {
    // setup REST API
    restapi_test_util.StartRestApiServer(t, "", REST_API_LISTEN_HOST, nil)
    defer restapi_test_util.StopRestApiServer()

    // setup base handler
    handler := BaseHandlerFactory(TEST_HANDLER_NAME)
    handler.SetRestApiAddr(REST_API_LISTEN_HOST)

    if handler.HasImplantSession("bogus-id") {
        t.Fatal("Implant bogus-id should not have an active session.")
    }
    err := sessions.AddSession(testSession)
    if err != nil {
        t.Fatal(err.Error())
    }
    if !handler.HasImplantSession(TEST_SESSION_ID) {
        t.Fatal("Expected implant session to be stored.")
    }
}
