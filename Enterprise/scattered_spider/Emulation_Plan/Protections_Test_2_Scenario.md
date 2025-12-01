# Protections Test 2 Scenario

[1]:https://cloud.google.com/blog/topics/threat-intelligence/sim-swapping-abuse-azure-serial

[2]:https://cloud.google.com/blog/topics/threat-intelligence/unc3944-sms-phishing-sim-swapping-ransomware/

[3]:https://www.microsoft.com/en-us/security/blog/2023/10/25/octo-tempest-crosses-boundaries-to-facilitate-extortion-encryption-and-destruction/

## Step 0 - Setup

### Procedures

- Initiate an RDP session to the Kali attack host `driftmark (174.3.0.70)`

  | Destination | Username | Password
  |  - | - | -
  | driftmark | op1 | Barbed-Directive

- Initiate an RDP session to the Jumpbox host `dreadfort (10.26.3.125)`

  | Destination | Username | Password
  |  - | - | -
  | 10.26.3.125 | op1 | Barbed-Directive

- ☣️ From the Windows Jumpbox `dreadfort (10.26.3.125)`, open FireFox and browse
to `https://sso.vale.net/`. Use the following credentials to log into the Single
Sign-On (SSO) portal.

  | Destination | Username | Password
  |  - | - | -
  | <https://sso.vale.net/> | gworm | Humble-Removed

- ☣️ When prompted for an MFA token, switch to the existing Kali RDP session, open
a new terminal window, and run the following command and provide the following
password when prompted. Copy the MFA code then switch back to the Windows
jumpbox to enter the MFA code into the SSO login page

  | Password
  |  -
  | Decode-Acclimate

  ```bash
  totp-cli generate --follow vale vale-gworm
  ```

  - ***Expected Output***

    ```text
    ######
    ```

- In the browser window click the three dots on the right hand side, select "More
Tools", then select "Developer Tools". In the newly opened panel, select
"Storage", then select "Cookies". Select the SSO domain sso.vale.net. Copy the
`authentik_csrf` and `authentik_session` into a text document for use later

## Step 1 - Initial Access

### Voice Track

Scattered Spider uses a previously compromised SSO cookie to log into the victims SSO
dashboard.

### Procedures

- ☣️ From the Kali attack host `driftmark (174.3.0.70)`, open FireFox and browse
to the real SSO portal at `https://sso.vale.net/`. Open the developer console
and go to the `Storage` tab and then to the `Cookies` drop-down menu. For the
SSO portal site, replace the `authentik_csrf` and `authentik_session` cookie
values with the values noted in the setup phase. After saving the cookie,
replace the URL with `https://sso.vale.net/` and reload the page.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Initial Access | T1133 | External Remote Services | Linux | Adversary initiated access to the <https://sso.vale.net/> | Calibrated - Not Benign | Scattered Spider connected to the external facing SSO portal. | bloodygate (10.26.5.101) | gworm | - | [2], [1]
| Initial Access | T1550.004 | Use Alternate Authentication Material: Web Session Cookie | Identity Provider | Adversary used gworm stolen web session cookie to bypass login | Calibrated - Not Benign | Scattered Spider uses a cookie from an existing authenticated session to access the SSO portal. | bloodygate (10.26.5.101) | gworm | - | [2], [1]
| Initial Access | T1078 | Valid Accounts | Linux, Identity Provider | Adversary successfully logged into <https://sso.vale.net/> as gworm | Calibrated - Not Benign | Scattered Spider uses a valid account to access the SSO portal. | bloodygate (10.26.5.101) | gworm | - | [2], [1]

## Step 2 - Privilege Escalation and Persistence

### Voice Track

After Scattered Spider logs into the SSO dashboard they configure a secondary IdP in the
SSO admin interface. Scattered Spider then logs back into the SSO dashboard using a valid
domain administrator account and the secondary IdP.

### Procedures

- ☣️ Inside the SSO dashboard, click the "Admin interface" button at the top
right. Under "Directory" on the left panel, select "Federation and Social
login". Create a new source and select the "SAML Source" type. Click "Next" to
reach the source configuration menu.

- ☣️ Set "Name" and "Slug" to `upstream`, ensure the source is enabled, set "User
matching mode" to "Link to a user with identical username", set "Group matching
mode" to "Link to a group with identical name", and keep the default icon.

- ☣️ Expand the Protocol settings, set the SSO URL to
`https://idp.braavos.com/application/saml/victim/sso/binding/post/`, and set
binding type to "Post-auto binding".

- ☣️ Expand the Advanced protocol settings, toggle on "Allow IDP-initiated
logins", then set "NameID Policy" to "Windows". Expand the Flow settings and
ensure "Pre-authentication flow" is set to `default-source-pre-authentication
(Pre-Authentication)`. Click the Finish button to finish creating the source.

