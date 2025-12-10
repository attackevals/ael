package mock_authentik

import (
    "context"
    "encoding/json"
    "fmt"
    "io/ioutil"
    "net/http"
    "time"

    "github.com/gorilla/mux"

    "aitm/pkg/authentik_util"
)

const (
    MOCK_AUTHENTIK_BIND_ADDR = "127.0.0.1:59999"
    MOCK_CSRF_COOKIE = "authentik_csrf=6CAZ7xj6pUqRglB2oNEIFbc7NuikZ5uK; expires=Wed, 08 Oct 2025 22:45:22 GMT; Max-Age=31449600; Path=/; SameSite=Lax"
    MOCK_CSRF_COOKIE_VAL = "6CAZ7xj6pUqRglB2oNEIFbc7NuikZ5uK"
    MOCK_AUTH_ATTEMPT_COOKIE = "authentik_session=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiJqMHdvYWduYmJldGJ3MW0zdnluZ2wwamU2NWJ3dzB3eiIsImlzcyI6ImF1dGhlbnRpayIsInN1YiI6ImFub255bW91cyIsImF1dGhlbnRpY2F0ZWQiOmZhbHNlLCJhY3IiOiJnb2F1dGhlbnRpay5pby9jb3JlL2RlZmF1bHQifQ.VaZUnGXuCNLnZ7U6jJEce0QeCkAv4LqdYWKMXY_B3wM; HttpOnly; Path=/; SameSite=None; Secure"
    MOCK_AUTH_ATTEMPT_COOKIE_VAL = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiJqMHdvYWduYmJldGJ3MW0zdnluZ2wwamU2NWJ3dzB3eiIsImlzcyI6ImF1dGhlbnRpayIsInN1YiI6ImFub255bW91cyIsImF1dGhlbnRpY2F0ZWQiOmZhbHNlLCJhY3IiOiJnb2F1dGhlbnRpay5pby9jb3JlL2RlZmF1bHQifQ.VaZUnGXuCNLnZ7U6jJEce0QeCkAv4LqdYWKMXY_B3wM"
    MOCK_SESSION_COOKIE = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiI0Nzhpcm9rcDNxbzB4YTFmZHNxc2Vyam9ud28wNmVjdyIsImlzcyI6ImF1dGhlbnRpayIsInN1YiI6IjIxZGU3YWViYjQ1ZmNiMTJjZTljY2Q4NjYxMzQzNmVkMzFkMzFhYjZkOWY3MmE0MDVjYjM2NWZjYjFjYWQwNzgiLCJhdXRoZW50aWNhdGVkIjp0cnVlLCJhY3IiOiJnb2F1dGhlbnRpay5pby9jb3JlL2RlZmF1bHQifQ.Pq5HGNywEXuZ0xbJFTnbxBFyjw70jKwV4eqdCuPOPcc; HttpOnly; Path=/; SameSite=None; Secure"
    MOCK_SESSION_COOKIE_VAL = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiI0Nzhpcm9rcDNxbzB4YTFmZHNxc2Vyam9ud28wNmVjdyIsImlzcyI6ImF1dGhlbnRpayIsInN1YiI6IjIxZGU3YWViYjQ1ZmNiMTJjZTljY2Q4NjYxMzQzNmVkMzFkMzFhYjZkOWY3MmE0MDVjYjM2NWZjYjFjYWQwNzgiLCJhdXRoZW50aWNhdGVkIjp0cnVlLCJhY3IiOiJnb2F1dGhlbnRpay5pby9jb3JlL2RlZmF1bHQifQ.Pq5HGNywEXuZ0xbJFTnbxBFyjw70jKwV4eqdCuPOPcc"
    MOCK_USERNAME = "dummyuser"
    MOCK_PASSWORD = "dummypassword"
    MOCK_TOKEN = "123456"

    AUTH_STAGE_ID_RESP_TEMPLATE = `{"flow_info": {"title": "Welcome to authentik!", "background": "/static/dist/assets/images/flow_background.jpg", "cancel_url": "/flows/-/cancel/", "layout": "stacked"}, "component": "ak-stage-password", "pending_user": "%s", "pending_user_avatar": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI2NHB4IiBoZWlnaHQ9IjY0cHgiIHZpZXdCb3g9IjAgMCA2NCA2NCIgdmVyc2lvbj0iMS4xIj48cmVjdCBmaWxsPSIjMzdjOGE1IiBjeD0iMzIiIGN5PSIzMiIgd2lkdGg9IjY0IiBoZWlnaHQ9IjY0IiByPSIzMiIvPjx0ZXh0IHg9IjUwJSIgeT0iNTAlIiBzdHlsZT0iY29sb3I6ICNmZmY7IGxpbmUtaGVpZ2h0OiAxOyBmb250LWZhbWlseTogJ1JlZEhhdFRleHQnLCdPdmVycGFzcycsb3ZlcnBhc3MsaGVsdmV0aWNhLGFyaWFsLHNhbnMtc2VyaWY7ICIgZmlsbD0iI2ZmZiIgYWxpZ25tZW50LWJhc2VsaW5lPSJtaWRkbGUiIGRvbWluYW50LWJhc2VsaW5lPSJtaWRkbGUiIHRleHQtYW5jaG9yPSJtaWRkbGUiIGZvbnQtc2l6ZT0iMjgiIGZvbnQtd2VpZ2h0PSI0MDAiIGR5PSIuMWVtIj5TUDwvdGV4dD48L3N2Zz4=", "allow_show_password": false}`
    AUTH_STAGE_PASSWORD_RESP_SUCCESS_TEMPLATE = `{"flow_info": {"title": "Welcome to authentik!", "background": "/static/dist/assets/images/flow_background.jpg", "cancel_url": "/flows/-/cancel/", "layout": "stacked"}, "component": "ak-stage-authenticator-validate", "pending_user": "%s", "pending_user_avatar": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI2NHB4IiBoZWlnaHQ9IjY0cHgiIHZpZXdCb3g9IjAgMCA2NCA2NCIgdmVyc2lvbj0iMS4xIj48cmVjdCBmaWxsPSIjMzdjOGE1IiBjeD0iMzIiIGN5PSIzMiIgd2lkdGg9IjY0IiBoZWlnaHQ9IjY0IiByPSIzMiIvPjx0ZXh0IHg9IjUwJSIgeT0iNTAlIiBzdHlsZT0iY29sb3I6ICNmZmY7IGxpbmUtaGVpZ2h0OiAxOyBmb250LWZhbWlseTogJ1JlZEhhdFRleHQnLCdPdmVycGFzcycsb3ZlcnBhc3MsaGVsdmV0aWNhLGFyaWFsLHNhbnMtc2VyaWY7ICIgZmlsbD0iI2ZmZiIgYWxpZ25tZW50LWJhc2VsaW5lPSJtaWRkbGUiIGRvbWluYW50LWJhc2VsaW5lPSJtaWRkbGUiIHRleHQtYW5jaG9yPSJtaWRkbGUiIGZvbnQtc2l6ZT0iMjgiIGZvbnQtd2VpZ2h0PSI0MDAiIGR5PSIuMWVtIj5TUDwvdGV4dD48L3N2Zz4=", "device_challenges": [{"device_class": "totp", "device_uid": "1", "challenge": {}}], "configuration_stages": []}`
    AUTH_STAGE_PASSWORD_RESP_FAILURE_TEMPLATE = `{"flow_info": {"title": "Welcome to authentik!", "background": "/static/dist/assets/images/flow_background.jpg", "cancel_url": "/flows/-/cancel/", "layout": "stacked"}, "component": "ak-stage-password", "response_errors": {"password": [{"string": "Invalid password", "code": "invalid"}]}, "pending_user": "%s", "pending_user_avatar": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI2NHB4IiBoZWlnaHQ9IjY0cHgiIHZpZXdCb3g9IjAgMCA2NCA2NCIgdmVyc2lvbj0iMS4xIj48cmVjdCBmaWxsPSIjMzdjOGE1IiBjeD0iMzIiIGN5PSIzMiIgd2lkdGg9IjY0IiBoZWlnaHQ9IjY0IiByPSIzMiIvPjx0ZXh0IHg9IjUwJSIgeT0iNTAlIiBzdHlsZT0iY29sb3I6ICNmZmY7IGxpbmUtaGVpZ2h0OiAxOyBmb250LWZhbWlseTogJ1JlZEhhdFRleHQnLCdPdmVycGFzcycsb3ZlcnBhc3MsaGVsdmV0aWNhLGFyaWFsLHNhbnMtc2VyaWY7ICIgZmlsbD0iI2ZmZiIgYWxpZ25tZW50LWJhc2VsaW5lPSJtaWRkbGUiIGRvbWluYW50LWJhc2VsaW5lPSJtaWRkbGUiIHRleHQtYW5jaG9yPSJtaWRkbGUiIGZvbnQtc2l6ZT0iMjgiIGZvbnQtd2VpZ2h0PSI0MDAiIGR5PSIuMWVtIj5TUDwvdGV4dD48L3N2Zz4=", "allow_show_password": false}    `
    AUTH_STAGE_MFA_RESP_SUCCESS = `{"component": "xak-flow-redirect", "to": "/"}`
    AUTH_STAGE_MFA_RESP_FAILURE_TEMPLATE = `{"flow_info": {"title": "Welcome to authentik!", "background": "/static/dist/assets/images/flow_background.jpg", "cancel_url": "/flows/-/cancel/", "layout": "stacked"}, "component": "ak-stage-authenticator-validate", "response_errors": {"code": [{"string": "Invalid Token. Please ensure the time on your device is accurate and try again.", "code": "invalid"}]}, "pending_user": "%s", "pending_user_avatar": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI2NHB4IiBoZWlnaHQ9IjY0cHgiIHZpZXdCb3g9IjAgMCA2NCA2NCIgdmVyc2lvbj0iMS4xIj48cmVjdCBmaWxsPSIjMzdjOGE1IiBjeD0iMzIiIGN5PSIzMiIgd2lkdGg9IjY0IiBoZWlnaHQ9IjY0IiByPSIzMiIvPjx0ZXh0IHg9IjUwJSIgeT0iNTAlIiBzdHlsZT0iY29sb3I6ICNmZmY7IGxpbmUtaGVpZ2h0OiAxOyBmb250LWZhbWlseTogJ1JlZEhhdFRleHQnLCdPdmVycGFzcycsb3ZlcnBhc3MsaGVsdmV0aWNhLGFyaWFsLHNhbnMtc2VyaWY7ICIgZmlsbD0iI2ZmZiIgYWxpZ25tZW50LWJhc2VsaW5lPSJtaWRkbGUiIGRvbWluYW50LWJhc2VsaW5lPSJtaWRkbGUiIHRleHQtYW5jaG9yPSJtaWRkbGUiIGZvbnQtc2l6ZT0iMjgiIGZvbnQtd2VpZ2h0PSI0MDAiIGR5PSIuMWVtIj5TUDwvdGV4dD48L3N2Zz4=", "device_challenges": [{"device_class": "totp", "device_uid": "1", "challenge": {}}], "configuration_stages": []}`
)

