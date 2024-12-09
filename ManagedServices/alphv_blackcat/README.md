---
category: Managed-Services
---

# ALPHV BlackCat

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Based on open-source intelligence, the MITRE ATT&CK&reg; Evaluations team created the ALPHV BlackCat scenario leveraging techniques seen in affiliate operations in the wild. The scenario was designed based on tools, resources, and intelligence available at the time.

## Adversary Overview :cat2:&zwj;:black_large_square:

ALPHV BlackCat, also known as Noberus, was a prolific Russian-speaking, ransomware-as-a-service (RaaS) group that emerged in 2021 and was linked to BlackMatter, DarkSide, REvil, and other RaaS groups [1](https://www.ic3.gov/Media/News/2022/220420.pdf). ALPHV BlackCat utilized ransomware coded in Rust, allowing for enhanced performance, flexibility, and cross-platform capabilities. During it's tenure, ALPHV BlackCat consistently upgraded its tooling and tradecraft, with the last variant, Sphynx, rewritten with enhanced defense evasion capabilities, according to the group [2](https://twitter.com/vxunderground/status/1649094229413761030), [3](https://aspr.hhs.gov/cyber/Documents/stopransomware-508.pdf). The group is alleged to have targeted over a thousand victims across the globe [4](https://www.state.gov/reward-for-information-alphv-blackcat-ransomware-as-a-service/).  In December 2023, The United States (U.S.) Department of Justice announced they had disrupted group operations and developed an decryption tool for victims [5](https://www.justice.gov/opa/pr/justice-department-disrupts-prolific-alphvblackcat-ransomware-variant). The group announced it was removing restrictions previously placed on affiliates, and the most recent targets included [MGM Resorts](https://blog.morphisec.com/mgm-resorts-alphv-spider-ransomware-attack) and [Change Healthcare](https://www.reuters.com/technology/cybersecurity/cyber-security-outage-change-healthcare-continues-sixth-straight-day-2024-02-26/) in March 2024. Following the Change Healthcare incident, the Department of State issued a [reward offer](https://www.state.gov/rewards-for-justice-reward-offer-for-information-on-alphv-blackcat-linked-cyber-actors-targeting-u-s-critical-infrastructure/) for information on the group due to its targeting of U.S. Critical Infrastructure.

# Quick Links

### Resources

The [Resources Folder](./Resources/) contains the emulated software source code.

All other pre-built executables have been removed. To rebuild the binaries,
follow the documentation for the respective binary.

### Emulation Key Software 💻

- [BlackCat ransomware](./Resources/blackcat/)
- [ExMatter](./Resources/ExMatter/)
- [InfoStealer](./Resources/infostealer/)

### Scenario Walkthrough

- [ALPHV BlackCat Scenario Overview](./CTI_Emulation_Resources/ALPHV-Blackcat_Scenario_Overview.md) - Overview of the ALPHV BlackCat scenario
- [ALPHV BlackCat Emulation Plan](./Emulation_Plan/ALPHV_BlackCat_Scenario.md) - Step by step walkthrough of the ALPHV BlackCat red team emulation plan (6 steps)

## Acknowledgements

We would like to formally thank the people that contributed to the content, review, and format of this document. This includes the MITRE ATT&CK and MITRE ATT&CK Evaluations teams, the organizations and people that provided public intelligence and resources, as well as the following organizations that participated in the community cyber threat intelligence contribution process:

- Microsoft
- CrowdStrike

## Connect with us 🗨️

We 💖 feedback! Let us know how using ATT&CK Evaluation results has helped you and what we can do better.

Email: <evals@mitre.org><br>
Twitter: <https://x.com/MITREcorp><br>
LinkedIn: <https://www.linkedin.com/showcase/attack-evaluations/><br>

## Liability / Responsible Usage

This content is only to be used with appropriate prior, explicit authorization for the purposes of assessing security posture and/or research.

## Notice

© 2024 MITRE Engenuity. Approved for Public Release. Document number CT0005.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

<http://www.apache.org/licenses/LICENSE-2.0>

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

This project makes use of ATT&CK®

[ATT&CK Terms of Use](https://attack.mitre.org/resources/terms-of-use/)
