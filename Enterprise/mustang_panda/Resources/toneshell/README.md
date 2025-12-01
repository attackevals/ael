---
order: PA
---

# TONESHELL and Protections Test 4

TONESHELL consists of a shellcode loader DLL (`wsdapi.dll`) and backdoor shellcode used by the Mustang Panda adversary.

The TONESHELL DLL will be executed via DLL sideloading/hijacking via `EssosUpdate.exe`, which is a renamed legitimate signed binary `wsddebug_host.exe`. All of the DLL's exports call the same malicious internal function, which executes the shellcode to perform backdoor functionality.<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html),[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup>

The TONESHELL DLL, with embedded shellcode, is packaged in a `.rar` file called `250325_Pentos_Board_Minutes.rar`, which also contains `EssosUpdate.exe` and a malicious LNK file `Essos Competitiveness Brief.lnk`. When run, the LNK file will execute `EssosUpdate.exe` via: `%COMSPEC% /c .\EssosUpdate.exe`. The `.rar` file is password protected with password: `Pentos`

The protections test 4 component is a dropper that drops a similar shellcode loader dll (`gflagsui.dll`) with similar embedded backdoor shellcode. The dropper will open a decoy PDF and execute the DLL via DLL sideloading, but this time by running the `gflags.exe` [legitimate signed binary](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/gflags).<sup>[3](https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit)</sup>

The test 4 DLL exports the `ShowGflagsUI` function that gets called by the legitimate binary, resulting in the same malicious routine as the TONESHELL DLL, except the test 4 shellcode is what will ultimately get executed.

Note that to avoid loader lock, the malicious routines are not performed in `DllMain`, but rather in the exported DLL functions that the legitimate executables are known to call.

## TONESHELL Main Features

This project builds a malicious DLL `wsdapi.dll` that will be executed via DLL sideloading/hijacking by the signed, legitimate `wsddebug_host.exe` binary, which is a [Windows debugging tool](https://learn.microsoft.com/en-us/windows/win32/wsdapi/debugging-tools) and is renamed as `EssosUpdate.exe`.

`wsdadpi.dll` is built from scratch, exporting only the functions that `EssosUpdate.exe` imports from the real DLL:

- `WSDSetConfigurationOption`
- `WSDXMLCreateContext`
- `WSDFreeLinkedMemory`
- `WSDCreateDiscoveryPublisher`
- `WSDAttachLinkedMemory`
- `WSDCreateDeviceHost`
- `WSDAllocateLinkedMemory`
- `WSDCreateDeviceHostAdvanced`
- `WSDCreateHttpAddress`

Calling any of these functions will trigger the malicious routine, which will execute the backdoor shellcode in memory.<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html)</sup> `wsdadpi.dll` is signed using a self-signed certificate using the following certificate subject: `CN=Tully Enterprises, O=Tully Enterprises, L=Riverrun, S=Riverlands, C=Westeros`.<sup>[3](https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit)</sup>

The DLL performs certain checks prior to executing its malicious routine.

- The first check consists of retrieving the current process name using the `GetModuleFileNameW` API call and making sure it matches the expected process name `EssosUpdate.exe`.<sup>[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup> This makes it harder to perform sandbox or dynamic analysis on the DLL, since the DLL will only continue if running from a process with a specific name.
- The second check consists of checking for foreground window changes every second for up to a minute or until the window changes at least 2 times.<sup>[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup> If the entire minute passes without enough window changes, the DLL will terminate early without executing the malicious routine. This check deters sandbox analyses that do not emulate real user behavior by switching windows.

If all checks pass, the DLL triggers the malicious routine using a custom C++ exception and associated exception handler.<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html),[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup>

The malicious routine will do the following:<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html),[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup>

- Register the current malicious DLL using `regsvr32.exe`, which will execute the DLL's exported `DllRegisterServer` function.
  - `C:\Windows\System32\regsvr32.exe /s "PATH_TO_DLL"`
- The `DllRegisterServer` export will create a victim `waitfor.exe` process and inject the current DLL into it using `mavinject.exe`.
  - `C:\Windows\System32\waitfor.exe Event183785251387`
  - `C:\Windows\System32\mavinject.exe WAITFOR_PID /INJECTRUNNING "PATH_TO_DLL"`
- Once `waitfor.exe` loads the malicious DLL, `DllMain` will create a thread to
execute the shellcode in the victim process memory.
  - `DllMain` checks if it is running within `C:\Windows\System32\waitfor.exe`

The shellcode is XOR-encrypted and stored in the `.data` section of the DLL at build time. The encryption is performed using a randomly-generated 32-byte key, which is then XOR-encrypted using a single-byte key `0x3F` and compiled into the DLL with the encrypted shellcode.<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html),[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup>

- The XOR encryption of the shellcode payload consists of three rounds with modifications made to the XOR key. The first round is standard XOR encryption using the original key, the second round rotates the key by one byte, and the third round rotates the key by 7 bytes.<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html)</sup>

When executed, the shellcode will do the following:

- Generate a random 16-byte victim ID using the `CoCreateGuid` API call.<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html)</sup>
  - The shellcode will save this victim ID to disk at `%USERPROFILE%\AppData\Roaming\Microsoft\Web.CompressShaders.config`<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html),[8](https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html)</sup>
