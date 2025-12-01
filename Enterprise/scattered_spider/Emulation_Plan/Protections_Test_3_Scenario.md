# Protections Test 3 Scenario

[3]:https://cloud.google.com/blog/topics/threat-intelligence/unc3944-sms-phishing-sim-swapping-ransomware/

[5]:https://permiso.io/blog/lucr-3-scattered-spider-getting-saas-y-in-the-cloud

[6]:https://www.youtube.com/watch?v=i0vPoKc9jG0

## Step 0 - Setup

### Procedures

- ☣️ Initiate an RDP session to the Kali host `driftmark (174.3.0.70)`

  | Destination | Username | Password
  |  - | - | -
  | driftmark | op1 | Barbed-Directive

- ☣️ Open a browser and browse to the victim SSO dashboard using the adversary
proxy. If prompted with SSL warnings, click "Advanced" > "Accept the Risk" >
"Resend". Authenticate as the compromised user `rarrayn`

  | Destination | Username | Password
  |  - | - | -
  | <https://shadowbinders.net> | rarrayn | Atlantic-Exhale

- ☣️ When prompted for an MFA token, open a new terminal window, and run the
following command and provide the following password when prompted. Copy the MFA
code then switch back to the browser to enter the MFA code into the SSO login
page

  | Password
  |  -
  | Decode-Acclimate

  ```bash
  totp-cli generate --follow vale vale-rarrayn
  ```

  - ***Expected Output***

    ```text
    ######
    ```

## Step 1 - Initial Access

### Voice Track

Scattered Spider leverages valid account `rarrayn` to authenticate to the SSO dashboard
of the victim domain. With authentication to the SSO dashboard, Scattered Spider connects
to the MDM dashboard in the victim domain and uses the MDM solution to install
persistence via RustDesk on the Windows victim hosts.

### Procedures

- ☣️ From the SSO dashboard, click to connect to Fleet MDM

- ☣️ From Fleet MDM, navigate to **Controls > Scripts** and upload the RustDesk
installation script located at
`/opt/kalidev/scattered_spider/Resources/payloads/WindowClientInstall.ps1`.

- Return to the Windows jumpbox and initiate an RDP to `runestone (10.26.4.101)`
as `rarrayn`

  | Destination | Username | Password
  |  - | - | -
  | runestone.vale.net | vale\rarrayn | Atlantic-Exhale

- ☣️ Return to Fleet MDM, navigate to the **Hosts** page and select the admin
workstation `runestone (10.26.4.101)`. On the details page, select **Actions >
Run Script**. From the **Run Script** menu, select the **Actions** dropdown for
the RustDesk install script and select **Run**. Click "Done" and scroll to the
Activity section, waiting a few minutes for the task to move under the "Past"
tab when  complete. When completed, hover over the task and click the
information "i" icon to ensure the script output matches what is seen in the
expected output.
Return to the **Hosts** page and repeat this process for the
domain controller `eyrie (10.26.3.100)`.

  - ***Expected Output***

    ```text
    Inputting Configuration now.
    Done!
    The RustDesk Service was stopped successfully.
    .....
    The RustDesk Service was started successfully.
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Initial Access | T1078 | Valid Accounts | Linux, Identity Provider | Adversary succcessfully connected to Fleet MDM through sso.vale.net | Calibrated - Not Benign | Scattered Spider connects to MDM through SSO dashboard | knights (10.26.3.103), bloodygate (10.26.5.101) | rarrayn | - | [3], [5], [6]
| Lateral Movement | T1072 | Software Deployment Tools | Linux | rarrayn added and executed WindowsClientInstall.ps1 script through fleet.vale.net | Calibrated - Not Benign | Scattered Spider uses MDM to execute scripts on remote hosts | eyrie (10.26.3.100), runestone (10.26.4.101) | rarrayn | - | [3], [5], [6]
| Execution | T1059.001 | Command and Scripting Interpreter: PowerShell | Windows | fleet executed powershell through orbit | Calibrated - Not Benign | Scattered Spider uses MDM to execute PowerShell commands | eyrie (10.26.3.100), runestone (10.26.4.101) | rarrayn | - | [3], [5], [6]
| Command and Control | T1105 | Ingress Tool Transfer | Windows | powershell.exe downloaded rustdesk.exe using Invoke-WebRequest | Calibrated - Not Benign | Scattered Spider uses MDM to download RustDesk to Windows hosts | eyrie (10.26.3.100), runestone (10.26.4.101) | rarrayn | - | [3], [5], [6]

## Step 2 - Lateral Movement

### Voice Track

Scattered Spider uses the MDM solution to laterally move to a workstation compromised
with RustDesk.

### Procedures

- ☣️ From Kali, search for and open the RustDesk application. Ensure the status on
the bottom bar of the application reads **Ready**.

- ☣️ Enter the RustDesk workstation ID `828578` in the **Control Remote Desktop**
box. Press **Connect** and enter the following password when prompted to connect
to the admin workstation `runestone (10.26.4.101)`.

  | Destination | Password
  |  - | -
  | 828578 | QJ1ABOIu6ekkZJop

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1219 | Remote Access Software | Windows | rarrayn executed rustdesk.exe and connected to livesso.com from runestone (10.26.4.101) to eyrie (10.26.3.100) | Calibrated - Not Benign | Scattered Spider connects to RustDesk session | runestone (10.26.4.101) | rarrayn | - | [3], [5], [6]

## End of Test

### Voice Track

This step includes the shutdown procedures for the end of this Protections Test

### Procedures

- From Kali, close the browser to Fleet and close the RustDesk application to
disconnect
