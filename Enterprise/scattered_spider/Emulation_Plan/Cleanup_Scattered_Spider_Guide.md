# Scattered Spider Cleanup Guide

## Setup

### Voice Track

The following procedures will a conduct a cleanup of the range for the Scattered Spider
scenario.

### Procedures

- ☣️ From your RDP to the Kali machine `driftmark (174.3.0.70)`, open a new
terminal window and start the simple file server:

  | Password
  |  -
  | Barbed-Directive

  ```bash
  cd /opt/kalidev/scattered_spider/Resources/controlServer
  sudo ./controlServer -c config/scattered_spider.yml
  ```

  - ***Expected Output***

    ```text
    ...
    [SUCCESS] 2024/04/01 10:10:11 Started handler simplefileserver
    ...
    [INFO] 2024/04/01 10:10:11 Waiting for connections
    ```

## AWS Cleanup

### Voice Track

This section resets the AWS environment used during the Scattered Spider scenario.

### Procedures

- From the Windows jumpbox, open Firefox and browse to the SSO portal and login
with the following credentials:

  | Destination | Username | Password
  |  - | - | -
  | <https://sso.kingslanding.net/> | tlannister | Engine-Dropkick

- When prompted for the MFA token, switch to Kali, and open a new terminal window
as `op1` if one does not already exist. Execute the TOTP CLI utility to generate
a new token using the following command, entering the following password when
prompted. Enter the OTP into the SSO portal, then click Continue.

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

- From the SSO dashboard, right-click to open the settings page in a new tab, then
click on the MFA devices menu. Select 'Static Tokens' from the list of enabled
MFA methods and click 'Delete'.

- Return to the SSO portal dashboard and click on AWS to open the AWS console

- In the AWS console, click "CloudShell" in the bottom-left corner of the window.
Run the following commands in the CloudShell terminal to delete the IAM user and
EC2 instance created during the scenario:

  ```bash
  export INSTANCE_NAME="goldroad";
  export INSTANCE_ID=$(aws ec2 describe-instances --filters "Name=tag:Name,Values=$INSTANCE_NAME" "Name=instance-state-name,Values=running,stopped" --query "Reservations[*].Instances[*].InstanceId" --output text);
  aws ec2 terminate-instances --instance-ids "$INSTANCE_ID";
  aws ec2 wait instance-terminated --instance-ids "$INSTANCE_ID";
  aws ec2 describe-instances --instance-ids "$INSTANCE_ID" --query "Reservations[*].Instances[*].State.Name" --output text 2>&1 | grep -q "terminated";
  aws ec2 delete-key-pair --key-name "goldroad";
  aws iam remove-role-from-instance-profile --instance-profile-name goldroad-role --role-name goldroad-role;
  aws iam detach-role-policy --role-name goldroad-role --policy-arn arn:aws:iam::aws:policy/AdministratorAccess;
  aws iam delete-role --role-name goldroad-role;
  aws iam delete-instance-profile --instance-profile-name goldroad-role;
  export ACCESS_KEYS=$(aws iam list-access-keys --user-name "ahightower" --query 'AccessKeyMetadata[*].AccessKeyId' --output text);
  for ACCESS_KEY in $ACCESS_KEYS; do
     if [ -n "$ACCESS_KEY" ]; then
         aws iam delete-access-key --user-name "ahightower" --access-key-id "$ACCESS_KEY"
     fi
  done
  export ADMIN_POLICY_ARN=$(aws iam list-policies --query "Policies[?PolicyName=='admin_policy'].Arn" --output text);
  aws iam detach-user-policy --user-name ahightower --policy-arn "$ADMIN_POLICY_ARN";
  aws iam delete-user --user-name "ahightower"
  ```

- Once the commands have completed running, collapse the CloudShell terminal.
Search for and open AWS IAM console in a new tab, then in the left navigation
pane, under "Access Management", click "Users". Confirm the user `ahightower` is
no longer listed. Close the tab once finished.

- Return to the AWS Console Home. Search for and open "EC2" in a new tab, under
"Instances" click on "Instances". Confirm the status of the attacker created EC2
instance `goldroad` is "Terminated". Close the tab once finished.

## Host Cleanup

### Voice Track

This section resets the hosts used during the Scattered Spider scenario.

### Procedures

