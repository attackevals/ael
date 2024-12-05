package util

import (
    "path/filepath"
    "runtime"
)

// For calculating project root directory
var (
    _, base, _, _ = runtime.Caller(0)
    ProjectRoot = filepath.Join(filepath.Dir(base), "../")
    UploadDir = filepath.Join(ProjectRoot, "files")
)
