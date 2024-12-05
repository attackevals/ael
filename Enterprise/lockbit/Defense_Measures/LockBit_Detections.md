# LOCKBIT Detection Analysis 

This guide covers key detection criteria for LOCKBIT-related malware activity as emulated by the ([Enterprise Round 6 Emulation Plan](../LockBit/Emulation_Plan/ER6_LockBit_Scenario.md)), such as STEALBIT and THUNDERSHELL. The focus is on External Remote Services (T1133) via VNC, Valid Accounts (T1078) for credential theft, and Command and Scripting (T1059) through Windows Shell. Further stages involve Credential Access (T1555), Process Discovery (T1057), and Inhibit System Recovery (T1490) by disabling Windows recovery features. Notably, LOCKBIT uses Encryption (T1486) for impact, targeting files and shares.

### Key Findings

* Common lateral movement using PsExec over SMB (T1021).
* Remote System Discovery (T1018) and Network Configuration Discovery (T1016) through API functions.

### Detection Tools

* Sysmon + Splunk: Detects file creation, process execution, and system calls.
* Suricata/Zeek: Monitors network traffic for HTTP/HTTPS connections to suspicious domains.
* Wireshark: Network communication analysis.
* OSQuery: File and process discovery.
* Sigma: Use rule-based log analysis for identifying adversary activity.

## Step 9

### Step 9.A.1 External Remote Services ([T1133](https://attack.mitre.org/techniques/T1133/)))

**Detection Criteria**

The adversary connected to `QUIRRELL (10.111.9.202)` through VNC

**Behavioral Anomalies**

* Unusual Remote Access: VNC access from unexpected external IPs, especially to systems that typically don't have VNC services running.
* Elevated Process Execution: Remote service processes like tvnserver.exe being launched under elevated privileges (SYSTEM) without prior logon activity.
* Chained Activity: Monitor for rapid successive logon attempts or lateral movement from compromised credentials.
* Unusual Access Patterns: Monitoring for unusual access patterns following login, such as VNC default ports 5900, 5800, or 5500 being targeted by a non-admin user.

**Detection SPL Queries**

```
index=<your_index> sourcetype="WinEventLog:Microsoft-Windows-Sysmon/Operational"  
(EventCode=3 OR EventCode=1) Image="*tvnserver.exe"  
| stats count by SourceIp, SourcePort, DestinationIp, DestinationPort, Image, ProcessId 
| where DestinationPort="5900, 5800, 5500"
```

### Step 9.A.2 Valid Accounts: Domain Account ([T1078.002](https://attack.mitre.org/techniques/T1078/002/))

**Detection Criteria**

`GORNUK` authenticated with domain credentials

**Behavioral Anomalies**

* Unusual Access Patterns: Monitor for deviations in the types of resources accessed.
* Volume of Activity: Monitor for unusual activity such as a sudden increase in data access or file transfers.
* Failed Login Attempts: A high number of failed login attempts can suggest an attempt to compromise an account.
* Concurrent Sessions: Monitor for multiple concurrent sessions from different locations or devices.
* Privileged Account Use: Unusual use of privileged accounts, such as accessing administrative tools or making unauthorized changes.

## Step 10

### Step 10.A.1 Command and Scripting Interpreter: Windows Command Shell ([T1059.003](https://attack.mitre.org/techniques/T1059/003/))

**Detection Criteria**

`GORNUK` executed `cmd.exe /c`

**Behavioral Anomalies**

* Execution of cmd.exe by non-administrative accounts, especially with elevated privileges.
* Patterns of repeated or scripted commands, indicating automation.
* Parent-child relationship anomalies, such as cmd.exe invoked by unexpected processes like Office applications or non-administrative services.

**Detection SPL Queries**

```
index=your_index sourcetype=WinEventLog EventCode=4688  
Process_Name=cmd.exe  
| eval ParentImage=lower(ParentImage) 
| search (ParentImage!="explorer.exe" AND ParentImage!="services.exe") 
| rex field=CommandLine "cmd\.exe\s*/c\s*(?<cmd>.*)"
```

### Step 10.A.2 Remote System Discovery ([T1018](https://attack.mitre.org/techniques/T1018/))

**Detection Criteria**

`cmd.exe` executed `nltest /dclist:decryptmalfoy.net`

**Behavioral Anomalies**

* Unusual Activity from User Accounts: Monitor volume of ARP requests and increased or abnormal use of commannds to enumerate domain information such as `nltest /dclist`.
* Increased Network Mapping Activities: Monitoring for increased ICMP sweeps originating from internal hosts that do not usually perform such actions.

### Step 10.A.3 Credentials from Password Stores ([T1555](https://attack.mitre.org/techniques/T1555/))

**Detection Criteria**

`cmd.exe` executed `cmdkey /list`

**Behavioral Anomalies**

* Unusual Process in Non-Admin Context: Tools accessing credential stores should generally run in a limited, non-admin context; elevated context access without appropriate administrative tasks could indicate malicious intent.
* Abnormal Access to Browser Directories: Access to specific browser directories (e.g., Firefox profile paths) outside of typical browser usage patterns or by unknown processes can signal anomalous credential extraction attempts.
* Command-Line Pattern with Browser Profile Paths: Detection of command-line execution that targets browser profiles for credential extraction, especially with uncommon parameters, can indicate malicious intent.

### Step 10.A.4 Permission Groups Discovery: Domain Groups ([T1069.002](https://attack.mitre.org/techniques/T1069/002/))

**Detection Criteria**

`cmd.exe` executed `net group 'Domain Admins' /domain` and `net group 'Enterprise Admins' /domain`

**Behavioral Anomalies**

* Unusual queries for domain group information using commands like `net group /domain` or `Get-ADGroup`, particularly if executed by accounts that do not typically interact with Active Directory.
* High frequency of domain group enumeration activities in a short period, which may indicate automated scripts or tools being used.

### Step 10.A.5 Permission Groups Discovery: Local Groups ([T1069.001](https://attack.mitre.org/techniques/T1069/001/))

**Detection Criteria**

`cmd.exe` executed `net localgroup Administrators /domin` and `net localgroup Administrators`

**Behavioral Anomalies**

* Unexpected execution of commands to list local groups, such as `net localgroup` or `Get-LocalGroup`, especially by non-administrative users or processes that do not typically perform these actions.
* Sudden interest in local group membership from accounts that do not usually require this information.

### Step 10.A.6 System Service Discovery ([T1007](https://attack.mitre.org/techniques/T1007/))

**Detection Criteria**

`cmd.exe` spawns powershell to execute `Get-WmiObject win32_service -ComputerName localhost | Where-Object {$_.PathName -notmatch 'c:win'} | select Name, DisplayName, State, PathName | findstr 'Running'`

**Behavioral Anomalies**

* Execution of discovery commands by accounts or systems without administrative privileges.
* Repeated or automated system discovery attempts.

**Detection SPL Queries**

```
index=your_index sourcetype=Sysmon EventCode=1
Parent_Process_Name=cmd.exe Process_Name=powershell.exe CommandLine="*Get-WmiObject*win32_service*"
```

## Step 11

### Step 11.A.1 Unsecured Credentials: Credentials in Registry ([T1552.002](https://attack.mitre.org/techniques/T1552/002/))

**Detection Criteria**

`GORNUK` modified `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon` subkey to enable automatic login through `AutoAdminLogin`, `DefaultUserName`, `DefaultPassword`, `DefaultDomainName` values

**Behavioral Anomalies**