- From the AWS Console Home. Search for and open "Systems Manager" in a new tab,
in the left navigation pane under "Node Tools" click "Run Command". Click the
"Run Command" button, select the `AWS-RunPowerShellScript` command document and
paste the following into the "Commands" box:

  ```psh
  if ("dragongate" -eq $env:COMPUTERNAME) { Remove-Item -Path "C:\Users\tlannister\Downloads\ADExplorer*" -Force; Remove-Item -Path "C:\Users\tlannister\Downloads\Eula.txt" -Force; }

  $tacticalHosts = "citadel", "sept", "dragongate", "fleabottom", "fishmonger";
  if ($tacticalHosts -contains $env:COMPUTERNAME) {
      Start-Process -FilePath "C:\Program Files\TacticalAgent\meshagent.exe" -ArgumentList "-fulluninstall" -Wait;
      sc.exe stop tacticalrmm;
      sc.exe delete tacticalrmm;
      Start-Process -FilePath "C:\Program Files\TacticalAgent\tacticalrmm.exe" -ArgumentList "-m cleanup" -Wait;
      Remove-Item -Path "C:\Program Files\TacticalAgent\meshagent.exe" -Force;
      Remove-Item -Path "C:\Program Files\TacticalAgent\tacticalrmm.exe" -Force;
      Remove-Item -Path "C:\Program Files\TacticalAgent" -Recurse -Force;}

  if ("fishmonger" -eq $env:COMPUTERNAME) {
      $accessToken = $(curl.exe -s -X POST http://rookery.kingslanding.net/api/v1/applications/token -H "Content-Type: application/json" --data '{\"client_id\": \"fa51ae53-3b3c-4527-a6c0-b10acd8bfc23\",\"client_secret\": \"MsyvIBHX6uuQEPVuDLSzMVd6r8I5UV7F\",\"grant-type\":\"client_credentials\"}' | ConvertFrom-Json).access_token;
      $workspaceId = $($(curl.exe -s http://rookery.kingslanding.net/api/public/v1/workspaces -H "Authorization: Bearer $accessToken" | ConvertFrom-Json).data | ?{ $_.name -eq "Default Workspace" } | Select-Object -First 1).workspaceId;
      $destinationId = ($(curl.exe -s -X GET --url http://rookery.kingslanding.net/api/public/v1/destinations -H "Authorization: Bearer $accessToken" -H 'accept: application/json' | ConvertFrom-Json).data| Where-Object { $_.Name -eq "Weekly backup" }).destinationId;
      curl.exe -s -X DELETE --url http://rookery.kingslanding.net/api/public/v1/destinations/$destinationId -H "Authorization: Bearer $accessToken";
      $connectionId = ($(curl.exe -s -X GET --url http://rookery.kingslanding.net/api/public/v1/connections -H "Authorization: Bearer $accessToken" -H 'accept: application/json' | ConvertFrom-Json).data| Where-Object { $_.Name -eq "Gitlab to S3" }).connectionId;
      curl.exe -s -X DELETE --url http://rookery.kingslanding.net/api/public/v1/connections/$connectionId -H "Authorization: Bearer $accessToken";
      $connectionId = ($(curl.exe -s -X GET --url http://rookery.kingslanding.net/api/public/v1/connections -H "Authorization: Bearer $accessToken" -H 'accept: application/json' | ConvertFrom-Json).data| Where-Object { $_.Name -eq "Wekan to S3" }).connectionId;
      curl.exe -s -X DELETE --url http://rookery.kingslanding.net/api/public/v1/connections/$connectionId -H "Authorization: Bearer $accessToken";

      winget uninstall --name "Cyberduck" --accept-source-agreements;
      Remove-Item -Path "C:\Users\atargaryen\Downloads\Cyberduck-Installer*" -Force;
  }

  if ("fleabottom" -eq $env:COMPUTERNAME) {
      Unregister-ScheduledTask -TaskName "NightlyBackup" -Confirm:$false;
      Remove-ADGroupMember -Identity "On-Prem System Admins" -Members "sclegane" -Confirm:$false;
  }
  ```

- Under "Target selection" select "Choose instances manually". Click into the
search bar, select "Platform Type" then select "Windows" and select all
remaining targets after filtering. Under "Output options" uncheck "Enable an S3
bucket" then click "Run". On the Command ID page, next to the "Cancel command"
button, click the arrow to refresh the status until the command has completed.

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

- In the Outlook interface, select the cog configuration icon at the top right
and then select "Options". On the left-hand menu, go to Mail -> Automatic
Processing -> Inbox and Sweep Rules. Select the rule `spam` and select the trash
icon to delete it.

- Sign out of Outlook and close the browser. Sign out of your RDP to the unmanaged
workstation `casterlyrock (12.78.44.201)`.

- From Kali, search for and open **Firefox** and login to the TacticalRMM login
page if you are not already logged in:

  | Destination | Username | Password
  |  - | - | -
  | <https://rmm.kingslanding-hr.com/> | lstrong | Structure-Doorway

- For each agent in the dashboard, right click and then select "Remove Agent".
Type "yes" in the confirmation dialogue to uninstall the agent.
