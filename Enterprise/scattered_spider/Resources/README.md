# Scattered Spider Resources

This directory contains all source code, tools, and infrastructure components required to execute the Scattered Spider adversary emulation scenario.

## Overview

The Scattered Spider emulation leverages social engineering, adversary-in-the-middle (AiTM) attacks, and cloud-focused techniques to emulate the behaviors of this cybercrime group. All components are provided as source code and must be built before use.

## Directory Structure

### Core Infrastructure

- **[controlServer/](controlServer/)** - Go-based C2 server with REST API for managing implant sessions and executing commands
- **[setup/](setup/)** - Infrastructure deployment automation using Terraform and Ansible
- **[payloads/](payloads/)** - Payload files including spearphishing templates and deployment scripts

### Emulation Tools

- **[AITM/](AITM/)** - Adversary-in-the-middle phishing framework that mimics Authentik SSO to harvest credentials and bypass MFA
- **[aws_consoler/](aws_consoler/)** - Tool for generating AWS console sessions from access keys
- **[jecretz/](jecretz/)** - Credential extraction tool for cloud environments
- **[rmmagent/](rmmagent/)** - TacticalRMM remote management agent for persistent access
- **[snaffler/](snaffler/)** - File share enumeration and sensitive data discovery tool
- **[trufflehog/](trufflehog/)** - Secret scanning tool for finding credentials in code repositories

### Supporting Resources

- **[email_generation/](email_generation/)** - Scripts for generating and sending spearphishing emails
- **[Images/](Images/)** - Image assets for documentation and operations
- **[assets/](assets/)** - Scenario diagrams and visual documentation

## Getting Started

1. **Review the scenario** - Start with the [Scenario Overview](../CTI_Emulation_Resources/Scattered_Spider_Scenario_Overview.md) to understand the attack flow
2. **Set up infrastructure** - Follow the [setup documentation](setup/) to deploy the required environment
3. **Build components** - Each tool directory contains build instructions in its README
4. **Configure C2 server** - Set up the [control server](controlServer/) to manage implant communications
5. **Execute emulation** - Follow the [Emulation Plan](../Emulation_Plan/Scattered_Spider_Scenario.md) for step-by-step execution

## Key Techniques Emulated

- **Initial Access**: Spearphishing with malicious links
- **Credential Access**: AiTM phishing to bypass MFA, credential dumping from cloud services
- **Persistence**: Remote management tools (TacticalRMM)
- **Discovery**: Cloud environment enumeration, file share scanning
- **Collection**: Sensitive data identification and aggregation
- **Command and Control**: Custom C2 infrastructure

## Prerequisites

- Go 1.23+ (for control server, AITM server, and TruffleHog)
- Python 3.x (for various scripts and tools)
- Terraform and Ansible (for infrastructure deployment)
- AWS account (for cloud-based scenarios)
- Target environment matching the scenario requirements

## Important Notes

- All executables have been removed from this repository for security reasons
- You must build all components from source following the documentation
- Ensure you have proper authorization before using these tools
- Review the [Liability / Responsible Usage](../README.md#liability--responsible-usage) section

## Documentation

Each subdirectory contains its own README with specific build instructions, usage examples, and technical details. Refer to individual component documentation for detailed information.

## Support

For questions or issues:

- Email: <evals@mitre.org>
- Review the main [Scattered Spider README](../README.md) for additional resources
