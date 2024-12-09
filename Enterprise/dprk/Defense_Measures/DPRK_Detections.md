# DPRK Detection Analysis 

This guide covers key detection criteria for DPRK-related malware activity as emulated by the ([Enterprise Round 6 Emulation Plan](../Emulation_Plan/ER6_DPRK_Scenario.md)), such as `FULLHOUSE.DOORED` and `STRATOFEAR`. The focus is on recognizing tool transfers, masquerading, system modifications, and encrypted communications.


### Key Findings

* Ingress Tool Transfer (T1105) is prevalent, where DPRK uses tools like `start.rb` to drop malware such as FULLHOUSE.DOORED.
* Masquerading (T1036.005) through the use of file names mimicking legitimate services like Docker or Zoom.
* System Information Discovery (T1082) and User Discovery (T1033) involve commands like `logname` and `gethostname` to gather system and user data.
* File Deletion (T1070.004) is used to hide activity after execution.

### Detection Tools

* `Sysmon` + `Splunk`: Detects file creation, process execution, and system calls.
* `Suricata/Zeek`: Monitors network traffic for HTTP/HTTPS connections to suspicious domains.
* `YARA`: Identify malicious binaries like FULLHOUSE.DOORED based on patterns.
* `Sigma`: Use rule-based log analysis for identifying adversary activity.

**Example Open Source Tools:**

* Zeek: Analyze network traffic to detect HTTP/HTTPS protocols.
* YARA/PEStudio: Examine malware binaries for signatures or behaviors.
* OSQuery: Query system-level artifacts for forensic analysis.

## Step 1  

### 1.A.1 Ingress Tool Transfer ([T1105](https://attack.mitre.org/techniques/T1105/))

**Detection Criteria**

Adversary used `start.rb` to drop `FULLHOUSE.DOORED` (`/usr/local/bin/com.docker.sock`) and configuration file (`/usr/local/bin/com.docker.sock.lock`) from `b0ring-t3min4l[.]kr` (199.88.44.199) to `HOGSHEAD` (10.55.4.50)

**Behavioral Anomalies**

* **Unusual Script Executions**: Executing scripts (e.g., `.rb`, `.sh`, `.py`) to download and drop files in non-standard directories.
* **External Network Communication**: Establishing connections to unfamiliar IPs or domains, especially those linked to C2 infrastructure.
* **File Creations in Non-Standard Directories**: Creation of executables or configuration files in locations outside the typical software or system directories (e.g., `/usr/local/bin`, `/Library`).
* **Process-File Relationship**: Newly spawned processes associated with dropped files exhibiting anomalous behavior, such as elevated privileges or unexpected communication patterns.

**Detection SPL Queries**

```
index=mac_os sourcetype=process_activity OR sourcetype=sysmon OR sourcetype=network_traffic
| eval is_abnormal=(case(
  command_line LIKE "%curl%" OR command_line LIKE "%wget%", 1, 
  process_name IN ("python", "ruby", "bash", "zsh") AND (parent_process_name="launchd" OR parent_process_name="zsh"), 1, 
  file_path LIKE "/usr/local/bin%" OR file_path LIKE "/Library/LaunchDaemons%", 1, 
  dest_ip IN ("199.88.44.199", "198.88.44.44"), 1, 
  process_parent IN ("com.docker.sock"), 1, 
  process_name="launchctl", 1,
  TRUE, 0))
| where is_abnormal=1
| transaction user, host startswith="curl" OR startswith="wget" OR startswith="launchctl" endswith="file_modification" maxspan=5m
| stats count by host, user, file_path, command_line, parent_process_name, dest_ip
| where count > 1
| sort by count desc 
```

### 1.A.2 Masquerading: Match Legitimate Name or Location ([T1036.005](https://attack.mitre.org/techniques/T1036/005/))

**Detection Criteria**  

Adversary used `FULLHOUSE.DOORED` (`/usr/local/bin/com.docker.sock`) on `HOGSHEAD`, masquerading as a legitimate Docker file to avoid detection.

**Behavioral Anomalies**

* Uncommon file creation in typically legitimate directories, like `/usr/local/bin`, especially when tied to non-system processes or users.
* File or process names that closely resemble legitimate binaries but may have slight differences in behavior or origin.
* Execution of binaries in legitimate directories by non-privileged users or processes without administrative tasks.

**Detection SPL Queries**