* Unusual User Access: Registry modification by accounts that typically do not have administrative privileges or should not be modifying authentication settings.
* Non-Standard Modification Times: Changes to these registry keys during unusual hours may indicate unauthorized access.
* High Frequency of Modifications: A series of modifications to the registry keys related to automatic login within a short timeframe may suggest automated credential abuse.

**Detection SPL Queries**

```
index=your_index sourcetype=WinEventLog EventCode=4657
RegistryPath="HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
(RegistryKeyName="AutoAdminLogon" OR RegistryKeyName="DefaultUserName" OR RegistryKeyName="DefaultPassword" OR RegistryKeyName="DefaultDomainName")
```

## Step 12

### Step 12.A.1 Software Deployment Tools ([T1072](https://attack.mitre.org/techniques/T1072/))

**Detection Criteria**

cmd.exe executed choco install FoxAdminPro

**Behavioral Anomalies**

* Unusual Deployment: Deployment actions initiated outside of maintenance windows or by non-administrative accounts.
* High Privilege Escalation: Installations done by accounts with newly elevated privileges, especially if the elevation lacks contextual justification.
* Lateral Execution: Software deployment triggered across multiple systems in a short timeframe, suggesting an attempt to establish persistence or increase control.
* Frequent Invocations: Multiple installations or deployments by the same account/system over a short period, indicating potential automation by adversaries.
* Outlier Applications: Deployment of non-standard or suspicious software packages (e.g., rare or unknown packages).

**Detection SPL Queries**

```
index=main source=WinEventLog:Microsoft-Windows-Sysmon/Operational EventCode=1 OR EventCode=11 
| search (Image="*choco.exe" OR CommandLine="*install*")
| stats count by User, Computer, CommandLine, ParentProcessName, Image
```

### Step 12.A.2 Ingress Tool Transfer ([T1105](https://attack.mitre.org/techniques/T1105/))

**Detection Criteria**

`choco.exe` creates `FireFox Password Dumper (C:\ProgramData\chocolatey\lib\FoxAdminPro\tools\FoxAdminPro.exe)`

**Behavioral Anomalies**

* Process-File Relationship: Newly spawned processes associated with dropped files exhibiting anomalous behavior, such as elevated privileges or unexpected communication patterns.
* Unusual File Path: Storing binaries in directories like `\lib\FoxAdminPro` where legitimate helper tools are typically placed.
* Use of Transfer Tools: Execution of command-line tools commonly used for file transfers, such as wget, curl, scp, or powershell commands for downloading files, especially by users or processes that do not typically use these tools.

### Step 12.A.3 Credentials from Password Stores: Credentials from Web Browsers ([T1555.003](https://attack.mitre.org/techniques/T1555/003/))

**Detection Criteria**

