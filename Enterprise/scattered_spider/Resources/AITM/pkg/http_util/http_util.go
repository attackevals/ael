package http_util

import (
    "bytes"
    "crypto/tls"
    "io"
    "io/ioutil"
    "net/http"
)

type IServer interface {
    StartServer() error
    StopServer() error
}

func PerformHttpGet(url string, cookieStr string, extraHeaders map[string]string) ([]byte, int, []*http.Cookie, error) {
    req, err := http.NewRequest("GET", url, nil)
    if err != nil {
        return nil, -1, nil, err
    }
    if extraHeaders != nil {
        for header, value := range extraHeaders {
            req.Header.Add(header, value)
        }
    }
    if len(cookieStr) > 0 {
        cookies, err := http.ParseCookie(cookieStr)
        if err != nil {
            return nil, -1, nil, err
        }
        for _, cookie := range cookies {
            req.AddCookie(cookie)
        }
    }
    // Skip TLS cert checks
    tr := &http.Transport{TLSClientConfig: &tls.Config{InsecureSkipVerify: true}}
    client := &http.Client{Transport: tr}
    resp, err := client.Do(req)
    if err != nil {
        return nil, -1, nil, err
    }
    defer resp.Body.Close()
    body, err := io.ReadAll(resp.Body)
    if err != nil {
        return nil, -1, nil, err
    }
    return body, resp.StatusCode, resp.Cookies(), nil
}

func performHttpReqWithData(url string, reqType string, contentType string, cookieStr string, extraHeaders map[string]string, data []byte) ([]byte, int, []*http.Cookie, error) {
    // setup HTTP request
    req, err := http.NewRequest(reqType, url, bytes.NewBuffer(data))
    if err != nil {
        return nil, -1, nil, err
    }
    if len(contentType) > 0 {
        req.Header.Set("Content-Type", contentType)
    }
    if extraHeaders != nil {
        for header, value := range extraHeaders {
            req.Header.Add(header, value)
        }
    }
    if len(cookieStr) > 0 {
        cookies, err := http.ParseCookie(cookieStr)
        if err != nil {
            return nil, -1, nil, err
        }
        for _, cookie := range cookies {
            req.AddCookie(cookie)
        }
    }

    // execute HTTP request and read response
    tr := &http.Transport{TLSClientConfig: &tls.Config{InsecureSkipVerify: true}}
    client := &http.Client{Transport: tr}
    response, err := client.Do(req)
    if err != nil {
        return nil, -1, nil, err
    }
    defer response.Body.Close()
    body, err := ioutil.ReadAll(response.Body)
    if err != nil {
        return nil, -1, nil, err
    }
    return body, response.StatusCode, response.Cookies(), nil
}

func PerformHttpPost(url string, contentType string, cookieStr string, extraHeaders map[string]string, data []byte) ([]byte, int, []*http.Cookie, error) {
    return performHttpReqWithData(url, "POST", contentType, cookieStr, extraHeaders, data)
}