```
index=mac_os sourcetype=process_activity OR sourcetype=sysmon OR sourcetype=file_system_events
| eval suspicious_file=(case(
  file_path LIKE "/usr/local/bin%" AND process_name IN ("bash", "zsh", "python", "ruby"), 1,
  file_path LIKE "/Library/LaunchDaemons%" AND process_name IN ("launchctl", "com.docker.sock"), 1,
  command_line LIKE "%com.docker.sock%", 1,
  TRUE, 0))
| where suspicious_file=1
| transaction user, host startswith="file_creation" endswith="process_execution" maxspan=5m
| stats count by host, user, process_name, file_path, command_line
| where count > 1
| sort by count desc
```

### 1.A.3 Application Layer Protocol: Web Protocols ([T1071.001](https://attack.mitre.org/techniques/T1071/001/))

**Detection Criteria**  

`FULLHOUSE.DOORED` (`/usr/local/bin/com.docker.sock`) on `HOGSHEAD` (10.55.4.50) connects to `forev3r1[.]kr` (199.88.44.44) over protocol HTTP on port 80

**Behavioral Anomalies**

* Outbound HTTP/HTTPS traffic to rare or untrusted external domains, especially over default ports like 80 or 443.
* Connections initiated from non-browser processes (e.g., `com.docker.sock`), which is unusual for typical HTTP communications.
* Consistent outbound traffic from systems that rarely initiate external communications, especially outside of business hours.
* Unencrypted HTTP traffic over port 80, which may expose sensitive data or facilitate C2 operations.

**Detection SPL Query:**

```
index=mac_os sourcetype=network_traffic
| search dest_port=80 OR dest_port=443
| eval suspicious_conn=(case(
  dest_ip IN ("199.88.44.44", "198.88.44.96"), 1,
  process_name NOT IN ("Safari", "Chrome", "Firefox", "curl", "wget"), 1,
  TRUE, 0))
| where suspicious_conn=1
| stats count by src_ip, dest_ip, dest_port, process_name, user
| where count > 5
| sort by count desc 
```

## Step 2

### 2.A.1 Ingress Tool Transfer ([T1105](https://attack.mitre.org/techniques/T1105/))

**Detection Criteria**

The adversary used `FULLHOUSE.DOORED (/usr/local/bin/com.docker.sock)` to download `STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)`, its LaunchDaemon (`/Library/LaunchDaemons/us.zoom.ZoomHelperTool.plist`), and configuration file (`/Library/Fonts/pingfang.ttf.md5`). These files were transferred from `b0ring-t3min4l[.]kr (199.88.44.44)` to `HOGSHEAD (10.55.4.50)`.

**Behavioral Anomalies**

* Unusual File Downloads: Downloading tools or binaries from untrusted remote servers like `b0ring-t3min4l[.]kr`.
* File Placement in Sensitive Locations: Storing binaries in directories like `/Library/LaunchDaemons` where legitimate helper tools are typically placed.
* Execution of Downloaded Files: Execution or registration of LaunchDaemons or services using the newly downloaded files.
* Use of Unusual Protocols or Ports: Adversary might use web protocols (HTTP/HTTPS) or obscure ports to blend into normal traffic patterns.

**Detection SPL Queries**

```
index=os_logs sourcetype=sysmon EventID=11 OR sourcetype=network_traffic (process_name="*curl*" OR process_name="*wget*")
| search (dest_ip="199.88.44.44" OR file_path="*/Library/LaunchDaemons/*")
| stats count by process_name, file_path, dest_ip, user, command_line
| where count > 1
| eval action="Ingress Tool Transfer detected", severity="high"
| table _time, process_name, user, file_path, dest_ip, command_line, action, severity
```

### 2.A.2 Command and Scripting Interpreter: Unix Shell ([T1059.004](https://attack.mitre.org/techniques/T1059/004/))

**Detection Criteria**

`FULLHOUSE.DOORED (/usr/local/bin/com.docker.sock)` executed commands via `/bin/zsh` using the NSTask class. This activity involves using the Unix shell to load a LaunchDaemon for persistence.

**Behavioral Anomalies**

* Unexpected Shell Execution: Running Unix shells like /bin/zsh outside regular administrative tasks.
* Loading Daemons: Commands attempting to load or modify system daemons, especially from uncommon binaries like /usr/local/bin/com.docker.sock.
* NSTask Usage: Monitoring the usage of NSTask to execute shell commands that target system processes like launchctl for persistence.
* Parent-Child Process Relationships: Detecting relationships between unexpected binaries and shell executions.

