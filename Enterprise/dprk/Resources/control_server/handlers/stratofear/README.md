# STRATOFEAR HTTPS Handler

The STRATOFEAR HTTPS Handler functions as the server-side counterpart to the STRATOFEAR implant, and communicates with it over HTTPS. The handler is configured to do the following:

- respond to implant beacons / task requests with a task command
- register a new implant with the control server, or indicate that a session already exists for the implant
- process the task response data and upload to the control server
- accept tasking from `evalsC2client.py` and send to implants when requested

## Components

The handler consists of an HTTP/S web server that listens on a specified address/port, and serves the following URL endpoints:

| URL Endpoint | Server Response | Description |
| ------------ | --------------- |------------ |
| `POST /` | Heartbeat (0x60) response | Initial implant registration |
| `GET /directory/v1/{identifier}` | Heartbeat (0x60) response, Implant task | Heartbeat / task request |
| `POST /directory/v1/{identifier}` | Heartbeat (0x60) response | Task response from implant |

The first packet from the STRATOFEAR implant will contain its UUID in the POST body.

## Encryption

All communication to/from the STRATOFEAR implant will be Base64 encoded and XOR'd. The entire JSON packet will be encoded.

## Usage

### Building

To build the control server binary, run the following command from the `evalsC2server` directory:

```
go build -o controlServer main.go
```

### Configuration

To enable and configure the STRATOFEAR HTTPS handler within the control server, provide a `STRATOFEAR` entry in your handler configuration YAML file (see example below). Adjust the host and port values as necessary.

HTTP example:

```
STRATOFEAR:
  host: 10.0.2.11
  port: 8080
  https: false
  enabled: true
```

HTTPS example:

```
STRATOFEAR:
  host: 10.0.2.11
  port: 8080
  cert_file: "path/to/cert_file"
  key_file: "path/to/key_file"
  https: true
  enabled: true
```

Run the `controlServer` binary as `sudo` and monitor the output to see STRATOFEAR handler updates. Ex:

```
sudo ./controlServer -c ./config/your_handler_config.yml
```

### Testing

Unit tests for the STRATOFEAR HTTPS handler are available in the `stratofear_test.go` file. To run these tests, run the following command from the `evalsC2server` directory:

```
sudo go test ./...
```

To run only a single test, navigate to the `evalsC2server\handlers\stratofear` directory and run:

```
sudo go test -run NameOfTestFunction
```

### Tasking

The STRATOFEAR C2 Handler does not validate the tasking string before sending it to the implant, double check to ensure your task fields are labeled correctly. To submit a task to the C2 server, pass the task information to the REST API server in a JSON dictionary string containing the following fields:

| Field | Data Type | Necessity | Description |
| ------------ | ----------- | ----------- |----------- |
| id | string | required | The ID of the task the implant will execute. A table detailing the different tasks is available below. |
| args | string | optional | Any information the implant neededs to complete the task. Ie. directory path or payload name |

The following table contains the various acceptable commands and their meanings:

| Command      | Arguments | Implant Output | Description |
| -----------  | --------- | -------------- | ----------- |
| 0x07 | None | POST request containing system name, current username, system architecture, modules available, configuration data | Collect system information (system name, current username, system's architecture), module information, and configuration data |
| 0x47 | None | None | ETF Monitoring |
| 0x60 | None | None | Heartbeat (do nothing command) |
| 0x61 | None | POST request containing module name, assigned ID, version, pointer address in memory | Retrieve module information |
| 0x62 | Dylib module bytes | None | Load module from disk and execute its `Initialize` function |
| 0x64 | None | POST request containing last saved command output OR file upload | Retrieve module execution result |
| 0x66 | Path to directory | None | Change directory |

Use the following format to submit the commands from the `evalsC2server` directory:

```
./evalsC2client.py --set-task <UUID> '{"id":"<id>", "args":"<args>"}'
```

Example commands:

- "0x07": Task system discovery

  ```
  ./evalsC2client.py --set-task <UUID> '{"id": "0x07""}'
  ```

- "0x62": Task download of module `myModule`

  ```
  ./evalsC2client.py --set-task <UUID> '{"id": "0x62", "args":"myModule"}'
  ```

- "0x64": Task retrieve module execution results

  ```
  ./evalsC2client.py --set-task <UUID> '{"id": "0x64"}'
  ```

### Troubleshooting

To assist in troubleshooting, follow the directions [here](../../DebuggingGuide.md) to set up the Go Debugger.

**General Comms Troubleshooting:**

- Double check that the implant and the C2 are both communicating via HTTP or HTTPS
- If HTTPS is enabled, ensure that the implant and the handler are using the same certificates

**Tasking Troubleshooting:**

- The handler will not validate the tasking strings sent by the operator, double check that your task string is in the correct format with the correct field names
- Ensure your dylibs are up to date

## CTI References

1. <https://www.mandiant.com/resources/blog/north-korea-supply-chain>