- ☣️ Under "Flows and Stages" on the left panel, select "Flows". Select
`default-authentication-flow` and click the "Stage Bindings" tab. Click the
"Edit Stage" button for the "default-authentication-identification" table entry.

- ☣️ Scroll down and expand "Source settings". Confirm that the `upstream` source
you just created is listed as an available source. Select it and then click the
single right arrow to add it to the "Select sources" list. Ensure that it is the
only selected source on the right side then click "Update"

- ☣️ Log out of the SSO dashboard and confirm that the secondary IdP icon appears
in the login prompt window under the login button.

- ☣️ Open an incognito window to avoid using any previously established cookies
and browse to <https://sso.vale.net/>. Click the secondary IdP icon below the
login button to be redirected to the attacker-controlled secondary IdP. If
prompted with a security warning, click "Advanced" > "Accept Security Risk" >
"Resend". Log in using the attacker-created domain administrator credentials and
confirm that you were successfully redirected to the victim SSO dashboard as the
domain user and that the Guacamole application is available.

  | Destination | Username | Password
  |  - | - | -
  | <https://sso.vale.net/> | kdrogo | Bypassed123

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Privilege Escalation | T1484.002 | Domain or Tenant Policy Modification: Trust Modification | Windows, Identity Provider | gworm changed the identity provider configuration to insert a new path to domain-level or admin-level access | Calibrated - Not Benign | Scattered Spider adds a secondary IdP to Authentik that will allow them to log in as a domain admin | bloodygate (10.26.5.101) | gworm | - | [3], [2]
| Persistence | T1078 | Valid Accounts | Linux, Identity Provider | Adversary successfully logged into <https://sso.vale.net/> with kdrogo domain account | Calibrated - Not Benign | Scattered Spider uses a valid domain admin account to access the SSO dashboard. | bloodygate (10.26.5.101) | kdrogo | - | [2], [1]
| Privilege Escalation | T1484.002 | Domain or Tenant Policy Modification: Trust Modification | Identity Provider | kdrogo successfully logs in through the newly added IdP | Calibrated - Not Benign | Scattered Spider uses the secondary IdP to log in as a domain admin user | bloodygate (10.26.5.101) | kdrogo | - | [3], [2]

## Step 3 - Lateral Movement

### Voice Track

After Scattered Spider logs into the SSO portal as the domain admin, they wait until the
legitimate domain admin initiates a Guacamole session then hijacks the session
to gain access to an endpoint in the victim environment.

### Procedures

- Return to the Windows Jumpbox `dreadfort (10.26.3.125)`, open FireFox and browse
to `https://sso.vale.net/`. Use the following credentials to log into the Single
Sign-On (SSO) portal.

  | Destination | Username | Password
  |  - | - | -
  | <https://sso.vale.net/> | kdrogo | Apply-Precision

- When prompted for an MFA token, switch to the existing Kali RDP session, open a
new terminal window, and run the following command and provide the following
password when prompted. Copy the MFA code then switch back to the Windows
jumpbox to enter the MFA code into the SSO login page

  | Password
  |  -
  | Decode-Acclimate

  ```bash
  totp-cli generate --follow vale vale-kdrogo
  ```

  - ***Expected Output***

    ```text
    ######
    ```

- From the SSO dashboard, right-click on the Guacamole application to Open in New
Tab and access the Guacamole dashboard with available remote machines. Select
`blacktyde` from the list and authenticate to the remote desktop session using
the following credentials. For `Domain` enter `vale`

  | Destination | Username | Password
  |  - | - | -
  | `vale` | kdrogo | Apply-Precision

- ☣️ Switch back to Kali. Inside the SSO dashboard, click the Guacamole
application. In the top right, confirm a user is logged in by clicking Settings
then under "Active Connections", observing text mentioning a "1 user is logged
in" next to `blacktyde`. Click the `blacktyde` link to hijack the session.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Lateral Movement | T1563 | Remote Service Session Hijacking | Windows | kdrogo successfully connected to eastgate (10.26.5.100) and blacktyde (10.26.3.101) through RDP | Calibrated - Not Benign | Scattered Spider uses the Guacamole application to hijack a Guacamole session to `blacktyde (10.26.3.101)` | bloodygate (10.26.5.101), eastgate (10.26.5.100), blacktyde (10.26.3.101) | kdrogo | - | [2], [1]

## End of Test

### Voice Track

This step includes the shutdown procedures for the end of this Protections Test

### Procedures

- From Kali, close the browser to Guacamole

- Return to the RDP to `dreadfort (10.26.3.125)`. Close the FireFox tabs with the
SSO and Guacamole
