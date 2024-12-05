# Protections Test 8 Binary

This protections test binary tests blocking propagation via net session
enumeration and remote scheduled tasks.

## Usage

This binary takes no arguments. Simply execute it directly via the command-line.

## Features

- Enumerates network sessions established on the initial server via `NetSessionEnum` API call
- For each discovered remote host that has an established network session to the server, the binary will
  copy itself to `C:\Windows\System32` on the remote system and create a remote scheduled task on the machine
  using Windows API calls.
- The created scheduled tasks have the following details:
  - Task name/path: `\Microsoft\Windows\Net Session File Management Task`
  - Task author: `Microsoft Corporation`
  - Task source: `Microsoft Corporation`
  - Description: `This task manages remote net share connections.`
  - Runs as SYSTEM user with highest available privileges
  - Will execute 15 seconds after task registration
  - Executes the copied protections test binary at `C:\Windows\System32`
- Logs output to `test8.log` in the current working directory.

## Logging

For operator usability, the protections test binary will log in plaintext `test8.log`
in the current working directory.

## Build

The protections test binary can be built in Debug or Release mode using the included `CMakePreset.json` configurations.

### Dependencies

- `CMake` version `3.26`
- `CMakePresets.json` version `6` support (Visual Studio 17.7)

### Quickstart

#### Command Line

Build both Debug and Release configurations of the binary. This will also kick off the GoogleTest unit tests.

Note - in order for all unit tests to succeed, the tests will need to be run in an elevated context, since some tests
will create and modify test services.

```PowerShell
cmake.exe --workflow --preset cicd-debug
cmake.exe --workflow --preset cicd-release
```

For simplicity, all of the generated artifacts can be bundled into the
top-level `install/` directory using the CMake installation facility.

```PowerShell
cmake.exe --install ./build --config release
cmake.exe --install ./build --config debug
```

You can then run the binary from the command line:

```PowerShell
.\install\Debug\main.exe

.\install\Release\main.exe
```

## Install

Installing the test binary simply copies all of the artifacts from both Release
and Debug builds into the top-level `install/` directory. This is simply for
ease of use and distributability.

The final product should be a top-level directory named `install/` with the
following layout:

```PowerShell

test8/
├─ install/
   ├─ Debug/
   │  ├─ main.exe
   │  ├─ main.pdb
   ├─ Release/
      ├─ main.exe
```

## Test 🧪

### Unit Tests

GoogleTest unit tests will automatically run when using the `cicd-debug` and `cicd-release` presets

You can also run them ad-hoc by running:

```PowerShell
ctest.exe --preset all
```
