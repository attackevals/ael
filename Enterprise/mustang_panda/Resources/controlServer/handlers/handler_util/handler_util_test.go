package handler_util

import (
    "fmt"
    "os"
    "testing"

    "evals.mitre.org/control_server/restapi"
    "evals.mitre.org/control_server/restapi/restapi_test_util"
    "evals.mitre.org/control_server/sessions"
    "evals.mitre.org/control_server/util"
    "evals.mitre.org/control_server/util/test_util"
    "evals.mitre.org/control_server/util/test_util/assert_util"
)

const (
    REST_API_LISTEN_HOST  = "127.0.0.1:9990" // need to check on port
    REST_API_BASE_URL     = "http://" + REST_API_LISTEN_HOST + "/api/v1.0/"
    TEST_SESSION_GUID     = "new-implant-beacon"
    INVALID_DATA          = "invalid data"
    EXAMPLE_TASK          = "{\"id\":\"task-id\"}"
    HELLO_WORLD_FILE_NAME = "hello_world.elf"
    HELLO_WORLD_FILE_HASH = "fe7c47d38224529c7d8f9a11a62cdd7a"
)

var testSession = sessions.Session{
    GUID: TEST_SESSION_GUID,
}

func init() {
    cwd, _ := os.Getwd()
    os.Chdir("../../")
    defer os.Chdir(cwd)
    util.SetRootDirectories()
}

func TestForwardImplantBeacon(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", REST_API_LISTEN_HOST, nil)
    defer restapi_test_util.StopRestApiServer()

    test_util.StartMockCalderaServer()
    restapi.CalderaForwardingEndpoint = "http://127.0.0.1:8888/plugins/emu/beacons"

    defer test_util.StopMockCalderaServer()
    restapi_test_util.CreateTestSession(t, REST_API_LISTEN_HOST, testSession)

    response, err := ForwardImplantBeacon(TEST_SESSION_GUID, REST_API_LISTEN_HOST)
    if err != nil {
        t.Fatal(err)
    }
    expectedOutput := fmt.Sprintf("Forwarded beacon for session: %s, received response: CALDERA server successfully received session: %s", TEST_SESSION_GUID, TEST_SESSION_GUID)
    assert_util.AssertEq(t, response, expectedOutput, "")
}

func TestForwardRegisterImplant(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", REST_API_LISTEN_HOST, nil)
    defer restapi_test_util.StopRestApiServer()

    // test that session can be registered
    expectedOutput := "Successfully added session."
    implantData := []byte("{\"guid\":\"test-implant-id\",\"hostName\":\"test-computerName\",\"pid\":\"1234\",\"user\":\"test-user\"}")
    restResponse, err := ForwardRegisterImplant(REST_API_LISTEN_HOST, implantData)
    if err != nil {
        t.Fatal(err.Error())
    }
    assert_util.AssertEq(t, restResponse, expectedOutput, "")

    // test for correct error message if invalid data is sent
    implantData = []byte(INVALID_DATA)
    restResponse, err = ForwardRegisterImplant(REST_API_LISTEN_HOST, implantData)
    if err != nil {
        t.Fatal(err.Error())
    }
    assert_util.AssertNe(t, restResponse, expectedOutput, "Expected error message")
}

func TestForwardGetTask(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", REST_API_LISTEN_HOST, nil)
    defer restapi_test_util.StopRestApiServer()

    // set a session
    restapi_test_util.CreateTestSession(t, REST_API_LISTEN_HOST, testSession)

    // test that empty string is received when no task is set
    sessionData, err := ForwardGetTask(REST_API_LISTEN_HOST, TEST_SESSION_GUID)
    if err != nil {
        t.Fatal(err.Error())
    }
    assert_util.AssertEq(t, sessionData, "", "")

    // set a task
    restapi_test_util.CreateTestTaskForSession(t, REST_API_LISTEN_HOST, TEST_SESSION_GUID, "", EXAMPLE_TASK)

    // check that we can receive a task
    sessionData, err = ForwardGetTask(REST_API_LISTEN_HOST, TEST_SESSION_GUID)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, sessionData, EXAMPLE_TASK, "")
}

func TestForwardGetFileFromServer(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", REST_API_LISTEN_HOST, nil)
    defer restapi_test_util.StopRestApiServer()

    // test downloading non-existent file
    expectedOutput := fmt.Sprintf("server did not return requested file: %s", INVALID_DATA)
    _, err := ForwardGetFileFromServer(REST_API_LISTEN_HOST, "", INVALID_DATA)
    if err != nil {
        assert_util.AssertEq(t, err.Error(), expectedOutput, "Error message mismatch")
    } else {
        t.Fatal("Expected error message, none received.")
    }

    // test downloading a file
    fileData, err := ForwardGetFileFromServer(REST_API_LISTEN_HOST, "", HELLO_WORLD_FILE_NAME)
    if err != nil {
        t.Fatal(err.Error())
    }

    // compare file hashes
    test_util.CheckMD5Hash(t, fileData, HELLO_WORLD_FILE_HASH)
}
