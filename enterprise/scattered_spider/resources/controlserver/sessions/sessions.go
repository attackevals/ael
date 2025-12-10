package sessions

import (
    "errors"
    "fmt"
    "sort"
    "time"

    "evals.mitre.org/control_server/tasks"
)

// Session describes an agent connection to the C2 server
type Session struct {
    GUID          string      `json:"guid"`
    Handler       string      `json:"handler,omitempty"`
    IPAddr        string      `json:"ipAddr,omitempty"`
    HostName      string      `json:"hostName,omitempty"`
    User          string      `json:"user,omitempty"`
    Dir           string      `json:"dir,omitempty"`
    PID           string      `json:"pid,omitempty"`
    PPID          string      `json:"ppid,omitempty"`
    Task          *tasks.Task `json:"task,omitempty"`
    SleepInterval int         `json:"sleepInterval,omitempty"`
    Jitter        float64     `json:"jitter,omitempty"`
    FirstCheckIn  string      `json:"firstCheckIn,omitempty"`
    LastCheckIn   string      `json:"lastCheckIn,omitempty"`
}

// Will contain the default bootstrap task to send to new sessions for the specified handler
var bootstrapTasks map[string]string

var SessionList map[string]Session

func init() {
    bootstrapTasks = make(map[string]string)
}

// GetCurrentTimeFmt returns the current time in a standard format: MM-DD-YY MM:HH:SS
func GetCurrentTimeFmt() string {
    currentTime := time.Now()
    formattedTime := currentTime.Format("01-02-2006 15:04:05")
    return formattedTime
}

// AddSession adds a new session to the SessionList object
func AddSession(s Session) error {
    // check if session already exists
    if SessionExists(s.GUID) {
        return errors.New("Session already exists")
    }
    s.FirstCheckIn = GetCurrentTimeFmt()
    if SessionList == nil {
        SessionList = make(map[string]Session)
    }
    SessionList[s.GUID] = s
    if !SessionExists(s.GUID) {
        return fmt.Errorf("failed to add session: %v", s.GUID)
    }
    return nil
}

// UpdateLastCheckin sets session.LastCheckIn with the current time
func UpdateLastCheckin(guid string) error {
    if s, ok := SessionList[guid]; !ok {
        return fmt.Errorf("unable to update last checkin for session: %v", guid)
    } else {
        s.LastCheckIn = GetCurrentTimeFmt()
        SessionList[guid] = s
        return nil
    }

}

// UpdateSession updates an active implant session with the provided data
func UpdateSession(guid string, updates map[string]string) (Session, error) {
    var s Session
    if !SessionExists(guid) {
        return s, fmt.Errorf("unable to update session: %v", guid)
    }
    s = SessionList[guid]
    for name, val := range updates {
        switch name {
        case "ipAddr":
            s.IPAddr = val
        case "hostName":
            s.HostName = val
        case "user":
            s.User = val
        case "dir":
            s.Dir = val
        case "pid":
            s.PID = val
        case "ppid":
            s.PPID = val
        }
    }
    SessionList[guid] = s
    return s, nil
}

// SessionExists checks if the session specified by guid
// is present in SessionList; if a matching session is found,
// the function returns the index for the session
func SessionExists(guid string) bool {
    _, ok := SessionList[guid]
    return ok
}

// GetSessionList returns a list of current C2 sessions
func GetSessionList() []Session {
    var sessions []Session
    for _, session := range SessionList {
        sessions = append(sessions, session)
    }

    // Sort using GUID
    sort.Slice(sessions, func(i, j int) bool {
        return sessions[i].GUID < sessions[j].GUID
    })

    return sessions
}

// GetSessionMap returns a list of the sessions mapped to their GUID
func GetSessionMap() map[string]Session {
    return SessionList
}

// GetSessionByGuid returns details for a session by name
func GetSessionByGuid(guid string) (Session, error) {
    if s, ok := SessionList[guid]; ok {
        return s, nil
    } else {
        return s, fmt.Errorf("unable to get info for nonexistent session: %v", guid)
    }
}

// SetTask creates a task for the session specified by guid
func SetTask(sessionGuid string, taskGuid string, taskCommand string) (*tasks.Task, error) {
    if s, ok := SessionList[sessionGuid]; ok {
        s.Task = tasks.TaskFactory(taskCommand, taskGuid, tasks.GenerateUUID)
        SessionList[sessionGuid] = s
        return s.Task, nil
    }
    return nil, fmt.Errorf("unable to set task for nonexistent session: %v", sessionGuid)
}

// GetTask returns the task for the session specified by guid
func GetTask(guid string) (*tasks.Task, error) {
    if s, ok := SessionList[guid]; ok {
        return s.Task, nil
    }
    return nil, fmt.Errorf("unable to get task for nonexistent session: %v", guid)
}

// RemoveTask clears the currently queued tasking
func RemoveTask(guid string) error {
    if s, ok := SessionList[guid]; ok {
        if s.Task != nil {
            s.Task.CancelTask()
        }
        s.Task = nil
        SessionList[guid] = s
        return nil
    }
    return fmt.Errorf("unable to remove nonexistent session: %v", guid)
}

// SetBootstrapTask sets the default bootstrap task for new sessions for the specified handler
func SetBootstrapTask(handler, task string) {
    bootstrapTasks[handler] = task
}

// GetBootstrapTask gets the default bootstrap task for the specified handler (empty string if no task is set).
func GetBootstrapTask(handler string) string {
    if task, ok := bootstrapTasks[handler]; ok {
        return task
    }
    return ""
}

// RemoveBootstrapTask clears the currently set default bootstrap task for the handler
func RemoveBootstrapTask(handler string) {
    delete(bootstrapTasks, handler)
}

// RemoveSession deletes the session specified by guid
// note that this function only removes the session from the SessionList object
// this function does not actually terminate the beacon process on the endpoint
func RemoveSession(guid string) error {
    if !SessionExists(guid) {
        return fmt.Errorf("unable to remove nonexistent session: %v", guid)
    }
    delete(SessionList, guid)
    return nil
}

// SetTaskOutput stores console output for the last executed task and marks the task as completed if specified.
func SetTaskOutput(guid string, output string, markAsComplete bool) error {
    s, ok := SessionList[guid]
    if !ok {
        return fmt.Errorf("unable to set task output for nonexistent session: %v", guid)
    }
    if s.Task == nil {
        return fmt.Errorf("unable to set task output for session %v with nonexistent task", guid)
    }
    s.Task.Output = output
    if markAsComplete {
        s.Task.FinishTask()
    }
    SessionList[guid] = s
    return nil
}

// GetTaskOutput returns the task for the session specified by guid
func GetTaskOutput(guid string) (string, error) {
    s, ok := SessionList[guid]
    if !ok {
        return "", fmt.Errorf("unable to get task output for nonexistent session: %v", guid)
    }
    if s.Task == nil {
        return "", fmt.Errorf("unable to get task output for session %v with nonexistent task", guid)
    }
    return s.Task.Output, nil
}

// DeleteTaskOutput returns the task for the session specified by guid
func DeleteTaskOutput(guid string) error {
    s, ok := SessionList[guid]
    if !ok {
        return fmt.Errorf("unable to delete task output for nonexistent session: %v", guid)
    }
    if s.Task == nil {
        return fmt.Errorf("unable to delete task output for session %v with nonexistent task", guid)
    }
    s.Task.Output = ""
    SessionList[guid] = s
    return nil
}