type MockAuthentikServer struct {
    bindAddr string // address+port to listen on
    server *http.Server
}

// Creates mock Authentik server instance
func mockAuthentikServerFactory(bindAddr string) *MockAuthentikServer {
    return &MockAuthentikServer{
        bindAddr: bindAddr,
    }
}

// generate mock Authentik server
func GenerateMockAuthentikServer() *MockAuthentikServer {
    return mockAuthentikServerFactory(MOCK_AUTHENTIK_BIND_ADDR)
}

func (a *MockAuthentikServer) StartServer() error {
    r := mux.NewRouter()
    r.StrictSlash(true)

    r.HandleFunc("/if/flow/default-authentication-flow/", a.handleMainFlowPage)
    r.HandleFunc("/api/v3/flows/executor/default-authentication-flow/", a.handleAuthRequestGet).Methods("GET")
    r.HandleFunc("/api/v3/flows/executor/default-authentication-flow/", a.handleAuthRequestPost).Methods("POST")

    a.server = &http.Server{
        Addr:         a.bindAddr,
        WriteTimeout: time.Second * 15,
        ReadTimeout:  time.Second * 15,
        IdleTimeout:  time.Second * 60,
        Handler:      r,
    }

    // start server in goroutine so it doesn't block
    go func() {
        err := a.server.ListenAndServe()
        if err != nil && err.Error() != "http: Server closed" {
            panic(err)
        }
    }()

    return nil
}

