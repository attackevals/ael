# Protections Test 7 Binary

This protections test binary tests blocking propagation via NetBIOS scans and
PsExec.

## Usage

This binary takes a single optional argument `--no-prop`, which will disable NetBIOS scanning
and propagation via PsExec. These features are enabled by default.

## Features

- Contains a compressed and AES-encrypted PsExec embedded in the binary at build time
- Determines its own IP addresses and local CIDR networks using the `GetAdaptersInfo` API call
- Scans each discovered local CIDR network using NetBIOS lookups to find remote Windows targets for PsExec propagation
- If at least one remote target was found, the binary will decrypt and decompress the embedded PsExec and
drop it to disk at `C:\Windows\temp\pm.exe`
- The binary will then propagate to the remote targets using the dropped PsExec binary and hardcoded credentials:

```cmd
C:\Windows\temp\pm.exe -accepteula \\target1,target2,... -u <username> -p <password> -s -d -f -c <path to self> --no-prop
```

- The PsExec command will copy itself to each target and then execute the binary as SYSTEM.
- Logs output to `test7.log` in the current working directory.

## Logging

For operator usability, the protections test binary will log in plaintext to `test7.log`
in the current working directory.

## Build

The protections test binary can be built in Debug or Release mode using the `cargo build` command:

```PowerShell
# Debug mode
cargo build

# Release mode
cargo build --release
```

When compiled, binaries are automatically placed in `.\target\release\` or `.\target\debug\` directories.

You can then run the binary from the command line:

```PowerShell
.\target\Debug\test7.exe

.\target\Release\test7.exe
```

Note - when building, you can control which version of PsExec is embedded by adjusting the `fetch_zip!` macro values in `src/psexec.rs`:

```rust
fetch_zip!("path\\to\\PSTools.zip", "PSTools.zip URL", "resources\\PSTools.zip", "resources");
```

- The first parameter is the path for a source zip file on disk that contains `PsExec64.exe` (e.g. `C:\Users\Public\PSTools.zip`). To skip using a source zip from disk, pass an empty string. If the specified file path exists, it will be copied to the destination path specified by the third parameter, and the downloading step is skipped.
- The second parameter is the URL to download PSTools.zip from (e.g. the [Microsoft PsTools Suite download link](https://download.sysinternals.com/files/PSTools.zip)) in the event that the third parameter file path doesn't already exist, and the first parameter file path is either empty or doesn't exist. The zip will be downloaded to the file path specified by the third parameter.

## Test 🧪

### Unit Tests

The protections test binary unit tests can be executed using the `cargo test` command:

```PowerShell
# Debug mode
cargo test

# Release mode
cargo test --release
```
