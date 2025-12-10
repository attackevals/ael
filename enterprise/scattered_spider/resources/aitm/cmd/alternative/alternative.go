package main

import (
	"errors"
    "flag"
	"fmt"
	"net/http"
    "os"

	"aitm/pkg/authentik_util"
    "aitm/pkg/logger"
)

func ObtainSessionCookie(authentikUrl string, username string, password string, token string) error {
    reqHeaders := map[string]string{
        "User-Agent": authentik_util.USER_AGENT,
    }

    // Get auth attempt cookie to use in future requests
    authAttemptCookie, err := authentik_util.GetAuthAttemptCookie(authentikUrl, username, reqHeaders)
    if err != nil {
        return errors.New(fmt.Sprintf("Failed to get auth attempt cookie for user %s: %s", username, err.Error()))
    }
    reqCookieStr := fmt.Sprintf("%s=%s", authentik_util.AUTH_ATTEMPT_COOKIE_NAME, authAttemptCookie)

    // Submit username
    _, statusCode, err := authentik_util.SubmitUsername(authentikUrl, username, reqCookieStr, reqHeaders)
    if err != nil {
        return errors.New(fmt.Sprintf("Failed to submit username %s to server: %s", username, err.Error()))
    } else if statusCode != http.StatusOK {
        return errors.New(fmt.Sprintf("Received non-200 HTTP status code when submitting username: %d", statusCode))
    }

    // Submit password
    _, statusCode, err = authentik_util.SubmitPassword(authentikUrl, username, password, reqCookieStr, reqHeaders)
    if err != nil {
        return errors.New(fmt.Sprintf("Failed to submit password for user %s to server: %s", username, err.Error()))
    } else if statusCode != http.StatusOK {
        return errors.New(fmt.Sprintf("Received non-200 HTTP status code when submitting password: %d", statusCode))
    }

    // Submit MFA
    _, statusCode, err = authentik_util.SubmitMfaToken(authentikUrl, username, token, reqCookieStr, reqHeaders)
    if err != nil {
        return errors.New(fmt.Sprintf("Failed to submit MFA token for user %s to server: %s", username, err.Error()))
    } else if statusCode != http.StatusOK {
        return errors.New(fmt.Sprintf("Received non-200 HTTP status code when submitting MFA token: %d", statusCode))
    }

    return nil
}

func main() {
    authentikUrl := flag.String("targetUrl", "", "Base Authentik server URL to target (e.g https://auth.romeo.com:9999).")
	username := flag.String("user", "", "Username for authentication.")
	password := flag.String("password", "", "Password for authentication.")
	token := flag.String("token", "", "MFA token for authentication.")
    flag.Parse()

    if len(*authentikUrl) == 0 {
		logger.Error("Authentik URL required - please provide.")
		os.Exit(1)
	} else if len(*username) == 0 {
		logger.Error("Username required - please provide.")
		os.Exit(1)
	} else if len(*password) == 0 {
		logger.Error("Password required - please provide.")
		os.Exit(1)
	} else if len(*token) == 0 {
		logger.Error("MFA token required - please provide.")
		os.Exit(1)
	}

    err := ObtainSessionCookie(*authentikUrl, *username, *password, *token)
    if err != nil {
        logger.Error(fmt.Sprintf("Failed to obtain session cookie: %s", err.Error()))
		os.Exit(2)
    } else {
		logger.Success("Successfully obtained session cookie!")
    }
}
