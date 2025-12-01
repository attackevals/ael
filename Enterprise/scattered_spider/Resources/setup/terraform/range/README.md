---
tags: [infrastructure, enterprise2025, aws, ec2, terraform, range, stage3]
label: Evaluation Environment
order: 1
---

# `range`

# ATT&CK Evaluations Enterprise 2025

Terraform deployment 3 of 3. Please see the main [Terraform Deployment](../README.md) page for more information.

Before proceeding with this stage, please review `deploy.auto.tfvars` and ensure these variables are set:

* `aws_trusted_account_id` — the 12-digit ID of the AWS account where on-prem scenarios will be emulated (`default`).
* `win_srv_admin_username` — desired domain admin username to be applied to hosts.
* `win_srv_admin_password` — desired domain admin password to be applied to hosts.

<!-- BEGIN_TF_DOCS -->
## Requirements

| Name | Version |
|------|---------|
| <a name="requirement_terraform"></a> [terraform](#requirement\_terraform) | >=1.1.0 |
| <a name="requirement_ansible"></a> [ansible](#requirement\_ansible) | ~> 1.3.0 |
| <a name="requirement_aws"></a> [aws](#requirement\_aws) | ~> 5.97.0 |

## Providers

| Name | Version |
|------|---------|
| <a name="provider_ansible"></a> [ansible](#provider\_ansible) | 1.3.0 |
| <a name="provider_aws"></a> [aws](#provider\_aws) | 5.97.0 |
| <a name="provider_aws.cloud-detections"></a> [aws.cloud-detections](#provider\_aws.cloud-detections) | 5.97.0 |
| <a name="provider_aws.cloud-protections"></a> [aws.cloud-protections](#provider\_aws.cloud-protections) | 5.97.0 |
| <a name="provider_aws.default"></a> [aws.default](#provider\_aws.default) | 5.97.0 |
| <a name="provider_terraform"></a> [terraform](#provider\_terraform) | n/a |
| <a name="provider_time"></a> [time](#provider\_time) | 0.13.1 |

## Modules

| Name | Source | Version |
|------|--------|---------|
| <a name="module_amis"></a> [amis](#module\_amis) | ../modules/aws/amis | n/a |
| <a name="module_amis-cloud"></a> [amis-cloud](#module\_amis-cloud) | ../modules/aws/amis | n/a |
| <a name="module_b3-access-srv1"></a> [b3-access-srv1](#module\_b3-access-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_b3-addc-srv1"></a> [b3-addc-srv1](#module\_b3-addc-srv1) | ../modules/aws/base-vm-win | n/a |
| <a name="module_b3-dmz-guacamole-srv1"></a> [b3-dmz-guacamole-srv1](#module\_b3-dmz-guacamole-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_b3-dmz-mfa-srv1"></a> [b3-dmz-mfa-srv1](#module\_b3-dmz-mfa-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_b3-file-srv1"></a> [b3-file-srv1](#module\_b3-file-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_b3-file-srv2"></a> [b3-file-srv2](#module\_b3-file-srv2) | ../modules/aws/base-vm | n/a |
| <a name="module_b3-mail-srv1"></a> [b3-mail-srv1](#module\_b3-mail-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_b3-mdm-srv1"></a> [b3-mdm-srv1](#module\_b3-mdm-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_b3-win-jumpbox1"></a> [b3-win-jumpbox1](#module\_b3-win-jumpbox1) | ../modules/aws/base-vm | n/a |
| <a name="module_b3-win11-desk1"></a> [b3-win11-desk1](#module\_b3-win11-desk1) | ../modules/aws/base-vm | n/a |
| <a name="module_b3-win11-desk2"></a> [b3-win11-desk2](#module\_b3-win11-desk2) | ../modules/aws/base-vm | n/a |
| <a name="module_b3-win11-desk3"></a> [b3-win11-desk3](#module\_b3-win11-desk3) | ../modules/aws/base-vm | n/a |
| <a name="module_b3-win11-desk4"></a> [b3-win11-desk4](#module\_b3-win11-desk4) | ../modules/aws/base-vm | n/a |
| <a name="module_choco-srv1"></a> [choco-srv1](#module\_choco-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_cloud-airbyte-srv1"></a> [cloud-airbyte-srv1](#module\_cloud-airbyte-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_cloud-defaults"></a> [cloud-defaults](#module\_cloud-defaults) | ../modules/aws/defaults | n/a |
| <a name="module_cloud-gitlab-srv1"></a> [cloud-gitlab-srv1](#module\_cloud-gitlab-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_cloud-wekan-srv1"></a> [cloud-wekan-srv1](#module\_cloud-wekan-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_defaults"></a> [defaults](#module\_defaults) | ../modules/aws/defaults | n/a |
| <a name="module_dmz-guacamole-srv1"></a> [dmz-guacamole-srv1](#module\_dmz-guacamole-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_dns-srv1"></a> [dns-srv1](#module\_dns-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_idp-srv1"></a> [idp-srv1](#module\_idp-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_on-prem-access-srv1"></a> [on-prem-access-srv1](#module\_on-prem-access-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_on-prem-addc-srv1"></a> [on-prem-addc-srv1](#module\_on-prem-addc-srv1) | ../modules/aws/base-vm-win | n/a |
| <a name="module_on-prem-file-srv1"></a> [on-prem-file-srv1](#module\_on-prem-file-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_on-prem-file-srv2"></a> [on-prem-file-srv2](#module\_on-prem-file-srv2) | ../modules/aws/base-vm | n/a |
| <a name="module_on-prem-mail-srv1"></a> [on-prem-mail-srv1](#module\_on-prem-mail-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_on-prem-mfa-srv1"></a> [on-prem-mfa-srv1](#module\_on-prem-mfa-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_on-prem-win11-desk1"></a> [on-prem-win11-desk1](#module\_on-prem-win11-desk1) | ../modules/aws/base-vm | n/a |
| <a name="module_on-prem-win11-desk2"></a> [on-prem-win11-desk2](#module\_on-prem-win11-desk2) | ../modules/aws/base-vm | n/a |
| <a name="module_on-prem-win11-desk3"></a> [on-prem-win11-desk3](#module\_on-prem-win11-desk3) | ../modules/aws/base-vm | n/a |
| <a name="module_on-prem-win11-desk4"></a> [on-prem-win11-desk4](#module\_on-prem-win11-desk4) | ../modules/aws/base-vm | n/a |
| <a name="module_openvpn-client"></a> [openvpn-client](#module\_openvpn-client) | ../modules/aws/vpn-client | n/a |
| <a name="module_postfix-srv1"></a> [postfix-srv1](#module\_postfix-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_postfix-srv2"></a> [postfix-srv2](#module\_postfix-srv2) | ../modules/aws/base-vm | n/a |
| <a name="module_postfix-srv3"></a> [postfix-srv3](#module\_postfix-srv3) | ../modules/aws/base-vm | n/a |
| <a name="module_preflights-box1"></a> [preflights-box1](#module\_preflights-box1) | ../modules/aws/base-vm | n/a |
| <a name="module_red-kali1"></a> [red-kali1](#module\_red-kali1) | ../modules/aws/base-vm | n/a |
| <a name="module_redirect-srv1"></a> [redirect-srv1](#module\_redirect-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_redirect-srv2"></a> [redirect-srv2](#module\_redirect-srv2) | ../modules/aws/base-vm | n/a |
| <a name="module_redirect-srv3"></a> [redirect-srv3](#module\_redirect-srv3) | ../modules/aws/base-vm | n/a |
| <a name="module_redirect-srv4"></a> [redirect-srv4](#module\_redirect-srv4) | ../modules/aws/base-vm | n/a |
| <a name="module_rmm-srv1"></a> [rmm-srv1](#module\_rmm-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_rustdesk-srv1"></a> [rustdesk-srv1](#module\_rustdesk-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_ssm_activation"></a> [ssm\_activation](#module\_ssm\_activation) | ../modules/aws/ssm-activation | n/a |
| <a name="module_unmanaged-win11-desk1"></a> [unmanaged-win11-desk1](#module\_unmanaged-win11-desk1) | ../modules/aws/base-vm | n/a |
| <a name="module_win-jumpbox1"></a> [win-jumpbox1](#module\_win-jumpbox1) | ../modules/aws/base-vm | n/a |
| <a name="module_wstunnel-srv1"></a> [wstunnel-srv1](#module\_wstunnel-srv1) | ../modules/aws/base-vm | n/a |

## Resources

| Name | Type |
|------|------|
| [ansible_group.domain_kingslanding](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.domain_vale](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.linux](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.linux_cloud](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.linux_kali](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.linux_preflights](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.linux_ubuntu](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.red_jumpbox_servers](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.windows](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.windows_desktops](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.windows_domain_controllers](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.windows_file_servers](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.windows_rds_servers](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_group.windows_servers](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/group) | resource |
| [ansible_host.b3-access-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-addc-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-dmz-guacamole-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-dmz-mfa-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-file-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-file-srv2](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-mail-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-mdm-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-win-jumpbox1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-win11-desk1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-win11-desk2](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-win11-desk3](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.b3-win11-desk4](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.choco-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.cloud-airbyte-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.cloud-gitlab-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.cloud-wekan-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.dmz-guacamole-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.dns-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.on-prem-access-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.on-prem-addc-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.on-prem-file-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.on-prem-file-srv2](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.on-prem-mail-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.on-prem-mfa-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.on-prem-win11-desk1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.on-prem-win11-desk2](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.on-prem-win11-desk3](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.on-prem-win11-desk4](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.postfix-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.postfix-srv2](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.postfix-srv3](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.preflights-box1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.red-idp-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.red-kali1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.red-win-jumpbox1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.redirect-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.redirect-srv2](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.redirect-srv3](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.redirect-srv4](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.rmm-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.rustdesk-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.unmanaged-win11-desk1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [ansible_host.wstunnel-srv1](https://registry.terraform.io/providers/ansible/ansible/latest/docs/resources/host) | resource |
| [aws_ebs_volume.on-prem-file-srv2-disks](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ebs_volume) | resource |
| [aws_ec2_managed_prefix_list.attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.attacker-cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.attacker-cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.b3](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.b3-cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.b3-cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.cloud-victim-cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.cloud-victim-cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.detections-cloud-victim-cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.main-victim-cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.neutral](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.neutral-cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.neutral-cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.onprem-cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.onprem-cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.victim-cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.vpn](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_transit_gateway.main](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway) | resource |
| [aws_ec2_transit_gateway_prefix_list_reference.cloud-protections-plr-attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_prefix_list_reference) | resource |
| [aws_ec2_transit_gateway_prefix_list_reference.cloud-protections-plr-neutral](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_prefix_list_reference) | resource |
| [aws_ec2_transit_gateway_prefix_list_reference.cloud-protections-plr-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_prefix_list_reference) | resource |
| [aws_ec2_transit_gateway_prefix_list_reference.cloud-protections-plr-victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_prefix_list_reference) | resource |
| [aws_ec2_transit_gateway_prefix_list_reference.cloud-protections-plr-victim-cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_prefix_list_reference) | resource |
| [aws_ec2_transit_gateway_route.cloud-protections-internet](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_route) | resource |
| [aws_ec2_transit_gateway_route_table.cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_route_table) | resource |
| [aws_ec2_transit_gateway_route_table_association.cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_route_table_association) | resource |
| [aws_ec2_transit_gateway_vpc_attachment.b3-tgwa](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_vpc_attachment) | resource |
| [aws_ec2_transit_gateway_vpc_attachment.cloud-protections-tgwa](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_vpc_attachment) | resource |
| [aws_ec2_transit_gateway_vpc_attachment.cloud-tgwa](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_vpc_attachment) | resource |
| [aws_ec2_transit_gateway_vpc_attachment.neutral-tgwa](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_vpc_attachment) | resource |
| [aws_ec2_transit_gateway_vpc_attachment.victim-tgwa](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_transit_gateway_vpc_attachment) | resource |
| [aws_eip.attacker-nat-eip](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/eip) | resource |
| [aws_eip.b3-nat-eip](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/eip) | resource |
| [aws_eip.cloud-nat-eip](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/eip) | resource |
| [aws_eip.victim-nat-eip](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/eip) | resource |
| [aws_iam_policy.admin_policy-detections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_policy) | resource |
| [aws_iam_policy.admin_policy-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_policy) | resource |
| [aws_iam_policy.vendor_iam_access](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_policy) | resource |
| [aws_iam_policy_attachment.detections_admin_policy_sso](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_policy_attachment) | resource |
| [aws_iam_policy_attachment.detections_vendor_policy_sso](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_policy_attachment) | resource |
| [aws_iam_policy_attachment.protections_admin_policy_sso](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_policy_attachment) | resource |
| [aws_iam_policy_attachment.protections_vendor_policy_sso](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_policy_attachment) | resource |
| [aws_iam_role.cloudwatch-detections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role) | resource |
| [aws_iam_role.cloudwatch-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role) | resource |
| [aws_iam_role.detections_authentik_admin_role](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role) | resource |
| [aws_iam_role.detections_authentik_vendor_role](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role) | resource |
| [aws_iam_role.detections_cross_account_role](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role) | resource |
| [aws_iam_role.protections_authentik_admin_role](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role) | resource |
| [aws_iam_role.protections_authentik_vendor_role](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role) | resource |
| [aws_iam_role.protections_cross_account_role](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role) | resource |
| [aws_iam_role_policy.detections_snapshot_management](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role_policy) | resource |
| [aws_iam_role_policy.protections_snapshot_management](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role_policy) | resource |
| [aws_iam_role_policy_attachment.ssmic-detections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role_policy_attachment) | resource |
| [aws_iam_role_policy_attachment.ssmic-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role_policy_attachment) | resource |
| [aws_iam_role_policy_attachment.ssmpatch-detections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role_policy_attachment) | resource |
| [aws_iam_role_policy_attachment.ssmpatch-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role_policy_attachment) | resource |
| [aws_iam_role_policy_attachment.ssmserver-detections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role_policy_attachment) | resource |
| [aws_iam_role_policy_attachment.ssmserver-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/iam_role_policy_attachment) | resource |
| [aws_internet_gateway.attacker-igw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/internet_gateway) | resource |
| [aws_internet_gateway.b3-igw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/internet_gateway) | resource |
| [aws_internet_gateway.cloud-igw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/internet_gateway) | resource |
| [aws_internet_gateway.victim-igw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/internet_gateway) | resource |
| [aws_key_pair.cloud-ssh-key-pair](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/key_pair) | resource |
| [aws_key_pair.ssh-key-pair](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/key_pair) | resource |
| [aws_key_pair.ssh-key-pair-vendor](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/key_pair) | resource |
| [aws_nat_gateway.attacker-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/nat_gateway) | resource |
| [aws_nat_gateway.b3-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/nat_gateway) | resource |
| [aws_nat_gateway.cloud-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/nat_gateway) | resource |
| [aws_nat_gateway.victim-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/nat_gateway) | resource |
| [aws_network_interface.redirect1-b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/network_interface) | resource |
| [aws_network_interface.redirect2-b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/network_interface) | resource |
| [aws_network_interface.redirect3-b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/network_interface) | resource |
| [aws_network_interface.redirect4-b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/network_interface) | resource |
| [aws_ram_principal_association.tgw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ram_principal_association) | resource |
| [aws_ram_principal_association.tgw-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ram_principal_association) | resource |
| [aws_ram_resource_association.tgw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ram_resource_association) | resource |
| [aws_ram_resource_share.tgw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ram_resource_share) | resource |
| [aws_ram_resource_share_accepter.tgw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ram_resource_share_accepter) | resource |
| [aws_ram_resource_share_accepter.tgw-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ram_resource_share_accepter) | resource |
| [aws_route_table.attacker-rtb](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table.attacker-rtb-public](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table.b3-rtb](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table.b3-rtb-public](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table.cloud-rtb](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table.cloud-rtb-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table.cloud-rtb-public](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table.neutral-rtb](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table.victim-rtb](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table.victim-rtb-public](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table_association.attacker-nat-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.attacker-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.b3-desk-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.b3-dmz-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.b3-nat-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.b3-srv-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.b3-tgw-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.cloud-nat-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.cloud-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.cloud-rtba-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.cloud-tgw-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.cloud-tgw-rtba-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.neutral-redirect1](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.neutral-redirect2](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.neutral-redirect3](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.neutral-redirect4](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.neutral-support](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.neutral-tgw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.on-prem-dmz-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.on-prem-tgw-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.on-prem-victim-desk-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.on-prem-victim-nat-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.on-prem-victim-srv-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.on-prem-vpn-rtba](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_secretsmanager_secret.gitlab_pat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/secretsmanager_secret) | resource |
| [aws_secretsmanager_secret_version.gitlab_pat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/secretsmanager_secret_version) | resource |
| [aws_security_group.allow-outbound-attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.allow-outbound-b3](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.allow-outbound-victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.b3](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.b3-dmz](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.block-outbound-attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.block-outbound-b3](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.block-outbound-victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.victim-cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.victim-cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.victim-dmz](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.victim-on-prem](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group_rule.outbound-allow-attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group_rule) | resource |
| [aws_security_group_rule.outbound-allow-b3](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group_rule) | resource |
| [aws_security_group_rule.outbound-allow-victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group_rule) | resource |
| [aws_security_group_rule.outbound-lockdown-attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group_rule) | resource |
| [aws_security_group_rule.outbound-lockdown-b3](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group_rule) | resource |
| [aws_security_group_rule.outbound-lockdown-victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group_rule) | resource |
| [aws_ssm_association.inventory](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ssm_association) | resource |
| [aws_ssm_parameter.activation_code](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ssm_parameter) | resource |
| [aws_ssm_parameter.activation_id](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ssm_parameter) | resource |
| [aws_subnet.attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.attacker-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.b3-desktops](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.b3-dmz](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.b3-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.b3-servers](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.b3-tgw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.cloud-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.cloud-tgw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.cloud-tgw-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.neutral-redirect1](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.neutral-redirect2](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.neutral-redirect3](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.neutral-redirect4](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.neutral-support](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.neutral-tgw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.victim-desktops](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.victim-dmz](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.victim-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.victim-servers](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.victim-tgw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.vpn](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_volume_attachment.on-prem-file-srv2-disks_attachment](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/volume_attachment) | resource |
| [aws_vpc.attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc) | resource |
| [aws_vpc.b3](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc) | resource |
| [aws_vpc.cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc) | resource |
| [aws_vpc.cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc) | resource |
| [aws_vpc.victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc) | resource |
| [aws_vpc_dhcp_options.dns_resolver](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_dhcp_options) | resource |
| [aws_vpc_dhcp_options.dns_resolver_cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_dhcp_options) | resource |
| [aws_vpc_dhcp_options_association.dns_resolver_cloud](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_dhcp_options_association) | resource |
| [aws_vpc_dhcp_options_association.dns_resolver_red](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_dhcp_options_association) | resource |
| [aws_vpc_dhcp_options_association.dns_resolver_victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_dhcp_options_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.attacker-redirect-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.b3-nat-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.b3-tgw-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.nat-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.neutral_cidrs](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.tgw-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.vpn-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_security_group_egress_rule.allow_all_outbound](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_egress_rule) | resource |
| [aws_vpc_security_group_egress_rule.attacker_egress](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_egress_rule) | resource |
| [aws_vpc_security_group_egress_rule.b3_allow_all_outbound](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_egress_rule) | resource |
| [aws_vpc_security_group_egress_rule.cloud_allow_all_outbound](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_egress_rule) | resource |
| [aws_vpc_security_group_egress_rule.cloud_allow_all_outbound_protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_egress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.attacker_allowed_self_ingress](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.attacker_prefix_list_ingress](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.b3_dmz_ingress](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.b3_prefix_list_ingress](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.b3_self_ingress](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.cloud_allowed_self_ingress](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.cloud_allowed_self_ingress_protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.cloud_prefix_list_ingress](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.cloud_prefix_list_ingress_protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.main](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.prefix_list_ingress](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_vpc_security_group_ingress_rule.self_ingress](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [time_sleep.wait_for_tgw_protections_share](https://registry.terraform.io/providers/hashicorp/time/latest/docs/resources/sleep) | resource |
| [time_sleep.wait_for_tgw_share](https://registry.terraform.io/providers/hashicorp/time/latest/docs/resources/sleep) | resource |
| [aws_caller_identity.cloud-detections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/data-sources/caller_identity) | data source |
| [aws_caller_identity.cloud-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/data-sources/caller_identity) | data source |
| [aws_caller_identity.current](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/data-sources/caller_identity) | data source |
| [aws_ec2_managed_prefix_list.outbound](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/data-sources/ec2_managed_prefix_list) | data source |
| [aws_iam_policy_document.detections_snapshot_management](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/data-sources/iam_policy_document) | data source |
| [aws_iam_policy_document.protections_snapshot_management](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/data-sources/iam_policy_document) | data source |
| [terraform_remote_state.orgs](https://registry.terraform.io/providers/hashicorp/terraform/latest/docs/data-sources/remote_state) | data source |

## Inputs

| Name | Description | Type | Default | Required |
|------|-------------|------|---------|:--------:|
| <a name="input_aws-region"></a> [aws-region](#input\_aws-region) | AWS region to use, (default: us-east-1) | `string` | `"us-east-1"` | no |
| <a name="input_aws-region-az"></a> [aws-region-az](#input\_aws-region-az) | AWS availability zone to use, (default: us-east-1a) | `string` | `"us-east-1a"` | no |
| <a name="input_aws-shared-credentials"></a> [aws-shared-credentials](#input\_aws-shared-credentials) | AWS credentials to use with Terraform | `list(string)` | n/a | yes |
| <a name="input_aws_trusted_account_id"></a> [aws\_trusted\_account\_id](#input\_aws\_trusted\_account\_id) | AWS account ID of the account to use to create a trust relationship | `string` | n/a | yes |
| <a name="input_category"></a> [category](#input\_category) | Category of resource group, (similar to name, used for ansible automation) | `string` | n/a | yes |
| <a name="input_charge-code"></a> [charge-code](#input\_charge-code) | String charge code for ER7 resources | `string` | n/a | yes |
| <a name="input_environment"></a> [environment](#input\_environment) | (Optional) Environment tag of resource group | `string` | `"Development"` | no |
| <a name="input_name-prefix"></a> [name-prefix](#input\_name-prefix) | String prefix for resource names | `string` | n/a | yes |
| <a name="input_round-name"></a> [round-name](#input\_round-name) | Uppercase description of the resource group, e.g., MSR3, ER7 | `string` | n/a | yes |
| <a name="input_shutdown_ok"></a> [shutdown\_ok](#input\_shutdown\_ok) | Whether the resource can be safely disabled or shutdown | `string` | `"vendor-schedule"` | no |
| <a name="input_ssh_private_key_path"></a> [ssh\_private\_key\_path](#input\_ssh\_private\_key\_path) | Path to SSH private key to use for Linux ssh systems (public and private key must be matching pair) | `string` | n/a | yes |
| <a name="input_ssh_private_key_path_vendor"></a> [ssh\_private\_key\_path\_vendor](#input\_ssh\_private\_key\_path\_vendor) | Path to SSH private key to use for preflights Linux box (public and private key must be matching pair) | `string` | n/a | yes |
| <a name="input_ssh_public_key_path"></a> [ssh\_public\_key\_path](#input\_ssh\_public\_key\_path) | Path to SSH public key to use for Linux ssh systems (public and private key must be matching pair) | `string` | n/a | yes |
| <a name="input_ssh_public_key_path_vendor"></a> [ssh\_public\_key\_path\_vendor](#input\_ssh\_public\_key\_path\_vendor) | Path to SSH public key to use for preflights Linux box (public and private key must be matching pair) | `string` | n/a | yes |
| <a name="input_vendor"></a> [vendor](#input\_vendor) | Name of vendor to associate to range | `string` | n/a | yes |
| <a name="input_win_srv_admin_password"></a> [win\_srv\_admin\_password](#input\_win\_srv\_admin\_password) | Default password for Windows Server administrators | `string` | n/a | yes |
| <a name="input_win_srv_admin_username"></a> [win\_srv\_admin\_username](#input\_win\_srv\_admin\_username) | Default username for Windows Server administrators | `string` | n/a | yes |

## Outputs

| Name | Description |
|------|-------------|
| <a name="output_b1_prefix"></a> [b1\_prefix](#output\_b1\_prefix) | n/a |
| <a name="output_cacert"></a> [cacert](#output\_cacert) | output cert and key separately to simplify redirecting to file used to generate vpn profiles |
| <a name="output_cakey"></a> [cakey](#output\_cakey) | n/a |
| <a name="output_detections_role_arn"></a> [detections\_role\_arn](#output\_detections\_role\_arn) | ARN of the created IAM role |
| <a name="output_detections_role_name"></a> [detections\_role\_name](#output\_detections\_role\_name) | Name of the created IAM role |
| <a name="output_endpointid"></a> [endpointid](#output\_endpointid) | n/a |
| <a name="output_gitlab_pat_secret_arn"></a> [gitlab\_pat\_secret\_arn](#output\_gitlab\_pat\_secret\_arn) | ARN of the GitLab PAT secret |
| <a name="output_protections_role_arn"></a> [protections\_role\_arn](#output\_protections\_role\_arn) | ARN of the created IAM role |
| <a name="output_protections_role_name"></a> [protections\_role\_name](#output\_protections\_role\_name) | Name of the created IAM role |
| <a name="output_rdp_data"></a> [rdp\_data](#output\_rdp\_data) | n/a |
| <a name="output_vendor_iam_access_policy_arn"></a> [vendor\_iam\_access\_policy\_arn](#output\_vendor\_iam\_access\_policy\_arn) | ARN of the vendor IAM access policy |
| <a name="output_vpn"></a> [vpn](#output\_vpn) | n/a |
| <a name="output_vpn-assoc"></a> [vpn-assoc](#output\_vpn-assoc) | module issue - if output, terraform will consider this a dependency to include in destroy plan |
<!-- END_TF_DOCS -->
