# Scenario 1 Infrastructure

The infrastructure listed below is specific to Scenario 1, and assumes that the target environment is configured to only capture detections, rather than to take protective/preventative action. However, the infrastructure for Scenario 2 is a subset of the infrastructure described here. The same configuration described here can be used for Scenario 2 as well.

The requirements described herein should be considered a bare minimum to execute the scenario.  If you have the time and resources to remain true-to-form, you may elect to stand up multiple of each of these servers, non-contiguous IP space, etc.  If you are not concerned with emulating Carbanak to this degree, this level of effort is not necessary.  You could for instance, phish, serve payload, and exfil from/to the same server.

Please note that binary executable files hosted in [Resources](/Enterprise/carbanak/Resources/) have been added to password protected zip files.  The password for these files is "malware."

We provide a [script](/Enterprise/carbanak/Resources/utilities/crypt_executables.py) to automatically decrypt these files:

```
cd carbanak

python3 Resources/utilities/crypt_executables.py -i ./ -p malware --decrypt
```

---

## Emulation Team Infrastructure

1. **Attacker Desktop**: tested and executed on Ubuntu 20.04 LTS
    - Remote Desktop Client
        - [xfreerdp](https://www.freerdp.com/)
    - VNC Client
        - [TightVNC](https://www.tightvnc.com/)
2. **Attack Platform**: tested and executed on Kali Linux 2019.1
    - C2 Frameworks
        - [ATT&CK Evals Carbanak C2 Server](/Enterprise/carbanak/Resources/utilities/carbanak_c2server/c2server/c2server.elf)
        - [Metasploit Framework](https://github.com/rapid7/metasploit-framework)
    - [tmux](https://github.com/tmux/tmux/wiki)

---

## Emulation Team Infrastructure Configuration

This methodology assumes the following static IP address configuration:

| Red Team System | IP Address |
| ------ | ------ |
| Attack Platform | 192.168.0.4 |

### A note about red team payloads

- This evaluation utilizes payloads that model malware previously used by Carbanak.

- Pre-compiled payloads are available in the [resources](/Enterprise/carbanak/Resources) directory; however, they are configured to connect back to static IP address 192.168.0.4.

- If you would like to build the payloads yourself, please make the necessary adjustments to the payload source to match your environment.

### Attack Platform Setup

1. Download the Carbanak ATTACK Evaluations Library to the home directory

2. Enable SSH Port Forwarding

    ```
    printf "\nGatewayPorts yes\n >> /etc/ssh/sshd_config
    systemctl restart sshd.service
    ```

3. Add low privilege SSH user

4. Enable Metasploit PostGRES database

5. Stage certain files for download

    ```
    mkdir /var/files
    cp /home/<attacker>/Enterprise/carbanak/Resources/step7/Java-Update.exe /var/files/
    cp /home/<attacker>/Enterprise/carbanak/Resources/step10/tightvnc-2.8.27-gpl-setup-64bit.msi /var/files/
    cp /home/<attacker>/Enterprise/carbanak/Resources/step10/vnc-settings.reg /var/files/
    chmod 777 -R /var/files
    ```

6. Generate SSH key to copy into domain

    ```
    ssh-keygen -t rsa -f "/home/<attacker>/Enterprise/carbanak/Resources/setup/ssh/id_rsa" -P ""
    ```

---

## Target Infrastructure

4 targets, all domain joined:

1. **Domain Controller**: tested and executed on Windows Server 2019 1809

2. **HR Manager Workstation**: tested and executed on Windows 10 1909

3. **CFO Workstation**: tested and executed on Windows 10 1909

    - Domain user with local admin access on only this machine

4. **Bank File Server**: tested and executed on CentOS 7.7

---

## Target Infrastructure Configuration

This methodology assumes the following hostname configurations:

| Target System | Hostname |
| ------ | ------ |
| Domain Controller | bankdc |
| HR Manager Workstation | hrmanager |
| CFO Workstation | cfo |
| Bank File Server | bankfileserver|

### Domain Controller Setup

1. If only testing detection capabilities, disable Windows Defender

    The `set-defender.ps1` PowerShell script in the [setup](/Enterprise/carbanak/Resources/setup) folder can be used to perform this.

### HR Manager Workstation Setup

1. If only testing detection capabilities, disable Windows Defender

    The `set-defender.ps1` PowerShell script in the [setup](/Enterprise/carbanak/Resources/setup) folder can be used to perform this.

2. Copy newly generated SSH keypair from attacker platform to the following directory:

    ```
    C:\Users\<domain_user>.<domain>\
    ```

3. Disable OLE Security to enable execution of initial access payload

    The `set-OLEsecurity.ps1` PowerShell script in the [setup](/Enterprise/carbanak/Resources/setup) folder can be used to perform this.

### CFO Workstation Setup

1. If only testing detection capabilities, disable Windows Defender

    The `set-defender.ps1` PowerShell script in the [setup](/Enterprise/carbanak/Resources/setup) folder can be used to perform this.

2. Give the CFO user RDP privileges on only this machine by adding them to the following local groups:

    ```
    Remote Desktop Users
    Remote Management Users
    ```

3. Copy Payment Transfer System executable onto Desktop of CFO user.

4. Cache CFO User's credentials in Microsoft Edge:

    ```
    1. Open Edge

    2. Navigate to github.com

    3. Click "Sign In"

    4. Enter fake creds:
        -Username: <cfo_user>@<domain_full>
        -Password: <fake_password>

    5. Save credentials to password vault when prompted

    6. Close Edge

    7. Close RDP session
    ```

### Bank File Server Setup

1. Ensure the server is domain joined to the rest of the domain:

    <https://www.redhat.com/sysadmin/linux-active-directory>

2. Copy the following prop files to the `/var/tmp` directory:

    ```
    carbanak/Resources/step5/network-diagram-financial.xml
    carbanak/Resources/step5/help-desk-ticket.txt
    ```

3. Copy the newly generated SSH keypair's public key `id_rsa.pub` to the SSH Authorized Keys file:

    `/home/<domain_user>/.ssh/authorized_keys`

---

### Setup Automation

The `setup-teamsherver.sh` and `setup-winhosts.ps1` scripts in the [setup](/Enterprise/carbanak/Resources/setup) folder can be used to automate this process.
These scripts use the default hostnames, but require usernames and host IP addresses to be substituted in place of placeholder values.

1. Run `setup-teamserver.sh` on the Attack Platform.

    **Note**: The script assumes that this repo is downloaded to the <attacker> user's home directory.

2. Run `setup-winhosts.ps1` on the Domain Controller.
