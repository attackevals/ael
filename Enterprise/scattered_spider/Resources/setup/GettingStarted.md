---
tags: [infrastructure, enterprise2025, cloud, terraform, ansible, setup, quickstart]
---

# Getting Started

This document covers the infrastructure setup for emulating [ATT&CK Evaluations](https://evals.mitre.org/) — Scattered Spider and Mustang Panda (2025).

- [Getting Started](#getting-started)
  - [Overview](#overview)
  - [Prerequisites](#prerequisites)
    - [Installation](#installation)
    - [Windows 11](#windows-11)
  - [Deploy Infrastructure with Terraform](#deploy-infrastructure-with-terraform)
    - [Amazon Marketplace Subscriptions](#amazon-marketplace-subscriptions)
    - [Deploy Environment](#deploy-environment)
  - [Configure Infrastructure with Ansible](#configure-infrastructure-with-ansible)
  - [Deploy Attack Platform Software](#deploy-attack-platform-software)

## Overview

ATT&CK Evaluations used the [Amazon Web Services (AWS)](https://aws.amazon.com/) cloud platform.

!!!

**AWS Cost Estimate:** A rough estimated cost for the environment is approximately $2500 _per month_, assuming the hosts are run 24/7.
!!!

To further reduce costs, EC2 instances should be [shutdown and stopped](https://aws.amazon.com/ec2/faqs/) when not in active use. Specific costs for AWS resources can vary greatly depending on your specific AWS account agreements, region, savings plans, reservations, how frequently the systems are online, and other cost reduction techniques you use.

## Prerequisites

It is assumed that you have a working knowledge of and access to the following concepts and tools:

- Two (2) AWS Accounts with permissions to perform required actions, such as creating and managing:
  - EC2 instances
  - VPCs
  - Internet Gateways
  - Security Groups
  - [Cross-account IAM configurations](./terraform/README.md#cross-account-management)
- One (1) GitHub account with token-based Multifactor Authentication (MFA) enabled for the Mustang Panda scenario
- Familiarity with common system administration concepts for Windows and Linux
- Essential tools installed:
  - `ssh-keygen`
  - `bash` shell
  - [Terraform](https://developer.hashicorp.com/terraform/downloads)
  - [Ansible](https://docs.ansible.com/)
  - [AWS CLI](https://aws.amazon.com/cli/)

### Installation

Please see [Terraform](https://developer.hashicorp.com/terraform/downloads), [Ansible](https://docs.ansible.com/ansible/latest/installation_guide/installation_distros.html), and [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html) documentation for installation instructions.

### Windows 11

!!!

**IMPORTANT**: AWS does not currently provide a pre-built Windows 11 Amazon Machine Image (AMI), and the team is unable to share the image used for the evaluation. As a result, you must provide your own Windows 11 AMI for use in the environment. Please review the Amazon and Microsoft terms of service to ensure you are compliant with all licensing guidelines.

!!!

<u>The included configuration uses Windows Server 2022 for all Windows 11 desktop systems to provide a convenient and functional deployment</u>.
References of `module.amis.windows-desktop-11` is a lookup for the Windows Server 2022 AMI ID until you change this at the module level.
It is not guaranteed that the emulation will operate as intended on Windows Server 2022 in instances where Windows 11 is expected.

It is strongly recommended that you BYO Windows 11 image, then see the [`amis` module documentation](terraform/modules/aws/amis/README.md) to set and use your private Windows 11 AMI. The following steps provide guidance on building your own Windows 11 AMI, which uses:

1. [Packer](https://www.packer.io/) to automate the build of a Windows 11 VM from the installation image.
2. Packer to upload the virtual machine file to an AWS S3 bucket.
3. [AWS VM Import](https://aws.amazon.com/blogs/modernizing-with-aws/bringing-your-windows-11-image-to-aws-with-vm-import-export/) to convert the VM to an AMI.

AWS provides additional [documentation on importing a Windows 11 VM](https://aws.amazon.com/blogs/modernizing-with-aws/bringing-your-windows-11-image-to-aws-with-vm-import-export/).

## Deploy Infrastructure with Terraform

> _⏳ Depending on bandwidth, deployment of infrastructure may take 40 minutes to 2 hours complete._

Terraform is used to initialize the infrastructure. Please review the architecture in the [Deployment Overview](./terraform/README.md) before proceeding.

### Amazon Marketplace Subscriptions

During the deployment process, you may be prompted to accept the Terms to subscribe for AWS Marketplace images. If prompted, follow the URL provided in the terminal then click the Subscribe button. Once the subscription process is complete, re-run and apply the Terraform plan.

### Deploy Environment

Infrastructure is initialized using a [3-stage Terraform deployment](./terraform/README.md) in the subdirectories:

1. AWS Account Setup — [`orgs`](./terraform/orgs/README.md)
2. Attacker Staging — [`s3`](./terraform/s3/README.md)
3. Evaluation Environment — [`range`](./terraform/range/README.md)

The following steps must be performed per stage in the order listed above:

1. Open the `deploy.auto.tfvars` in an editor, and configure with desired values.
2. Run Terraform:

    ```bash
    terraform init                      # 1. Initialize your Terraform environment
    terraform validate                  # 2. Validate your Terraform configuration
    terraform plan --out=tfplan         # 3. Create a Terraform plan of your deployment
    terraform apply tfplan              # 4. Execute the Terraform plan (estimated time to deploy: 20-40 minutes)
    ```

    After deploying `range/`, use the Makefile in this directory to:

3. Generate VPN Profiles

    ```bash
    make create-vpn-profiles            # Create OpenVPN profiles to connect to your infrastructure
    ```

    The OpenVPN profiles will be stored as `.zip` files in the `publish/` subdirectory, which you will need to connect to the range and run host-based configurations.

4. All systems have been deployed! They still have to be configured, however time to celebrate! :pizza: :confetti_ball: :fireworks:

## Configure Infrastructure with Ansible

> _⏳ Depending on bandwidth, configuration of infrastructure may take 1-3 hours to complete._

Ansible is used to configure the infrastructure. Please review playbook information in the [Configuration Overview](./ansible/README.md) before proceeding.

1. Use any one of the VPN profiles generated in the previous step.
2. Change to Ansible base directory.

    ```bash
    cd ../../ansible
    ```

3. Run Ansible configuration.

    ```bash
    make deploy
    ```

4. Wait. Enjoy a hot chocolate or other delicious beverage :coffee:. Ensure your provisioning host will not sleep or disconnect from the VPN during the provisioning step.

## Deploy Attack Platform Software

[Clone](https://docs.github.com/en/repositories/creating-and-managing-repositories/cloning-a-repository) the [ATTACK Evaluations Library](https://github.com/attackevals/ael) to the `driftmark` and `jumpbox` (Kali Linux and Windows Jumpbox hosts). Follow the instructions to setup the red team tooling.

## Notice

© 2025 MITRE. Approved for public release. Document number 25-2969.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
file except in compliance with the License. You may obtain a copy of the License at

<http://www.apache.org/licenses/LICENSE-2.0>

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied. See the License for the specific language governing
permissions and limitations under the License.
