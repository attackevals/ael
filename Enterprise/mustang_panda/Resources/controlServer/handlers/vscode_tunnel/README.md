# VSCode Tunnel Code Handler

This handler will handle HTTP POST requests containing the `code tunnel` VSCode tunnel command output with the Github authentication code.
The handler will extract the authentication code and log it for the operator to see and use in a browser to connect to the tunnel.

The authentication code is expected to be of the format `XXXX-XXXX`, where each `X` is a hex digit (0-9, A-F).

## Components

The handler consists of an HTTP listener that listens on a specified address/port.
The single URL that it supports is `/code`, which only handles POST requests.

## Usage

### Building

To build the control server binary, run the following command from the `evalsC2server` directory:

```shell
go build -o controlServer main.go
```

### Configuration

To enable and configure the handler within the control server, provide a `vscode_tunnel` entry in your handler configuration YAML file (see example below).
Adjust the host and port values as necessary.

Example:

```yaml
vscode_tunnel:
  host: 10.100.0.70
  port: 8080
  enabled: true
```

Run the `controlServer` binary as `sudo` and monitor the output to see handler updates. Ex:

```shell
sudo ./controlServer -c ./config/your_handler_config.yml
```

### Testing

To run unit tests, run the following command from the `evalsC2server` directory:

```shell
sudo go test ./...
```

To run only a single test, navigate to the `evalsC2server\handlers\vscode_tunnel` directory and run:

```shell
sudo go test -run NameOfTestFunction
```

### Troubleshooting

If the authentication code is not received or if the text blob is malformed, verify the associated batch script logs on the victim workstation to see if the `code tunnel` command failed for any reason:

- `code tunnel` command output is logged to `C:\Windows\temp\startcode.dat`
- general batch script logs are located at `C:\Windows\temp\startcode.log`

## CTI References

1. <https://unit42.paloaltonetworks.com/stately-taurus-abuses-vscode-southeast-asian-espionage/>