**Detection SPL Queries**

```
index=os_logs sourcetype=sysmon EventID=1 OR sourcetype=mac_os_logs (process_name="/bin/zsh" OR process_name="launchctl")
| search (file_path="*/Library/LaunchDaemons/*" OR command_line="*NSTask*")
| stats count by process_name, command_line, user, file_path
| where count > 1
| eval action="Suspicious Shell Execution", severity="high"
| table _time, process_name, user, file_path, command_line, action, severity

```

### 2.A.3 Create or Modify System Process: Launch Daemon ([T1543.004](https://attack.mitre.org/techniques/T1543/004/))

**Detection Criteria**

`FULLHOUSE.DOORED (/usr/local/bin/com.docker.sock)` registered `STRATOFEAR (/Library/LaunchDaemons/us.zoom.ZoomHelperTool.plist)` as a LaunchDaemon. This action is used to maintain persistence on the system by ensuring that the `STRATOFEAR` tool is launched at system startup.

**Behavioral Anomalies**

* Unauthorized LaunchDaemon Creation: Detection of new or modified LaunchDaemon files in `/Library/LaunchDaemons/` without associated system updates.
* Non-standard File Path: The presence of files like `us.zoom.ZoomHelperTool.plist` mimicking legitimate names but originating from unusual binaries.
* Unexpected Process Interaction: Processes such as `com.docker.sock` interacting with the `launchctl` process in non-standard execution contexts.
* Persistence Mechanism: Multiple executions of a specific daemon file on system startup or reboots.

**Detection SPL Queries**

```
index=os_logs sourcetype=mac_os syslog_message="launchctl" 
| search file_path="/Library/LaunchDaemons/*" 
| stats count by file_path, user, process_name, command_line 
| where count > 1 
| eval action="Suspicious LaunchDaemon Modification", severity="high" 
| table _time, file_path, user, process_name, command_line, action, severity
```

### 2.A.4 File and Directory Permissions Modification: Linux and Mac File and Directory Permissions Modification ([T1222.002](https://attack.mitre.org/techniques/T1222/002/))

**Detection Criteria**

`FULLHOUSE.DOORED (/usr/local/bin/com.docker.sock)` modified the permissions of `STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` using the chmod command. This action is part of the malware's strategy to ensure it has the necessary permissions to execute and maintain persistence on the system.

**Behavioral Anomalies**

* Unauthorized Permission Changes: Detection of chmod applied to system-critical paths, especially to binary files or directories used by legitimate processes like Zoom.
* Non-Administrative User Modification: Permission changes made by non-administrative users on system-protected directories or files.
* Repeated Permission Adjustments: Multiple permission changes on the same file or directory within a short period, indicating an attempt to modify execution privileges.
* Chained Activity: If combined with file creation or modification events, this strengthens the malicious context.

**Detection SPL Queries**

```
index=os_logs sourcetype=mac_os syslog_message="chmod"
| search file_path="/Library/PrivilegedHelperTools/*" 
| stats count by file_path, user, process_name, command_line 
| where count > 1 
| eval action="Suspicious File/Directory Permission Modification", severity="high" 
| table _time, file_path, user, process_name, command_line, action, severity
```

### 2.A.5 System Services: Launchctl ([T1569.001](https://attack.mitre.org/techniques/T1569/001/))

**Detection Criteria**

`FULLHOUSE.DOORED (/usr/local/bin/com.docker.sock)` executed `STRATOFEAR (/Library/LaunchDaemons/us.zoom.ZoomHelperTool.plist)` using the launchctl command. This activity indicates the malware's attempt to maintain persistence by registering a LaunchDaemon.

**Behavioral Anomalies**

* Unusual Service Registration: Detecting the registration of a LaunchDaemon from unexpected directories or non-standard file paths such as `/Library/LaunchDaemons`.
* Unauthorized Use of `launchctl`: Monitoring for `launchctl` being executed by users or processes that are not typical administrators or system services.
* Repeated Service Modifications: Detecting multiple instances of services being registered or modified, especially for files not commonly associated with legitimate services.

**Detection SPL Queries**

```
index=os_logs sourcetype=mac_os syslog_message="launchctl"
| search file_path="/Library/LaunchDaemons/*"
| stats count by file_path, user, command_line, process_name
| where count > 1
| eval action="Suspicious LaunchDaemon Registration", severity="high"
| table _time, file_path, user, command_line, process_name, action, severity
```

