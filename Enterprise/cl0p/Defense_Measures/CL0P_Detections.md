# CL0P Detection Analysis

This guide covers key detection criteria for CL0P-related ransomware as emulated by the Enterprise Round 6 Emulation Plan It involves tracking the malicious activities of SDBbot, a loader used to deploy CL0P, and its associated behaviors like process injection, reflective code loading, and file manipulation. Key indicators of compromise include:

* **Registry Modifications**: Monitoring for unauthorized registry changes ([T1112](https://attack.mitre.org/techniques/T1112/)) and IFEO debugger settings modification ([T1546.012](https://attack.mitre.org/techniques/T1546/012)).
* **Reflective Code Loading**: Identifying reflective DLL injections by SDBbot in winlogon.exe ([T1620](https://attack.mitre.org/techniques/T1620/)).
* **Process & System Discovery**: Detecting information-gathering behaviors ([T1082](https://attack.mitre.org/techniques/T1082/)) targeting hostname, user, and system data.
* **File and Directory Discovery**: Monitoring suspicious file access and discovery commands ([T1083](https://attack.mitre.org/techniques/T1083/)).
* **Exfiltration and Network Activity**: Detecting C2 communication over TCP port 443 ([T1095](https://attack.mitre.org/techniques/T1095/)) and exfiltration attempts ([T1041](https://attack.mitre.org/techniques/T1041/)).

### Key Findings

* **SDBbot Deployment**: Rundll32 is used to proxy-load malicious DLLs.
* **Fileless Persistence**: Registry modifications ensure persistence without explicit files on disk.
* **Reflective Code Loading**: SDBbot loads payloads into processes such as winlogon.exe to evade detection.
* **System Discovery and Exfiltration**: Gathers system information and exfiltrates over C2 channels.

### Detection Tools

* **Sysmon**: For detecting reflective DLL loading, registry modifications, and process execution.
* **YARA**: To write signatures for detecting CL0P malware components and DLLs.
* **Zeek (formerly Bro)**: Network monitoring for identifying suspicious network connections to C2 domains.
* **Suricata**: For detecting abnormal traffic patterns, especially around port 443.

## Step 5

### 5.A.1 System Binary Proxy Execution: Rundll32 ([T1218.011](https://attack.mitre.org/techniques/T1218/011/))

**Detection Criteria**

`SDBbot` malware execution is detected through the usage of `rundll32.exe` to install an unsigned DLL (`IFInstaller.dll`) located in the user's Roaming directory. The event indicates `GRIPHOOK` executing this command on the system named `DIAGONALLEY`.

**Behavioral Anomalies**

* Execution of `rundll32.exe` with command-line parameters specifying an unusual DLL path.
* Loading of an unsigned DLL from a non-standard directory (AppData\Roaming).
* Anomalous process creation and command-line execution.

**Detection SPL Queries**

```
index=* source=WinEventLog:Microsoft-Windows-Sysmon/Operational 
| where CommandLine="rundll32.exe C:\\Users\\griphook\\AppData\\Roaming\\IFInstaller.dll,install" 
```

### 5.A.2 Obfuscated Files or Information: Embedded Payloads ([T1027.009](https://attack.mitre.org/techniques/T1027/009/))

**Detection Criteria**

The SDBbot Installer (`C:\Users\griphook\AppData\Roaming\IFInstaller.dll`) contains an embedded Portable Executable (PE) file within its resource section. This technique hides malicious payloads within seemingly benign files to evade detection.

**Behavioral Anomalies**

* Unusual or excessive use of resource sections within DLL files.
* Presence of embedded PE files within DLL resources.

### 5.A.3 Modify Registry ([T1112](https://attack.mitre.org/techniques/T1112/))

**Detection Criteria**

The `SDBbot` installer stores the loader shellcode and embedded PE file in the Windows registry under the key `HKLM\SOFTWARE\Microsoft\skw`. This technique involves hiding the malicious payload within the registry to avoid detection.

**Behavioral Anomalies**

* Unusual registry writes to HKLM\SOFTWARE\Microsoft\skw.
* Presence of binary data within the registry key, indicating potential embedded PE file.

**Detection SPL Queries**

```
index=windows EventCode=13 ObjectType=Registry KeyName="HKLM\SOFTWARE\Microsoft\skw" 
```

### 5.A.4 Modify Registry ([T1112](https://attack.mitre.org/techniques/T1112/))

**Detection Criteria**

`rundll32.exe` is used to download the `SDBbot Loader DLL` to `C:\Windows\temp\tmp8AB2.tmp`, creating a symbolic link `C:\Windows\System32\msverload.dll`. It then modifies the Image File Execution Options (IFEO) debugger registry settings to point to this DLL. The registry keys modified are:

* HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\
* VerifierDLL set to msverload
* FilterFullPath set to winlogon

**Behavioral Anomalies**

* Unusual usage of rundll32.exe to download DLL files.
* Modifications to the IFEO registry settings, especially setting VerifierDLL and FilterFullPath values.
* Creation of symbolic links in the System32 directory.

**Detection SPL Queries**

```
index=windows EventCode=13 ObjectType=Registry KeyName="HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\*" ValueName IN ("VerifierDLL", "FilterFullPath") 
```

### 6.A.1 Event Triggered Execution: Image File Execution Options Injection ([T1546.012](https://attack.mitre.org/techniques/T1546/012/))

**Detection Criteria**

`winlogon.exe` executed with an embedded `SDBbot Loader`, an unsigned DLL, found in `C:\Windows\Temp\tmp8AB2.tmp`, triggered post-login. This is indicative of Image File Execution Options (IFEO) injection, a technique where an attacker configures the system to execute a malicious payload when a specific executable is launched.

**Behavioral Anomalies**

* Execution of unexpected DLLs by winlogon.exe.
* Presence of unsigned DLLs in C:\Windows\Temp\.
* Modifications in IFEO registry settings for winlogon.exe.

**Detection SPL Queries**

```
(process_name="rundll32.exe" file_path="C:\\Windows\\Temp\\tmp8AB2.tmp") OR (process_name="winlogon.exe" ImageLoaded="C:\\Windows\\Temp\\tmp8AB2.tmp")
```

### 6.A.2 Reflective Code Loading ([T1620](https://attack.mitre.org/techniques/T1620/))

**Detection Criteria**

`winlogon.exe` reflectively loads the `SDBbot RAT` by replacing `setupapi.dll` with shellcode. This involves manipulating winlogon.exe to use a malicious setupapi.dll, allowing the attacker to inject and execute the RAT without writing the payload to disk.

**Behavioral Anomalies**

* Loading of an unexpected or unsigned setupapi.dll by winlogon.exe.
* Execution of processes or DLLs not typically associated with winlogon.exe

**Detection SPL Queries**

```
index=process_monitor sourcetype=process_creation process_name="winlogon.exe" image_path="C:\\Windows\\System32\\setupapi.dll" 
```

### 6.A.3 Process Discovery ([T1057](https://attack.mitre.org/techniques/T1057/))

**Detection Criteria**

The `SDBbot` Remote Access Trojan (RAT) uses `winlogon.exe` to gather information about running processes by calling API functions such as `GetCurrentProcess`, `GetTokenInformation`, and `OpenProcessToken`. This behavior can indicate attempts to gather information about other processes for further exploitation or evasion techniques.

**Behavioral Anomalies**

* winlogon.exe calling process discovery API functions unexpectedly.
* Elevated privileges used by winlogon.exe to query process information.

### 6.A.4 System Information Discovery ([T1082](https://attack.mitre.org/techniques/T1082/))

**Detection Criteria**

The `SDBbot` RAT uses `winlogon.exe` to discover system information, specifically obtaining the domain name through API functions `gethostname` and `getaddrinfo`. This technique allows the malware to gather network configuration details, aiding in further exploitation and lateral movement.

**Behavioral Anomalies**

* Unusual network-related API calls from winlogon.exe.
* Increased frequency of gethostname and getaddrinfo calls, indicating potential reconnaissance activity.

### 6.A.5 System Information Discovery ([T1082](https://attack.mitre.org/techniques/T1082/))

**Detection Criteria**

The `SDBbot` RAT leverages winlogon.exe to gather system information, specifically obtaining the computer name through the API function `GetComputerName`. This helps the malware identify the system it has compromised, which can assist in further exploitation or lateral movement.

**Behavioral Anomalies**

* winlogon.exe making unusual calls to GetComputerName.
* Increased frequency of system information queries from winlogon.exe.

### 6.A.6 System Owner/User Discovery ([T1033](https://attack.mitre.org/techniques/T1033/))

**Detection Criteria**

The `SDBbot` RAT leverages winlogon.exe to gather information about the system's owner or user by utilizing the `GetUserNameA` API function. This allows the malware to identify the username of the current user on the compromised system, aiding in tailored attacks or further exploitation.

**Behavioral Anomalies**

* Unusual or frequent calls to GetUserNameA by winlogon.exe.
* winlogon.exe accessing user information outside of normal operating times or patterns.

### 6.A.7 System Information Discovery ([T1082](https://attack.mitre.org/techniques/T1082/))

**Detection Criteria**

The `SDBbot` RAT leverages winlogon.exe to gather geographical information about the system by utilizing the `GetUserGeoID` and `GetGeoInfoW` API functions. This allows the malware to identify the country code and other geographical details, aiding in targeted attacks or tailoring further actions based on the system's location.

**Behavioral Anomalies**

* Unusual or frequent calls to GetUserGeoID and GetGeoInfoW by winlogon.exe.
* winlogon.exe accessing geographical information outside of normal operating patterns or times.

### 6.A.8 System Information Discovery ([T1082](https://attack.mitre.org/techniques/T1082/))

**Detection Criteria**

The `SDBbot` RAT leverages `winlogon.exe` to gather system information by calling the `RtlGetVersionAPI` function to determine the Windows version. This information helps the malware tailor its behavior based on the operating system it is running on.

**Behavioral Anomalies**

* Unexpected calls to RtlGetVersion by winlogon.exe, especially if not part of standard operating procedures.
* Frequent or repetitive access to the RtlGetVersion API, indicating potential reconnaissance activity.

### 6.A.9 Non-Application Layer Protocol ([T1095](https://attack.mitre.org/techniques/T1095/))

**Detection Criteria**
Winlogon.exe establishes communication to fortius-quo-fidelius[.]co[.]uk (14.121.222.11) over TCP port 443. This behavior indicates a potential misuse of the winlogon.exe process for unauthorized network communication.

**Behavioral Anomalies**

* Unusual outbound connections from winlogon.exe.
* Communication to uncommon or suspicious external IP addresses.
* Usage of TCP port 443 by non-browser applications.

**Detection SPL Queries**

```
index=network_processes sourcetype=network_traffic dest_ip="14.121.222.11" dest_port=443 process_name="winlogon.exe" 
```

### 7.A.1 File and Directory Discovery ([T1083](https://attack.mitre.org/techniques/T1083/))

**Detection Criteria**

The `SDBbot` RAT uses winlogon.exe to execute cmd.exe with the command dir `C:\users\` to enumerate user directories on the host `diagonalley`. This behavior helps the RAT gather information about the file system structure and user accounts.

**Behavioral Anomalies**

* winlogon.exe invoking cmd.exe to list directories, which is uncommon for normal operations.
* Repeated directory listing commands that do not align with typical user or system processes.

**Detection SPL Queries**

```
index=* sourcetype=* host=diagonalley CommandLine="*dir C:\\users\\*" 
```

### 7.A.2 Exfiltration Over C2 Channel ([T1041](https://attack.mitre.org/techniques/T1041/))

**Detection Criteria**

The `SDBbot` RAT exfiltrates a file to `fortius-quo-fidelius[.]co[.]uk (14.121.222.11)` over `TCP (port 443)` on the host `diagonalley`.

**Behavioral Anomalies**

* Unusual network traffic to uncommon external IP addresses over port 443.
* Unexpected outbound connections from winlogon.exe.
* Data exfiltration activities occurring during non-business hours.

**Detection SPL Queries**

```
index=* sourcetype=* host=diagonalley dest_ip="14.121.222.11" dest_port=443 
```

### 8.A.1 Ingress Tool Transfer ([T1105](https://attack.mitre.org/techniques/T1105/))

**Detection Criteria**

The SDBbot RAT uses winlogon.exe to transfer and execute CL0P (C:\Windows\System32\SysMonitor.exe ) via cmd.exe.

**Behavioral Anomalies**

* Execution of cmd.exe by winlogon.exe, which is unusual for typical system behavior.
* New or unfamiliar executables running from C:\Windows\System32.
* Network activity associated with newly introduced executables.

**Detection SPL Queries**

```
index=* sourcetype=* host=diagonalley process_name="winlogon.exe" EventCode=11 "*C:\\Windows\\System32\\SysMonitor.exe" 
```

### 8.A.2 Command and Scripting Interpreter: Windows Command Shell ([T1059.003](https://attack.mitre.org/techniques/T1059/003/))

**Detection Criteria**

The `CL0P` malware is executed by `winlogon.exe` through `cmd.exe` using the command `cmd.exe /C C:\Windows\System32\SysMonitor.exe`. This indicates the use of the Windows Command Shell to launch malicious executables.

**Behavioral Anomalies**

* Execution of cmd.exe by winlogon.exe, which is unusual for typical system operations.
* Unexpected or unauthorized executables running from C:\Windows\System32.
* Elevated command-line activity involving execution of potentially harmful scripts or executables.

**Detection SPL Queries**

```
index=* sourcetype=* host=diagonalley process_name="winlogon.exe" CommandLine="cmd.exe /C C:\\Windows\\System32\\SysMonitor.exe" 
```

### 8.A.3 System Location Discovery: System Language Discovery ([T1614.001](https://attack.mitre.org/techniques/T1614/001/))

**Detection Criteria**

`CL0P (C:\Windows\System32\SysMonitor.exe)` uses API functions `GetKeyboardLayout` and `GetTextCharset` to determine the system's keyboard layout and text charset, which helps identify the system language and regional settings.

**Behavioral Anomalies**

* Unexpected calls to GetKeyboardLayout and GetTextCharset by non-standard applications.
* Analysis of system language settings by executable files in critical system directories.
* Execution of SysMonitor.exe or similar files that typically should not engage in language discovery activities.

### 8.A.4 Command and Scripting Interpreter: Windows Command Shell ([T1059.003](https://attack.mitre.org/techniques/T1059/003/))

**Detection Criteria**

CL0P (C:\Windows\System32\SysMonitor.exe) executed commands using cmd.exe /c to carry out malicious activities on the host.

**Behavioral Anomalies**

* cmd.exe being used to execute unexpected commands or scripts.
* Unusual command-line arguments associated with cmd.exe, especially those invoking SysMonitor.exe.
* High frequency of cmd.exe executions within a short time frame.

**Detection SPL Queries**

```
index=* sourcetype=* host=diagonalley process_name="cmd.exe" CommandLine="* /c *SysMonitor.exe" 
```

### 8.A.5 Inhibit System Recovery ([T1490](https://attack.mitre.org/techniques/T1490/))

**Detection Criteria**

CL0P (C:\Windows\System32\SysMonitor.exe) attempts to inhibit system recovery by deleting shadow copies and resizing shadow storage on multiple drives (C:, D:, E:, F:, G:, H:) using the vssadmin command.

**Behavioral Anomalies**

* Unexpected or unauthorized use of vssadmin commands.
* Sudden deletion of shadow copies.
* Significant changes in shadow storage sizes.

**Detection SPL Queries**

```
index=* sourcetype=* host=diagonalley process_name="cmd.exe" CommandLine="*vssadmin*" 
```

### 8.A.6 Inhibit System Recovery ([T1490](https://attack.mitre.org/techniques/T1490/))

**Detection Criteria**

CL0P (C:\Windows\System32\SysMonitor.exe) disables boot recovery options on a system by executing commands to modify boot configuration data (BCD) settings. Detect instances where cmd.exe executes bcdedit commands to disable recovery options:

* cmd.exe /C bcdedit /set (default) recoveryenabled No
* cmd.exe /C bcdedit /set {default} bootstatuspolicy ignoreallfailures

**Behavioral Anomalies**

* Unauthorized changes to system boot configurations.
* Disabled recovery options that hinder system recovery after failures.

**Detection SPL Queries**

```
index=* sourcetype=* host=diagonalley process_name="cmd.exe" CommandLine="*bcdedit*"
```

### 8.A.7 Service Stop ([T1489](https://attack.mitre.org/techniques/T1489/))

**Detection Criteria**

CL0P (C:\Windows\System32\SysMonitor.exe) stops several services to inhibit system defenses and recovery mechanisms. Detect instances where cmd.exe executes the command to stop services:

* **Full Command Line**: cmd.exe /C net stop /y

**Behavioral Anomalies**

* Sudden stopping of multiple system services.
* Services related to security, backup, or recovery are stopped unexpectedly.

**Detection SPL Queries**

```
index=* sourcetype=* host=diagonalley process_name="cmd.exe" CommandLine="*net stop * /y" 
```

### 8.A.9 Data Encrypted for Impact ([T1486](https://attack.mitre.org/techniques/T1486/))

**Detection Criteria**

CL0P (C:\Windows\System32\SysMonitor.exe) encrypts files using AES, appending the [.]C_I0p extension

**Behavioral Anomalies**

* Unwarranted and unexpected encryption of files
* Unusual file extension not known to be used by common file types

**Detection SPL Queries**

```
index=* 
| rex field=TargetFilename "(?<file_extension>\.[^\.]+$)" 
| stats count by file_extension 
| sort count
```

### 8.A.10 Defacement: Internal Defacement ([T1491.001](https://attack.mitre.org/techniques/T1491/001/)

**Detection Criteria**

CL0P (C:\Windows\System32\SysMonitor.exe) creates a ransom note (Readme!_CI0P!.txt)

**Behavioral Anomalies**

* Threatening note left by adversary indicating a compromise of sensitive data
* Unauthorized and unusual artifacts from adversary activity

**Detection SPL Queries**

```
index=* EventCode=11 | search TargetFilename="*.txt" | stats count by _time, TargetFilename | sort - count
```

### 8.A.11 Indicator Removal: Clear Windows Event Logs ([T1070.001](https://attack.mitre.org/techniques/T1070/001/))

**Detection Criteria**

CL0P (C:\Windows\System32\SysMonitor.exe) clears several Windows Event Logs

**Behavioral Anomalies**

* Unexpected and unusual deletion of Windows Event Logs
* Adversary tactics to hide malicious activities

**Detection SPL Queries**

```
(`wineventlog_security` EventCode=1102) OR (`wineventlog_system` EventCode=104)
```

### 8.A.12 Indicator Removal: File Deletion ([T1070.004](https://attack.mitre.org/techniques/T1070/004/))

**Detection Criteria**

CL0P (C:\Windows\System32\SysMonitor.exe) deleted itself

**Behavioral Anomalies**

* Self-deleting executables are atypical from how a benign executable usually operates

**Detection SPL Queries**

```
index=* process_name="cmd.exe" | search "del" "*.exe"
```