`cmd.exe` executed `FoxAdminPro -d C:\Users\gornuk\AppData\Roaming\Mozilla\Firefox\Profiles\ohbrdd1o.default-release\`

**Behavioral Anomalies**

* Unusual Process in Non-Admin Context: Tools accessing credential stores should generally run in a limited, non-admin context; elevated context access without appropriate administrative tasks could indicate malicious intent.
* Abnormal Access to Browser Directories: Access to specific browser directories (e.g., Firefox profile paths) outside of typical browser usage patterns or by unknown processes can signal anomalous credential extraction attempts.
* Command-Line Pattern with Browser Profile Paths: Detection of command-line execution that targets browser profiles for credential extraction, especially with uncommon parameters, can indicate malicious intent.

**Detection SPL Queries**

```
index=your_index sourcetype="WinEventLog:Microsoft-Windows-Sysmon/Operational"
(EventCode=1)
(Image="*\\FoxAdminPro.exe" OR Image="*\\browser_password_dumper.exe" OR CommandLine="*-d*AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\*")
```

## Step 13

### Step 13.A.1 Remote Services: SSH ([T1021.004](https://attack.mitre.org/techniques/T1021/004/))

**Detection Criteria**

`GORNUK.DA` connected to `HORCRUX (10.111.8.40)` from `QUIRRELL (10.111.9.202)` over `SSH (port 22)`

**Behavioral Anomalies**

* High-Frequency Connections: Multiple, rapid SSH connection attempts to sensitive systems may indicate a brute-force or scanning attempt.
* Cross-Platform Connections: SSH sessions initiated from Windows to Linux systems can be unusual, especially if not commonly practiced in the environment.
* Privileged Account Use: The use of high-privilege accounts (such as GORNUK.DA) for SSH connections, especially outside normal business hours or from unexpected source IPs.

**Detection SPL Queries**

```
index=your_index sourcetype="WinEventLog:Microsoft-Windows-Sysmon/Operational" EventCode=3
(Image="*\\ssh.exe" OR DestinationPort=22) 
```

### Step 13.A.2 Process Discovery ([T1057](https://attack.mitre.org/techniques/T1057/))

**Detection Criteria**

`/bin/bash` executed admin commands to enumerate VMs (`sudo podman ps -a`)

**Behavioral Anomalies**

* Unusual execution of commands related to container enumeration (such as podman ps -a).
* Unusual Command Execution: Unusual use of process enumeration commands like `tasklist` or `ps`, especially if executed by non-administrative users or new processes or at unusual times.
* Chained Activity: Monitor for unusual sequence of commands that suggests reconnaissance or environment enumeration without associated normal administrative activity.

**Detection SPL Queries**

```
index=<your_linux_index> sourcetype=syslog OR sourcetype=linux_secure 
| search (COMMAND="*podman ps -a*" OR COMMAND="*ps*" OR COMMAND="*top*" OR COMMAND="*pgrep*")
```

### Step 13.A.3 (Service Stop) ([T1489](https://attack.mitre.org/techniques/T1489/))

**Detection Criteria**

`/bin/sh` executed admin commands to stop VMs and disable VM autostart (`sudo podman stop -a` to stop VMs and `sudo systemctl disable --now podman sudo systemctl disable --now podman-restart`)

**Behavioral Anomalies**

* Sudden service stop or disable actions, particularly involving security or monitoring services.
* Service management commands executed at unexpected times or by unusual accounts.
* Sequence of commands that suggests preparation for system shutdown or application/service disruptions.

**Detection SPL Queries**

```
index=<your_linux_index> sourcetype=syslog OR sourcetype=linux_secure 
| search (COMMAND="*systemctl disable*" OR COMMAND="*podman stop*" OR COMMAND="*systemctl stop*") 
```

## Step 14

### Step 14.A.1 Ingress Tool Transfer ([T1105](https://attack.mitre.org/techniques/T1105/))

**Detection Criteria**

`choco.exe` created `THUNDERSHELL (C:\ProgramData\chocolatey\lib\Termini\tools\termini.ps1)​`

**Behavioral Anomalies**

* Process-File Relationship: Newly spawned processes associated with dropped files exhibiting anomalous behavior, such as elevated privileges or unexpected communication patterns.
* Unusual File Path: Storing binaries in directories like `\lib\Termini` where legitimate helper tools are typically placed.
* Use of Transfer Tools: Execution of command-line tools commonly used for file transfers, such as wget, curl, scp, or powershell commands for downloading files, especially by users or processes that do not typically use these tools.

### Step 14.A.2 Command and Scripting Interpreter: Powershell ([T1059.001](https://attack.mitre.org/techniques/T1059/001/))

**Detection Criteria**

`powershell.exe` executed `THUNDERSHELL (C:\ProgramData\chocolatey\lib\Termini\tools\termini.ps1)`

**Behavioral Anomalies**

* Unusual PowerShell Parameters: Commands such as -ExecutionPolicy Bypass, -NoProfile, and -EncodedCommand can indicate potentially malicious PowerShell activity. The presence of these parameters should be monitored closely.
* Parent Process Analysis: When powershell.exe is spawned by unusual or unexpected parent processes, such as choco.exe in this case, it could be indicative of suspicious behavior, as PowerShell may be launched to facilitate installation or configuration tasks.
* Network Connections Initiated by PowerShell: Look for powershell.exe establishing outbound connections, especially over HTTP/S. This behavior could suggest command-and-control (C2) communication.
* Script Block Logging: High-entropy or obfuscated script blocks can indicate attempts to hide the script's true intentions. Attackers may use encoding or obfuscation to bypass detection.
* File Creation and Execution: Scripts or executable files created by PowerShell in temporary or user data directories are suspicious, particularly if they are executed immediately after creation.
* Persistence Mechanisms: PowerShell can be used to create scheduled tasks or modify registry keys for persistence.

### Step 14.A.3 Application Layer Protocol: Web Protocols ([T1071.001](https://attack.mitre.org/techniques/T1071/001/))

**Detection Criteria**

`powershell.exe` connected to `knuts-to-galleons[.]net (27.21.12.21)` over protocol HTTP `(port 80)​`

**Behavioral Anomalies**

* Creation of new files in temporary directories with specific PowerShell or tool-based scripts.
* Sudden introduction of tools or scripts that are not typically used in the environment.
* High-frequency file creation actions from unusual or administrative users.

**Detection SPL Queries**

```
sourcetype=WinEventLog
| search (TargetFilename="*Termini.ps1*" OR Image="*choco.exe*" OR CommandLine="*wget*" OR CommandLine="*curl*")
```

## Step 15

### Step 15.A.1 Command and Scripting Interpreter: PowerShell ([T1059.001](https://attack.mitre.org/techniques/T1059/001/))

**Detection Criteria**

`THUNDERSHELL (C:\ProgramData\chocolatey\lib\Termini\tools\termini.ps1)` executed `chocolatey`

**Behavioral Anomalies**

* Unusual PowerShell Parameters: Commands such as -ExecutionPolicy Bypass, -NoProfile, and -EncodedCommand can indicate potentially malicious PowerShell activity. The presence of these parameters should be monitored closely.
* Parent Process Analysis: When powershell.exe is spawned by unusual or unexpected parent processes, such as choco.exe in this case, it could be indicative of suspicious behavior, as PowerShell may be launched to facilitate installation or configuration tasks.
* Network Connections Initiated by PowerShell: Look for powershell.exe establishing outbound connections, especially over HTTP/S. This behavior could suggest command-and-control (C2) communication.
* Script Block Logging: High-entropy or obfuscated script blocks can indicate attempts to hide the script's true intentions. Attackers may use encoding or obfuscation to bypass detection.
* File Creation and Execution: Scripts or executable files created by PowerShell in temporary or user data directories are suspicious, particularly if they are executed immediately after creation.
* Persistence Mechanisms: PowerShell can be used to create scheduled tasks or modify registry keys for persistence.

**Detection SPL Queries**

```
index=main sourcetype="WinEventLog:Microsoft-Windows-PowerShell/Operational" EventCode=4100, 4103, 4104, 1
(Image="*\\ssh.exe" OR DestinationPort=22) 
```

### Step 15.A.2 Ingress Tool Transfer ([T1105](https://attack.mitre.org/techniques/T1105/))

**Detection Criteria**

`choco.exe` created `STEALBIT (C:\ProgramData\chocolatey\lib\Connhost\tools\connhost.exe)​`

**Behavioral Anomalies**

* Process-File Relationship: Newly spawned processes associated with dropped files exhibiting anomalous behavior, such as elevated privileges or unexpected communication patterns.
* Unusual File Path: Storing binaries in directories like `\lib\Connhost` where legitimate helper tools are typically placed.
* Use of Transfer Tools: Execution of command-line tools commonly used for file transfers, such as wget, curl, scp, or powershell commands for downloading files, especially by users or processes that do not typically use these tools.

### Step 15.A.3 Command and Scripting Interpreter: PowerShell ([T1059.001](https://attack.mitre.org/techniques/T1059/001/))

**Detection Criteria**

`THUNDERSHELL (C:\ProgramData\chocolatey\lib\Termini\tools\termini.ps1)` executed `STEALBIT (C:\ProgramData\chocolatey\lib\Connhost\tools\connhost.exe)​`

**Behavioral Anomalies**

* Unusual PowerShell Parameters: Commands such as -ExecutionPolicy Bypass, -NoProfile, and -EncodedCommand can indicate potentially malicious PowerShell activity. The presence of these parameters should be monitored closely.
* Parent Process Analysis: When powershell.exe is spawned by unusual or unexpected parent processes, such as choco.exe in this case, it could be indicative of suspicious behavior, as PowerShell may be launched to facilitate installation or configuration tasks.
* Network Connections Initiated by PowerShell: Look for powershell.exe establishing outbound connections, especially over HTTP/S. This behavior could suggest command-and-control (C2) communication.
* Script Block Logging: High-entropy or obfuscated script blocks can indicate attempts to hide the script's true intentions. Attackers may use encoding or obfuscation to bypass detection.
* File Creation and Execution: Scripts or executable files created by PowerShell in temporary or user data directories are suspicious, particularly if they are executed immediately after creation.
* Persistence Mechanisms: PowerShell can be used to create scheduled tasks or modify registry keys for persistence.

### Step 15.A.4 System Information Discovery ([T1082](https://attack.mitre.org/techniques/T1082/))

**Detection Criteria**

`STEALBIT (C:\ProgramData\chocolatey\lib\connhost\tools\connhost.exe)` uses API function `GetComputerName` to get the computer name

**Behavioral Anomalies**

* Unusual Command Execution: Unexpected execution of system information commands like `systeminfo`, `hostname`, or `uname` by users or processes that do not typically require this information. Host information retrieval by unauthorized or non-system-critical processes.
* File Integrity Monitoring: Detecting unauthorized changes to critical files or configurations.
* Unexpected API calls to getcomputername from unusual processes.

### Step 15.A.5 System Network Configuration Discovery ([T1016](https://attack.mitre.org/techniques/T1016/))

**Detection Criteria**

`STEALBIT (C:\ProgramData\chocolatey\lib\connhost\tools\connhost.exe)` uses API function `GetHostName` and `GetAddrInfo` to get domain names

**Behavioral Anomalies**

* Unusual File System Events: Monitoring frequent read/write operations (such as those initiated by main.exe).
* Unusual Network Scanning Activity: An increase in network scanning activities, such as port scans or IP sweeps, may indicate an attempt to discover network configurations.
* Unexpected Configuration Changes: Unauthorized or unexpected changes to network configurations, such as routing tables, firewall rules, or DNS settings, can indicate malicious activity.
* Unauthorized Use of Discovery Protocols: Unauthorized use of network discovery protocols, such as SNMP, LLDP, or CDP, can indicate attempts to gather network configuration information.
* Unusual Command Execution: Unexpected execution of network configuration commands such as `ipconfig`, `ifconfig`, or `route` by users or processes that do not typically require this information.

### Step 15.A.6 File and Directory Discovery ([T1083](https://attack.mitre.org/techniques/T1083/))

**Detection Criteria**

`STEALBIT (C:\ProgramData\chocolatey\lib\connhost\tools\connhost.exe)` enumerated folders and files through `FindFirstFile and FindNextFile`

**Behavioral Anomalies**

* Access to Sensitive or Uncommon Directories: Look for attempts to access directories that contain sensitive information, such as configuration files, user data, or system logs, especially if accessed by unauthorized users.
* Frequent Access to System Directories: Repeated or systematic access to system directories (e.g., /etc, /var, C:\Windows\System32) can indicate an attempt to gather information about the system.
* Use of Discovery Commands: Monitor for unusual use of commands commonly used for file and directory discovery, such as ls, dir, find, tree, grep, or PowerShell equivalents.
* Execution of Scripts for Enumeration: Detection of scripts or automated tools that perform directory enumeration or file listing.
* Access from Unusual Accounts: Attempts to access files or directories from accounts that typically do not perform such actions, especially if the account has elevated privileges.
* Login Anomalies: Logins from unusual locations or at odd hours, followed by file and directory access, can indicate malicious activity.

### Step 15.A.7 Automated Collection ([T1119](https://attack.mitre.org/techniques/T1119/))

**Detection Criteria**

`STEALBIT (C:\ProgramData\chocolatey\lib\connhost\tools\connhost.exe)` collected files from various folders

**Behavioral Anomalies**

* Unusual execution of commands to collect internal data.
* Execution of discovery commands by accounts or systems without administrative privileges.
* Repeated or automated system discovery attempts.
* Unusual File System Events: Unusual write operations in system directories.
* Processes creating hidden or rarely-used files.
* Frequent writing to non-logging directories that typically host static files (e.g., fonts).

**Detection SPL Queries**

```
sourcetype=WinEventLog image="file_access" AND action="read" OR "file_transfer" AND action="copy"
```

### Step 15.A.8 Automated Exfiltration: Traffic Duplication ([T1020.001](https://attack.mitre.org/techniques/T1020/001/))

**Detection Criteria**

`STEALBIT (C:\ProgramData\chocolatey\lib\connhost\tools\connhost.exe)` exfiltrated hashed files in 1024 chunks

**Behavioral Anomalies**

* Monitor for Unusual Data Transfers: Look for large volumes of data being sent to external IP addresses; Identify data transfers occurring at unusual times or from unexpected sources.
* Identify Anomalous Protocol Usage: Monitor for uncommon protocols or ports being used for data transfer.
* Track Repeated Data Transfers: Look for repeated transfers of the same data or patterns that suggest duplication.
* Detect New or Unusual External Connections: Monitor for connections to new or rare external IP addresses; Monitor for connections to known malicious IPs or domains.

**Detection SPL Queries**

```
index=network_traffic
sourcetype=your_sourcetype_name
| eval is_external=if(cidrmatch("0.0.0.0/0", dest_ip) AND NOT cidrmatch("your_internal_ip_range", dest_ip), "external", "internal")
| where is_external="external"
| stats sum(bytes_out) as total_bytes_out, dc(dest_ip) as unique_dest_ips by src_ip, dest_ip, user
```

### Step 15.A.9 Application Layer Protocol: Web Protocols ([T1071.001](https://attack.mitre.org/techniques/T1071/001/))

**Detection Criteria**

`STEALBIT (C:\ProgramData\chocolatey\lib\connhost\tools\connhost.exe)` connects to `muggle-m0ney[.]com (27.21.12.54)` over HTTP `(port 80)`

**Behavioral Anomalies**

* Outbound HTTP/HTTPS traffic to rare or untrusted external domains, especially over default ports like 80 or 443.
* Connections initiated from non-browser processes, which is unusual for typical HTTP communications.
* Consistent outbound traffic from systems that rarely initiate external communications, especially outside of business hours.
* Unencrypted HTTP traffic over port 80, which may expose sensitive data or facilitate C2 operations.

**Detection SPL Queries**

```
index=main sourcetype=network_traffic
| search dest_port=80 OR dest_port=443
| eval suspicious_conn=(case(
  dest_ip IN ("27.21.12.54"), 1,
  process_name NOT IN ("Safari", "Chrome", "Firefox", "curl", "wget"), 1,
  TRUE, 0))
