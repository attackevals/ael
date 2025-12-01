---
order: PB
---

# Plug X and Protections Test 5

Plug X consists of the following:

- shellcode used in the Plug X payload
- malicious loader DLL that will execute the shellcode when sideloaded by the legitimate `gup.exe` binary
- an MSI installer, `2025p2.msi` that will install the different components and execute the legitimate binary to trigger the malicious routine.
- an HTML file that performs HTML smuggling to download the MSI installer when opened in the browser.

The shellcode will be placed on disk as `WinGUpdate.dat` for the Plug X loader to read, decrypt, and execute at runtime.

The protections test 5 component is an MSI installer that drops a similar legitimate binary (rc.exe), loader DLL (rcdll.dll), and shellcode payload (resinfo.dat).<sup>[3](https://go.recordedfuture.com/hubfs/reports/cta-cn-2025-0109.pdf)</sup>

## Plug X Main Features

The MSI installer will drop the following files in the current user's `%LOCALAPPDATA%\EvRDRunMP` directory:

- `gup.exe` legitimate signed executable vulnerable to DLL side-loading
- `gup.xml` XML file required for `gup.exe` to execute properly
- `libcurl.dll` malicious loader DLL that will get side-loaded
- `WinGUpdate.dat` RC4-encrypted shellcode file that the loader DLL will read in and execute

After dropping the files, the MSI installer will execute `gup.exe` to sideload the malicious loader DLL, then decrypt and execute the shellcode.<sup>[2](https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/)</sup>. The legitimate `GUP.exe` binary is a [generic updater for Windows applications](https://github.com/gup4win/wingup).

`libcurl.dll` is built from scratch, written in Nim<sup>[3](https://go.recordedfuture.com/hubfs/reports/cta-cn-2025-0109.pdf )</sup> exporting only the functions that `GUP.exe` imports from the real DLL. Calling `curl_easy_init()` will trigger the malicious routine, which will execute the shellcode.

- Note that the malicious routines are not performed in `DllMain`, but rather in the exported DLL functions that the legitimate executables are known to call, to make it harder to detect<sup>[7](https://lab52.io/blog/mustang-pandas-plugx-new-variant-targetting-taiwanese-government-and-diplomats/)</sup>.

When executed, the shellcode will do the following:

- Establish an http(s)-based C2 channel and send data via the `InternetOpenUrlA` API call from `wininet.h`.<sup>[2](https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/),[9](https://blog.eclecticiq.com/mustang-panda-apt-group-uses-european-commission-themed-lure-to-deliver-plugx-malware)</sup>
  - The implant and C2 use two headers to correlate communication, `Sec-Site` and `Sec-Dest`.
    - The `Sec-Dest` header is used as a unique implant identifier. (currently static)
    - The `Sec-Site` header is used to transmit data from the implant to the C2 server. (command output, file uploads, etc.)
- Modules are registered in the implant based on the `AutoRegisterModules` function in the `shellcode/registry/module_list.hpp` file.

### HTML Smuggling Delivery

The HTML file `msi_smuggler.html` will perform HTML smuggling when opened in a browser to download the MSI installer. The Javascript in the HTML file performs HTML smuggling by doing the following:<sup>[2](https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/)</sup>

- decodes and decrypts the base64 string containing the MSI installer
- dynamically creates an anchor (`<a>`) HTML element with the `href` URL pointing to the decrypted MSI installer payload blob
- imitates the user clicking on the link to automatically download the installer as `2025p2.msi`

The HTML file is also obfuscated,<sup>[2](https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/)</sup> though the original, unobfuscated template file can be found in `PlugxMSI/msi_smuggler.html.in` for readability. The project build process will automatically embed the encrypted MSI installer into the Javascript and obfuscate the HTML template.

### Communication

The implant communicates by sending binary data in the `Sec-Site` header in a specific structure.
The structure looks as follows:

```cpp
// C2 Communications Structure
typedef struct  {
    uint32_t id;            // instruction identifier
    uint32_t argLength;     // arg length
    uint32_t contentLength; // content length
    char* args;             // optional: cmd string
    char* content;          // optional: binary blob
} c2_packet;
```

- The `id` value is the instruction used to task a specific module in the implant `0x1234`, `0x1001`, etc.
- `argLength` is the array length of args. This value will be set dynamically on the c2 server side based on the
number of arguments provided in the tasking command.
- `contentLength` is the total length of additional content attached to the packet. This is used for passing binary
data to the server (think additional shellcode, downloading files, etc).
- `args` are the actual command arguments that go with the tasking commands. Some modules/tasks may not require
this field where the action is static in nature. An example would be passing a command `whoami`
to a module that executes commands.
- `content` is the binary data that will be passed to modules that perform actions like writing a file to disk,
executing shellcode, or other tasks that require large binary blobs to be transferred to the implant.

All communication to and from the C2 server will be RC4 encrypted with a hardcoded key. After connecting to the C2 server, the implant will send beacon messages to request tasking. Once a task is available, the server will respond with one of the following module codes:

| Command      | Arguments | Implant Output | Description |
| -----------  | --------- | -------------- | ----------- |
| 0x1001 | Message box string | None | Open a message box on the victim host |
| 0x9009 | None | C2 tasking string | Implant echos the tasking sent by C2 server |
| 0x1003 | File name, file path | None | File download |
| 0x1002 | None | None | Creates a run-key in the registry |
| 0x1100 | None | None | Opens a decoy PDF on the victim host with hardcoded path. Implant will run this automatically. |
| 0x1004 | None | None | Keylogger |
| 0x1000 | Command line arguments | Command output if applicable | Command execution |
| 0xFFFF | None | None | Terminate Plug X implant |

### Logging

The Plug X loader DLL will log in plaintext to a log file in the `C:\Windows\Temp\` directory. If executed via the MSI installer, the loader DLL will log to `C:\Windows\Temp\WinGUpdateInstaller.log`. Otherwise, it will log to `C:\Windows\Temp\WinGUpdate.log`.

The log messages for Plug X shellcode, Test 5 shellcode, and the Test 5 loader are encrypted by default using AES-256-CTR and then base64-encoded.
The encryption key is: `c47001f8de67d8fe23b76d7685fe75fbb0abec9b3bb23f4cf99d7f3ece345c18`, and a randomly generated 16-byte IV is used for each message and prepended to the ciphertext prior to base64 encoding.

The Plug X shellcode component logs to `C:\Windows\System32\wingupsvc.log`.

The Test 5 shellcode component logs to `C:\Windows\System32\rcsvc.log`.

The Test 5 `rcdll.dll` loader logs to `C:\Windows\System32\rcdll_i.log` when executed by the MSI installer; otherwise, it logs to `C:\Windows\System32\rcdll.log`

To decrypt the logs, run the log decryptor Python utility:

```bash
python3 aes_base64_log_decryptor.py -i wingupsvc.log.enc -o wingupsvc.log --aes-256-ctr -k c47001f8de67d8fe23b76d7685fe75fbb0abec9b3bb23f4cf99d7f3ece345c18
```

### Modularization

The implant is designed with extendibility in mind. Modules can be added or removed from the implant by making adjustments
to the module list in `module_list.hpp`. Part of the module registration process requires the module to
have a selected `module_id` (see the code block below). This identifier is used to call a specific module from C2.

For example:
Sending the id `0x1234` could be used to call a custom module that adds two numbers together and returns the value.
That module could be added to the modules list as: `{RegisterModule_AddNumbers, 0x1234},`

The example below shows how to structure a basic module for the implant.

Registering a module (modify this list to include or exclude modules from the implant):

```// module_list.hpp.
module_descriptor_t modules[] = {
    {RegisterModule_Echo_C2, 0x9009},
    {RegisterModule_MessageBoxA, 0x1001},
};
```

#### NOTE: Excluding a module from this list does not exclude it from compilation. To reduce shellcode bloat un-used modules should be removed from the modules folder

Existing modules and their corresponding IDs are as follows:

| ID | Module Description |
| -- | ------------------ |
| 0x1001 | Message box |
| 0x9009 | Echo C2 tasking string |
| 0x1003 | Download file from C2 |
| 0x1002 | Create Registry Run Key |
| 0x1100 | Display decoy PDF |
| 0x1004 | Keylogger |
| 0x1000 | Execute command |

**Example - Building an example module:**

HelloWorld Module - Sends a "Hello World!" message back to the C2 server.

Building a module has two requirements to be compatible with the implant. The first, is to create
a function that contains your module logic. In the example below the logic containing function
is named `HandleHelloWorld`. The second is a registration function. The registration function requires two
arguments `ctx` and `module_id`.

- `ctx` is the context being passed around in the shellcode containing function pointers, and
other structures required for the shellcode to operate correctly.
- `module_id` will be the module registration number, or number used to call your function
from the C2 server. Examples of this are `0x1234`, `0x1010`, etc.

```cpp
// /modules/HelloWorld/HelloWorld.cpp
#include "module_registry.hpp"
#include "HelloWorld.hpp"


void HandleHelloWorld(void* data, void* context) {
    module_context_t* m_ctx = (module_context_t*)data;
    sh_context* ctx = (sh_context*)context;

    const char* msg = "Hello World!";
    size_t msgLength = strlen(msg) + 1;

    char* buffer = (char*)ctx->fp.fp_malloc(msgLength);
    if (!buffer) {
        return;
    }
    pi_memcpy(buffer, msg, msgLength);

    m_ctx->output = buffer;
    m_ctx->outputSize = msgLength;

    return;
}

DWORD RegisterModule_HelloWorld(sh_context* ctx, uint32_t module_id) {
    DWORD result = RegisterModule(ctx, module_id, HandleHelloWorld, ctx);
    if (result != ERROR_SUCCESS) {
        return CUSTOM_MODULE_HELLOWORLD_FAIL_REGISTER_MODULE;
    }
    return ERROR_SUCCESS;
}

```

The header file in this case is really simple. We have our function defined and
a custom error code specific to our module.

```cpp
// /modules/HelloWorld/HelloWorld.hpp
#include "shellcode.hpp"

#define CUSTOM_MODULE_HELLOWORLD_FAIL_REGISTER_MODULE  0x9001

DWORD RegisterModule_HelloWorld(sh_context* ctx, uint32_t module_id);
```

At a minimum the `CMakeLists.txt` file needs to have the module .cpp source file added to
target sources. You may need to add additional files or components depending on the
complexity of the module being created.

Once the `CMakeLists.txt` file is created the module source will be compiled with the shellcode.

```cpp
// /modules/HelloWorld/CMakeLists.txt
target_sources(
    shellcode-pe
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld.cpp
)
```

The final piece of adding a custom module is to register it in the implant. If the module is not registered
it will still be compiled with the implant, but the module won't be accessible.

To register the module you need to add the module to the modules list in the `module_list.hpp` file. See the example
below where the HelloWorld module was added to the list of registered modules.
Registering with the `AutoRegisterModules` function.

```cpp
// /registry/module_list.hpp
#pragma once
#include "module_registry.hpp"

DWORD RegisterModule_MessageBoxA(sh_context* ctx, uint32_t module_id);
DWORD RegisterModule_Echo_C2(sh_context* ctx, uint32_t module_id);
DWORD RegisterModule_HelloWorld(sh_context* ctx, uint32_t module_id); // New HelloWorld module defined


DWORD AutoRegisterModules(sh_context* ctx) {
    module_descriptor_t modules[] = {
        {RegisterModule_Echo_C2, 0x9009},
        {RegisterModule_MessageBoxA, 0x1001},
        {RegisterModule_HelloWorld, 0x1234}, // New HelloWorld module registered with ID 0x1234
    };
    size_t count = sizeof(modules) / sizeof(modules[0]);
    for (size_t i = 0; i < count; i++) {
        DWORD result = modules[i].func(ctx, modules[i].module_id);
        if (result != ERROR_SUCCESS) {
            return FAIL_AUTO_REGISTER_MODULES;
        }
    }
    return ERROR_SUCCESS;
}
```

## Protections Test 5 Main Features

The Test 5 MSI installer will do the following:<sup>[3](https://go.recordedfuture.com/hubfs/reports/cta-cn-2025-0109.pdf)</sup>

- Attempt to create `C:\Program Files\Sarnor\2025erdt\` and drop the the embedded DLL and legitimate signed binary files there as `rcdll.dll` and `rc.exe`, respectively.
- Execute `rc.exe` in order to trigger the loader function via DLL side-loading and execute the shellcode.

## Usage

Since the public release of this repository does not include the vulnerable legitimate 3rd-party binaries, you will need to download them.

- For `GUP.exe` and `gup.xml`, you will need to grab the files from [Notepad++ v8.6.4](https://notepad-plus-plus.org/downloads/v8.6.4/).
  - Installation is not necessary, since you can download and extract the portable zip from the [Github release](https://github.com/notepad-plus-plus/notepad-plus-plus/releases/download/v8.6.4/npp.8.6.4.portable.x64.zip):

  ```powershell
  curl.exe -L https://github.com/notepad-plus-plus/notepad-plus-plus/releases/download/v8.6.4/npp.8.6.4.portable.x64.zip -o npp.8.6.4.portable.x64.zip
  Expand-Archive .\npp.8.6.4.portable.x64.zip
  cd .\npp.8.6.4.portable.x64\
  ls .\updater\
  ```

  - The `updater\GUP.exe` file should have a SHA256 hash of `35663BF0E84CD3F9BA8949375FAE8451263954154274AD4454B86920252424DC`. `gup.xml` will also be in the same directory.
  - Place both files in the `Resources\plugx\src\loader\` directory prior to building.
- For [`rc.exe`](https://learn.microsoft.com/en-us/windows/win32/menurc/resource-compiler), you will need to grab the executable from a Windows machine with Visual Studio or the Windows Software Development Kit (SDK) installed.
  - The executable path will typically follow the format: `%PROGRAMFILES%\Windows Kits\10\bin\%VERSION%\%VERSION%\rc.exe`. The binary used in the 2025 evaluations came from `C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\rc.exe` with a SHA256 hash of `9274B05389A0A99C9D7C7ABA9ECB6341023B2ADDC3435DD814FBF04AF641C5E5`.
  - Save the executable as `Resources\plugx\src\test5\rcdll\rc.exe` prior to building.

Option 1 (for testing) - Execute `shellcode.exe` as a test.
Option 2 - Execute `GUP.exe` with `gup.xml`, the malicious `libcurl.dll`, and the `WinGUpdate.dat` shellcode data in the same directory.

## Build

The components can be built in Debug or Release mode using the included `CMakePreset.json` configurations.

### Dependencies

- `CMake` version `3.26`
- `CMakePresets.json` version `6` support (Visual Studio 17.7)
- Visual Studio with the Installer Projects Extension (to build the MSI)
- `Nim` compiler Version `2`
- [winim](https://github.com/khchen/winim)

### Third-Party Libraries

The project leverages the following third party libraries/projects:

- [assemblyline](https://github.com/robleh/assemblyline)

These projects are pulled down and built automatically as part of the build process.

### Quickstart

Build both Debug and Release configurations of the component.

```powershell
cd mustang_panda\Resources\plugx
cmake.exe --workflow --preset cicd-debug
cmake.exe --workflow --preset cicd-release
```

For simplicity, all generated artifacts can be bundled into the
top-level `install/` directory using the CMake installation facility.

```powershell
cmake.exe --install ./build --config debug
cmake.exe --install ./build --config release
```

## Developer Notes

The following table describes the project files and their purposes:
<details>
  <summary>Click to expand project structure table</summary>

  | File/Directory | Description |
  | -------- | ------- |
  | `PlugxMSI/CMakeLists.txt` | Contains instructions for building the Plug X MSI installer |
  | `PlugxMSI/embed_smuggled_payload.ps1` | PowerShell script that encrypts, encodes, and embeds a given payload into an HTML template file and obfuscates hardcoded strings and variable names |
  | `PlugxMSI/msi_smuggler.html` | Auto-generated HTML smuggling file with the payload embedded in malicious JavaScript |
  | `PlugxMSI/msi_smuggler.html.in` | HTML smuggling template file with placeholders for the embedded payload and XOR key |
  | `PlugxMSI/PlugxMSI.sln` | MSI installer solution file |
  | `PlugxMSI/PlugxMSI.vdproj` | MSI installer project file |
  | `src/CMakeLists.txt` | Contains instructions for building the various components |
  | `src/loader/CMakeLists.txt` | Contains instructions for building the DLL |
  | `src/loader/libcurl.nim` | Source code for building the malicious DLL |
  | `src/loader/RC4.nim` | Source code for RC4-decrypting the shellcode |
  | `src/loader/GUP.exe` | Legitimate WinGUp executable, vulnerable to DLL side-loading for `libcurl.dll` |
  | `src/loader/gup.xml` | Config file required by GUP.exe |
  | `src/shellcode/CMakeLists.txt` | Contains instructions for building the backdoor shellcode |
  | `src/shellcode/modules/*` | Contains modular extensions for the implant |
  | `src/shellcode/external_helpers/rc4_encrypt_file.py` | Script to RC4 encrypt the shellcode |
  | `src/shellcode/registry/module_registry.cpp` | Implements dynamic module registration functionality |
  | `src/shellcode/registry/module_registry.hpp` | Defines dynamic module registration functionality |
  | `src/shellcode/registry/module_list.cpp` | Defines and Implements dynamic modules |
  | `src/shellcode/base64.cpp` | Implements base64 encoding functionality |
  | `src/shellcode/base64.hpp` | Defines base64 encoding functionality |
  | `src/shellcode/comms.cpp` | Implements C2 channel functionality |
  | `src/shellcode/comms.hpp` | Defines C2 channel functionality |
  | `src/shellcode/entry.cpp` | Main shellcode logic |
  | `src/shellcode/shellcode.hpp` | Defines types and structs for shellcode |
  | `src/shellcode/util.cpp` | Utility and helper functions |
  | `src/shellcode/util.hpp` | Defines utility and helper functions |
  | `CMakeLists.txt` | Defines the overall cmake project |
  | `CMakePresets.json` | Defines cmake presets |

</details>

## CTI

1. <https://www.trendmicro.com/vinfo/us/threat-encyclopedia/web-attack/112/pulling-the-plug-on-plugx>
2. <https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/>
3. <https://go.recordedfuture.com/hubfs/reports/cta-cn-2025-0109.pdf>
4. <https://www.trendmicro.com/en_us/research/24/i/earth-preta-new-malware-and-strategies.html>
5. <https://www.trendmicro.com/en_us/research/24/b/earth-preta-campaign-targets-asia-doplugs.html>
6. <https://www.justice.gov/archives/opa/media/1384136/dl>
7. <https://lab52.io/blog/mustang-pandas-plugx-new-variant-targetting-taiwanese-government-and-diplomats/>
8. <https://csirt-cti.net/2024/01/23/stately-taurus-targets-myanmar/>
9. <https://blog.eclecticiq.com/mustang-panda-apt-group-uses-european-commission-themed-lure-to-deliver-plugx-malware>
10. <https://unit42.paloaltonetworks.com/stately-taurus-attacks-se-asian-government/>
11. <https://web.archive.org/web/20250102192805/https://blogs.blackberry.com/en/2022/12/mustang-panda-uses-the-russian-ukrainian-war-to-attack-europe-and-asia-pacific-targets>
12. <https://www.welivesecurity.com/2022/03/23/mustang-panda-hodur-old-tricks-new-korplug-variant/>
