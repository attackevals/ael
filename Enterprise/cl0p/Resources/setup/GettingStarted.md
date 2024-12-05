# Getting Started

This document covers the infrastructure setup for emulating [ATT&CK Evaluations](https://attackevals.mitre-engenuity.org/) - CL0P, LockBit, & DPRK (2024).

- [Getting Started](#getting-started)
  - [Overview](#overview)
    - [AWS Cost Estimate](#aws-cost-estimate)
    - [MacOS Instances](#macos-instances)
  - [Prerequisites](#prerequisites)
    - [Installation](#installation)
      - [Linux](#linux)
      - [macOS](#macos)
  - [Deploy Infrastructure with Terraform](#deploy-infrastructure-with-terraform)
    - [Amazon Marketplace Subscriptions](#amazon-marketplace-subscriptions)
    - [Windows 11](#windows-11)
    - [Deploy environment](#deploy-environment)
  - [Configure Infrastructure with Ansible](#configure-infrastructure-with-ansible)
  - [Deploy Attack Platform Software](#deploy-attack-platform-software)

## Overview

ATT&CK Evaluations used the [Amazon Web Services (AWS)](https://aws.amazon.com/) cloud platform.

### AWS Cost Estimate

A rough estimated cost for the environment is approximately $2500 _per month_, assuming the hosts are run 24/7. To further reduce costs, EC2 instances should be [shutdown and stopped](https://aws.amazon.com/ec2/faqs/) when not in active use.

Specific costs for the AWS resources used can vary greatly depending on your specific AWS account agreements, region, savings plans, reservations, how frequently the systems are online, and other cost reduction techniques you use.

### MacOS Instances

MacOS instances currently account for roughly a third of the standard overall cost of the ER6 Infrastructure, despite representing only two (2) of 31 instances. This is due an AWS requirement for provisioning MacOS on [_Dedicated Hosts_](https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/ec2-mac-instances.html):

> Mac instances are available only as bare metal instances on Dedicated Hosts, with a minimum allocation period of 24 hours before you can release the Dedicated Host. You can launch one Mac instance per Dedicated Host. You can share the Dedicated Host with the AWS accounts or organizational units within your AWS organization, or the entire AWS organization.

Note that stopping or terminating a Mac instance initiates a scrubbing workflow of the underlying Dedicated Host, during which the dedicated host enters a pending state and becomes unavailable. This scrubbing workflow includes tasks such as erasing the internal SSD, resetting NVRAM, and more. AWS provides an estimated time of 110 minutes to complete the scrubbing process for Apple Silicon macOS instances.

## Prerequisites

It is assumed that you have access to, and a working knowledge of, the following concepts and tools:

- Existing AWS Account with permissions to perform required actions (create/manage resources including EC2 instances, VPCs, Internet Gateways, Security Groups, and related resources)
- Familiar with common system administration concepts for Windows, MacOS, and Linux
- Tools
  - `ssh-keygen`
  - `bash` shell
  - [Terraform](https://developer.hashicorp.com/terraform/downloads)*
  - [Ansible](https://docs.ansible.com/)*
  - [AWS CLI](https://aws.amazon.com/cli/) installed

### Installation

#### Linux

On Linux, see [Terraform](https://developer.hashicorp.com/terraform/downloads) and [Ansible](https://docs.ansible.com/ansible/latest/installation_guide/installation_distros.html) documentation.

#### macOS

On Mac, using [Homebrew](https://brew.sh/):

```shell
brew tap hashicorp/tap
brew install hashicorp/tap/terraform
brew install ansible
```

## Deploy Infrastructure with Terraform

Terraform is used to initialize the infrastructure.

### Amazon Marketplace Subscriptions

You may be prompted to accept the Terms to subscribe for AWS Marketplace images. If prompted, follow the URL provided in the terminal then click the Subscribe button. Once the subscription process is complete, re-run the Terraform plan and apply commands below.

### Windows 11

!!! Windows 11 AMI
**IMPORTANT**: AWS does not currently provide a pre-built Windows 11 Amazon Machine Image (AMI), and the team is unable to share the image used for the evaluation. As a result, you will have to provide your own Windows 11 AMI for use in the environment. The following steps provide some general guidance on building your own Windows 11 AMI. Please review the Amazon and Microsoft terms of service to ensure you are compliant with all licensing guidelines.
!!!

1. [Packer](https://www.packer.io/) is used to automate the build of a Windows 11 VM from the installation image.
2. Packer uploads the virtual machine file to an AWS S3 bucket.
3. [AWS VM Import](https://docs.aws.amazon.com/vm-import/latest/userguide/what-is-vmimport.html) to convert the VM to an AMI.

AWS provides additional [documentation on importing a Windows 11 VM](https://aws.amazon.com/blogs/modernizing-with-aws/bringing-your-windows-11-image-to-aws-with-vm-import-export/).

By default, the included configuration uses Windows Server 2022 for all Windows 11 desktop systems to provide a functional system. This is for convenience, however it is not guaranteed that the emulation plan will operate as intended on Windows Server 2022 in instances where Windows 11 is expected.

### Deploy environment

1. Create a Terraform settings file from the included template.

    ```bash
    cp -a deploy.auto.tfvars.template deploy.auto.tfvars
    ```

2. Open the `deploy.auto.tfvars` in an editor, and configure with desired values.
3. In `deploy.auto.tfvars`, uncomment and set `aws-bucket-name` to the name of the S3 bucket you would like to use for storing Terraform state files. _Note: AWS S3 bucket names have to be unique and resolvable by DNS._
4. Edit the `backend.tf` file
    - Uncomment the `bucket` variable in the `terraform.backend.s3` section (bottom).
    - Set the value to the name of the S3 bucket in the previous step.
    - **IMPORTANT**: The value must be a string. You cannot use a variable in this section, you must hardcode the bucket name.
5. Run Terraform:

    ```bash
    terraform init                      # 1. Initialize your Terraform environment
    terraform validate                  # 2. Validate your Terraform configuration
    terraform plan --out=tfplan         # 3. Create a Terraform plan of your deployment
    terraform apply tfplan              # 4. Execute the Terraform plan (estimated time to deploy: 15-30 minutes)
    ```

6. Generate VPN Profiles

    ```bash
    make run create-vpn-profiles        # Create OpenVPN profiles to connect to your infrastructure
    ```

    The OpenVPN profiles will be stored as `.zip` files in the `publish/` subdirectory.

7. All systems have been deployed! They still have to be configured, however time to celebrate! :pizza: :confetti_ball: :fireworks:

## Configure Infrastructure with Ansible

For this round, all post deployment configuration was performed via Ansible.

1. Use one of the VPN profiles generated in the previous step. Any profile will work.
2. Use the Terraform Makefile to configure the Ansible deployment. The following command copies the SSH key to the Ansible setup directory with the proper permissions.

    ```bash
    make ansible-setup
    ```

3. Change to Ansible base directory.

    ```bash
    cd ../../ansible
    ```

4. Run Ansible configuration.

    ```bash
    make configure-range-from-scratch
    ```

5. Wait. Enjoy a hot chocolate or other delicious beverage :coffee:. Ensure your provisioning host will not sleep or disconnect from the VPN during the provisioning step. Depending on bandwidth, the deployment could take 1-3 hours to fully configure all infrastructure.

## Deploy Attack Platform Software

[Clone](https://docs.github.com/en/repositories/creating-and-managing-repositories/cloning-a-repository) the [ATTACK Evaluations Library](https://github.com/attackevals/ael) to the `corsair` and `spitfire` (Kali Linux and Windows Jumpbox hosts). Follow the instructions to setup the red team tooling.
