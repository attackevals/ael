# Configuration Overview

The following is an overview of configurations (i.e., Ansible playbooks) applied to the ER6 infrastructure. Configurations are organized by:

1. Scenario-specific playbooks - CL0P, LockBit, DPRK, and MITRE ATT&CK infrastructure
2. Common playbooks that are generally applied to multiple instances across the environment, categorized against function (e.g., based on the operating system)

## Table of Contents

- [Configuration Overview](#configuration-overview)
  - [Table of Contents](#table-of-contents)
  - [Per-Scenario](#per-scenario)
    - [Detections Scenario A](#detections-scenario-a)
    - [Detections Scenario B](#detections-scenario-b)
    - [Protections Scenario](#protections-scenario)
    - [Internal Resources: Red (Attacker) and Support](#internal-resources-red-attacker-and-support)
  - [Common Playbooks](#common-playbooks)
    - [Role-Based](#role-based)
    - [Windows](#windows)
    - [macOS](#macos)
    - [Linux](#linux)

---

## Per-Scenario

### Detections Scenario A

Emulated adversaries include `CL0P` and `DPRK`.

| Ansible File                         | Description                                                |
|--------------------------------------|:-----------------------------------------------------------|
| add-admin-diagonalley.yml            | Add admin `diagonalley`                                    |
| configure-addc-encryptpotter.yml     | Configure Scenario A range - Windows Domain Controller     |
| configure-addc-encryptpotter-gpo.yml | Configure Scenario A range - Windows Domain Controller GPO |
| exchange2019.yml                     | Install Microsoft Exchange on `hangleton` (mail server)    |
| fix-mac-hogshead.yml                 | Fix MacOS specific issues on `hogshead`                    |
| victim-a-diagonalley.yml             | Install programs on `diagonalley` host                     |
| victim-a-populate.yml                | Pre-populate profiles on victim Scenario A hosts            |

### Detections Scenario B

Emulated adversaries include `LockBit`.

| Ansible File                         | Description                                                |
|--------------------------------------|:-----------------------------------------------------------|
| add-admin-quirrell.yml               | Add admin on `quirrell` host to Local Administrator group                                       |
| add-admin-knockturnalley.yml         | Add admin on `knockturnalley` host to Local Administrator group                                 |
| configure-addc-decryptmalfoy.yml     | Configure Scenario B range - Windows Domain Controller     |
| configure-addc-decryptmalfoy-gpo.yml | Configure Scenario B range - Windows Domain Controller GPO |
| exchange2019-b.yml                   | Install Microsoft Exchange on `zonkos` (mail server)       |
| victim-b-populate.yml                | Pre-populate profiles on victim Scenario B hosts            |
| victim-b-quirrell.yml                | Pre-populate profiles on admin host `quirrel`               |

### Protections Scenario

The Protections/Prevention scenario consisted of standalone tests meant to evaluate product responses to specific adversary behaviors.

| Ansible File                      | Description                                                 |
|-----------------------------------|:------------------------------------------------------------|
| add-admin-bts.yml                 | Add admin on `bts` host to Local Administrator group                                             |
| configure-addc-sonicbeats.yml     | Configure Protections range - Windows Domain Controller     |
| configure-addc-sonicbeats-gpo.yml | Configure Protections range - Windows Domain Controller GPO |
| exchange2019-protections.yml      | Install Microsoft Exchange on `vixx` (mail server)          |
| fix-mac-itzy.yml                  | Fix MacOS specific issues on `itzy`                         |
| victim-protections-populate.yml   | Pre-populate profiles on victim Protections hosts            |

### Internal Resources: Red (Attacker) and Support

Support infrastructure includes components such as the DNS server and Chocolatey mirror.

| Ansible File                       | Description                                                     |
|------------------------------------|:----------------------------------------------------------------|
| configure-kali-all.yml             | Configure the Kali box in Red subnet                            |
| choco-mirror-setup.yml             | Install and Configure SonaType Nexus container for [Choco Mirror](https://docs.chocolatey.org/en-us/features/host-packages/) |
| kali-download-files.yml            | Download the most up-to-date artifacts                          |
| redirect-setup-red1.yml            | Configure redirect1                                             |
| redirect-setup.yml                 | Redirector setup                                                |
| remote-workers.yml                 | Configure remote workers                                        |
| red-cfg-win-git.yml                | Configure Red systems                                           |
| red-cfg-win.yml                    | Configure Red systems                                           |
| support-unbound-dns.yml            | Configure DNS                                                   |

---

## Common Playbooks

### Role-Based

| Ansible Role Path               | Description                             |
|---------------------------------|:----------------------------------------|
| ./ansible/roles/evals_ad_domain | Role applied to AD Domain Controller(s) |
| ./ansible/roles/evals_exchange  | Role applied to Mail Server(s)          |
| ./ansible/roles/evals_macos     | Role applied to MacOS instances         |
| ./ansible/roles/kali            | Role applied to Kali instance(s)        |

### Windows

| Ansible File                          | Description                                                                |
|---------------------------------------|:---------------------------------------------------------------------------|
| allow-ping.yml                        | Allow ping to Windows hosts                                                |
| autologon-disabled-win11.yml          | Disable autologin                                                          |
| cleanup-win.yml                       | Clear Choco, event and PowerShell logs.                                    |
| configure-addc-gpo.yml                | Configure GPO policy on DC                                                 |
| configure-shadow-copy.yml             | Configure shadow copy                                                      |
| configure-and-join-domain-win-all.yml | Configures DNS to use AD server; joins Windows hosts to respective domains |
| decoy-files.yml                       | Deploy decoy files                                                         |
| disable-defender-task.yml             | Disable scheduled Defender scan and Defender redux                         |
| fix-rdp.yml                           | Repair domain trust to troubleshoot RDP connection issues                  |
| fix-expired-login-win-servers.yml     | Fix expired local logins for all Windows server hosts                      |
| fix-expired-login-win-desktops.yml    | Fix expired local logins for all Windows 11 desktops                       |
| force-gp-update.yml                   | Force group policy update                                                  |
| info-lookup.yml                       | Lookup checksums for `curl` (Windows and MacOS)                            |
| install-packages-win-dev.yml          | Common installs and configs across all Windows desktops                    |
| prepop-profile-directories.yml        | Pre-populate profile directories                                            |
| registry-win-all.yml                  | Common install and config across all victim Windows systems                |

### macOS

| Ansible File                     | Description                                                         |
|----------------------------------|:--------------------------------------------------------------------|
| configure-and-join-macos-all.yml | Calls role `evals_macos` to join MacOS hosts to respective domains  |
| fix-mac-dns-and-groups.yml       | Fix DNS and `sudo` group membership issues on MacOS detections      |
| info-lookup.yml                  | Lookup checksums for `curl` (Windows and MacOS)                     |
| install-packages-mac.yml         | Common software installation and configuration across MacOS hosts   |
| mac-configure-base.yml           | Platform-specific configurations                                    |
| mac-osquery-setup.yml            | Configure `osquery` on MacOS instances                              |
| test-os-linux-mac.yml            | Verify operating system for MacOS and Linux instances               |

### Linux

| Ansible File                        | Description                                                          |
|-------------------------------------|:---------------------------------------------------------------------|
| configure-and-join-domain-linux.yml | Install packages required for AD and other useful tools              |
| linux-disable-updates.yml           | Locks kernel at specified version and disables updates               |