```

### Step 15.A.10 Exfiltration Over C2 Channel ([T1041](https://attack.mitre.org/techniques/T1041/))

**Detection Criteria**

`STEALBIT (C:\ProgramData\chocolatey\lib\connhost\tools\connhost.exe)` exfiltrated files over HTTP `(port 80)`

**Behavioral Anomalies**

* Outbound HTTP/HTTPS traffic to rare or untrusted external domains, especially over default ports like 80 or 443.
* Monitor for Unusual HTTP Traffic Patterns: Look for HTTP requests with large payloads or frequent requests that are atypical for the user or device.
* Analyze User-Agent Strings: Detect uncommon or suspicious User-Agent strings that might indicate non-standard HTTP clients or tools.
* Track Data Volume and Frequency: Monitor for unusually high volumes of data being sent over HTTP. Look for repeated or scheduled data transfers that could indicate automated exfiltration.
* Inspect HTTP Headers and Content: Analyze HTTP headers for anomalies or indicators of tunneling.
* Identify New or Unusual External Connections: Detect connections to new or known malicious IPs/domains over HTTP.

**Detection SPL Queries**

```
index=network_traffic
sourcetype=your_sourcetype_name
dest_port=80
| eval is_external=if(cidrmatch("0.0.0.0/0", dest_ip) AND NOT cidrmatch("your_internal_ip_range", dest_ip), "external", "internal")
| where is_external="external"
| stats sum(bytes_out) as total_bytes_out, values(user_agent) as user_agents, dc(dest_ip) as unique_dest_ips by src_ip, dest_ip
```

### Step 15.A.11 Indicator Removal: File Deletion ([T1070.004](https://attack.mitre.org/techniques/T1070/004/))

**Detection Criteria**

`STEALBIT (C:\ProgramData\chocolatey\lib\connhost\tools\connhost.exe)` deletes itself

**Behavioral Anomalies**

* Batch Scripts: Detection of batch scripts or PowerShell scripts that automate file deletion, particularly if they target log files or other forensic artifacts.
* New or Unusual Processes: Execution of processes associated with file deletion tools or utilities that are not commonly used on the system.
* Antivirus Alerts: Detection of alerts related to file deletion tools or potentially unwanted programs that facilitate file removal.
* Unusual Access Times: File deletions occurring outside normal business hours or from accounts that do not typically perform maintenance tasks.

## Step 16

### Step 16.A.1 Obfuscated Files or Information: Dynamic API Resolution ([T1027.007](https://attack.mitre.org/techniques/T1027/007/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` uses `DJB2` hashing algorithm to obfuscate API calls

**Behavioral Anomalies**

* Changes in Input Data Characteristics: Sudden changes in the characteristics of input data being hashed (e.g., size, format) could indicate an attempt to exploit the hashing process.
* Unauthorized Algorithm Changes: Unauthorized changes to the hashing algorithm or its configuration can compromise security.

### Step 16.A.2 Debugger Evasion ([T1622](https://attack.mitre.org/techniques/T1622/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` hid threads via `NtSetInformationThread`

**Behavioral Anomalies**

* Self-Debugging via Thread Hiding: Creating threads that are hidden from the debugger's view, such as `NtSetInformationThread`.
* System Calls: Malware may use specific system calls to check for the presence of a debugger. For example, using IsDebuggerPresent() on Windows.

### Step 16.A.3 System Owner/User Discovery ([T1033](https://attack.mitre.org/techniques/T1033/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` uses API function `GetUserNameExW` to get usernames​