func (a *MockAuthentikServer) StopServer() error {
    emptyContext := context.Background()
    return a.server.Shutdown(emptyContext)
}

func (a *MockAuthentikServer) handleMainFlowPage(w http.ResponseWriter, r *http.Request) {
    // If no CSRF cookie, send one. Otherwise, send back the same cookie
    if c, err := r.Cookie(authentik_util.CSRF_COOKIE_NAME); err == http.ErrNoCookie {
        retCookie, err := http.ParseSetCookie(MOCK_CSRF_COOKIE)
        if err != nil {
            handleServerError(w, "Failed to parse mock CSRF cookie")
            return
        }
        http.SetCookie(w, retCookie)
    } else if err == nil {
        http.SetCookie(w, c)
    } else {
        handleServerError(w, err.Error())
    }
}

func (a *MockAuthentikServer) handleAuthRequestGet(w http.ResponseWriter, r *http.Request) {
    // If no auth attempt cookie, send one. Otherwise, send back the same cookie
    if c, err := r.Cookie(authentik_util.AUTH_ATTEMPT_COOKIE_NAME); err == http.ErrNoCookie {
        retCookie, err := http.ParseSetCookie(MOCK_AUTH_ATTEMPT_COOKIE)
        if err != nil {
            handleServerError(w, "Failed to parse mock auth attempt cookie: " + err.Error())
            return
        }
        http.SetCookie(w, retCookie)
    } else if err == nil {
        http.SetCookie(w, c)
    } else {
        handleServerError(w, err.Error())
    }
}

