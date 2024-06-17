# Getting Started

This document covers the infrastructure setup for emulating [ATT&CK Evaluations](https://attackevals.mitre-engenuity.org/) - ALPHV BlackCat and menuPass (2024).

- [Getting Started](#getting-started)
  - [Overview](#overview)
    - [AWS Cost Estimate](#aws-cost-estimate)
    - [WARNING - $$$](#warning---)
      - [KVM Server Cost](#kvm-server-cost)
  - [Prerequisites](#prerequisites)
  - [Deploy Infrastructure with Terraform](#deploy-infrastructure-with-terraform)
    - [Amazon Marketplace Subscriptions](#amazon-marketplace-subscriptions)
    - [Windows 11](#windows-11)
    - [Deploy environment](#deploy-environment)
  - [Configure Infrastructure with Ansible](#configure-infrastructure-with-ansible)
  - [Deploy Attack Platform Software](#deploy-attack-platform-software)

## Overview

ATT&CK Evaluations used the [Amazon Web Services (AWS)](https://aws.amazon.com/) cloud platform.

### AWS Cost Estimate

Specific costs for the AWS resources used will depend on your specific AWS account agreements, region, savings plans, reservations, how frequently the systems are online, and other cost reduction techniques you use. A rough estimated cost for the environment is approximately $6000 - $10000 _per month_, assuming the hosts are run 24/7. Note that the `leomon` host, which uses the `m5.metal` instance type, accounts for 50%+ of the overall cost. Instances should be [shutdown and stopped](https://aws.amazon.com/ec2/faqs/) when not in use to further reduce cost.

### WARNING - $$$

#### KVM Server Cost

WARNING: As mentioned above, the most expensive component is the Kernel Virtual Machine (KVM) Linux server, `leomon`, used to run virtual machines. The KVM server uses the AWS instance type `m5.metal`, which alone costs $4.608 per hour (at the time of the evaluation), and accounts for ~50% of cost. To reduce cost, you can keep the KVM server off until used, or skip provisioning the KVM host.

## Prerequisites

It is assumed you have a working knowledge of the following concepts and access to related tools:

* Existing AWS Account with permissions to perform required actions (create/manage resources including EC2 instances, VPCs, Internet Gateways, Security Groups, and related resources)

* Tools

  * `ssh-keygen`,
  * `bash` shell,
  * [Terraform](https://developer.hashicorp.com/terraform/downloads),
  * [Ansible](https://docs.ansible.com/)
  * [AWS CLI](https://aws.amazon.com/cli/) installed

* Install

  * On Mac with [Homebrew](https://brew.sh/) installed

    * Example:

      ```
      brew tap hashicorp/tap
      brew install hashicorp/tap/terraform
      brew install ansible
      ```

  * On Linux, see [Terraform](https://developer.hashicorp.com/terraform/downloads) and [Ansible](https://docs.ansible.com/ansible/latest/installation_guide/installation_distros.html) documentation

*  and common system administration concepts for Windows and Linux

## Deploy Infrastructure with Terraform

Terraform is used to initialize the infrastructure.

### Amazon Marketplace Subscriptions

**NOTE**: You may be prompted to accept the Terms and subscribe for images used in the AWS Marketplace. If prompted, follow the URL provided in the terminal and click the Subscribe button. Once the process completes, re-run the Terraform plan and apply commands below.

### Windows 11

**IMPORTANT**: AWS does not currently provide a pre-built Windows 11 Amazon Machine Image (AMI), and the team is unable to share the image used for the evaluation. As a result, you will have to provide your own Windows 11 AMI for use in the environment. The following steps provide some general guidance on building your own Windows 11 AMI. Please review the Amazon and Microsoft terms of service to ensure you are compliant with all licensing guidelines.

1. [Packer](https://www.packer.io/) is used to automate the build of a Windows 11 VM from the installation image.
2. Packer uploads the virtual machine file to an AWS S3 bucket.
3. [AWS VM Import](https://docs.aws.amazon.com/vm-import/latest/userguide/what-is-vmimport.html) to convert the VM to an AMI.

 AWS provides additional [documentation on importing a Windows 11 VM](https://aws.amazon.com/blogs/modernizing-with-aws/bringing-your-windows-11-image-to-aws-with-vm-import-export/).

By default, the included configuration uses Windows Server 2022 for all Windows 11 desktop systems to provide a functional system. This is for convenience, however it is not guaranteed that the emulation plan will operate as intended on Windows Server 2022 in instances where Windows 11 is expected.

### Deploy environment

1. Create a Terraform settings file from the included template.
   1. `cp -a deploy.auto.tfvars.template deploy.auto.tfvars`
   2. Open the `deploy.auto.tfvars` in an editor, and configure with desired values.

2. Edit the `deploy.auto.tfvars` file:
   1. Uncomment the `aws-bucket-name` variable, and set to the name of the S3 bucket you would like to use for storing terraform state files. AWS S3 bucket names have to be unique and resolvable by DNS.

3. Edit the `backend.tf` file:
   1. Towards the bottom of the file, uncomment the `bucket` variable in the `terraform.backend.s3` section.

   2. Set the value to the name of the S3 bucket in the previous step.
      1. * **IMPORTANT**: The value must be a string. You cannot use a variable in this section, you must hardcode the bucket name.

4. Time to run Terraform.
   1. Initialize your Terraform environment.
      1. `terraform init`
   2. Validate your Terraform configuration.
      1. `terraform validate`
   3. Plan your deployment with Terraform.
      1. `terraform plan --out=tfplan`
   4. Execute the Terraform plan created in the previous step.
      1. `terraform apply tfplan`
      2. NOTE: This step may take a 15-30 minutes to properly provision all resources. This is normal.
5. Generate VPN profiles.
   1. Run `make create-vpn-profiles` to create OpenVPN profiles to connect to the infrastructure you just deployed.
   2. The final profiles will be stored as `.zip` files in the `publish/` subdirectory.

6. All systems have been deployed! They still have to be configured, however time to celebrate! :pizza: :confetti_ball: :fireworks:

## Configure Infrastructure with Ansible

For this round, all post deployment configuration was performed via Ansible.

1. Use one of the VPN profiles generated in the previous step. Any profile will work.
2. Use the Terraform Makefile to configure the Ansible deployment. The following command copies the SSH key to the Ansible setup directory with the proper permissions.
   1. `make ansible-setup`
3. Change to Ansible base directory.
   1. `cd ../../ansible`
4. Run Ansible configuration.
   1. `make configure-range-from-scratch`
5. Wait. Ensure your provisioning host will not sleep or disconnect from the VPN during the provisioning step. Depending on bandwidth, the deployment could take 1-3 hours to fully configure all infrastructure.

## Deploy Attack Platform Software

1. Download the [ATTACK Evaluations Library](https://github.com/attackevals/ael) to the `kraken` and `homelander` (Kali Linux and Windows Jumpbox hosts). Follow the instructions to setup the red team tooling.
