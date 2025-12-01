---
tags: [infrastructure, enterprise2025, cloud, diagram, networking, hosts]
order: A
---

# Infrastructure for [ATT&CK Evaluations](https://evals.mitre.org/) — Scattered Spider & Mustang Panda (2025)

**Note:** During development, the threat actors were referred to by the codenames "Demeter" (Scattered Spider) and "Hermes" (Mustang Panda). These codenames may still appear in internal documentation and configuration files outside of this public release.

Please see [Getting Started](GettingStarted.md) for prerequisites, tooling, and setup guidance for the emulation of Scattered Spider and Mustang Panda.

Intermediate understanding of Terraform, Ansible, AWS, and AWS Billing are considered prerequisites to deploy the infrastructure configuration.

## Providers

!!!
**NOTE**: Access to at least two AWS accounts is required:

1. `aws.default` — an AWS account with cross-account permissions, where on-prem scenarios are emulated.
2. `aws.cloud` — an AWS account with organizations permissions, where cloud-based scenarios are emulated.

Alternatively, you can deploy the infrastructure with existing Detections/Protections accounts:

1. `aws.default` — an AWS account for emulating on-prem scenarios, with cross-account permissions into `aws.cloud-detections` and `aws.cloud-protections`.
2. `aws.cloud-detections`  — an AWS account for emulating cloud-based Detections scenarios.
3. `aws.cloud-protections` — an AWS account for emulating cloud-based Protections scenarios.

Please see the [`orgs`](./terraform/orgs/README.md) deployment for approaches to AWS Organizations & Accounts.

!!!

## Infrastructure Overview

The infrastructure below was staged for both Scattered Spider & Mustang Panda (2025).

Initial infrastructure was setup using Terraform, with configurations applied via scripts and configuration files.
Please see the [Deployment Overview](./terraform/README.md) and [Configuration Overview](./ansible/README.md) for technical documentation.

For an overview of traffic redirection used for obfuscation during emulation of Scattered Spider and Mustang Panda adversaries, please see [Traffic Redirection](Traffic-Redirection.md).

-------

