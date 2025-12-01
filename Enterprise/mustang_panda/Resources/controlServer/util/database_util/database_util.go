package database_util

import (
    "database/sql"
    "encoding/json"
    "errors"
    "fmt"
    "os"
    "time"

    "evals.mitre.org/control_server/display"
    "evals.mitre.org/control_server/logger"
    "evals.mitre.org/control_server/sessions"
    _ "github.com/mattn/go-sqlite3"
)

// RemoveSessionFromDatabase removes the session with the given guid from the database
func RemoveSessionFromDatabase(db *sql.DB, guid string) error {
    // check that session exists in database
    err := db.QueryRow("select session from sessions where session->>'guid' = ?", guid).Scan()
    if err == sql.ErrNoRows {
        return errors.New("session does not exist in database")
    }
    _, err = db.Exec("delete from sessions where session->>'guid' = ?", guid)
    if err != nil {
        return err
    }
    return nil
}

// UpdateSessionInDatabase takes a json string with session information and guid of existing session and updates session in database
func UpdateSessionInDatabase(db *sql.DB, guid string, session string) error {
    // check that session exists in database
    err := db.QueryRow("select session from sessions where session->>'guid' = ?", guid).Scan()
    if err == sql.ErrNoRows {
        return errors.New("session does not exist in database")
    }
    _, err = db.Exec("update sessions set session = ? where session->>'guid' == ?", session, guid)
    if err != nil {
        return err
    }
    return nil
}

// AddSessionToDatabase takes a json string with session information and GUID and adds it to the database
func AddSessionToDatabase(db *sql.DB, session string, guid string) error {
    // check if session already exists in DB
    err := db.QueryRow("select session from sessions where session->>'guid' = ?", guid).Scan()
    if err != sql.ErrNoRows {
        return errors.New("session already exists in database")
    }
    _, err = db.Exec("insert into sessions(session) values(?)", session)
    if err != nil {
        return err
    }
    return nil
}

// initDatabase initializes a sqlite database to store/load implant sessions
func initDatabase(db *sql.DB) error {
    _, err := db.Exec(`create table sessions (session jsonb)`)
    if err != nil {
        logger.Error("Unable to create database table: ", err)
        return err
    }
    return nil
}

// reinitDatabase loads all of the implant session data from the database and into the restAPI server
func reinitDatabase(db *sql.DB) (bool, error) {
    rows, err := db.Query(`select session from sessions`)
    if err != nil {
        logger.Error("Unable to query database for sessions:", err)
        return false, err
    }
    defer rows.Close()

    for rows.Next() {
        var jsonSession []byte
        if err = rows.Scan(&jsonSession); err != nil {
            logger.Error(err)
            return false, err
        }
        // Create new session in rest API
        var session sessions.Session
        err = json.Unmarshal(jsonSession, &session)
        if err != nil {
            logger.Error(fmt.Sprintf("CreateSession json.Unmarshal error for session: %s", err.Error()))
            return false, err
        }
        // pass session struct to handler
        err = sessions.AddSession(session)
        if err != nil {
            logger.Error(fmt.Sprintf("CreateSession error adding session: %s", err.Error()))
            return false, err
        }
        display.PrintSession(session)
    }
    return true, nil
}

// PrepareDatabase opens the database at dbPath and initializes or reinitializes it; if the reinitialization fails it will create a new database to initialize the rest API with
func PrepareDatabase(dbPath string, restartFromDatabase bool) (*sql.DB, error) {
    // check that database exists if reinitializing, ensure no database already exists at dbPath if not reinitializing
    if _, err := os.Stat(dbPath); os.IsNotExist(err) && restartFromDatabase {
        logger.Warning("Initializing server without existing database: Database not found")
        restartFromDatabase = false
    } else if !restartFromDatabase {
        err := os.Rename(dbPath, dbPath+" "+(time.Now()).Format("01-02-2006 15:04:05"))
        if err != nil {
            return nil, err
        }
    }

    db, err := sql.Open("sqlite3", dbPath)
    if err != nil {
        logger.Error("Unable to open database:", err)
        return nil, err
    }

    // Initialize or re-initialize the database
    dbInitSuccess := false
    if restartFromDatabase {
        dbInitSuccess, err = reinitDatabase(db)
        if err != nil {
            logger.Warning("Initializing server without existing database, unable to reinitialize:", err)
        } else {
            logger.Success("Reinitialized server with database")
        }
    }
    if !dbInitSuccess {
        err = initDatabase(db)
        if err != nil {
            logger.Warning("Starting server without database: Unable to initialize")
            db.Close()
            return nil, err
        }
        logger.Success("Initialized server with database")
    }
    return db, nil
}