### 2.A.6 Masquerading: Match Legitimate Name or Location ([T1036.005](https://attack.mitre.org/techniques/T1036/005/))

**Detection Criteria**

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` masquerades as a legitimate Zoom file, exploiting user trust in common software to avoid detection.

**Behavioral Anomalies**

* Mismatch Between File Path and File Properties: Detect when files located in trusted directories (e.g., `/Library/PrivilegedHelperTools/`) do not match their expected properties (e.g., signed by a trusted vendor like Zoom).
* Unusual File Creation in Critical Directories: Monitor directories typically reserved for trusted applications or privileged tools for the creation of files that deviate from known software.
* Unverified File Origin: Detection of unsigned or incorrectly signed binaries located in sensitive directories associated with legitimate software.

**Detection SPL Queries**

```
index=os_logs sourcetype=mac_os file_path="/Library/PrivilegedHelperTools/*"
| search NOT [| inputlookup legitimate_zoom_files.csv | fields file_name, file_hash]
| stats count by file_path, file_name, file_hash, user
| where count > 1
| eval action="Suspicious File Masquerading Detected", severity="high"
| table _time, file_path, file_name, file_hash, user, action, severity
```

### 2.A.7 Application Layer Protocol: Web Protocols ([T1071.001](https://attack.mitre.org/techniques/T1071/001/))

**Detection Criteria**

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` communicates with `t1g3r-heart[.]kr (198.88.44.96)` over HTTPS on port 443. This indicates potential data exfiltration or command-and-control activities.

**Behavioral Anomalies**

* Unusual HTTPS Connection: Monitor for unusual or infrequent connections to external domains over HTTPS, particularly those not matching known organizational traffic patterns.
* Connections to Newly Registered Domains: Connections to domains that have recently been registered (e.g., less than 30 days old).
* Suspicious External IPs: Traffic from sensitive internal systems (such as `/Library/PrivilegedHelperTools/`) to IPs outside regular business workflows.

**Detection SPL Queries**

```
index=network_logs sourcetype="mac_os" destination_ip="198.88.44.96" dest_port=443
| eval time_diff=now() - _time
| where time_diff < 300
| stats count by source_ip, destination_ip, user, app_protocol, bytes_sent, bytes_received
| eval action="Suspicious HTTPS Communication Detected", severity="high"
| table _time, source_ip, destination_ip, user, app_protocol, bytes_sent, bytes_received, action, severity

```

### 2.A.8 Encrypted Channel: Asymmetric Cryptography ([T1573.002](https://attack.mitre.org/techniques/T1573/002/))

**Detection Criteria**

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` establishes a connection to `t1g3r-heart[.]kr (198.88.44.96)` over encrypted protocol HTTPS.

**Behavioral Anomalies**

* Unusual Encrypted Communications: Monitoring for HTTPS connections to unknown or suspicious domains that do not align with regular business traffic.
* Frequent, High-Volume Encrypted Traffic: Repeated connections over HTTPS from privileged system directories (like `/Library/PrivilegedHelperTools/`) that are unexpected.
* Use of Non-standard Ports for HTTPS: Watch for attempts to use encryption protocols over non-standard ports.
* Encrypted Traffic without Prior Establishment: Sudden initiation of encrypted communication to external servers without prior resolution or DNS lookups from known trusted sources.

**Detection SPL Queries**

```
index=network_logs sourcetype="mac_os" destination_ip="198.88.44.96" dest_port=443 
| eval time_diff=now() - _time 
| where time_diff < 600 
| stats count by source_ip, destination_ip, user, ssl_version, app_protocol, bytes_sent, bytes_received 
| eval action="Encrypted HTTPS Communication to Suspicious IP Detected", severity="high" 
| table _time, source_ip, destination_ip, user, ssl_version, app_protocol, bytes_sent, bytes_received, action, severity
```

## Step 3

### 3.A.1 Indicator Removal: File Deletion ([T1070.004](https://attack.mitre.org/techniques/T1070/004/))

**Detection Criteria**

`FULLHOUSE.DOORED (/usr/local/bin/com.docker.sock)` deletes itself and its configuration file `(/usr/local/bin/com.docker.sock.lock)` using the command `rm -f com.docker.sock com.docker.sock.lock && (sleep 7 && pkill com.docker.sock) &`.

**Behavioral Anomalies**

* Unexpected Deletion of Executables: Monitor for high-privilege processes executing file deletion commands, particularly in non-standard directories (e.g., `/usr/local/bin`).
* Timing-based Process Termination: Monitor for process termination commands (e.g., `pkill`) that are invoked after file deletion, especially with timing components like sleep.
* Multiple Related File Deletions: Detect simultaneous deletion of executable files and their configuration counterparts, suggesting potential malware cleanup activity.

**Detection SPL Queries**

```
index=system_logs sourcetype="mac_os" event_type=process_deletion OR file_deletion
| search "command"="rm -f" AND "pkill" AND "/usr/local/bin/com.docker.sock"
| stats count by process_name, user, file_path, command
| eval anomaly="Suspicious File Deletion & Process Termination Detected", severity="high"
| table _time, process_name, user, file_path, command, anomaly, severity
```

## Step 4

### 4.A.1 Command and Scripting Interpreter: Unix Shell ([T1059.004](https://attack.mitre.org/techniques/T1059/004/))

**Detection Criteria**

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` executed discovery commands through `/bin/sh` on `HOGSHEAD (10.55.4.50)`.

