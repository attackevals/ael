package sessions_test

import (
    "testing"

    "evals.mitre.org/control_server/sessions"
    "evals.mitre.org/control_server/tasks"
    "evals.mitre.org/control_server/util/test_util/assert_util"
)

var mySession = sessions.Session{
    GUID:          "abcdef123456",
    IPAddr:        "127.0.0.1",
    HostName:      "myHostName",
    User:          "myUserName",
    Dir:           "C:\\MyDir\\",
    PID:           "1234",
    PPID:          "4",
    SleepInterval: 60,
    Jitter:        1.5,
}

var expectedTask = tasks.Task{
    GUID:     mySession.GUID,
    Command:  "",
    Output:   "",
    Status:   tasks.TASK_STATUS_NEW,
    ExitCode: -1,
}

func TestAddSession(t *testing.T) {
    err := sessions.AddSession(mySession)
    if err != nil {
        t.Fatal(err)
    }
}

func TestUpdateLastCheckin(t *testing.T) {
    if !sessions.SessionExists(mySession.GUID) {
        t.Fatal("Invalid session: ", mySession.GUID)
    }
    sessions.UpdateLastCheckin(mySession.GUID)
    got := sessions.SessionList[mySession.GUID].LastCheckIn
    want := sessions.GetCurrentTimeFmt()
    assert_util.AssertEq(t, got, want, "")
}

func TestUpdateSession(t *testing.T) {
    valid := sessions.SessionExists(mySession.GUID)
    if !valid {
        t.Error("Invalid session: ", mySession.GUID)
    }

    updates := map[string]string{
        "ipAddr":   "127.0.0.1",
        "hostName": "updatedHost",
        "user":     "updatedUser",
        "dir":      "C:\\UpdatedDir\\",
        "pid":      "9999",
        "ppid":     "9",
    }
    session, err := sessions.UpdateSession(mySession.GUID, updates)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, session.HostName, updates["hostName"], "")
    assert_util.AssertEq(t, session.User, updates["user"], "")
    assert_util.AssertEq(t, session.Dir, updates["dir"], "")
    assert_util.AssertEq(t, session.PID, updates["pid"], "")
    assert_util.AssertEq(t, session.PPID, updates["ppid"], "")
}

func TestSessionExists(t *testing.T) {
    got := sessions.SessionExists(mySession.GUID)
    assert_util.AssertEq(t, got, true, "")

    // test error condition
    got = sessions.SessionExists("this should return false")
    assert_util.AssertEq(t, got, false, "")
}

func TestGetSessionList(t *testing.T) {
    sessionList := sessions.GetSessionList()
    got := sessions.SessionExists(sessionList[0].GUID)
    assert_util.AssertEq(t, got, true, "")
}

func TestGetSessionByGuid(t *testing.T) {
    theSession, err := sessions.GetSessionByGuid(mySession.GUID)
    if err != nil {
        t.Fatal(err)
    }
    got := sessions.SessionExists(theSession.GUID)
    assert_util.AssertEq(t, got, true, "")

    _, err = sessions.GetSessionByGuid("test nonexistent session id")
    if err == nil {
        t.Fatal("Expected an error for bad session, got nil")
    }
}

func TestSetSessionTask(t *testing.T) {
    taskCommand := "exec-cmd \"whoami\""
    expectedTask.Command = taskCommand
    returnedTask, err := sessions.SetTask(mySession.GUID, expectedTask.GUID, taskCommand)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, expectedTask, *returnedTask, "")

    taskCommand = "ipconfig"
    expectedTask.Command = taskCommand
    returnedTask, err = sessions.SetTask(mySession.GUID, expectedTask.GUID, taskCommand)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, expectedTask, *returnedTask, "")
}

func TestGetSessionTask(t *testing.T) {
    _, err := sessions.GetTask(mySession.GUID)
    if err != nil {
        t.Fatal(err)
    }
}

func TestRemoveSessionTask(t *testing.T) {
    err := sessions.RemoveTask(mySession.GUID)
    if err != nil {
        t.Fatal(err)
    }
    blankTask, err := sessions.GetTask(mySession.GUID)
    if err != nil {
        t.Fatal(err)
    }
    if blankTask != nil {
        t.Fatalf("Expected blank task \"\" got %v", blankTask)
    }
}

func TestBootstrapTask(t *testing.T) {
    expectedTask := "exec-cmd \"whoami\""
    handler := "handler1"
    currBootstrap := sessions.GetBootstrapTask(handler)
    assert_util.AssertEq(t, currBootstrap, "", "Expected empty bootstrap task")

    sessions.SetBootstrapTask(handler, expectedTask)
    currBootstrap = sessions.GetBootstrapTask(handler)
    assert_util.AssertEq(t, expectedTask, currBootstrap, "")
    currBootstrap = sessions.GetBootstrapTask("handler2")
    assert_util.AssertEq(t, currBootstrap, "", "Expected empty bootstrap task")

    sessions.RemoveBootstrapTask(handler)
    currBootstrap = sessions.GetBootstrapTask(handler)
    assert_util.AssertEq(t, currBootstrap, "", "Expected empty bootstrap task")
}

// this tests SetTaskOutput, GetTaskOutput, and DeleteTaskOutput
// To Do - split these into individual tests
func TestTaskOutput(t *testing.T) {
    valid := sessions.SessionExists(mySession.GUID)
    if !valid {
        t.Fatal("Invalid session: ", mySession.GUID)
    }

    taskCommand := "whoami"
    _, err := sessions.SetTask(mySession.GUID, "", taskCommand)
    if err != nil {
        t.Fatal(err)
    }

    // test SetTaskOutput
    expectedOutput := "user"
    err = sessions.SetTaskOutput(mySession.GUID, expectedOutput, true)
    if err != nil {
        t.Fatal(err)
    }

    // test GetTaskOutput
    actualOutput, err := sessions.GetTaskOutput(mySession.GUID)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, expectedOutput, actualOutput, "")

    // test DeleteTaskOutput
    err = sessions.DeleteTaskOutput(mySession.GUID)
    if err != nil {
        t.Fatal(err)
    }
    want := ""
    got, err := sessions.GetTaskOutput(mySession.GUID)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, got, want, "")
}

func TestRemoveSession(t *testing.T) {
    sessions.RemoveSession(mySession.GUID)
    sessionFound := sessions.SessionExists(mySession.GUID)
    if sessionFound == true {
        t.Fatal("Unable to delete session")
    }
}
