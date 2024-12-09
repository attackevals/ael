# Ransomware Protections Micros

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Based on open-source intelligence, the MITRE ATT&CK&reg; Evaluations team created
the ransomware protections micro emulations by leveraging common ransomware behaviors seen
in operations in the wild. The micros were designed based on tools, resources,
and intelligence available at the time.

## Threat Overview

Ransomware continues to be one of the most significant [global cybercriminal threats](https://www.dni.gov/files/CTIIC/documents/products/Worldwide_Ransomware_Activity_Oct_2022Sep_2023.pdf) across industry verticals. Ransomware operators, in response to [government](https://www.whitehouse.gov/briefing-room/statements-releases/2023/11/01/fact-sheet-biden-harris-administration-convenes-third-global-gathering-to-counter-ransomware/) and [law enforcement](https://www.justice.gov/opa/pr/us-department-justice-disrupts-hive-ransomware-variant) actions and evolving defensive capabilities, are continuously adjusting their activities within an ecosystem shaped by advancing technology and [Ransomware-as-a-service (RaaS)](https://www.ibm.com/topics/ransomware-as-a-service) models. The evolution to the more adaptable RaaS model reduces barriers to entry for malicious groups, eliminating the need for custom malware and enabling less experienced operators to successfully target organizations. This democratization of ransomware has fueled a [surge in ransomware attacks](https://www.corvusinsurance.com/blog/q4-ransomware-report) worldwide.

The Protections micro emulations focus on evaluating protection mechanisms against key ransomware behaviors in an assumed-compromise, micro-emulation format. The evaluated behaviors include:

- Enumeration and exfiltration of files
- Enumeration and encryption of files
- Propagation via automated host discovery and lateral movement

Each Protections micro is built to be stand-alone, with no dependencies on the success of a prior micro emulation.

## Quick Links

### Resources

The [Resources Folder](./Resources/) contains the emulated software source code.

All other pre-built executables have been removed. To rebuild the binaries,
follow the documentation for the respective binary.

### Protections Micro Walkthrough

- [Ransomware Protections Micros](./Emulation_Plan/ER6_Ransomware_Protections.md): Step by step
walkthrough for executing the Protections micros

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