**Behavioral Anomalies**

* Unexpected Command Execution: Monitoring `/bin/sh` executions for commands not typically run by system administrators or automated scripts.
* Privilege Escalation: If the shell commands are executed with elevated privileges, it could indicate malicious intent.
* Command Chaining: Anomalous sequences of discovery-related commands executed in quick succession.

**Detection SPL Queries**

```
index=system_logs sourcetype="mac_os" event_type=process_execution
| search "command"="/bin/sh" AND ("whoami" OR "logname" OR "ifconfig")
| stats count by process_name, user, command
| eval anomaly="Suspicious Shell Execution Detected", severity="high"
| table _time, process_name, user, command, anomaly, severity
```

### 4.A.2 System Owner/User Discovery ([T1033](https://attack.mitre.org/techniques/T1033/))

**Detection Criteria**

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` executed the command `sh -c logname` to retrieve the current user on `HOGSHEAD (10.55.4.50)`.

**Behavorial Anomalies**

* Unexpected Command Execution: Monitoring execution of discovery commands such as logname using root privileges.
* Unusual Shell Execution: If observed shell (`/bin/sh`) commands are uncommon for legitimate Zoom files, this could indicate malicious intent.
* Chained Activity: If combined with file creation or modification events, this strengthens the malicious context.

**Detection SPL Queries**

```
index=main sourcetype="mac_os" 
| search "command"="sh -c logname"
| stats count by host, user, parent_process, process, command_line
| where parent_process != "expected_process"
| table _time, host, user, parent_process, process, command_line
```

### 4.A.3 System Owner/User Discovery ([T1033](https://attack.mitre.org/techniques/T1033/))

**Detection Criteria**

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` executed the command `sh -c dscl . -list /Users | grep -v '^_'` to gather a list of users on the system on `HOGSHEAD (10.55.4.50)`.

**Behavorial Anomalies**

* Execution of the `dscl` command outside typical administrative workflows.
* Suspicious account enumeration activity by unauthorized processes or users.
* Filtering of system accounts to focus on interactive users for further targeting.

**Detection SPL Queries**

```
index=main sourcetype="mac_os"
| search "command"="sh -c dscl . -list /Users | grep -v '^_'"
| stats count by host, user, parent_process, process, command_line
| where parent_process != "expected_process"
| table _time, host, user, parent_process, process, command_line
```

### 4.A.4 System Information Discovery ([T1082](https://attack.mitre.org/techniques/T1082/))

 **Detection Criteria**

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` used the API function `gethostname()` to gather the system's hostname on `HOGSHEAD (10.55.4.50)`.

**Behavioral Anomalies**

* Unexpected API calls to gethostname from unusual processes.
* Host information retrieval by unauthorized or non-system-critical processes.

### 4.A.5 System Information Discovery ([T1082](https://attack.mitre.org/techniques/T1082/))

**Detection Criteria**

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` executed the command `/usr/sbin/system_profiler SPHardwareDataType` to collect hardware information on `HOGSHEAD (10.55.4.50)`.

**Behavorial Anomalies**

* Unusual execution of system_profiler from non-administrative processes or users.
* Hardware information collection outside of routine maintenance or expected usage.

**Detection SPL Queries**

```
index=main sourcetype="mac_os"
| search "system_profiler SPHardwareDataType"
| stats count by user, process, parent_process, host
| where parent_process != "expected_admin_process"
| table _time, host, user, process, parent_process 
```

