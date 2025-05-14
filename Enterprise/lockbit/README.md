# LockBit

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Based on open-source intelligence, the MITRE ATT&CK&reg; Evaluations team created
the LockBit scenario leveraging techniques seen in operations in the wild. The
scenario was designed based on tools, resources, and intelligence available at
the time.

## Adversary Overview

First launched as “ABCD” ransomware in 2019, LockBit is a notorious ransomware variant known for its use of sophisticated tools, extortion methods, and high-severity attacks. LockBit operates a Ransomware-as-a-Service (RaaS) model, enabling affiliates to launch attacks using its tools. This decentralized approach democratizes access to ransomware tools and fosters a variety of behaviors that make detection and response more complex and unpredictable. Law enforcement agencies [deemed](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a) LockBit the “most deployed ransomware variant across the world” in 2022, and [nearly 30%](https://www.trendmicro.com/vinfo/us/security/news/ransomware-by-the-numbers/lockbit-blackcat-and-clop-prevail-as-top-raas-groups-for-1h-2023) of all ransomware attacks between 2022 and 2023 were conducted by LockBit affiliates. LockBit adapted continuously, launching [bug bounty programs](https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-lockbit) with LockBit 3.0 to improve its tools, targeting both Windows and Linux systems with evolving evasion tactics, and incorporating elements from the now-defunct Conti ransomware into LockBit Green, reflecting the code and tactic recycling trend in the ransomware ecosystem. To gain access on target systems, LockBit attackers have used legitimate resources (e.g., penetration testing tools Metasploit and Cobalt Strike, leveraging remote desktop protocols (RDP), native utilities such as PowerShell and batch scripts) as well as the exploitation of well-known [vulnerabilities](https://www.fortinet.com/blog/threat-research/lockbit-most-prevalent-ransomware) (e.g., [CVE-2018-13379](https://nvd.nist.gov/vuln/detail/CVE-2018-13379), [CVE-2019-0708](https://nvd.nist.gov/vuln/detail/cve-2019-0708), and [CVE-2020-1472](https://nvd.nist.gov/vuln/detail/cve-2020-1472)).  LockBit remained pervasive until its global impact prompted a [multinational law enforcement operation](https://www.nationalcrimeagency.gov.uk/news/nca-leads-international-investigation-targeting-worlds-most-harmful-ransomware-group) to disrupt LockBit-affiliated platforms in February 2024. Despite the operation, LockBit continued to be the most prevalent strain until May 2024. Ongoing efforts under [Operation Cronos](https://www.europol.europa.eu/media-press/newsroom/news/lockbit-power-cut-four-new-arrests-and-financial-sanctions-against-affiliates) led to the [arrest and sanction LockBit operators](https://cyberscoop.com/lockbit-arrests-ransomware-fbi-uk-nca-evil-corp/) in October 2024.

## Quick Links

### Resources

The [Resources Folder](./Resources/) contains the emulated software source code.

All other pre-built executables have been removed. To rebuild the binaries,
follow the documentation for the respective binary.

#### Infrastructure Configuration

The [Resources/setup folder](./Resources/setup/) contains the infrastructure setup, including Terraform, Ansible, and related configurations and code.

### Emulation Key Software 💻

- [Firepwd](./Resources/firepwd/)
- [StealBit](./Resources/Stealbit/)
- [ThunderShell](./Resources/ThunderShell/)
- [LockBit](./Resources/Lockbit/)

### Scenario Walkthrough

- [LockBit Scenario Overview](./CTI_Emulation_Resources/LockBit_Scenario_Overview.md):
Overview of the LockBit scenario
- [LockBit Emulation Plan](./Emulation_Plan/ER6_LockBit_Scenario.md): Step by step
walkthrough of the LockBit red team emulation plan

## Acknowledgements

We would like to formally thank the people that contributed to the content,
review, and format of this document. This includes the MITRE ATT&CK and MITRE
ATT&CK Evaluations teams, the organizations and people that provided public
intelligence and resources, as well as the following organizations that
participated in the community cyber threat intelligence contribution process:

- CrowdStrike
- Microsoft
- Sophos

## Connect with us 🗨️

We 💖 feedback! Let us know how using ATT&CK Evaluation results has helped you
and what we can do better.

- Email: <evals@mitre.org>
- Twitter: <https://x.com/MITREcorp>
- LinkedIn: <https://www.linkedin.com/showcase/attack-evaluations/>

## Liability / Responsible Usage

This content is only to be used with appropriate prior, explicit authorization
for the purposes of assessing security posture and/or research.

## Notice

© 2024 MITRE Engenuity. Approved for Public Release. Document number CT0005.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

<http://www.apache.org/licenses/LICENSE-2.0>

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
language governing permissions and limitations under the License.

This project makes use of ATT&CK®

[ATT&CK Terms of Use](https://attack.mitre.org/resources/terms-of-use/)
