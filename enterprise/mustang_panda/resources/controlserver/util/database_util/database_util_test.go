package database_util_test

import (
    "database/sql"
    "encoding/json"
    "os"
    "testing"

    "evals.mitre.org/control_server/sessions"
    "evals.mitre.org/control_server/util/database_util"
    "evals.mitre.org/control_server/util/test_util/assert_util"
    _ "github.com/mattn/go-sqlite3"
)

const (
    TEST_SESSION_1_GUID = "test-session-1"
    TEST_HANDLER_1      = "test-handler-1"
)

var TEST_SESSION_1 = sessions.Session{
    GUID:          TEST_SESSION_1_GUID,
    Handler:       TEST_HANDLER_1,
    IPAddr:        "127.0.0.1",
    HostName:      "myHostName",
    User:          "myUserName",
    Dir:           "C:\\MyDir\\",
    PID:           "1234",
    PPID:          "4",
    SleepInterval: 60,
    Jitter:        1.5,
}

var TEST_SESSION_1_UPDATED = sessions.Session{
    GUID:          TEST_SESSION_1_GUID,
    Handler:       TEST_HANDLER_1,
    IPAddr:        "127.0.0.2",
    HostName:      "myHostName2",
    User:          "myUserName2",
    Dir:           "C:\\MyDir2\\",
    PID:           "3456",
    PPID:          "5",
    SleepInterval: 61,
    Jitter:        2,
}

var (
    TEST_SESSION_1_JSON, _         = json.Marshal(TEST_SESSION_1)
    TEST_SESSION_1_UPDATED_JSON, _ = json.Marshal(TEST_SESSION_1_UPDATED)
)

var TEST_RESP_SESSION_LIST = []sessions.Session{
    TEST_SESSION_1,
}

func setupDatabase(t *testing.T) (*sql.DB, func()) {
    db, err := sql.Open("sqlite3", "./testDatabase.db")
    if err != nil {
        t.Fatal(err)
    }
    _, err = db.Exec(`create table sessions (session jsonb)`)
    if err != nil {
        t.Fatal(err)
    }
    return db, func() {
        db.Close()
        os.Remove("./testDatabase.db")
    }
}

func TestRemoveSessionFromDatabase(t *testing.T) {
    db, teardownDB := setupDatabase(t)
    defer teardownDB()

    // attempt to remove nonexistent session
    err := database_util.RemoveSessionFromDatabase(db, TEST_SESSION_1_GUID)
    assert_util.AssertEq(t, err.Error(), "session does not exist in database", "Expected error message")

    // add session to database and then remove
    _, err = db.Exec("insert into sessions(session) values(?)", string(TEST_SESSION_1_JSON))
    if err != nil {
        t.Fatal(err)
    }
    err = database_util.RemoveSessionFromDatabase(db, TEST_SESSION_1_GUID)
    if err != nil {
        t.Fatal(err)
    }

    // attempt to query for session
    err = db.QueryRow("select session from sessions where session->>'guid' = ?", TEST_SESSION_1_GUID).Scan()
    assert_util.AssertEq(t, err, sql.ErrNoRows, "Expected error message")
}

func TestUpdateSessionInDatabase(t *testing.T) {
    db, teardownDB := setupDatabase(t)
    defer teardownDB()

    // attempt to update nonexistent session
    err := database_util.UpdateSessionInDatabase(db, TEST_SESSION_1_GUID, string(TEST_SESSION_1_JSON))
    assert_util.AssertEq(t, err.Error(), "session does not exist in database", "Expected error message")

    // add session to database and then update
    _, err = db.Exec("insert into sessions(session) values(?)", string(TEST_SESSION_1_JSON))
    if err != nil {
        t.Fatal(err)
    }
    err = database_util.UpdateSessionInDatabase(db, TEST_SESSION_1_GUID, string(TEST_SESSION_1_UPDATED_JSON))
    if err != nil {
        t.Fatal(err)
    }

    var updatedSession []byte
    err = db.QueryRow("select session from sessions where session->>'guid' = ?", TEST_SESSION_1_GUID).Scan(&updatedSession)
    if err != nil {
        t.Fatal(err)
    }
    assert_util.AssertEq(t, string(TEST_SESSION_1_UPDATED_JSON), string(updatedSession), "")
}

func TestAddSessionToDatabase(t *testing.T) {
    db, teardownDB := setupDatabase(t)
    defer teardownDB()

    // add session and query to check it exists
    err := database_util.AddSessionToDatabase(db, string(TEST_SESSION_1_JSON), TEST_SESSION_1_GUID)
    if err != nil {
        t.Fatal(err)
    }
    err = db.QueryRow("select session from sessions where session->>'guid' = ?", TEST_SESSION_1_GUID).Scan()
    if err == sql.ErrNoRows {
        t.Fatal(err)
    }

    // try to add an already existing session
    err = database_util.AddSessionToDatabase(db, string(TEST_SESSION_1_JSON), TEST_SESSION_1_GUID)
    assert_util.AssertEq(t, err.Error(), "session already exists in database", "Expected error message")
}
