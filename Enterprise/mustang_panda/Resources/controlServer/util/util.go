package util

import (
    "os"
    "path/filepath"
)

var (
    ProjectRoot string
    UploadDir   string
)

func init() {
    SetRootDirectories()
}

func SetRootDirectories() {
    var err error
    ProjectRoot, err = os.Getwd()
    if err != nil {
        panic(err)
    }
    UploadDir = filepath.Join(ProjectRoot, "files")
}
