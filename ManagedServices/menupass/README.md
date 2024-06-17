
# menuPass

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Based on open-source intelligence, the MITRE ATT&CK&reg; Evaluations team created the menuPass scenario leveraging techniques seen in the wild. The scenario was built based on tools, intelligence, and resources available at the time.

## Adversary Overview 🐼
Active since at least 2006, [menuPass](https://attack.mitre.org/groups/G0045/) (aka APT10) is a threat group believed to be sponsored by the Chinese Ministry of State Security (MSS). 
The group has focused on the exfiltration of sensitive data in support of Chinese national security objectives. 
menuPass has targeted the aerospace, construction, engineering, government, and telecommunications sectors primarily in the US, Europe, Japan, and other parts of Southeast Asia. 
The group is known for orchestrating [Operation Cloud Hopper](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-report-april-2017.pdf), a global espionage campaign discovered in 2016 and 2017 that involved the sustained exploitation of MSPs.
In December 2018, the US DOJ filed [criminal charges](https://www.justice.gov/opa/pr/two-chinese-hackers-associated-ministry-state-security-charged-global-computer-intrusion) against two menuPass members in connection to the group’s activities.

MSS-affiliated groups such as menuPass have demonstrated a pattern of exploiting targets on a much broader geographic scope than the groups sponsored by the People Liberation Army (PLA), and are believed to be more likely to target the United States and regions outside of China’s direct sphere of influence. menuPass is known specifically for its use of living-off-the-land techniques, leveraging custom malware, utilizing fileless and anti-analysis tactics for defense evasion, and exploiting trusted third-party relationships for credential access.

## Quick Links

### Resources

The [Resources Folder](./Resources/) contains the emulated software source code.

All other pre-built executables have been removed. To rebuild the binaries,
follow the documentation for the respective binary.

### Emulation Key Software 💻

- [SigLoader](./Resources/SigLoader/)
- [FYAnti](./Resources/fyanti/)
- [SodaMaster](./Resources/SodaMaster/)
- [Quasar](./Resources/Quasar/)
- [Impacket](./Resources/impacket/)
- [WMIexec.vbs](https://github.com/Twi1ight/AD-Pentest-Script/blob/master/wmiexec.vbs)

### Scenario Walkthrough

- [menuPass Scenario Overview](./CTI_Emulation_Resources/menuPass_Scenario_Overview.md) - Overview of the menuPass scenario
- [menuPass Emulation Plan](./Emulation_Plan/MSR2_menuPass_Scenario.md) - Step by step walkthrough of the menuPass red team emulation plan (9 steps)

## Acknowledgements

We would like to formally thank the people that contributed to the content, review, and format of this document. 
This includes the MITRE ATT&CK and MITRE ATT&CK Evaluations teams, the organizations and people that provided public intelligence and resources, as well as the following organizations that participated in the community cyber threat intelligence contribution process:

- Microsft
- CrowdStrike

## Connect with us 🗨️

We 💖 feedback! Let us know how using ATT&CK Evaluation results has helped you and what we can do better. 

Email: <evals@mitre-engenuity.org><br>
Twitter: https://twitter.com/MITREengenuity<br>
LinkedIn: https://www.linkedin.com/company/mitre-engenuity/<br>

## Liability / Responsible Usage

This content is only to be used with appropriate prior, explicit authorization for the purposes of assessing security posture and/or research.

## Notice

© 2024 MITRE Engenuity. Approved for Public Release. Document number CT0005.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

This project makes use of ATT&CK®

[ATT&CK Terms of Use](https://attack.mitre.org/resources/terms-of-use/)