**Behavioral Anomalies**

* Unexpected Command Execution: Monitoring execution of discovery commands such as logname using root privileges.
* Unusual Shell Execution: If observed shell `(/bin/sh)` commands are uncommon for legitimate Zoom files, this could indicate malicious intent.
* Chained Activity: If combined with file creation or modification events, this strengthens the malicious context.

### Step 16.A.4 System Network Configuration Discovery ([T1016](https://attack.mitre.org/techniques/T1016/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` uses API function `GetComputerNameExW` to get domains​

**Behavioral Anomalies**

* Unusual File System Events: Monitoring frequent read/write operations (such as those initiated by main.exe).
* Unusual Network Scanning Activity: An increase in network scanning activities, such as port scans or IP sweeps, may indicate an attempt to discover network configurations.
* Unexpected Configuration Changes: Unauthorized or unexpected changes to network configurations, such as routing tables, firewall rules, or DNS settings, can indicate malicious activity.
* Unauthorized Use of Discovery Protocols: Unauthorized use of network discovery protocols, such as SNMP, LLDP, or CDP, can indicate attempts to gather network configuration information.
* Unusual Command Execution: Unexpected execution of network configuration commands such as `ipconfig`, `ifconfig`, or `route` by users or processes that do not typically require this information.

### Step 16.A.5 System Information Discovery ([T1082](https://attack.mitre.org/techniques/T1082/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` uses API function `GetComputerNameW` to get hostnames​

**Behavioral Anomalies**

* Unexpected API calls to getcomputername or gethostname from unusual processes.
* Unusual Command Execution: Unexpected execution of system information commands like `systeminfo`, `hostname`, or `uname` by users or processes that do not typically require this information. Host information retrieval by unauthorized or non-system-critical processes.
* File Integrity Monitoring: Detecting unauthorized changes to critical files or configurations.

### Step 16.A.6 System Information Discovery ([T1082](https://attack.mitre.org/techniques/T1082/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` uses API function `GetNativeSystemInfo` to get OS architecture​

**Behavioral Anomalies**

* Unexpected API calls to getsysteminfo from unusual processes.
* Unusual Command Execution: Unexpected execution of system information commands like `systeminfo`, `hostname`, or `uname` by users or processes that do not typically require this information. Host information retrieval by unauthorized or non-system-critical processes.
* File Integrity Monitoring: Detecting unauthorized changes to critical files or configurations.

### Step 16.A.7 System Information Discovery ([T1082](https://attack.mitre.org/techniques/T1082/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` uses API function `RtlGetVersion` to get OS version

**Behavioral Anomalies**

* Unexpected API calls to get OS version from unusual processes.
* Unusual Command Execution: Unexpected execution of system information commands like `systeminfo`, `hostname`, or `uname` by users or processes that do not typically require this information. Host information retrieval by unauthorized or non-system-critical processes.
* System Activity Monitoring: Observing unexpected changes in system performance or configuration, like sudden spikes in CPU usage or unauthorized software installations.
* File Integrity Monitoring: Detecting unauthorized changes to critical files or configurations.

### Step 16.A.8 Execution Guardrails ([T1480](https://attack.mitre.org/techniques/T1480/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` created a `mutex` `(Global\7be06735)`

**Behavioral Anomalies**

* Environment Checks: The presence of scripts or binaries that perform extensive checks on the environment before executing. This might include checking for specific files, registry keys, or system configurations.
* Geolocation Restrictions: Code that checks the geolocation of the system and only executes if it is in a specific region or country.
* User Interaction Requirements: Malicious code that requires specific user actions to execute, such as opening a particular application or document.

### Step 16.A.9 System Location Discovery: System Language Discovery ([T1614.001](https://attack.mitre.org/techniques/T1614/001/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` uses API functions `NtQueryInstallUILanguage` and `NtQueryDefaultUILanguage` to perform system language checks​

**Behavioral Anomalies**

* Increased Load on Language Processing Components: Unexplained spikes in CPU or memory usage related to language processing components might suggest attempts to exploit or overload these systems.
* New or Unusual User Agents: Detection of new or uncommon user agents that might be used to mimic legitimate language processing requests.
* High Volume of Requests from a Single Source: A high volume of requests or queries from a single IP address or user account could indicate automated probing of language capabilities.
* Frequent Access to Language Files or Libraries: An adversary might attempt to access or analyze language-specific files, libraries, or configurations to understand how the system processes different languages.
* Repeated Requests for Language-Specific Resources: Look for repeated or unusual requests for resources that are specific to certain languages, such as language packs or localization files.
* Mixed or Obfuscated Language Inputs: Inputs that mix multiple languages or use obfuscation techniques might indicate attempts to probe the system's language detection capabilities.
* Unexpected Code or Script Submissions: Submissions of code or scripts in unexpected languages or formats could be an attempt to test the system's response to different language inputs.

### Step 16.A.10 File and Directory Discovery ([T1083](https://attack.mitre.org/techniques/T1083/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` uses API function `GetLogicalDriveStringW` to enumerate logical drives​

**Behavioral Anomalies**

* Access to Sensitive or Uncommon Directories: Look for attempts to access directories that contain sensitive information, such as configuration files, user data, or system logs, especially if accessed by unauthorized users.
* Frequent Access to System Directories: Repeated or systematic access to system directories (e.g., /etc, /var, C:\Windows\System32) can indicate an attempt to gather information about the system.
* Use of Discovery Commands: Monitor for unusual use of commands commonly used for file and directory discovery, such as ls, dir, find, tree, grep, or PowerShell equivalents.
* Execution of Scripts for Enumeration: Detection of scripts or automated tools that perform directory enumeration or file listing.
* Access from Unusual Accounts: Attempts to access files or directories from accounts that typically do not perform such actions, especially if the account has elevated privileges.
* Login Anomalies: Logins from unusual locations or at odd hours, followed by file and directory access, can indicate malicious activity.

### Step 16.A.11 Impair Defenses: Disable or Modify System Firewall ([T1562.004](https://attack.mitre.org/techniques/T1562/004/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` disables the firewall (domain and standard profiles) via registry edits (`RegCreateKeyExW` and `RegSetValueExW`)

Registry key edits are as follows:
* `HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\WindowsFirewall\DomainProfile`
  * `EnableFirewall set to 0`
* `HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\WindowsFirewall\StandardProfile`
  * `EnableFirewall set to 0`

**Behavioral Anomalies**

* Unexpected Firewall Configuration Changes: Sudden or unauthorized changes to firewall rules or settings, especially those that allow inbound or outbound traffic that was previously blocked.
* Disabling of Firewall Services: Attempts to stop or disable firewall services, such as the Windows Firewall service or equivalent services on other operating systems.
* Log Tampering: Deletion or modification of firewall logs, which may indicate an attempt to cover tracks after changing firewall settings.

**Detection SPL Queries**

```
(sourcetype=WinEventLog:Sysmon OR sourcetype=WinEventLog:Security)
(EventCode=13 OR EventCode=4657)
RegistryKeyName IN ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\WindowsFirewall\\DomainProfile\\EnableFirewall" "HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\WindowsFirewall\\StandardProfile\\EnableFirewall")
```

### Step 16.A.12 Windows Management Instrumentation ([T1047](https://attack.mitre.org/techniques/T1047/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` executed WMI query `"SELECT * FROM Win32_ShadowCopy"` via CoInitializeEx, IWbemLocator::ConnectServer, and IWbemServices::ExecQuery

