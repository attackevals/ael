# CL0P

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Based on open-source intelligence, the MITRE ATT&CK&reg; Evaluations team created
the CL0P scenario leveraging techniques seen in operations in the wild. The
scenario was designed based on tools, resources, and intelligence available at
the time.

## Adversary Overview

Active since at least 2019, [CL0P](https://attack.mitre.org/software/S0611/) is a sophisticated ransomware family that is associated with the [TA505](https://attack.mitre.org/groups/G0092/) cybercriminal group and possibly FIN11. CL0P employs phishing campaigns to gain initial access, often using macro-enabled documents to deploy malicious loaders. Like most other ransomware families, CL0P leverages “steal, encrypt, and leak” strategy - they identify and encrypt files, append various extensions (e.g., .clop) and leverage threats of data leaks on their Tor site, CL0P^_-LEAKS, to pressure victims into paying ransoms. Recent attacks have exploited vulnerabilities in software such as [MOVEit Transfer](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-158a) and [GoAnywhere](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-158a) MFT, leading to significant breaches and prompting U.S. authorities to offer [rewards](https://x.com/RFJ_USA/status/1669740545403437056?prefetchTimestamp=1731528766880) for information on the group's members. CL0P is utilized for financial gain and therefore, has a more randomized approach in its targeting and does not appear to target any particular region or sector. CL0P's adaptive tactics and extensive targeting capabilities make it one of the most formidable ransomware threats today.

## Quick Links

### Resources

The [Resources Folder](./Resources/) contains the emulated software source code.

All other pre-built executables have been removed. To rebuild the binaries,
follow the documentation for the respective binary.

#### Infrastructure Configuration

The [Resources/setup folder](./Resources/setup/) contains the infrastructure setup, including Terraform, Ansible, and related configurations and code.

### Emulation Key Software 💻

- [SDBbot](./Resources/sdbbot/)
- [CL0P](./Resources/Cl0p/)

### Scenario Walkthrough

- [CL0P Scenario Overview](./CTI_Emulation_Resources/CL0P_Scenario_Overview.md):
Overview of the CL0P scenario
- [CL0P Emulation Plan](./Emulation_Plan/ER6_CL0P_Scenario.md): Step by step
walkthrough of the CL0P red team emulation plan

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
