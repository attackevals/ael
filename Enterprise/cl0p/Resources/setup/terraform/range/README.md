# ER6 Infrastructure Project

## Evaluations Enterprise Round 6

<!-- BEGINNING OF PRE-COMMIT-TERRAFORM DOCS HOOK -->
## Requirements

| Name | Version |
|------|---------|
| <a name="requirement_terraform"></a> [terraform](#requirement\_terraform) | >=1.1.0 |
| <a name="requirement_aws"></a> [aws](#requirement\_aws) | ~> 5.29.0 |

## Providers

| Name | Version |
|------|---------|
| <a name="provider_aws"></a> [aws](#provider\_aws) | ~> 5.29.0 |

## Modules

| Name | Source | Version |
|------|--------|---------|
| <a name="module_a-addc-srv1"></a> [a-addc-srv1](#module\_a-addc-srv1) | ../modules/aws/base-vm-win | n/a |
| <a name="module_a-file-srv1"></a> [a-file-srv1](#module\_a-file-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_a-macos-desk1"></a> [a-macos-desk1](#module\_a-macos-desk1) | ../modules/aws/base-vm-macos | n/a |
| <a name="module_a-mail-srv1"></a> [a-mail-srv1](#module\_a-mail-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_a-win11-desk1"></a> [a-win11-desk1](#module\_a-win11-desk1) | ../modules/aws/base-vm | n/a |
| <a name="module_a-win11-desk2"></a> [a-win11-desk2](#module\_a-win11-desk2) | ../modules/aws/base-vm | n/a |
| <a name="module_b-addc-srv1"></a> [b-addc-srv1](#module\_b-addc-srv1) | ../modules/aws/base-vm-win | n/a |
| <a name="module_b-file-srv1"></a> [b-file-srv1](#module\_b-file-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_b-file-srv1-add-volume"></a> [b-file-srv1-add-volume](#module\_b-file-srv1-add-volume) | ../modules/aws/add-volume | n/a |
| <a name="module_b-kvm-srv1"></a> [b-kvm-srv1](#module\_b-kvm-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_b-mail-srv1"></a> [b-mail-srv1](#module\_b-mail-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_b-win11-desk1"></a> [b-win11-desk1](#module\_b-win11-desk1) | ../modules/aws/base-vm | n/a |
| <a name="module_b-win11-desk2"></a> [b-win11-desk2](#module\_b-win11-desk2) | ../modules/aws/base-vm | n/a |
| <a name="module_choco-mirror-srv1"></a> [choco-mirror-srv1](#module\_choco-mirror-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_defaults"></a> [defaults](#module\_defaults) | ../../modules/aws/defaults | n/a |
| <a name="module_openvpn-client"></a> [openvpn-client](#module\_openvpn-client) | ../../modules/aws/vpn-client | n/a |
| <a name="module_protections-addc-srv1"></a> [protections-addc-srv1](#module\_protections-addc-srv1) | ../modules/aws/base-vm-win | n/a |
| <a name="module_protections-file-srv1"></a> [protections-file-srv1](#module\_protections-file-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_protections-file-srv1-add-volume"></a> [protections-file-srv1-add-volume](#module\_protections-file-srv1-add-volume) | ../modules/aws/add-volume | n/a |
| <a name="module_protections-linux-srv1"></a> [protections-linux-srv1](#module\_protections-linux-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_protections-macos-desk1"></a> [protections-macos-desk1](#module\_protections-macos-desk1) | ../modules/aws/base-vm-macos | n/a |
| <a name="module_protections-mail-srv1"></a> [protections-mail-srv1](#module\_protections-mail-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_protections-win11-desk1"></a> [protections-win11-desk1](#module\_protections-win11-desk1) | ../modules/aws/base-vm | n/a |
| <a name="module_protections-win11-desk2"></a> [protections-win11-desk2](#module\_protections-win11-desk2) | ../modules/aws/base-vm | n/a |
| <a name="module_red-kali1"></a> [red-kali1](#module\_red-kali1) | ../modules/aws/base-vm | n/a |
| <a name="module_redirect-srv1"></a> [redirect-srv1](#module\_redirect-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_redirect-srv2"></a> [redirect-srv2](#module\_redirect-srv2) | ../modules/aws/base-vm | n/a |
| <a name="module_redirect-srv3"></a> [redirect-srv3](#module\_redirect-srv3) | ../modules/aws/base-vm | n/a |
| <a name="module_redirect-srv4"></a> [redirect-srv4](#module\_redirect-srv4) | ../modules/aws/base-vm | n/a |
| <a name="module_remote-worker-srv1"></a> [remote-worker-srv1](#module\_remote-worker-srv1) | ../modules/aws/base-vm-win | n/a |
| <a name="module_remote-worker-srv2"></a> [remote-worker-srv2](#module\_remote-worker-srv2) | ../modules/aws/base-vm-win | n/a |
| <a name="module_support-dns-srv1"></a> [support-dns-srv1](#module\_support-dns-srv1) | ../modules/aws/base-vm | n/a |
| <a name="module_win-jumpbox1"></a> [win-jumpbox1](#module\_win-jumpbox1) | ../modules/aws/base-vm | n/a |

## Resources

| Name | Type |
|------|------|
| [aws_default_route_table.attacker-rtb](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/default_route_table) | resource |
| [aws_default_route_table.victim-rtb](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/default_route_table) | resource |
| [aws_dynamodb_table.dynamodb-terraform-state-lock](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/dynamodb_table) | resource |
| [aws_ec2_managed_prefix_list.attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.ext-benev](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_ec2_managed_prefix_list.victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/ec2_managed_prefix_list) | resource |
| [aws_eip.attacker-nat-eip](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/eip) | resource |
| [aws_eip.victim-nat-eip](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/eip) | resource |
| [aws_internet_gateway.attacker-igw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/internet_gateway) | resource |
| [aws_internet_gateway.victim-igw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/internet_gateway) | resource |
| [aws_key_pair.ssh-key-pair](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/key_pair) | resource |
| [aws_nat_gateway.attacker-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/nat_gateway) | resource |
| [aws_nat_gateway.victim-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/nat_gateway) | resource |
| [aws_network_interface.redirect1-b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/network_interface) | resource |
| [aws_network_interface.redirect2-b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/network_interface) | resource |
| [aws_network_interface.redirect3-b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/network_interface) | resource |
| [aws_network_interface.redirect4-b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/network_interface) | resource |
| [aws_network_interface.redirect4-c](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/network_interface) | resource |
| [aws_route_table.attacker-rtb-public](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table.victim-rtb-public](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table_association.rtba-attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba-attacker-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba-victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba-victim-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba-vpn](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_s3_object.enterpriseround6](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/s3_object) | resource |
| [aws_security_group.allow-outbound-attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.allow-outbound-victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.block-outbound-attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.block-outbound-victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.victim-a](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.victim-b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.victim-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group_rule.outbound-allow-attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group_rule) | resource |
| [aws_security_group_rule.outbound-allow-victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group_rule) | resource |
| [aws_security_group_rule.outbound-lockdown-attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group_rule) | resource |
| [aws_security_group_rule.outbound-lockdown-victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group_rule) | resource |
| [aws_subnet.attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.attacker-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.ext-benev-redirect1](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.ext-benev-redirect2](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.ext-benev-redirect3](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.ext-benev-redirect4](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.victim-a](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.victim-b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.victim-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.victim-protections](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.vpn](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_vpc.attacker](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc) | resource |
| [aws_vpc.victim](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc) | resource |
| [aws_vpc_ipv4_cidr_block_association.attacker-redirect-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.b_cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.ext-benev-redirect1-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.ext-benev-redirect2-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.ext-benev-redirect3-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.ext-benev-redirect4-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.nat_cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.protections_cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.vpn_cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_peering_connection.red2victim-pcx](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_peering_connection) | resource |
| [aws_vpc_security_group_ingress_rule.main](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_ec2_managed_prefix_list.outbound](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/data-sources/ec2_managed_prefix_list) | data source |
| [aws_s3_bucket.vendors](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/data-sources/s3_bucket) | data source |

## Inputs

| Name | Description | Type | Default | Required |
|------|-------------|------|---------|:--------:|
| <a name="input_aws-bucket-name"></a> [aws-bucket-name](#input\_aws-bucket-name) | Name of S3 bucket for managing Terraform state across vendor ranges | `string` | n/a | yes |
| <a name="input_aws-region"></a> [aws-region](#input\_aws-region) | AWS region to use, (default: us-east-1) | `string` | `"us-east-1"` | no |
| <a name="input_aws-region-az"></a> [aws-region-az](#input\_aws-region-az) | AWS availability zone to use, (default: us-east-1a) | `string` | `"us-east-1a"` | no |
| <a name="input_aws-shared-credentials"></a> [aws-shared-credentials](#input\_aws-shared-credentials) | AWS credentials to use with Terraform | `list(string)` | n/a | yes |
| <a name="input_category"></a> [category](#input\_category) | Category of resource group, (similar to name, used for ansible automation) | `string` | n/a | yes |
| <a name="input_charge_code"></a> [charge\_code](#input\_charge\_code) | String charge code for ER6 resources | `string` | n/a | yes |
| <a name="input_deploy_mac_hosts"></a> [deploy\_mac\_hosts](#input\_deploy\_mac\_hosts) | Should mac hosts be deployed. NOTE: Only the literal lowercase string yes will enable mac deployment. | `string` | `"no"` | no |
| <a name="input_description"></a> [description](#input\_description) | Description for the resource group | `string` | `"ER6"` | no |
| <a name="input_dev_linux_password"></a> [dev\_linux\_password](#input\_dev\_linux\_password) | Default developer password for Linux hosts | `string` | n/a | yes |
| <a name="input_dev_linux_username"></a> [dev\_linux\_username](#input\_dev\_linux\_username) | Default developer username for Linux hosts | `string` | n/a | yes |
| <a name="input_dev_win_admin_password"></a> [dev\_win\_admin\_password](#input\_dev\_win\_admin\_password) | Default password for Windows 11 developer resources | `string` | n/a | yes |
| <a name="input_dev_win_admin_username"></a> [dev\_win\_admin\_username](#input\_dev\_win\_admin\_username) | Default username for Windows 11 developer resources | `string` | n/a | yes |
| <a name="input_environment"></a> [environment](#input\_environment) | (Optional) Environment tag of resource group | `string` | `"Development"` | no |
| <a name="input_local_domain_admin_password"></a> [local\_domain\_admin\_password](#input\_local\_domain\_admin\_password) | Default password for local domain admins | `string` | n/a | yes |
| <a name="input_local_domain_admin_username"></a> [local\_domain\_admin\_username](#input\_local\_domain\_admin\_username) | Default username for local domain admins | `string` | n/a | yes |
| <a name="input_name-prefix"></a> [name-prefix](#input\_name-prefix) | String prefix for resource names | `string` | n/a | yes |
| <a name="input_shutdown_ok"></a> [shutdown\_ok](#input\_shutdown\_ok) | Whether the resource can be safely disabled or shutdown | `string` | `"vendor-schedule"` | no |
| <a name="input_ssh_private_key_path"></a> [ssh\_private\_key\_path](#input\_ssh\_private\_key\_path) | Path to SSH private key to use for Linux ssh systems (public and private key must be matching pair) | `string` | n/a | yes |
| <a name="input_ssh_public_key_path"></a> [ssh\_public\_key\_path](#input\_ssh\_public\_key\_path) | Path to SSH public key to use for Linux ssh systems (public and private key must be matching pair) | `string` | n/a | yes |
| <a name="input_win_desk_password"></a> [win\_desk\_password](#input\_win\_desk\_password) | Default password for Windows 11 Desktops | `string` | n/a | yes |
| <a name="input_win_desk_username"></a> [win\_desk\_username](#input\_win\_desk\_username) | Default username for Windows 11 Desktops | `string` | n/a | yes |
| <a name="input_win_domain_name"></a> [win\_domain\_name](#input\_win\_domain\_name) | Domain name for AD, e.g. reddev.org | `string` | n/a | yes |
| <a name="input_win_netbios_name"></a> [win\_netbios\_name](#input\_win\_netbios\_name) | Netbios name for AD, e.g. reddev | `string` | n/a | yes |
| <a name="input_win_srv_admin_password"></a> [win\_srv\_admin\_password](#input\_win\_srv\_admin\_password) | Default password for Windows Server administrators | `string` | n/a | yes |
| <a name="input_win_srv_admin_username"></a> [win\_srv\_admin\_username](#input\_win\_srv\_admin\_username) | Default username for Windows Server administrators | `string` | n/a | yes |

## Outputs

| Name | Description |
|------|-------------|
| <a name="output_cacert"></a> [cacert](#output\_cacert) | output cert and key separately to simplify redirecting to file used to generate vpn profiles |
| <a name="output_cakey"></a> [cakey](#output\_cakey) | n/a |
| <a name="output_endpointid"></a> [endpointid](#output\_endpointid) | n/a |
| <a name="output_mac-info"></a> [mac-info](#output\_mac-info) | n/a |
| <a name="output_rdp_data"></a> [rdp\_data](#output\_rdp\_data) | n/a |
| <a name="output_vpn"></a> [vpn](#output\_vpn) | n/a |
<!-- END OF PRE-COMMIT-TERRAFORM DOCS HOOK -->