### 4.A.6 System Information Discovery ([T1082](https://attack.mitre.org/techniques/T1082/))

Description

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` executed the command `/usr/bin/sw_vers` to collect OS and build information on `HOGSHEAD (10.55.4.50)`.

**Behavorial Anomalies**

* Unexpected Command Execution: Monitoring for execution of system profiler commands by non-administrative applications. Repeated access to system profiling commands strengthens the malicious context.
* Unusual User-Level Execution: Non-administrative processes executing system information commands.

**Detection SPL Queries**  

```
index=main sourcetype="mac_os" "command_line"="/usr/bin/sw_vers" 
```

### 4.A.7 Shared Modules ([T1129](https://attack.mitre.org/techniques/T1129/))

**Detection Criteria**

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` dynamically loads modules from C2 using the `Initialize` function with `dlopen()` and `dlsym()`.

 **Behavorial Anomalies**

* Sudden use of dlopen() to load unusual shared libraries or temporary files.
* Files written in /tmp directories, often named with random alphanumeric strings, signal potential malicious code or modules loaded remotely.

**Detection SPL Queries**

```
index=macOS sourcetype=syslog
| search "dlopen" "dlsym"
| eval file_path_extracted=if(match(file_path, "/tmp/*"), file_path, null())
| search file_path_extracted!=null
| stats count by process_name, file_path_extracted
| where count > 1  # Threshold to reduce noise
| table _time, process_name, file_path_extracted
```

### 4.A.8 Credentials from Password Stores ([T1555.001](https://attack.mitre.org/techniques/T1555/001/))

**Detection Criteria**

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` reads the system `(/Library/Keychains/System.keychain)` and user `(/Users/ranrok/Library/Keychains/login.keychain-db)` Keychain files on the host `HOGSHEAD (10.55.4.50)`.

**Behavorial Anomalies**

* Unusual access to the Keychain, especially in non-typical processes or outside regular administrative activity, should raise suspicion.
* Monitoring the execution of SecKeychainItemCopyContent and analyzing for processes that attempt unauthorized keychain access can reveal anomalies.

**Detection SPL Queries**  

```
index=your_index sourcetype=your_sourcetype
| search "SecKeychainItemCopyContent" OR "/Library/Keychains/System.keychain" OR "/Users/*/Library/Keychains/login.keychain-db"
| stats count by process_name, user, keychain_access_attempt
| where count > 1
```

### 4.A.9 Network Share Discovery ([T1135](https://attack.mitre.org/techniques/T1135/))

**Detection Criteria**

`STRATOFEAR` is a persistent threat actor that uses the `us.zoom.ZoomHelperTool` located in `/Library/PrivilegedHelperTools/` to monitor and collect activity on shared drives and mounted devices. This tool leverages FSEventStream to track changes within the `/Volumes` folder, capturing details whenever a device (including USB or software) is mounted to the system.

**Behavorial Anomalies**

* High-Frequency Access: Monitoring unusual high-frequency access to the /Volumes folder.
* Unusual File System Events: Monitoring frequent read/write operations (such as those initiated by us.zoom.ZoomHelperTool).
* Unusual File Operations: Multiple fsgetpath, getfsstat64, and getattrlist operations targeting the /Volumes folder, which can indicate unauthorized surveillance of file system activities.
* Unexpected mounting of devices that do not align with normal user behavior.

**Detection SPL Queries**  

```
index=<your_index> sourcetype="fsmon" event_type="device_mount"
| eval suspicious_mount = if((mount_point == "/Volumes") AND (process_name != "Finder"), 1, 0)
| stats count by host, process_name, mount_point
| where suspicious_mount > 0
| table host, process_name, mount_point
| sort - count
```

### 4.A.10 Automated Collection ([T1119](https://attack.mitre.org/techniques/T1119/))

**Detection Criteria**

`STRATOFEAR (/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool)` records device activity to `/Library/Fonts/AppleSDGothicNeo.ttc.1` on `HOGSHEAD (10.55.4.50)`.

**Behavorial Anomalies**

* Unusual write operations in system directories like `/Library/Fonts`.
* Processes creating hidden or rarely-used files.
* Frequent writing to non-logging directories that typically host static files (e.g., fonts).

**Detection SPL Queries**

```
index=<your_index> sourcetype="fsmon" file_path="/Library/Fonts/AppleSDGothicNeo.ttc.1"
| stats count by host, process_name, file_path
| where count > 0
| table host, process_name, file_path
| sort - count
```
