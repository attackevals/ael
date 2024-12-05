package http_util

import (
    "bytes"
    "fmt"
    "io"
    "io/ioutil"
    "net/http"
)

// Used for mocking HTTP GET request in unit tests. Takes URL string parameter and additional headers and returns response bytes or error.
type HttpGetRequestResponseGetter func(string, map[string]string) ([]byte, error)

// Used for mocking HTTP GET request in unit tests.. Takes URL, content type, additional headers, and POST data parameter, and returns response bytes or error.
type HttpPostRequestResponseGetter func(string, string, map[string]string, []byte) ([]byte, error)

func PerformHttpGet(url string, extraHeaders map[string]string) ([]byte, int, error) {
    req, err := http.NewRequest("GET", url, nil)
    if err != nil {
        return nil, -1, err
    }
    if extraHeaders != nil {
        for header, value := range extraHeaders {
            req.Header.Add(header, value)
        }
    }
    client := &http.Client{}
    resp, err := client.Do(req)
    if err != nil {
        return nil, -1, err
    }
    defer resp.Body.Close()
    body, err := io.ReadAll(resp.Body)
    if err != nil {
        return nil, -1, err
    }
    return body, resp.StatusCode, nil
}

func PerformHttpGetAssertSuccess(url string, extraHeaders map[string]string) ([]byte, error) {
    resp, statusCode, err := PerformHttpGet(url, extraHeaders)
    if err != nil {
    	return nil, err
    }
    if statusCode != http.StatusOK {
        return nil, fmt.Errorf("Expected error code 200, got %d", statusCode)
    }
    return resp, nil
}

// Perform HTTP post. Returns an error if non-200 status code is returned.
func PerformHttpPostAssertSuccess(url string, contentType string, extraHeaders map[string]string, data []byte) ([]byte, error) {
    resp, statusCode, err := PerformHttpPost(url, contentType, extraHeaders, data)
    if err != nil {
    	return nil, err
    }
    if statusCode != http.StatusOK {
        return nil, fmt.Errorf("Expected error code 200, got %d", statusCode)
    }
    return resp, nil
}

func performHttpReqWithData(url string, reqType string, contentType string, extraHeaders map[string]string, data []byte) ([]byte, int, error) {
    // setup HTTP request
    req, err := http.NewRequest(reqType, url, bytes.NewBuffer(data))
    if err != nil {
        return nil, -1, err
    }
    if len(contentType) > 0 {
        req.Header.Set("Content-Type", contentType)
    }
    if extraHeaders != nil {
        for header, value := range extraHeaders {
            req.Header.Add(header, value)
        }
    }

    // execute HTTP request and read response
    client := &http.Client{}
    response, err := client.Do(req)
    if err != nil {
        return nil, -1, err
    }
    defer response.Body.Close()
    body, err := ioutil.ReadAll(response.Body)
    if err != nil {
        return nil, -1, err
    }
    return body, response.StatusCode, nil
}

func PerformHttpPost(url string, contentType string, extraHeaders map[string]string, data []byte) ([]byte, int, error) {
    return performHttpReqWithData(url, "POST", contentType, extraHeaders, data)
}

func PerformHttpPut(url string, contentType string, extraHeaders map[string]string, data []byte) ([]byte, int, error) {
    return performHttpReqWithData(url, "PUT", contentType, extraHeaders, data)
}

// Perform HTTP delete. Returns an error if non-200 status code is returned.
func PerformHttpDeleteAssertSuccess(url string, extraHeaders map[string]string) ([]byte, error) {
    resp, statusCode, err := PerformHttpDelete(url, extraHeaders)
    if err != nil {
    	return nil, err
    }
    if statusCode != http.StatusOK {
        return nil, fmt.Errorf("Expected error code 200, got %d", statusCode)
    }
    return resp, nil
}

func PerformHttpDelete(url string, extraHeaders map[string]string) ([]byte, int, error) {
    client := &http.Client{}
    req, err := http.NewRequest("DELETE", url, nil)
    if err != nil {
        return nil, -1, err
    }
    if extraHeaders != nil {
        for header, value := range extraHeaders {
            req.Header.Add(header, value)
        }
    }
    response, err := client.Do(req)
    if err != nil {
        return nil, -1, err
    }
    defer response.Body.Close()
    body, err := ioutil.ReadAll(response.Body)
    if err != nil {
        return nil, -1, err
    }
    return body, response.StatusCode, nil
}
