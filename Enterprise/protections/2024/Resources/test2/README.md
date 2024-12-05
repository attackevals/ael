# Protections Test 2 Binary

This protections test binary tests blocking exfiltration via SFTP of data that has been packaged into zip archives.

## Usage

This binary takes no arguments. Simply compile it and execute it directly via the command-line:

```PowerShell
test2.exe
```

## Features

- Searches `C:\Users\` for `Desktop`, `Downloads`, and `Documents` subdirectories.
- Packages found directories into zip archives with 15-30 seconds of jitter between creation of archives.
- Archives are created in the binary's current working directory.
- Uploads created zip archives to an SFTP server with 15-30 seconds of jitter between uploads.

## Logging

For operator usability, the protections test binary will log in plaintext to the console.

## Build

Pull dependencies, then build:

```PowerShell
go get .
go build -o test2.exe main.go
```

### Dependencies

- `golang` version `1.22.2`
- `github.com/pkg/sftp` version `v1.13.6`
- `golang.org/x/crypto` version `v0.1.0`

### Quickstart

- `const DEBUG` can be modified to enable (`false`) or disable (`true`) the wait between zip creation and SFTP upload.
- `target_folder` can be modified to search a different directory than `C:\Users\`.
- `user`, `pass`, and `host` can be updated to use a different SFTP server and account credentials.