**Behavioral Anomalies**

* WMI Scripting Abuse: Use of WMI scripting (e.g., VBScript or PowerShell scripts leveraging WMI) to perform tasks that are typically not automated or are outside normal business operations.
* Log Anomalies: Unusual entries in event logs related to WMI activity, such as Event ID 5858, or 5861, which may indicate errors or suspicious activity.
* High-frequency or repeated WMI queries targeting specific namespaces (root\cimv2, Win32_* classes).
* Unexpected WMI connections to remote systems using non-standard credentials.
* Abnormal invocation of COM APIs such as IWbemLocator::ConnectServer or IWbemServices::ExecQuery.

**Detection SPL Queries**

```
index=windows sourcetype=WinEventLog:* (EventCode=4688 OR EventCode=5861 OR EventCode=4104)
| eval commandline=coalesce(CommandLine, ScriptBlockText)
| search commandline="*SELECT*FROM*Win32_ShadowCopy*" OR commandline="*root\\cimv2*" OR commandline="*ExecQuery*"
| table _time, ComputerName, User, ParentImage, commandline, is_wmi_abuse
```

### Step 16.A.13 Inhibit System Recovery ([T1490](https://attack.mitre.org/techniques/T1490/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` deletes shadow copies via `IWbemServices::DeleteInstance`

**Behavioral Anomalies**

* Disabling or Deleting Shadow Copies: Commands or scripts that delete Volume Shadow Copies (e.g., using vssadmin delete shadows), which are used by Windows to create backup copies of files.
* Unexpected File Deletions: Deletion of critical system files or logs that could hinder forensic analysis or system recovery efforts.

### Step 16.A.14 Inhibit System Recovery ([T1490](https://attack.mitre.org/techniques/T1490/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` disabled automatic Windows recovery features with `bcdedit.exe /set {default} bootstatuspolicy ignoreallfailures` OR `bcdedit.exe /set {default} recoveryenabled no`

**Behavioral Anomalies**

* Tampering with Recovery Partitions: Access or modifications to recovery partitions on disk drives, which are often used to restore systems to factory settings.
* Disabling System Restore: Unexpected changes to system settings that disable or delete system restore points, which are critical for recovering from system failures or malware infections.
* Suspicious Use of System Tools: Use of system tools like bcdedit, wbadmin, or vssadmin in a manner inconsistent with normal administrative tasks, especially by non-administrative users.

### Step 16.A.15 System Network Configuration Discovery ([T1016](https://attack.mitre.org/techniques/T1016/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` used API function `GetAdapterInfo` to discover local IP networks

**Behavioral Anomalies**

* Unusual File System Events: Monitoring frequent read/write operations (such as those initiated by main.exe).
* Unusual Network Scanning Activity: An increase in network scanning activities, such as port scans or IP sweeps, may indicate an attempt to discover network configurations.
* Unexpected Configuration Changes: Unauthorized or unexpected changes to network configurations, such as routing tables, firewall rules, or DNS settings, can indicate malicious activity.
* Unauthorized Use of Discovery Protocols: Unauthorized use of network discovery protocols, such as SNMP, LLDP, or CDP, can indicate attempts to gather network configuration information.
* Unusual Command Execution: Unexpected execution of network configuration commands such as `ipconfig`, `ifconfig`, or `route` by users or processes that do not typically require this information.

### Step 16.A.16 Remote System Discovery ([T1018](https://attack.mitre.org/techniques/T1018/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` discovers additional targets through ICMP or echo request/response​ via `IcmpSendEcho`

**Behavioral Anomalies**

* Unusual Activity from User Accounts: Monitor volume of ARP requests and increased or abnormal use of commannds to enumerate domain information such as `nltest /dclist`.
* Increased Network Mapping Activities: Monitoring for increased ICMP sweeps originating from internal hosts that do not usually perform such actions.

### Step 16.A.17 Remote Services: SMB/Windows Admin Shares ([T1021.002](https://attack.mitre.org/techniques/T1021/002/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` executed `C:\ProgramData\chocolatey\lib\sysinternals\tools\PsExec64.exe" -accepteula \\10.111.8.50,10.111.9.201,10.111.9.202 -u decryptmalfoy.net\gornuk.da -p Frown-Cement -s -e -d -f -c "C:\Windows\main.exe" -d -w` over SMB protocol​

**Behavioral Anomalies**

* Unusual Processes: Monitor interactions with network shares, such as reads or file transfers, using Server Message Block (SMB).
* Cross-Platform Connections: SSH sessions initiated from Windows to Linux systems can be unusual, especially if not commonly practiced in the environment.
* Privileged Account Use: The use of high-privilege accounts (such as GORNUK.DA) for SMB connections, especially outside normal business hours or from unexpected source IPs.
* Chained Activity: Monitor for newly constructed network connections (typically over ports 139 or 445) correlated with remote login events and associated SMB-related activity such as file transfers and remote process execution.
* File Writes: Creation of malicious executables (e.g., ransomware binaries) in shared directories.
* Registry and Service Manipulation: Changes to services or firewall rules post-execution to maintain persistence.

**Detection SPL Queries**

```
EventCode=1
| eval suspicious_command=if(like(CommandLine, "%PsExec64.exe%") OR like(CommandLine, "%-accepteula%"), "true", "false")
| where suspicious_command="true"
| stats count by ComputerName, User, ParentImage, CommandLine, ParentCommandLine
```

### Step 16.A.18 Valid Accounts: Domain Accounts ([T1078.002](https://attack.mitre.org/techniques/T1078/002/))

**Detection Criteria**

`GORNUK` successfully authenticated with domain admin credentials

**Behavioral Anomalies**

* Unusual Access Patterns: Monitor for deviations in the types of resources accessed.
* Volume of Activity: Monitor for unusual activity such as a sudden increase in data access or file transfers.
* Failed Login Attempts: A high number of failed login attempts can suggest an attempt to compromise an account.
* Concurrent Sessions: Monitor for multiple concurrent sessions from different locations or devices.
* Privileged Account Use: Unusual use of privileged accounts, such as accessing administrative tools or making unauthorized changes.

**Detection SPL Queries**

```
(Account_Name=username EventCode IN (4648))
```

### Step 16.A.19 Create or Modify System Process: Windows Service ([T1543.003](https://attack.mitre.org/techniques/T1543/003/))

**Detection Criteria**

`PsExec64.exe` creates a service (`PSExecSvc`)

**Behavioral Anomalies**

* Log Anomalies: Unusual entries in system logs related to service creation or modification, such as Event ID 7045 (A service was installed in the system) in the Windows Event Log.
* Use of System Tools for Service Management: Use of command-line tools like psexec.exe, sc.exe, powershell, or wmic to create or modify services, especially by users who do not typically perform administrative tasks.
* Creation of New Services: Detection of new services being created on a system, especially if they are created by non-administrative users or at unusual times.

**Detection SPL Queries**

```
index=<yourindex> EventCode=7045
```

### Step 16.A.20 Lateral Tool Transfer ([T1570](https://attack.mitre.org/techniques/T1570/))

**Detection Criteria**

`PsExec64.exe` copies `LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` to `KNOCKTURNALLEY`, `VAULT12`, `GAUNTSHACK​`

**Behavioral Anomalies**

* Use of Built-in Tools for File Transfer: Use of native tools like PsExec, Robocopy, xcopy, certutil, or PowerShell to copy files between systems, particularly by users who do not typically perform such tasks.
* Suspicious Network Connections: Network connections between internal systems that are not part of regular communication patterns, potentially indicating tool transfer.

