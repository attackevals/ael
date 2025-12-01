---
order: C
---

# ATT&CK Evaluations Control Server

This ATT&CK Evaluations Control Server is used to execute behaviors under test during ATT&CK Evaluations.

The Control Server provides features for generating implant stagers, controlling agents, and executing modules.

The Control Server is backed with a REST API to support automation and integration with other tools.

## Usage

This repository is meant to act as a submodule within other adversary emulation or ATT&CK evaluation repositories. To use this control server repository, you may either run the binary as is or reference the entire repository as a submodule.

The control server expects the following folder structure for payloads/uploads:

- File uploads will be stored in the `files` subdirectory of the current working directory of the C2 server. The repository already has this directory available, but you will need to create the folder before running the binary from a different location.
- Payload downloads will be fetched from the `payloads` folder of the parent directory of the C2 server's working location. The repository does not contain this payload directory, since the end user will be responsible for providing the payloads according to their specific use case. Ensure that this folder exists in the parent directory when running the C2 server. For example, if the control server is being run from the `Resources/control_server` directory, the payloads must be placed in `Resources/payloads/` in order for the handlers to successfully serve them.
Note that some handlers expect their payloads to be in a separate subdirectory under the `payloads` directory (e.g. `Resources/payloads/handlername`)

The control server is configured using two configuration YAML files - one for handler configuration and one for REST API configuration. By default, the control server will
pull the REST API config from `./config/restAPI_config.yml`. The handler configuration file is a required argument and must be provided using the `-c` or `--config` command-line flags.
A sample handler configuration file is provided in `./config/sample_handler_config.yml` as a reference only.

To change the REST API config path, use the `-r` or `--rest-config` command-line flags.

To run the REST API with a SQL database, use the `-db` command-line flag. To reinitialize the REST API from a database, use the `--existing-db` command-line flag. To specify the
database path, add an entry in your REST API configuration file with entry `dbPath` [here](./config/restAPI_config.yml#L2). By default the REST API will **not** be initialized with a database.

Example:

```shell
# start control server with an existing database

sudo ./controlServer -c ./config/myhandlerconfig.yml -r ./config/myrestapiconfig.yml -db=true --existing-db=true
```

## Build from source

```shell
go build -o controlServer main.go
```

To run with default REST API config path:

```shell
sudo ./controlServer -c ./config/myhandlerconfig.yml
```

To specify REST API config path:

```shell
sudo ./controlServer -c ./config/myhandlerconfig.yml -r ./config/myrestapiconfig.yml
sudo ./controlServer --config ./config/myhandlerconfig.yml --rest-config ./config/myrestapiconfig.yml
```

To start the server with an existing database:

```shell
sudo ./controlServer -c ./config/myhandlerconfig.ymle --existing-db=true
```

## Test Instructions

```shell
sudo go test ./...
```

Note: you may need to specify the full path to your golang binary if running under `sudo`.

For example:

```shell
sudo /usr/local/go/bin/go test ./...
```

## Usage Examples

1. Enable C2 handlers and adjust configuration values to change IP address and ports to listen on by providing your own handler config YAML file.
To enable a handler, set `enabled` to `true`, like below:

```yaml
handlername:
  host: 192.168.0.8
  port: 443
  enabled: true
```

To disable a handler, set `enabled` to `false`, like below:

```yaml
handlername:
  host: 192.168.0.8
  port: 443
  enabled: false
```

See `./config/sample_handler_config.yml` for an example YAML file structure.

1. Start the control server:

To run with default REST API config path:

```shell
sudo ./controlServer -c ./config/myhandlerconfig.yml
```

To specify REST API config path:

```shell
sudo ./controlServer -c ./config/myhandlerconfig.yml -r ./config/myrestapiconfig.yml
sudo ./controlServer --config ./config/myhandlerconfig.yml --rest-config ./config/myrestapiconfig.yml
```

To run server and create a new database:

```shell
sudo ./controlServer -c ./config/myhandlerconfig.yml -db=true
```

Alternatively run directly from source:

```shell
go run main.go -c ./config/myhandlerconfig.yml
```

Note: you may need to specify the full path to your golang binary if running under `sudo`.

For example:

```shell
sudo /usr/local/go/bin/go run main.go -c ./config/myhandlerconfig.yml
```

## Installation Dependencies

Install Go version 1.15 or higher.

```shell
sudo apt-get install golang
```

Run tests from the main repository directory. Go should automatically fetch needed dependencies.

```shell
sudo go test ./...
```

Note: you may need to specify the full path to your golang binary if running under `sudo`.

For example:

```shell
sudo /usr/local/go/bin/go test ./...
```

Look at the go.mod file if you want to see the dependencies in detail.

## ATT&CK Evaluations C2 Client

This client is provided to interact with the C2 server via its REST API.

## Build Instructions

Install dependencies using pip3:

```shell
pip3 install -r requirements.txt
```

## Usage Examples

1. Enable C2 handlers and adjust configuration values to change IP address and ports to listen on by providing your own handler config YAML file.
To enable a handler, set `enabled` to `true`, like below:

```yaml
handlername:
  host: 192.168.0.8
  port: 443
  enabled: true
```

To disable a handler, set `enabled` to `false`, like below:

```yaml
handlername:
  host: 192.168.0.8
  port: 443
  enabled: false
```

1. Start the control server:

```shell
sudo ./controlServer -c ./config/myhandlerconfig.yml
```

To specify REST API config path:

```shell
sudo ./controlServer -c ./config/myhandlerconfig.yml -r ./config/myrestapiconfig.yml
sudo ./controlServer --config ./config/myhandlerconfig.yml --rest-config ./config/myrestapiconfig.yml
```

To run server and create a new database:

```shell
sudo ./controlServer -c ./config/myhandlerconfig.yml -db=true
```

1. Establish an implant session

Run an implant program of your choice to connect to the C2 (make sure that the corresponding handler was enabled).

1. Run the client python script to manage implant sessions.

```shell
# view help
./evalsC2client.py --help

# list sessions; pay attention to session guids
./evalsC2client.py --get-sessions

# get detailed info for a session
./evalsC2client.py --get-session <guid>

# delete a session
./evalsC2client.py --del-session <guid>

# set session task
./evalsC2client.py --set-task <guid> <cmd>

# get session task
./evalsC2client.py --get-task <guid>

# delete session task
./evalsC2client.py --del-task <guid>

# get task output
./evalsC2client.py --get-output <guid>

# set a bootstrap task for a handler
./evalsC2client.py --set-bootstrap-task <handler> <cmd>

# get current bootstrap task for a handler
./evalsC2client.py --get-bootstrap-task <handler>

# delete current bootstrap task for a handler
./evalsC2client.py --del-bootstrap-task <handler>

# set a task for a session and then wait for it to complete, using default timeout of 120 seconds. Will return the task output or an error message
./evalsC2client.py --set-and-complete-task <session guid> <cmd>

# set a task for a session and then wait for it to complete, using a specified timeout value in seconds
./evalsC2client.py --set-and-complete-task <session guid> <cmd> --task-wait-timeout 180

# set a task for a session and then wait for it to complete, using verbose output
./evalsC2client.py --set-and-complete-task <session guid> <cmd> -v
./evalsC2client.py --set-and-complete-task <session guid> <cmd> --verbose
```

For specific instructions on tasking a particular implant, reference the appropriate README.