- [Infrastructure for ATT\&CK Evaluations — Scattered Spider \& Mustang Panda (2025)](#infrastructure-for-attck-evaluations--scattered-spider--mustang-panda-2025)
  - [Scenario Domains & Hosts](#scenario-domains--hosts)
    - [Detections Domain — `kingslanding[.]net`](#detections-domain--kingslandingnet)
      - [Detections On-Premises](#detections-on-premises)
      - [Detections Cloud](#detections-cloud)
    - [Protections Domain — `vale[.]net`](#protections-domain--valenet)
      - [Protections On-Premises](#protections-on-premises)
      - [Protections Cloud](#protections-cloud)
    - [Support and Red Team Hosts](#support-and-red-team-hosts)
      - [External Benevolent Hosts](#external-benevolent-hosts)
      - [Red Team Hosts](#red-team-hosts)
  - [Network Diagram](#network-diagram)

## Scenario Domains & Hosts

This document provides an overview of the infrastructure support used for the evaluation.
In addition to setup and configuration of virtual machines, this document covers infrastructure support services —
such as domain name services (DNS), mail, and traffic redirection — used to support the evaluation.
Support services are used throughout the evaluation for resource efficiency.

The [Game of Thrones](https://en.wikipedia.org/wiki/Game_of_Thrones) television series inspired the naming scheme for this evaluation's infrastructure.

Enterprise 2025 infrastructure consists of an organization with on-premises resources and an AWS-provided cloud environment,
with network isolation into two environments for Detections and Protections.

-------

## Detections Domain — `kingslanding[.]net`

The Detections domain `kingslanding[.]net` contains fourteen (14) virtual machines.

### Detections On-Premises

The Detections On-Prem environment consists of eleven (11) virtual machines joined to the `kingslanding[.]net` Active Directory domain.

Detections On-Prem resources are provisioned under [`aws.default`](#providers).

> *DMZ Subnet — 10.55.2.0/24*

| **Hostname** | **OS**           | **IP**        | **Role**         |
|:-------------|:-----------------|:--------------|:-----------------|
| `kingswood`  | Ubuntu 24.04 LTS | `10.55.2.100` | Guacamole Server |

<br>

> *Servers Subnet — 10.55.3.0/24*

| **Hostname** | **OS**              | **IP**        | **Role**                 |
|:-------------|:--------------------|:--------------|:-------------------------|
| `redkeep`    | Windows Server 2022 | `10.55.3.100` | AD Domain Controller     |
| `sept`       | Windows Server 2022 | `10.55.3.101` | Exchange Server          |
| `citywatch`  | Ubuntu 24.04 LTS    | `10.55.3.102` | Authentik IdP/SSO Server |
| `citadel`    | Windows Server 2022 | `10.55.3.103` | File Server              |
| `dragongate` | Windows Server 2022 | `10.55.3.104` | Remote Desktop Server    |
| `conclave`   | Windows Server 2022 | `10.55.3.105` | File Server              |

<br>

> *Desktops Subnet — 10.55.4.0/24*

| **Hostname** | **OS**     | **IP**        | **Role** |
|:-------------|:-----------|:--------------|:---------|
| `fleabottom` | Windows 11 | `10.55.4.101` | Desktop  |
| `fishmonger` | Windows 11 | `10.55.4.102` | Desktop  |
| `harrenhal`  | Windows 11 | `10.55.4.103` | Desktop  |
| `stepstones` | Windows 11 | `10.55.4.104` | Desktop  |

<br>

### Detections Cloud

The Detections Cloud environment consists of three (3) virtual machines joined to the `kingslanding[.]net` Active Directory domain.

Detections Cloud resources are provisioned under [`aws.cloud-detections`](#providers).

> *Servers Subnet — 10.212.3.0/24*

| **Hostname** | **OS**           | **IP**         | **Role**       |
|:-------------|:-----------------|:---------------|:---------------|
| `dragonpit`  | Ubuntu 24.04 LTS | `10.212.3.105` | Wekan Server   |
| `guildhall`  | Ubuntu 24.04 LTS | `10.212.3.106` | Gitlab Server  |
| `rookery`    | Ubuntu 24.04 LTS | `10.212.3.107` | Airbyte Server |

<br>

-------

## Protections Domain — `vale[.]net`

The Protections domain `vale[.]net` contains twelve (12) virtual machines.

### Protections On-Premises

The Protections On-Prem Scenario consists of twelve (12) virtual machines joined to the `vale[.]net` Active Directory domain.

Protections On-Prem resources are provisioned under [`aws.default`](#providers).

> *Servers Subnet — 10.26.3.0/24*

| **Hostname** | **OS**              | **IP**        | **Role**              |
|:-------------|:--------------------|:--------------|:----------------------|
| `eyrie`      | Windows Server 2022 | `10.26.3.100` | AD Domain Controller  |
| `blacktyde`  | Windows Server 2022 | `10.26.3.101` | Remote Desktop Server |
| `godswood`   | Windows Server 2022 | `10.26.3.102` | Exchange Server       |
| `knights`    | Ubuntu 24.04 LTS    | `10.26.3.103` | MDM Server (Fleet)    |
| `redfort`    | Windows Server 2022 | `10.26.3.105` | File Server           |
| `heartshome` | Windows Server 2022 | `10.26.3.106` | File Server           |

<br>

> *Desktops Subnet — 10.26.4.0/24*

| **Hostname**   | **OS**     | **IP**        | **Role** |
|:---------------|:-----------|:--------------|:---------|
| `runestone`    | Windows 11 | `10.26.4.101` | Desktop  |
| `tentowers`    | Windows 11 | `10.26.4.102` | Desktop  |
| `bitterbridge` | Windows 11 | `10.26.4.103` | Desktop  |
| `ironoaks`     | Windows 11 | `10.26.4.104` | Desktop  |

<br>

> *DMZ Subnet — 10.26.5.0/24*

| **Hostname** | **OS**           | **IP**        | **Role**                 |
|:-------------|:-----------------|:--------------|:-------------------------|
| `eastgate`   | Ubuntu 24.04 LTS | `10.26.5.100` | Guacamole Server         |
| `bloodygate` | Ubuntu 24.04 LTS | `10.26.5.101` | Authentik IdP/SSO Server |

<br>

### Protections Cloud

> *Subnet — 10.115.5.0/24*

The Protections Cloud Scenario does not involve any EC2 hosts.

Protections Cloud resources are provisioned under [`aws.cloud-protections`](#providers).

<br>

-------

## Support and Red Team Hosts

The following hosts are dedicated to networking support and red team use.

### Validation Hosts

The hosts below are used to perform validation on victim infrastructure in each scenario domain.

| **Hostname**  | **OS**            | **IP**        | **Role**               |
|:--------------|:------------------|:--------------|:-----------------------|
| `preflights`  | Kali Linux 2024.4 | `10.55.4.120` | Detections Validation  |
| `preflights2` | Kali Linux 2024.4 | `10.26.4.120` | Protections Validation |

<br>

### External Benevolent Hosts

The hosts below are not accessible by evaluation participants.

> *Support Subnet — 12.78.0.0/16*

| **Hostname**   | **OS**           | **IP**         | **Role**          |
|:---------------|:-----------------|:---------------|:------------------|
| `twins`        | Ubuntu 24.04 LTS | `12.78.0.60`   | DNS Server        |
| `volantis`     | Ubuntu 24.04 LTS | `12.78.0.91`   | RMM Server        |
| `dorne`        | Ubuntu 24.04 LTS | `12.78.0.92`   | Chocolatey Mirror |
| `stonehedge`   | Ubuntu 24.04 LTS | `12.78.0.93`   | RustDesk Server   |
| `sunspear`     | Ubuntu 24.04 LTS | `12.78.0.94`   | WSTunnel Server   |
| `casterlyrock` | Windows          | `12.78.44.201` | Unmanaged Desktop |

<br>

> *Redirector Subnet — 86.234.0.0/16*

| **Hostname** | **OS**           | **IP**         | **Role**             |
|:-------------|:-----------------|:---------------|:---------------------|
| `stormlands` | Ubuntu 24.04 LTS | `14.121.222.9` | Redirection Server 1 |
| `stark`      | Ubuntu 24.04 LTS | `49.67.12.9`   | Redirection Server 2 |
| `bolton`     | Ubuntu 24.04 LTS | `10.44.44.9`   | Redirection Server 3 |
| `greyjoy`    | Ubuntu 24.04 LTS | `10.44.44.9`   | Redirection Server 4 |

*For more about redirection, please see [Traffic Redirection](Traffic-Redirection.md).*

<br>

### Red Team Hosts

The hosts below are not accessible by evaluation participants.

| **Hostname** | **OS**              | **IP**         | **Scenario**        |
|:-------------|:--------------------|:---------------|:--------------------|
| `driftmark`  | Kali Linux 2024.4   | `174.3.0.70`   | Attacker            |
| `braavos`    | Kali Linux 2024.4   | `174.3.0.100`  | Attacker IdP        |
| `jumpbox`    | Windows Server 2022 | `12.78.110.37` | Detections Jumpbox  |
| `dreadfort`  | Windows Server 2022 | `10.26.3.125`  | Protections Jumpbox |

<br>

-------

## Network Diagram

The diagram below shows the layout of all victim hosts, attack platform, and support hosts.
![Evaluations Enterprise 2025](assets/evaluations-enterprise-round-7.svg)

## Notice

© 2025 MITRE. Approved for public release. Document number 25-2969.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
file except in compliance with the License. You may obtain a copy of the License at

<http://www.apache.org/licenses/LICENSE-2.0>

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied. See the License for the specific language governing
permissions and limitations under the License.