**Detection SPL Queries**

```
(host="src_host" EventCode=1 Image=“*psexec*” CommandLine=“*main.exe*”) OR
(host="dst_host" EventCode=11 TargetFileName=“*main.exe*”) 
```

### Step 16.A.21 System Services: Service Execution ([T1569.002](https://attack.mitre.org/techniques/T1569/002/))

**Detection Criteria**

`PsExec64.exe` executes `LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` on `KNOCKTURNALLEY`, `VAULT12`, `GAUNTSHACK​`

**Behavioral Anomalies**

* Use of System Tools for Service Management: Use of command-line tools like psexec.exe, sc.exe, powershell, or wmic to create or modify services, especially by users who do not typically perform administrative tasks.
* Network Connections from Service Processes: Network connections initiated by service processes to external or unusual IP addresses, which could indicate command-and-control communication or data exfiltration.
* Unusual Service Creation: Creation of services with suspicious names or pointing to non-standard directories.
Services pointing to executables within user-writable paths or temp folders.
* Parent Process Relationships: Execution of PsExec64.exe with command-line parameters invoking service execution on remote targets. Parent processes such as PowerShell or cmd.exe initiating PsExec64.exe.
* Network Communication: Outbound SMB or RPC traffic to remote systems initiating service execution. Unusual frequency of service creation across multiple systems in a short time.
* Process Behavior: Remote execution of processes like ransomware binaries (main.exe) as child processes of services. Services being executed with admin privileges or domain credentials.

**Detection SPL Queries**

```
EventCode IN (7045, 1)
| eval is_psexec=if(like(CommandLine, "%PsExec64.exe%") OR like(CommandLine, "%-accepteula%"), "true", "false")
| eval is_service_execution=if(like(Service_Name, "%PSEXESVC%"), "true", "false")
| eval correlation_flag=if(is_psexec="true" OR is_service_execution="true", "true", "false")
| where correlation_flag="true"
| stats count by ComputerName, User, CommandLine, ParentImage, ParentCommandLine, TargetHost
```

### Step 16.A.22 Process Discovery ([T1057](https://attack.mitre.org/techniques/T1057/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` enumerates processes via `CreateToolhelp32Snapshot`, `Process32FirstW`, and `Process32NextW`

**Behavioral Anomalies**

* Unusual Command Execution: Unusual use of process enumeration commands like `tasklist` or `ps`, especially if executed by non-administrative users or new processes or at unusual times.
* Chained Activity: Monitor for unusual sequence of commands that suggests reconnaissance or environment enumeration without associated normal administrative activity.

### Step 16.A.23 Service Stop ([T1489](https://attack.mitre.org/techniques/T1489/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` terminates processes via `OpenProcess` and `TerminateProcess`

**Behavioral Anomalies**

* Sudden service stop or disable actions, particularly involving security or monitoring services.
* Service management commands executed at unexpected times or by unusual accounts.
* Sequence of commands that suggests preparation for system shutdown or application/service disruptions.

### Step 16.A.24 Service Stop ([T1489](https://attack.mitre.org/techniques/T1489/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` terminates and disables services (such as `VSSC`) via `ControlService` and `ChangeServiceConfigW`

**Behavioral Anomalies**

* Sudden service stop or disable actions, particularly involving security or monitoring services.
* Service management commands executed at unexpected times or by unusual accounts.
* Sequence of commands that suggests preparation for system shutdown or application/service disruptions.

### Step 16.A.25 Data Destruction ([T1485](https://attack.mitre.org/techniques/T1485/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` clears the recycle bin

**Behavioral Anomalies**

* Mass File Deletions: Sudden and large-scale deletion of files or directories, especially in critical or sensitive areas, which is not part of regular maintenance or operations.
* Log Tampering: Deletion or modification of system or application logs, which may indicate an attempt to cover tracks after data destruction.

### Step 16.A.26 Modify Registry ([T1112](https://attack.mitre.org/techniques/T1112/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` writes registry keys `HKCR\.sk12uyqzk` and `HKCR\.sk12uyqzk\DefaultIcon` to customize icons for encrypted files

**Behavioral Anomalies**

* Use of Registry Editing Tools: Use of tools like regedit, reg.exe, or PowerShell cmdlets to modify the registry, especially by non-administrative users or from unusual locations.
* Alteration of File Associations: Changes to registry keys that control file associations, which could be used to hijack file types and execute malicious code.

### Step 16.A.27 Modify Registry ([T1112](https://attack.mitre.org/techniques/T1112/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` writes RSA public and private session keys to `HKCU\SOFTWARE\BlockBlit` via `RegCreateKeyExW` and `RegSetValueExW`

**Behavioral Anomalies**

* Use of Registry Editing Tools: Use of tools like regedit, reg.exe, or PowerShell cmdlets to modify the registry, especially by non-administrative users or from unusual locations.
* Unexpected Registry Key Changes: Modifications to critical registry keys or values that are not part of regular system updates or administrative tasks. This includes keys related to startup programs, services, or security settings.
* Modification of Security-Related Keys: Changes to registry keys that affect security settings, such as disabling Windows Defender, firewall settings, or User Account Control (UAC).

### Step 16.A.28 Obfuscated Files or Information ([T1027](https://attack.mitre.org/techniques/T1027/))

**Detection Criteria**

RSA private session key is encrypted using RSA

**Behavioral Anomalies**

* Use of Encoding or Encryption: Detection of files or scripts that are encoded or encrypted, especially if they are not typically used in the environment. This includes the use of Base64 encoding, XOR encryption, or custom encryption algorithms.
* Network Traffic with Encrypted Payloads: Detection of network traffic with encrypted or encoded payloads that do not match typical patterns for the organization, potentially indicating data exfiltration or command-and-control communication (i.e. data is encrypted with RSA prior to exfiltration).

### Step 16.A.29 File and Directory Discovery ([T1083](https://attack.mitre.org/techniques/T1083/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` enumerates target files in main `C:` drive

**Behavioral Anomalies**

* Access to Sensitive or Uncommon Directories: Look for attempts to access directories that contain sensitive information, such as configuration files, user data, or system logs, especially if accessed by unauthorized users.
* Frequent Access to System Directories: Repeated or systematic access to system directories (e.g., /etc, /var, C:\Windows\System32) can indicate an attempt to gather information about the system.
* Use of Discovery Commands: Monitor for unusual use of commands commonly used for file and directory discovery, such as ls, dir, find, tree, grep, or PowerShell equivalents.
* Execution of Scripts for Enumeration: Detection of scripts or automated tools that perform directory enumeration or file listing.
* Access from Unusual Accounts: Attempts to access files or directories from accounts that typically do not perform such actions, especially if the account has elevated privileges.
* Login Anomalies: Logins from unusual locations or at odd hours, followed by file and directory access, can indicate malicious activity.

### Step 16.A.30 Data Encrypted for Impact ([T1486](https://attack.mitre.org/techniques/T1486/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` encrypts target files in main `C:` using `256-bit AES`

**Behavioral Anomalies**

* Mass File Access and Encryption: Sudden and widespread access to a large number of files, followed by modifications, can indicate encryption activities. This often involves changing file extensions to those commonly associated with ransomware.
* Creation of Encrypted File Copies: Detection of new files with encrypted content alongside original files, often with extensions like .encrypted, .locked, or specific ransomware-related extensions.
* Data Exfiltration Patterns: Unusual patterns of data being sent out of the network, which might occur before or during the encryption process.
* Processes with High I/O Activity: Processes that exhibit unusually high disk I/O activity, such activity could indicate bulk file encryption.

