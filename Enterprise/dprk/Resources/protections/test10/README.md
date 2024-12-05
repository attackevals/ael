# Protections Test 10

`test10` is an ARM64 Mach-O intended to be run as `root` that will attempt to
access passwords in the System Keychain.

Of particular note is success in retrieving the machine account's plaintext
password after the victim machine has been joined to the domain.

## Usage Examples

### Requirements

* Sonoma M1
* Victim host must be domain joined
* Remote desktop/VNC access to the victim host
* Terminal running as root

### Basic Execution

`test10` takes in no arguments and will enumerate all `genp` entries in the
System Keychain.

* In Terminal, elevate to root:

    ```zsh
    sudo su
    ```

* Execute the test10 binary:

    ```zsh
    ./test10
    ```

  * Example output:

      ```text
      [SUCCESS] Keychain is unlocked
      [DEBUG] Trying: Identity Root
      [ERROR] The user name or passphrase you entered is not correct.
      ...
      [DEBUG] Trying: searchparty
      [INFO] Password is empty
      [DEBUG] Trying: 10.100.0.10$
      [SUCCESS] 1qaz2wsx3edc4rf
      ...
      ```

## Build Instructions 🏗️

Protections Test 10 can be built in Debug or Release mode using the included
`CMakePresets.json` configurations.

### Dependencies

* `CMake` minimum version `3.26`
* `CMakePresets.json` version `6` support
* `Ninja` minimum version `1.11.1`
* `Google Test` minimum version `1.14.0` (for testing)

On the macOS host for building:

```bash
brew install cmake ninja googletest
```

### Quickstart

#### CI/CD Pipeline

TBD if we will have a build server to host the macOS runner

##### Latest compilation

TBD if we will have a build server to host the macOS runner

##### Recompile

TBD if we will have a build server to host the macOS runner

#### Build Command Line

Build both Debug and Release configurations of Protections Test 10.

```bash
cmake --workflow --preset cicd-debug
cmake --workflow --preset cicd-release
```

:bulb: This will Configure, Build and Test. To Configure only, run: `cmake --preset default`

For simplicity, all of the generated artifacts can be bundled into the
top-level `install/` directory using the CMake installation facility.

```bash
cmake --install ./build --config release
cmake --install ./build --config debug
```

`./install/Release/test10` is the production payload for Protections Test 10.

### Build Presets

#### Command Line

*NOTE: This section assumes Configuration has been completed.*

For building for debug:

```bash
cmake --build --preset debug --clean-first
```

`./build/src/Debug/test10` is the debug payload for Protections Test 10.

For building for release:

```bash
cmake --build --preset release --clean-first
```

`./build/src/Release/test10` is the production payload for Protections Test 10.

## Install 📦

Installing Protections Test 10 simply copies all of the artifacts from both Release
and Debug builds into the top-level `install/` directory. This is simply for
ease of use and distributability.

The final product should be a top-level directory named `install/` with the
following layout:

```bash
test10/
├─ install/
   ├─ Debug/
   │  ├─ test10
   ├─ Release/
   |  ├─ test10
```

### Install Command Line

```bash
cmake --install ./build
```

## Cleanup 🧹

Remove the binary:

```bash
rm test10
```

## Troubleshoot 🤔

* If the keychain is locked:
  * Check that you are running as root
  * Sometimes the default keychain gets set incorrectly:
    * Check the current setting for the default keychain:

        ```bash
        security default-keychain
        ```

      * If this returns anything other than the following, follow the next
      bullet to remove the current configuration

        | Output | Description |
        | ------ | ----------- |
        | `security: SecKeychainCopyDefault: A default keychain could not be found.` | This is normal, meaning no default keychain is set for root |
        | `"/private/var/root/Library/Keychains"` | This most likely implies the default was set in a previous run. You may remove the current configuration to see if that resolves any issues |

    * Remove the current configuration:
      * Make a copy of the plist for safekeeping:

          ```bash
          cp /var/root/Library/Preferences/com.apple.security.plist /tmp/com.apple.security.plist.tmp
          ```

      * Delete the default keychain configuration:

          ```bash
          rm /var/root/Library/Preferences/com.apple.security.plist
          ```

      * This file will get recreated if a default keychain is set later

### Return Codes

| Code | Description |
| ---- | ----------- |
| 1 | Failed to set system keychain |
| 2 | Failed to access (copy) the default keychain |
| 3 | Keychain is locked |
| 4 | Queried keychain items not found |
| 5 | Error performing query |

### Debugger

Development and debugging of Protections Test 10 was conducted using Visual Studio Code. The following extensions were required:

* C/C++
* C/C++ Extension Pack
* CMake
* CMake Tools

If using VS Code Remote Explorer, ensure developer mode is enabled for the session over SSH:

```bash
sudo DevToolsSecurity -enable
```

**NOTE:** Because `test10` is intended to run as root, the debugging must occur
as root as well for the System keychain to be accessed as expected.

Use the following `launch.json` and `tasks.json` for debugging with Visual Studio Code:

launch.json:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Protections Test 10",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/src/Debug/test10",
            "MIMode": "lldb",
            "args": [],
            "cwd": "${workspaceFolder}",
            "externalConsole": false,
            "environment": [],
            "preLaunchTask": "Build Protections Test 10"
          }
    ]
}
```

tasks.json:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "type": "cmake",
            "label": "Build Protections Test 10",
            "command": "build",
            "targets": [
                "test10"
            ],
            "preset": "${command:cmake.activeBuildPresetName}",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": [],
            "detail": "CMake template build task"
        }
    ]
}
```

This CMakePresets.json follows version 6. Because the CMake extension in VS Code compares against version 4, you will need to ensure `cmake.allowUnspportedPresetsVersion` is set to `true` in `settings.json`. The additional field `debug.allowBreakpointsEverywhere` was also added to allow breakpoints to be set in the Objective-C++ (.mm) files:

```json
{
    "cmake.allowUnsupportedPresetsVersions": true,
    "debug.allowBreakpointsEverywhere": true
}
```

## References 📝

| Description                  | URL |
| ---------------------------- | --------------------------------------------------------------- |
| LockSmith by Cody Thomas | [LockSmith](https://github.com/its-a-feature/LockSmith/) |
| Apple Developer Documentation | [Security Framework Results Codes](https://developer.apple.com/documentation/security/1542001-security_framework_result_codes?language=objc) |
| OSStatus SecBase.h | [OS Status](https://www.osstatus.com/search/results?platform=all&framework=Security&search=SecBase.h) |
