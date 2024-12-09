# DPRK

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Based on open-source intelligence, the MITRE ATT&CK&reg; Evaluations team created
the DPRK scenario leveraging techniques seen in operations in the wild. The
scenario was designed based on tools, resources, and intelligence available at
the time.

## Adversary Overview

Threat actors linked to the Democratic People's Republic of Korea (DPRK) conduct cyber operations primarily targeting financial institutions (cryptocurrency, banking, blockchain) as well as the defense and the technology sectors. These actors are known to conduct comprehensive research on specific targets, often taking a [personalized approach](https://www.nsa.gov/Press-Room/Press-Releases-Statements/Press-Release-View/Article/3413621/us-rok-agencies-alert-dprk-cyber-actors-impersonating-targets-to-collect-intell/) to build rapport and frequently impersonating legitimate contacts or organizations to enhance credibility. DPRK-affiliated adversaries have [expanded](https://www.sentinelone.com/blog/dprk-crypto-theft-macos-rustbucket-droppers-pivot-to-deliver-kandykorn-payloads/) their targeting to include macOS systems, employing social engineering techniques to compel users to execute malware disguised as legitimate software, launching sophisticated supply chain attacks, masquerading malicious files as legitimate applications, and evading detection while maintaining control over victim environments. Actions on objectives for these actors typically involve the theft of sensitive information such as admin passwords, session cookies, and macOS keychains. DPRK state-sponsored actors continue to refine their targeting, leveraging their campaigns to [fund the advancement](https://www.dni.gov/files/ODNI/documents/assessments/ATA-2023-Unclassified-Report.pdf) of their nuclear capabilities.

## Quick Links

### Resources

The [Resources Folder](./Resources/) contains the emulated software source code.

All other pre-built executables have been removed. To rebuild the binaries,
follow the documentation for the respective binary.

### Emulation Key Software 💻

- [FULLHOUSE.DOORED](./Resources/FULLHOUSE.DOORED/)
- [STRATOFEAR](./Resources/STRATOFEAR/)

### Scenario Walkthrough

- [DPRK Scenario Overview](./CTI_Emulation_Resources/DPRK_Scenario_Overview.md):
Overview of the DPRK scenario
- [DPRK Emulation Plan](./Emulation_Plan/ER6_DPRK_Scenario.md): Step by step
walkthrough of the DPRK red team emulation plan

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
