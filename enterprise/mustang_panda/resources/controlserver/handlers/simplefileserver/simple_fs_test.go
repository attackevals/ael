package simplefileserver

import (
    "bytes"
    "crypto/md5"
    "encoding/hex"
    "fmt"
    "io/ioutil"
    "net/http"
    "os"
    "sync"
    "testing"

    "evals.mitre.org/control_server/config"
    "evals.mitre.org/control_server/restapi/restapi_test_util"
    "evals.mitre.org/control_server/util/test_util"
    "evals.mitre.org/control_server/util/test_util/assert_util"
)

const (
    REST_API_LISTEN_HOST = "127.0.0.1:10000"
    BASE_SFS_URL         = "http://127.0.0.1:10001"
    UPLOAD_URL           = BASE_SFS_URL + "/uploads"
)

var (
    testConfigEntry = config.HandlerConfigEntry{
        "host":    "127.0.0.1",
        "port":    10001,
        "enabled": false,
        "mappings": []interface{}{
            map[string]interface{}{"prefix": "/files", "dir": "./testsfs"},
            map[string]interface{}{"prefix": "/files2", "dir": "./testsfs2"},
        },
    }
    testDirs       = []string{"./testsfs/subdir/level2", "./testsfs2"}
    parentTestDirs = []string{"./testsfs", "./testsfs2"}
    testFiles      = [][]string{
        []string{"./testsfs/test.txt", "test file 1 contents"},
        []string{"./testsfs/subdir/test2.txt", "test file 2 contents"},
        []string{"./testsfs/subdir/level2/test3.txt", "test file 3 contents"},
        []string{"./testsfs2/test4.txt", "test file 4 contents"},
    }
)

func setupTestDirs(t *testing.T) error {
    for _, dir := range testDirs {
        err := os.MkdirAll(dir, 0750)
        if err != nil {
            return err
        }
        t.Logf("Created test dir %s", dir)
    }
    for _, fileInfo := range testFiles {
        err := os.WriteFile(fileInfo[0], []byte(fileInfo[1]), 0644)
        if err != nil {
            return err
        }
        t.Logf("Created test file %s", fileInfo[0])
    }
    return nil
}

func cleanupTestDirs(t *testing.T) {
    for _, dir := range parentTestDirs {
        err := os.RemoveAll(dir)
        if err != nil {
            t.Error(err)
        }
    }
}

func startSimpleFileServerHandler(handler *SimpleFileServerHandler, t *testing.T) {
    err := setupTestDirs(t)
    if err != nil {
        cleanupTestDirs(t)
        t.Fatal(err)
    }
    test_util.StartHandlerForUnitTest(t, handler, REST_API_LISTEN_HOST, testConfigEntry)
}

func stopSimpleFileServerHandler(handler *SimpleFileServerHandler, t *testing.T) {
    test_util.StopHandlerForUnitTest(t, handler)
    cleanupTestDirs(t)
}

func uploadFile(url string, filename string, filedata []byte) (string, error) {
    // setup HTTP POST request
    req, err := http.NewRequest("POST", url, bytes.NewReader(filedata))
    if err != nil {
        return "", err
    }
    req.Header.Set("filename", filename)

    // execute HTTP POST and read response
    client := &http.Client{}
    response, err := client.Do(req)
    if err != nil {
        return "", err
    }
    defer response.Body.Close()
    if response.StatusCode != 200 {
        return "", fmt.Errorf("Expected error code 200, got %v", response.StatusCode)
    }
    body, err := ioutil.ReadAll(response.Body)
    return string(body[:]), err
}

func getAndCheckFileRequest(t *testing.T, filepath string, expectedHash string, expectedRespCode int) {
    url := BASE_SFS_URL + filepath
    resp, err := http.Get(url)
    if err != nil {
        t.Fatal(err)
    }
    defer resp.Body.Close()
    assert_util.AssertEq(t, resp.StatusCode, expectedRespCode, "HTTP status code mismatch")
    if expectedRespCode == http.StatusOK {
        body, err := ioutil.ReadAll(resp.Body)
        if err != nil {
            t.Fatal(err)
        }
        h := md5.Sum(body)
        actualHash := hex.EncodeToString(h[:])
        assert_util.AssertEq(t, expectedHash, actualHash, "File hash mismatch")
    }
}

func TestStartStopSimpleFileServerHandler(t *testing.T) {
    handler := simpleFileServerHandlerFactory(&sync.WaitGroup{})
    startSimpleFileServerHandler(handler, t)
    defer stopSimpleFileServerHandler(handler, t)
}

func TestUpload(t *testing.T) {
    restapi_test_util.StartRestApiServer(t, "", REST_API_LISTEN_HOST, nil)
    defer restapi_test_util.StopRestApiServer()

    handler := simpleFileServerHandlerFactory(&sync.WaitGroup{})
    startSimpleFileServerHandler(handler, t)
    defer stopSimpleFileServerHandler(handler, t)

    filename := "sfstestupload.txt"
    contents := []byte("sfstestuploadcontent")
    resp, err := uploadFile(UPLOAD_URL, filename, contents)
    if err != nil {
        t.Fatal(err)
    }
    defer test_util.RemoveUploadedFile(t, filename)
    wantResp := "OK"
    assert_util.AssertEq(t, wantResp, resp, "")
    test_util.CheckUploadedFileHash(t, filename, "6dd308d964400a9e2c6ab60b1cec0781")
}

func TestNoDirListing(t *testing.T) {
    handler := simpleFileServerHandlerFactory(&sync.WaitGroup{})
    startSimpleFileServerHandler(handler, t)
    defer stopSimpleFileServerHandler(handler, t)

    toCheck := []string{
        "/",
        "/files",
        "/files/",
        "/nonexistent/",
        "/files2",
        "/files2/",
        "/files/subdir",
        "/files/subdir/",
        "/files2/subdir",
        "/files2/subdir/",
        "/files/subdir/test",
        "/files/subdir/test/",
    }

    for _, dir := range toCheck {
        t.Logf("Checking dir %s", dir)
        getAndCheckFileRequest(t, dir, "", http.StatusNotFound)
    }
}

func TestGetFiles(t *testing.T) {
    handler := simpleFileServerHandlerFactory(&sync.WaitGroup{})
    startSimpleFileServerHandler(handler, t)
    defer stopSimpleFileServerHandler(handler, t)

    existing := [][]string{
        []string{"/files/test.txt", "7bf785b63975cb6be85ac8a005e801a9"},
        []string{"/files/subdir/test2.txt", "0ccc9b1a435e7d40a91ac7dd04c67fe8"},
        []string{"/files/subdir/level2/test3.txt", "132c0a2736faaffbde55e390e25cea8d"},
        []string{"/files2/test4.txt", "339c8547e573647a80bf49a5154035d8"},
    }

    nonexisting := []string{
        "/files/a",
        "/files/subdir/a",
        "/files/a/subdir/b",
        "/files2/subdir/a",
        "/a/b",
        "/c",
        "/files/subdir/level2/a",
    }

    for _, fileEntry := range existing {
        t.Logf("Checking file %s", fileEntry[0])
        getAndCheckFileRequest(t, fileEntry[0], fileEntry[1], http.StatusOK)
    }

    for _, file := range nonexisting {
        t.Logf("Making sure file %s does not exist", file)
        getAndCheckFileRequest(t, file, "", http.StatusNotFound)
    }
}
