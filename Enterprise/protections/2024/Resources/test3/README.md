# Protections Test 3 Binary

This protections test binary tests vendors abilities to block exfiltration via HTTP of data in 1MB chunks that has been packaged into zip archives on Linux.

## Usage

- Ensure you have a HTTP server running to receive the zip archives.

- Set the `TARGET_DIRECTORY` and `SERVER_URL` variables to the correct values.

- From your linux terminal, run the binary:

    ```cmd
    ./test3
    ```

## Features

- Enumerates `/users/` directory for `Desktop`, `Downloads`, and `Documents` subdirectories.
- Packages all found subdirectories into 1MB zip archives.
- Archives are created in the binary's current working directory.
- Uploads the created zip archives to an HTTP server using `POST` requests. The file name is included as an HTTP header `filename`.

## Logging

For operator usability, the protections test3 binary will log in plaintext to the console. Error messages
are written in red and success messages are written in green.

## Build

From your Linux terminal, pull the dependencies and then build the binary:

```cmd
go get .
go build -o test3 main.go
```

## Cleanup

- From the same directory you ran the executable from, run the following commands to remove
the zip archives and the executable, and check that all of the files have been removed successfully.

    ```cmd
    rm archive*
    rm test3
    ls
    ```

- Kill your HTTP server

### Dependencies

- `golang` version `1.22.2`