- Obtain the victim hostname using the `GetComputerNameA` API call to send to the C2 server.<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html)</sup>
- Establish a TCP-based C2 channel and send data via the `send` API from `Ws2_32.dll`<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html)</sup>
- Routinely beacons out to the C2 server to request tasking. The following tasks are supported:
  - Execute process<sup>[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html),[5](https://unit42.paloaltonetworks.com/stately-taurus-attacks-se-asian-government/),[8](https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html)</sup>
  - Download files<sup>[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html),[5](https://unit42.paloaltonetworks.com/stately-taurus-attacks-se-asian-government/)</sup>
  - Upload files<sup>[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html),[5](https://unit42.paloaltonetworks.com/stately-taurus-attacks-se-asian-government/)</sup>
  - Terminate self

The main DLL also dynamically resolves certain Windows API calls to hide them from its import table. The shellcode dynamically resolves all of its API calls to maintain its position-independent nature. In both cases, API calls are resolved by traversing DLL exports and comparing them to the desired API name using the FNV1-A hash function. Any required libraries besides `ntdll` and `kernel32` are imported at run-time using `LoadLibraryW`. <sup>[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup>

Key string literals are XOR-encrypted at compile-time and are decrypted at run-time to obfuscate them within the compiled binary.<sup>[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup>

## TONESHELL C2 Communications

All communications between TONESHELL and the C2 server will begin with the magic bytes `0x18 0x04 0x04` (the original malware used `0x17 0x03 0x03`).<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html),[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup>

Messages sent to the C2 server have the following structure:<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html),[8](https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html)</sup>

| Offset | Size | Field Name/Description |
| -------- | ------- | ------- |
| 0x0 | 0x3 | Magic bytes signature: `0x18 0x04 0x04` |
| 0x3 | 0x2 | Size of data after encryption key |
| 0x5 | 0x100 | XOR data encryption key |
| 0x105 | 0x10 | Unique victim ID |
| 0x115 | 0x1 | Message type |
| 0x116 | x | Message-specific data |

Messages received from the C2 server have the following structure:<sup>[8](https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html)</sup>

| Offset | Size | Field Name/Description |
| -------- | ------- | ------- |
| 0x0 | 0x3 | Magic bytes signature: `0x18 0x04 0x04` |
| 0x3 | 0x2 | Size of remaining bytes (including message type) |
| 0x5 | 0x1 | Message type |
| 0x6 | x | Message-specific data |

Each message sent to the C2 server has its non-header data (everything past the XOR key field) encrypted using a 256-byte XOR key that is randomly generated for each message<sup>[1](https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html),[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup>. The server response will encrypt its non-header data (everything past the size field) using the same XOR key, which the backdoor shellcode will use to decrypt the response.

The backdoor shellcode will first establish C2 communications by establishing a handshake with the C2 server. The handshake protocol is as follows:<sup>[8](https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html)</sup>

- The client sends a handshake message to the C2 server (message type `0x1`), where the message-specific data consists of the following:
  - hostname length (4 bytes)
  - current hostname
- The C2 server sends a handshake response (message type `0x08`) with a 1-byte data payload.

After establishing the handshake, the shellcode will then send beacon messages (message type `0x02`) to request tasking.
The server will respond with one of the following task codes:

- `0x3` - file download. The shellcode will then send file chunk requests (type `0x13`) until the entire file is downloaded or until an error occurs.
- `0x4` - no tasking. The shellcode will simply sleep until the next beacon.
- `0x5` - execute process. The server will provide a timeout value in seconds and a command line to execute that contains an executable name and optional arguments.
- `0x7` - file upload. The shellcode will then send file upload chunks as task output until the entire file is uploaded or until an error occurs.
- `0x9` - task output acknowledgment. Server sends this to the implant whenever it receives task output from command execution or file uploads.
- `0xA` - reconnect handshake request. The server needs the implant to re-identify itself by re-sending a handshake to the server.
- `0xFF` - terminate self. The shellcode will exit after receiving this task.

While the shellcode performs its tasking, it may send the following task codes to the C2 server:

- `0x15` - sends task output, such as output from process execution or a file upload chunk. The message contains output data no larger than 4096 bytes. Large output will result in multiple output messages. Expects a server response acknowledging the output chunk (type `0x9`)
- `0x14` - successful task completion. Will include an exit code if the task required process execution. If the task resulted in any output, this message is sent after all output has been successfully sent to the C2 server. Does not expect a server response.
- `0x13` - sends a file chunk request, specifying the associated task number, file offset in bytes, and the maximum chunk size that it can handle.
- `0x3` - task error. This message will contain an error code and is sent to the C2 server if the shellcode fails to perform its task. Does not expect a server response.

## Protections Test 4 Main Features

The Test 4 dropper executable will do the following:<sup>[3](https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit)</sup>

- Attempt to create the `C:\ProgramData\GFlagEditor` directory. If it doesn't already exist, the dropper will decrypt and drop the embedded DLL and legitimate signed binary in the directory as `gflagsui.dll` and `gflags.exe`, respectively
  - Components are embedded and encrypted the same way as with the TONESHELL detections scenario components
- Open the decoy PDF located at `..\Appendix II\Assessing Westeros-Essos Global Influence (1).pdf` relative to the dropper executable
- Create a scheduled task `GFlagEditor` that will execute `C:\ProgramData\GFlagEditor\gflags.exe` every 3 minutes
  - `schtasks /F /create /TN GFlagEditor /SC minute /MO 3  /TR C:\ProgramData\GFlagEditor\gflags.exe`

The `gflagsui.dll` DLL is effectively the same as the TONESHELL DLL, except for the following:

- Exports the `ShowGflagsUI` function instead of the `wsdapi.dll`-specific functions
- Performs a mutex check using the global mutex `Global\1247893689173278` to avoid running more than one instance of the shellcode via scheduled tasks

The shellcode is effectively the same as the TONESHELL shellcode, except for the following:

- Encrypts C2 communications with RC4 instead of XOR<sup>[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup>
  - 256-byte key is randomly generated for each message
- The victim GUID is generated using the hostname and volume serial number<sup>[4](https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html)</sup>

Both the dropper and the DLL are signed using separate self-signed certificates with the following certificate subject: `CN=Casterly Enterprises, O=Casterly Enterprises, L=Casterly Rock, S=Lannisport, C=Westeros`.<sup>[3](https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit)</sup>

## Usage

Since the public release of this repository does not include the vulnerable legitimate 3rd-party binaries, you will need to download them.

- For `EssosUpdate.exe`, you will need to grab and rename [`wsddebug_host.exe`](https://learn.microsoft.com/en-us/windows/win32/wsdapi/debugging-tools) from a Windows machine that has Windows SDK or the Windows Driver Kit (WDK) installed.
  - The executable path will typically follow the format: `%PROGRAMFILES%\Windows Kits\10\bin\%VERSION%\x64\wsddebug_host.exe`. The binary used in the 2025 evaluations came from `C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\wsddebug_host.exe` with a SHA256 hash of `3DC7F38CB68FA316205BEC35AFEF875DC0A748030D4005A491BB6FE350E6F8B2`
  - Save the executable as `Resources/toneshell/EssosUpdate.exe` prior to building.
- For [`gflags.exe`](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/gflags), you will need to grab the executable from a Windows machine with [Debugging Tools for Windows 10 (WinDbg)](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools) installed. These should already be available if the Windows SDK or WDK are already installed.
  - The executable path will typically follow the format: `%PROGRAMFILES%\Windows Kits\10\Debuggers\%VERSION%\gflags.exe`. The binary used in the 2025 evaluations came from `C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\gflags.exe` with a SHA256 hash of `8A5DD351E4A1FB5CCE2816D17FA7130240938735B5AB5F0C7C67996D687557DA`.
  - Save the executable as `Resources/toneshell/gflags.exe` prior to building.

To execute TONESHELL, run `EssosUpdate.exe` with `wsdadpi.dll` in the same directory.

To execute Protections test 4, run the dropper executable.

- For best results, ensure the decoy PDF is staged at `..\Appendix II\Assessing Westeros-Essos Global Influence (1).pdf` relative to the dropper executable

## Troubleshooting

Both the DLL and shellcode components will log to hidden files on disk.

Log messages are encrypted by default using AES-256-CTR and then base64-encoded.
The encryption key used is: `c47001f8de67d8fe23b76d7685fe75fbb0abec9b3bb23f4cf99d7f3ece345c18`, and a randomly generated 16-byte IV is prepended to each ciphertext prior to base64 encoding.

To decrypt the logs, use the log decryptor Python utility:

```bash
python3 aes_base64_log_decryptor.py -i /path/to/log -o /path/to/output --aes-256-ctr -k c47001f8de67d8fe23b76d7685fe75fbb0abec9b3bb23f4cf99d7f3ece345c18
```

### TONESHELL

The shellcode component logs to `C:\Windows\System32\wsdapi_dat.log`, while the DLL component logs to different log files depending on which part of the DLL component is running:

- `C:\Windows\System32\wsdapih.log`: log file for the main handler routine up until the DLL is registered via `regsvr32`
- `C:\Windows\System32\wsdapireg.log`: log file for when the DLL is run by `regsvr32` to inject into `waitfor.exe` using `mavinject`.
- `C:\Windows\System32\wsdapisr.log`: log file for when the DLL is injected into the victim `waitfor.exe` process and runs the shellcode payload.

### Protections Test 4

The dropper logs to `C:\Windows\System32\t4.log`.

The shellcode component logs to `C:\Windows\System32\gflagsui_dat.log`, while the DLL component logs to different log files depending on which part of the DLL component is running:

- `C:\Windows\System32\gflagsh.log`: log file for the main handler routine up until the DLL is registered via `regsvr32`
- `C:\Windows\System32\gflagsreg.log`: log file for when the DLL is run by `regsvr32` to inject into `waitfor.exe` using `mavinject`.
- `C:\Windows\System32\gflagssr.log`: log file for when the DLL is injected into the victim `waitfor.exe` process and runs the shellcode payload.

## Build

All components can be built in Debug or Release mode using the included `CMakePreset.json` configurations.

The following PowerShell command was used to build the LNK file:

```PowerShell
$WshShell = New-Object -COMObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("C:\path_to_mustang_panda_repository\Resources\toneshell\src\wsdapi\Essos Competitiveness Brief.lnk")
$Shortcut.TargetPath = "%COMSPEC%"
$Shortcut.Arguments = "/c .\EssosUpdate.exe"
$Shortcut.IconLocation = "C:\Windows\System32\shell32.dll,70"
$Shortcut.Save()
```

### Dependencies

- `CMake` version `3.26`
- `CMakePresets.json` version `6` support (Visual Studio 17.7)

### Third-Party Libraries

The project leverages the following third party libraries/projects:

- [assemblyline](https://github.com/robleh/assemblyline)

These projects are pulled down and built automatically as part of the build process.

### Quickstart

#### Command Line

Build both Debug and Release configurations of the component.

```PowerShell
cd Resources\toneshell
cmake.exe --workflow --preset cicd-debug
cmake.exe --workflow --preset cicd-release
```

For simplicity, all of the generated artifacts can be bundled into the
top-level `install/` directory using the CMake installation facility.

```PowerShell
cmake.exe --install ./build --config release
cmake.exe --install ./build --config debug
```

## Developer Notes

The following table describes the project files and their purposes:
<details>
  <summary>:link: Click to expand project structure table</summary>

  | File/Directory | Description |
  | -------- | ------- |
  | `src/common/api_helper.hpp` | Implements Windows API hashing for the DLLs |
  | `src/common/api_helper_test.cpp` | Unit tests for Windows API hashing |
  | `src/common/checks.cpp` | Implements anti-sandbox checks |
  | `src/common/checks.hpp` | Defines anti-sandbox checks |
  | `src/common/checks_test.cpp` | Anti-sandbox check unit tests |
  | `src/common/embed_payload.ps1` | Script that encrypts and embeds the shellcode bytes in the specified header file along with the encrypted random XOR key. |
  | `src/common/embedded.hpp.in` | Header file template that `embed_payload.ps1` populates with the encrypted shellcode bytes and encrypted XOR key in the specified header file |
  | `src/common/fnv1a.hpp` | Implements FNV1A hash |
  | `src/common/handler.cpp` | Implements malicious DLL routines |
  | `src/common/handler.hpp` | Defines malicious DLL routines |
  | `src/common/handler_util.cpp` | Implements utilites for malicious DLL |
  | `src/common/handler_util.hpp` | Defines utilites for malicious DLL |
  | `src/common/handler_util_test.cpp` | Unit tests for malicious DLL utilities |
  | `src/common/logger.cpp` | Implements logging functionality |
  | `src/common/logger.hpp` | Defines logging functionality |
  | `src/common/logger_test.cpp` | Logging unit tests |
  | `src/common/obfuscation.hpp` | Implements compile-time string literal encryption |
  | `src/common/obfuscation_test.cpp` | Unit tests for compile-time string literal encryption |
  | `src/common/pi_aes_ctr.cpp` | Implements AES encryption functionality |
  | `src/common/pi_aes_ctr.hpp` | Defines AES encryption functionality |
  | `src/common/pi_aes_ctr_test.cpp` | AES encryption unit tests |
  | `src/common/pi_base64.cpp` | Implements Base64 encoding functionality |
  | `src/common/pi_base64.hpp` | Defines Base64 encoding functionality |
  | `src/common/pi_base64_test.cpp` | Base64 encoding unit tests |
  | `src/common/register.cpp` | Implements Regsvr32 registration and DLL injection via mavinject |
  | `src/common/register.hpp` | Defines Regsvr32 registration and DLL injection via mavinject |
  | `src/common/shared_func.hpp` | Defines function pointers used by both the DLL and shellcode components |
  | `src/common/sign_artifact.ps1` | Script that generates a code signing certificate and signs a given binary. Used to sign the TONESHELL DLL and the Protections test 4 dropper and DLL. |
  | `src/common/util.cpp` | Implements shared utility functions |
  | `src/common/util.hpp` | Defines shared utility functions |
  | `src/common/win_helper.h` | Defines certain Windows structs |
  | `src/common/xor.cpp` | Contains XOR encryption routines |
  | `src/common/xor.hpp` | XOR encryption header file |
  | `src/common/xor_test.cpp` | XOR encryption unit tests |
  | `src/shellcode/CMakeLists.txt` | Contains instructions for building the backdoor shellcode |
  | `src/shellcode/comms.cpp` | Implements C2 channel functionality |
  | `src/shellcode/comms.hpp` | Defines C2 channel functionality |
  | `src/shellcode/comms_encryption_det.cpp` | Implements C2 communications encryption for the detections scenario shellcode |
  | `src/shellcode/comms_encryption_test4.cpp` | Implements C2 communications encryption for the Protections Test 4 shellcode |
  | `src/shellcode/comms_test.cpp` | C2 comms unit tests |
  | `src/shellcode/entry.cpp` | Main shellcode logic |
  | `src/shellcode/exec.cpp` | Implements task execution functionality |
  | `src/shellcode/exec.hpp` | Defines task execution functionality |
  | `src/shellcode/exec_test.cpp` | Task execution unit tests |
  | `src/shellcode/rc4.cpp` | Implements RC4 encryption functionality |
  | `src/shellcode/rc4.hpp` | Defines RC4 encryption functionality |
  | `src/shellcode/rc4_test.cpp` | Unit tests for RC4 encryption |
  | `src/shellcode/shellcode.hpp` | Defines types and structs for shellcode |
  | `src/shellcode/shellcode_util.cpp` | Utility and helper functions |
  | `src/shellcode/shellcode_util.hpp` | Defines utility and helper functions |
  | `src/shellcode/shellcode_util_id_d.cpp` | TONESHELL (detections) implementation of generating the victim GUID |
  | `src/shellcode/shellcode_util_id_p.cpp` | Protections test 4 implementation of generating the victim GUID |
  | `src/shellcode/shellcode_util_test.cpp` | Shellcode utility unit tests |
  | `src/shellcode/test4shellcode.hpp` | Stub header for Test 4 shellcode to build via assemblyline |
  | `src/test4/dropper/Assessing Westeros-Essos Global Influence (1).pdf` | Decoy PDF for Test 4 Dropper to open |
  | `src/test4/dropper/CMakeLists.txt` | Contains instructions for building the Test 4 dropper executable |
  | `src/test4/dropper/dropper.cpp` | Implements main logic for the Test 4 Dropper |
  | `src/test4/dropper/dropper_util.cpp` | Implements utilities for the Test 4 Dropper |
  | `src/test4/dropper/dropper_util.hpp` | Defines utilities for the Test 4 Dropper |
  | `src/test4/dropper/dropper_util_test.cpp` | Unit tests for utilities for the Test 4 Dropper |
  | `src/test4/gflagsui/CMakeLists.txt` | Contains instructions for building the sideloaded Test 4 DLL |
  | `src/test4/gflagsui/dllmain.cpp` | Implements main logic for the Test 4 DLL |
  | `src/test4/gflagsui/embeds.hpp` | Includes the dynamically generated header file with the embedded payloads for the Test 4 DLL |
  | `src/test4/gflagsui/gflagsui.def` | Defines Test 4 DLL exports |
  | `src/test4/gflagsui/gflagsui_util.cpp` | Implements utilities for the Test 4 DLL |
  | `src/test4/gflagsui/gflagsui_util.hpp` | Defines utilities for the Test 4 DLL |
  | `src/test4/gflagsui/gflagsui_util_test.cpp` | Unit tests for utilities for the Test 4 DLL |
  | `src/test4/CMakeLists.txt` | Contains instructions for building the Test 4 components |
  | `src/wsdapi/` | Contains resources for building the malicious `wsdapi.dll` for TONESHELL DLL hijacking |
  | `src/wsdapi/CMakeLists.txt` | Contains instructions for building the TONESHELL DLL |
  | `src/wsdapi/Essos Competitiveness Brief.lnk` | Malicious LNK file that will execute `EssosUpdate.exe` in the same directory. |
  | `src/wsdapi/dllmain.cpp` | Contains main TONESHELL DLL source code |
  | `src/wsdapi/embeds.hpp` | Includes the dynamically generated header file with the embedded payloads for the TONESHELL DLL |
  | `src/wsdapi/wsdapi.def` | Defines TONESHELL DLL exports |
  | `src/CMakeLists.txt` | Contains instructions for building the various components and unit test source files |
  | `tests/test_util.cpp` | Implements unit test utilities |
  | `tests/test_util.hpp` | Defines unit test utilities |
  | `CMakeLists.txt` | Defines the overall cmake project |
  | `CMakePresets.json` | Defines cmake presets |
  | `EssosUpdate.exe` | Renamed `wsddebug_host.exe`, a legitimate signed [Windows debugging tool](https://learn.microsoft.com/en-us/windows/win32/wsdapi/debugging-tools) that is vulnerable to DLL hijacking for `wsdapi.dll` |
  | `gflags.exe` | Global Flags Editor, a legitimate signed [Windows debugging tool](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/gflags) that is vulnerable to DLL hijacking for `gflagsui.dll` |

</details>

## CTI

1. <https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html>
2. <https://unit42.paloaltonetworks.com/stately-taurus-abuses-vscode-southeast-asian-espionage/>
3. <https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit>
4. <https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html>
5. <https://unit42.paloaltonetworks.com/stately-taurus-attacks-se-asian-government/>
6. <https://www.trendmicro.com/en_us/research/23/c/earth-preta-updated-stealthy-strategies.html>
7. 2022-overwatch-q3-report.pdf
8. <https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html>
