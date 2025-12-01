# Protections Test 6 Scenario (NOISE ONLY)

## Step 0 - Setup

### Procedures

- If not already open, initiate an RDP session to the Windows jumpbox `dreadfort
(10.26.3.125)`.

  | Destination | Username | Password
  |  - | - | -
  | 10.26.3.125 | op1 | Barbed-Directive

- From the Windows jumpbox dreadfort, using the following credentials, RDP to the
Windows workstation `tentowers 10.26.4.102` (if not already connected).

  | Destination | Username | Password
  |  - | - | -
  | 10.26.4.102 | vale\tharlaw | Wrecking-Pumice

## Step 1

### Voice Track

### Procedures

- Open your RDP session to `tentowers (10.26.4.102)`. Press win+R and then enter `cmd` to open Command Prompt.

- Run the following command to schedule a daily backup or
routine maintenance task, ensuring that it runs automatically at a set time each
day without manual intervention.

  ```cmd
  schtasks /create /tn "DailyTask" /tr "powershell.exe -File C:\Scripts\Backup.ps1" /sc daily /st 14:00 /ru "System"
  ```

- Enter `exit` to exit the terminal window.

- Wait one minute, then open notepad by pressing win+R and then entering `notepad`. Add the following contents to the file, one line at a time:

  ```text
  Create promotional material for worker morale. Add a new section to the company website.

  Look into trouble coming from Dorne.
  ```

- Save the file as `C:\Users\Public\SupervisorNote.txt` and then close out of notepad.

- Wait one minute and then open Firefox by pressing win+R and then entering `firefox`.

- Type `windows what services to stop to speed up` into the search bar and press enter to perform the query.

- Wait one minute, then type `https://www.komando.com/news/pc-speed-boost/843692/` into the search bar and press enter to visit the site.

- Wait one minute, then type `powershell get win32_service only running` into the search bar and press enter to perform the query.

- Wait one minute, then type `https://superuser.com/questions/1136143/script-to-get-all-stopped-services-with-startup-type-automatic-windows` into the search bar and press enter to visit the site.

- Close Firefox.

- Wait one minute, then press win+R and then enter `cmd` to open Command Prompt.

- Run the following command to list all running
processes with verbose output filtered by the running status.

  ```cmd
  tasklist /v /fi "STATUS eq running"
  ```

- Enter `exit` to exit the terminal window.

- Wait one minute, then press win+R and then enter `cmd` to open Command Prompt.

- Run the following command to display
detailed network configuration information.

  ```cmd
  ipconfig /all
  ```

- Enter `exit` to exit the terminal window.

- Wait one minute, then press win+R and then enter `mstsc /v:10.26.3.101` to start an RDP connection to the remote access server for
system administration tasks. Connect using the following credentials:

  | Destination | Username | Password
  |  - | - | -
  | 10.26.3.101 | vale\tharlaw | Wrecking-Pumice

- Wait until you are fully connected to the remote computer's desktop, and then sign out of the inner RDP session.

- Wait one minute, then press win+R and then enter `cmd` to open Command Prompt.

- Run the following command to download TaskCoach from SourceForge.

  ```cmd
  curl -L -o C:\Users\%username%\Downloads\TaskCoachSetup.exe https://sourceforge.net/projects/taskcoach/files/latest/download
  ```

- Enter `exit` to exit the terminal window.

- Wait one minute, then press win+R and then enter `cmd` to open Command Prompt.

- Run the following command to switch to the Downloads directory:

  ```cmd
  cd C:\Users\%username%\Downloads
  ```

- In the same terminal prompt, type `.\TaskCoach` and then press tab to auto-complete to the downloaded file. Press enter to execute the installer.

- In the installer, keep pressing "Next" to keep the default options and then press "Install" to install TaskCoach. Once installation is complete, uncheck the "Show recent changes" box and press "Finish".

- Once the TaskCoach program opens up, close the TaskCoach window and exit out of your terminal prompt.

- Wait one minute, then press win+R and then enter `cmd` to open Command Prompt.

- Run the following command to retrieve the System Locale (information about
the language and regional settings) from the system's configuration by running
the systeminfo command and filtering the results with findstr:

  ```cmd
  systeminfo | findstr /B /C:"System Locale"
  ```

- Enter `exit` to exit the terminal window.

- Sign out of the RDP session

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1053.005 | Scheduled Task/Job: Scheduled Task | Windows | cmd.exe executed schtask to execute a Backup powershell script | Calibrated - Benign | - | tentowers 10.26.4.102 | tharlaw | - | -
| - | - | - | Windows | tharlaw creates a SupervisorReport document | Calibrated - Benign | - | tentowers 10.26.4.102 | tharlaw | - | -
| - | - | - | Windows | tharlaw browses in firefox for windows services | Calibrated - Benign | - | tentowers 10.26.4.102 | tharlaw | - | -
| Discovery | T1057 | Process Discovery | Windows | cmd.exe executed tasklist | Calibrated - Benign | - | tentowers 10.26.4.102 | tharlaw | - | -
| Discovery | T1016 | System Network Configuration Discovery | Windows | cmd.exe executed ipconfig | Calibrated - Benign | - | tentowers 10.26.4.102 | tharlaw | - | -
| Lateral Movement | T1021.001 | Remote Services: Remote Desktop Protocol | Windows | tharlaw connects to Remote Access Server from tentowers through RDP. | Calibrated - Benign | - | tentowers 10.26.4.102 | tharlaw | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | cmd.exe executed curl to download TaskCoachSetup | Calibrated - Benign | - | tentowers 10.26.4.102 | tharlaw | - | -
| Execution | T1059.003 | Command and Scripting Interpreter: Windows Command Shell | Windows | cmd.exe executed TaskCoach | Calibrated - Benign | - | tentowers 10.26.4.102 | tharlaw | - | -
| Discovery | T1082 | System Information Discovery | Windows | cmd.exe executed systeminfo | Calibrated - Benign | - | tentowers 10.26.4.102 | tharlaw | - | -
