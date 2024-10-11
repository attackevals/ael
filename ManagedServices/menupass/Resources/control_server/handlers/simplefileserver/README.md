# Simple File Server

The Simple File Server Handler allows file downloads and file uploads.

## Features

- Supports file downloads via HTTP requests to endpoints specified in the handler configuration entry. Multiple URL prefixes are supported.
  - Supports downloading via curl, certutil.exe, bitsadmin
  - If no prefixes are provided in the configuration entry, the default will be to serve files from `handlers/simplefileserver/files` at URL prefix `/files`
- Supports file uploads via HTTP POST request to the `/uploads` endpoint
  - Requires sending file content via POST body and file name via `filename` HTTP header
- Prevents directory listing for file endpoints.

## Usage

### Configuration

To enable and configure the Simple File Server handler within the control server, edit the corresponding handler config file within the `config/`
directory in the main C2 server repo. Create or adjust an existing handler entry as needed.

Example:

```
simplefileserver:
  host: 10.0.2.7
  port: 80
  mappings:
    - prefix: /files
      dir: /tmp/mypayloads
    - prefix: /files2
      dir: /tmp/mypayloads2
  enabled: true
```

for each entry in `mappings`, `prefix` specifies the URL prefix for file requests, and `dir` specifies the path on disk to fetch files from. Subdirectories are automatically supported as long as they exist on disk. *Do not* add a `/` at the end of a `prefix` value.

Run the `controlServer` binary as `sudo` and look for success messages in starting up the Simple File Server handler:

```
sudo ./controlServer -c config/yourconfigfile.yml
```