### Step 16.A.31 File and Directory Discovery ([T1083](https://attack.mitre.org/techniques/T1083/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` enumerates target files in mounted file share `Z:` drive

**Behavioral Anomalies**

* Access to Sensitive or Uncommon Directories: Look for attempts to access directories that contain sensitive information, such as configuration files, user data, or system logs, especially if accessed by unauthorized users.
* Frequent Access to System Directories: Repeated or systematic access to system directories (e.g., /etc, /var, C:\Windows\System32) can indicate an attempt to gather information about the system.
* Use of Discovery Commands: Monitor for unusual use of commands commonly used for file and directory discovery, such as ls, dir, find, tree, grep, or PowerShell equivalents.
* Execution of Scripts for Enumeration: Detection of scripts or automated tools that perform directory enumeration or file listing.
* Access from Unusual Accounts: Attempts to access files or directories from accounts that typically do not perform such actions, especially if the account has elevated privileges.
* Login Anomalies: Logins from unusual locations or at odd hours, followed by file and directory access, can indicate malicious activity.

### Step 16.A.32 Data Encrypted for Impact ([T1486](https://attack.mitre.org/techniques/T1486/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)`, on `QUIRRELL`, encrypts target files in mounted file share `Z:` (on `GAUNTSHACK`) drive using `256-bit AES`

**Behavioral Anomalies**

* Mass File Access and Encryption: Sudden and widespread access to a large number of files, followed by modifications, can indicate encryption activities. This often involves changing file extensions to those commonly associated with ransomware.
* Creation of Encrypted File Copies: Detection of new files with encrypted content alongside original files, often with extensions like .encrypted, .locked, or specific ransomware-related extensions.
* Data Exfiltration Patterns: Unusual patterns of data being sent out of the network, which might occur before or during the encryption process.
Processes with High I/O Activity: Processes that exhibit unusually high disk I/O activity, such activity could indicate bulk file encryption.

### Step 16.A.33 Defacement: Internal Defacement ([T1491.001](https://attack.mitre.org/techniques/T1491/001/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` creates a ransom note (`C:\Users\gornuk\.ssh\sk12uyqzk.README.txt`)

**Behavioral Anomalies**

* Unexpected File Uploads: Detection of unauthorized file uploads to web servers, which could be used to replace legitimate content with defaced pages.
* Network Traffic Anomalies: Unusual network traffic patterns involving web servers, such as unexpected outbound connections or data transfers.
* Unauthorized Changes to Web Content: Detection of unexpected modifications to web pages or internal applications, such as altered text, images, or links, especially if not part of scheduled updates or maintenance.

**Detection SPL Queries**

```
Image="*main.exe*" EventCode=11 TargetFilename "README.txt"
```

### Step 16.A.34 Modify Registry ([T1112](https://attack.mitre.org/techniques/T1112/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` modifies `HKCU\Control Panel\Desktop` via `RegCreateKeyExW` and `RegSetValueExW`

**Behavioral Anomalies**

* Execution Context and Use of Registry Editing Tools: Use of uncommon APIs like RegSetValueExW or tools like reg.exe, regedit, or PowerShell cmdlets to modify the registry, especially by non-administrative users or from unusual locations.
* Unexpected Registry Key Changes: Modifications to critical registry keys or values that are not part of regular system updates or administrative tasks. This includes keys related to startup programs, services, or security settings.
* Unusual Modification of Security-Related Keys: Changes to registry keys that affect security settings, such as disabling Windows Defender, firewall settings, or User Account Control (UAC). Keys like HKCU\Control Panel\Desktop or HKLM being modified by unexpected processes (e.g., BlockLit).
* User Behavior: High-privilege users modifying sensitive keys during unusual times.

**Detection SPL Queries**

```
EventCode=13
| eval is_registry_modify=if(like(TargetObject, "%Control Panel\\Desktop\\Wallpaper%") AND (Details="%C:\\ProgramData%"), "true", "false")
| eval is_wallpaper_change=if(is_registry_modify="true" AND (Image like "%main.exe%"), "true", "false")
| where is_registry_modify="true"
| table _time, ComputerName, User, Image, CommandLine, TargetObject
```

### Step 16.A.35 Defacement: Internal Defacement ([T1491.001](https://attack.mitre.org/techniques/T1491/001/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` changes `GORNUK's` wallpaper to `C:\ProgramData\sk12uyqzk.bmp`

**Behavioral Anomalies**

* Unexpected File Uploads: Detection of unauthorized file uploads to web servers, which could be used to replace legitimate content with defaced pages.
* Network Traffic Anomalies: Unusual network traffic patterns involving web servers, such as unexpected outbound connections or data transfers.
* Unauthorized Changes to Web Content: Detection of unexpected modifications to web pages or internal applications, such as altered text, images, or links, especially if not part of scheduled updates or maintenance.

### Step 16.A.36 Indicator Removal: Clear Windows Event Logs ([T1070.001](https://attack.mitre.org/techniques/T1070/001/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` clears Windows Event Logs via EvtClearLog

**Behavioral Anomalies**

* Use of Log Clearing Commands: Execution of commands or scripts that clear event logs, such as EvtClearLog, wevtutil cl or PowerShell cmdlets like Clear-EventLog, particularly by non-administrative users or from unusual locations.
* Unexpected Log Clearing: Detection of event logs being cleared without a legitimate reason, especially on critical systems or outside of scheduled maintenance windows.
* Unusual System Activity: Anomalous system activity preceding or following log clearing, such as unexpected process executions, file modifications, or network connections.
* Absence of Expected Logs: Missing expected log entries for critical events, such as security alerts, system errors, or administrative actions.

**Detection SPL Queries**

```
 "LogName=Security" EventCode=1102
```

### Step 16.A.37 Impair Defenses: Disable Windows Event Logging ([T1562.002](https://attack.mitre.org/techniques/T1562/002/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` disables event log channels via `EvtSetChannelConfigProperty`

**Behavioral Anomalies**

* Unusual System Activity: Anomalous system activity preceding or following changes to logging settings, such as unexpected process executions, file modifications, or network connections.
* Log Anomalies: Sudden absence or reducion of expected log entries, particularly for security-related events, which may indicate that logging has been disabled or tampered with.
* Unauthorized Access to Logging Configurations: Attempts to access or modify logging configurations by users who do not typically perform such tasks, or from unexpected IP addresses or geographic locations.

### Step 16.A.38 Indicator Removal: File Deletion ([T1070.004](https://attack.mitre.org/techniques/T1070/004/))

**Detection Criteria**

`LOCKBIT (C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe)` executes `C:\Windows\temp\381jwk.tmp` to overwrite, rename, and delete `LOCKBIT`

**Behavioral Anomalies**

* Unexpected Deletion of Executables: Monitor for high-privilege processes executing file deletion commands, particularly in non-standard directories (such as, `\temp\381jwk.tmp`).
* Timing-based Process Termination: Monitor for process termination commands that are invoked after file deletion, especially with timing components like sleep.
* Multiple Related File Deletions: Detect simultaneous deletion of executable files and their configuration counterparts, suggesting potential malware cleanup activity.
* Privilege Escalation: Users escalating privileges shortly before or during file deletion activities, which may indicate attempts to gain the necessary permissions to delete protected files.
* Unusual Access Times: File deletions occurring outside normal business hours or from accounts that do not typically perform maintenance tasks.
