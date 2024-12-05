package base_handler

import (
    "fmt"
    "testing"
    
    "attackevals.mitre-engenuity.org/control_server/config"
    "attackevals.mitre-engenuity.org/control_server/util/test_util/assert_util"
)

const (
    TEST_HANDLER_NAME = "testbase"
    TEST_SESSION_ID = "implant1"
)

var testConfigEntry = config.HandlerConfigEntry{
    "host": "127.0.0.1",
    "port": 18085,
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

func TestHasImplantSessionAndStoreImplantSession(t *testing.T) {
    handler := BaseHandlerFactory(TEST_HANDLER_NAME)
    
    if handler.HasImplantSession("bogus-id") {
        t.Fatal("Implant bogus-id should not have an active session.")
    }
    err := handler.storeImplantSession(TEST_SESSION_ID)
    if err != nil {
        t.Fatal(err.Error())
    }
    if !handler.HasImplantSession(TEST_SESSION_ID) {
        t.Fatal("Expected implant session to be stored.")
    }
    err = handler.storeImplantSession(TEST_SESSION_ID)
    want := fmt.Sprintf("Session %s already exists.", TEST_SESSION_ID)
    if err != nil {
    	assert_util.AssertEq(t, err.Error(), want, "")
    } else {
        t.Fatalf("Expected error message: %s; got no error message", want)
    }
}