func (a *MockAuthentikServer) handleAuthRequestPost(w http.ResponseWriter, r *http.Request) {
    // If no auth attempt cookie, send error
    if c, err := r.Cookie(authentik_util.AUTH_ATTEMPT_COOKIE_NAME); err == http.ErrNoCookie {
        handleServerError(w, "No auth attempt cookie")
        return
    } else if err == nil {
        // Responses include the original cookie
        http.SetCookie(w, c)
    } else if err != nil {
        handleServerError(w, err.Error())
        return
    }

    // Get login request info
    var loginInfo authentik_util.LoginRequest
    loginData, err := ioutil.ReadAll(r.Body)
    if err != nil {
        handleServerError(w, fmt.Sprintf("Failed to read login request data: %s", err.Error()))
        return
    }
    if err = json.Unmarshal(loginData, &loginInfo); err != nil {
        handleServerError(w, fmt.Sprintf("Failed to parse login request struct: %s", err.Error()))
        return
    }
    if loginInfo.Stage == authentik_util.AUTH_STAGE_IDENTIFICATION {
        if len(loginInfo.Username) == 0 {
            handleServerError(w, "No username provided")
            return
        }

        w.Write([]byte(fmt.Sprintf(AUTH_STAGE_ID_RESP_TEMPLATE, loginInfo.Username)))
    } else if loginInfo.Stage == authentik_util.AUTH_STAGE_PASSWORD {
        if len(loginInfo.Password) == 0 {
            handleServerError(w, "No password provided")
            return
        }

        if loginInfo.Password == MOCK_PASSWORD {
            w.Write([]byte(fmt.Sprintf(AUTH_STAGE_PASSWORD_RESP_SUCCESS_TEMPLATE, MOCK_USERNAME)))
        } else {
            w.Write([]byte(fmt.Sprintf(AUTH_STAGE_PASSWORD_RESP_FAILURE_TEMPLATE, MOCK_USERNAME)))
        }
    } else if loginInfo.Stage == authentik_util.AUTH_STAGE_MFA {
        if len(loginInfo.Token) == 0 {
            handleServerError(w, "No token provided")
            return
        }

        if loginInfo.Token == MOCK_TOKEN {
            csrfCookie, err := http.ParseSetCookie(MOCK_CSRF_COOKIE)
            if err != nil {
                handleServerError(w, "Failed to parse mock CSRF cookie")
                return
            }
            http.SetCookie(w, csrfCookie)
            w.Write([]byte(AUTH_STAGE_MFA_RESP_SUCCESS))
        } else {
            w.Write([]byte(fmt.Sprintf(AUTH_STAGE_MFA_RESP_FAILURE_TEMPLATE, MOCK_USERNAME)))
        }
    } else {
        handleServerError(w, fmt.Sprintf("Unsupported login stage: %s", loginInfo.Stage))
    }
}

func handleServerError(w http.ResponseWriter, msg string) {
    w.WriteHeader(http.StatusInternalServerError)
    w.Write([]byte(msg))
}
