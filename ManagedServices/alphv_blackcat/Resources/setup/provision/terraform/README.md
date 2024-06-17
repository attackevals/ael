# MSR2 Infrastructure Project

## Managed Services Evaluation Round 2

<!-- BEGINNING OF PRE-COMMIT-TERRAFORM DOCS HOOK -->
## Requirements

| Name | Version |
|------|---------|
| <a name="requirement_terraform"></a> [terraform](#requirement\_terraform) | >=1.1.0 |
| <a name="requirement_aws"></a> [aws](#requirement\_aws) | ~> 5.29.0 |

## Providers

| Name | Version |
|------|---------|
| <a name="provider_aws"></a> [aws](#provider\_aws) | 5.29.0 |

## Modules

| Name | Source | Version |
|------|--------|---------|
| <a name="module_a-ad-srv1"></a> [a-ad-srv1](#module\_a-ad-srv1) | ../../modules/aws/base-vm-win | n/a |
| <a name="module_a-desk1"></a> [a-desk1](#module\_a-desk1) | ../../modules/aws/base-vm | n/a |
| <a name="module_a-desk2"></a> [a-desk2](#module\_a-desk2) | ../../modules/aws/base-vm | n/a |
| <a name="module_a-desk3"></a> [a-desk3](#module\_a-desk3) | ../../modules/aws/base-vm | n/a |
| <a name="module_a-ex-srv1"></a> [a-ex-srv1](#module\_a-ex-srv1) | ../../modules/aws/base-vm-win | n/a |
| <a name="module_a-iis-srv1"></a> [a-iis-srv1](#module\_a-iis-srv1) | ../../modules/aws/base-vm-win | n/a |
| <a name="module_b-ad-srv1"></a> [b-ad-srv1](#module\_b-ad-srv1) | ../../modules/aws/base-vm-win | n/a |
| <a name="module_b-desk1"></a> [b-desk1](#module\_b-desk1) | ../../modules/aws/base-vm | n/a |
| <a name="module_b-desk2"></a> [b-desk2](#module\_b-desk2) | ../../modules/aws/base-vm | n/a |
| <a name="module_b-desk3"></a> [b-desk3](#module\_b-desk3) | ../../modules/aws/base-vm | n/a |
| <a name="module_b-ex-srv1"></a> [b-ex-srv1](#module\_b-ex-srv1) | ../../modules/aws/base-vm-win | n/a |
| <a name="module_b-file-srv1"></a> [b-file-srv1](#module\_b-file-srv1) | ../../modules/aws/base-vm-win | n/a |
| <a name="module_b-kvm-srv1"></a> [b-kvm-srv1](#module\_b-kvm-srv1) | ../../modules/aws/base-vm | n/a |
| <a name="module_b-sql-srv1"></a> [b-sql-srv1](#module\_b-sql-srv1) | ../../modules/aws/base-vm-win | n/a |
| <a name="module_c-desk1"></a> [c-desk1](#module\_c-desk1) | ../../modules/aws/base-vm | n/a |
| <a name="module_defaults"></a> [defaults](#module\_defaults) | ../../modules/aws/defaults | n/a |
| <a name="module_openvpn-client"></a> [openvpn-client](#module\_openvpn-client) | ../../modules/aws/vpn-client | n/a |
| <a name="module_red-kali1"></a> [red-kali1](#module\_red-kali1) | ../../modules/aws/base-vm | n/a |
| <a name="module_support-dns-srv1"></a> [support-dns-srv1](#module\_support-dns-srv1) | ../../modules/aws/base-vm | n/a |
| <a name="module_support-jumpbox-srv1"></a> [support-jumpbox-srv1](#module\_support-jumpbox-srv1) | ../../modules/aws/base-vm-win | n/a |
| <a name="module_support-pf-srv1"></a> [support-pf-srv1](#module\_support-pf-srv1) | ../../modules/aws/base-vm | n/a |
| <a name="module_support-redirect-srv1"></a> [support-redirect-srv1](#module\_support-redirect-srv1) | ../../modules/aws/base-vm | n/a |
| <a name="module_support-redirect-srv2"></a> [support-redirect-srv2](#module\_support-redirect-srv2) | ../../modules/aws/base-vm | n/a |
| <a name="module_support-redirect-srv3"></a> [support-redirect-srv3](#module\_support-redirect-srv3) | ../../modules/aws/base-vm | n/a |

## Resources

| Name | Type |
|------|------|
| [aws_default_route_table.alpha-rtb](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/default_route_table) | resource |
| [aws_default_route_table.beta-rtb](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/default_route_table) | resource |
| [aws_dynamodb_table.dynamodb-terraform-state-lock](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/dynamodb_table) | resource |
| [aws_eip.alpha-nat-eip](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/eip) | resource |
| [aws_eip.beta-nat-eip](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/eip) | resource |
| [aws_internet_gateway.alpha-igw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/internet_gateway) | resource |
| [aws_internet_gateway.beta-igw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/internet_gateway) | resource |
| [aws_key_pair.ssh-key-pair](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/key_pair) | resource |
| [aws_nat_gateway.alpha-nat-gw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/nat_gateway) | resource |
| [aws_nat_gateway.beta-nat-gw](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/nat_gateway) | resource |
| [aws_route_table.alpha-public-rtb](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table.beta-public-rtb](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table) | resource |
| [aws_route_table_association.rtba_a_desk](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba_a_nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba_a_srv](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba_a_vpn](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba_b_combined](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba_c_desk](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba_red](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba_redirect_a](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba_redirect_b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_route_table_association.rtba_support_nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/route_table_association) | resource |
| [aws_s3_object.msr2](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/s3_object) | resource |
| [aws_security_group.alpha](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_security_group.beta](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/security_group) | resource |
| [aws_subnet.a_desk](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.a_srv](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.a_vpn](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.alpha_nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.b_combined](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.beta_nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.c_desk](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.support_a](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.support_b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_subnet.support_red](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet) | resource |
| [aws_vpc.alpha](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc) | resource |
| [aws_vpc.beta](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc) | resource |
| [aws_vpc_ipv4_cidr_block_association.alpha-a](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.alpha-b](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.alpha-c](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.alpha-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.beta-nat](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.redirect-a-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_ipv4_cidr_block_association.redirect-b-cidr](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_ipv4_cidr_block_association) | resource |
| [aws_vpc_peering_connection.range-to-red-pcx](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_peering_connection) | resource |
| [aws_vpc_security_group_ingress_rule.main](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/vpc_security_group_ingress_rule) | resource |
| [aws_s3_bucket.vendors](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/data-sources/s3_bucket) | data source |

## Inputs

| Name | Description | Type | Default | Required |
|------|-------------|------|---------|:--------:|
| <a name="input_aws-bucket-name"></a> [aws-bucket-name](#input\_aws-bucket-name) | Name of S3 bucket for managing Terraform state across vendor ranges | `string` | `"msr2-vendor-resources"` | no |
| <a name="input_aws-region"></a> [aws-region](#input\_aws-region) | AWS region to use, defaults to us-east-1 | `string` | `"us-east-1"` | no |
| <a name="input_aws-region-az"></a> [aws-region-az](#input\_aws-region-az) | AWS availability zone to use, defaults to us-east-1a | `string` | `"us-east-1a"` | no |
| <a name="input_category"></a> [category](#input\_category) | (Optional) Category of resource group, (similar to name, used for ansible automation) | `string` | `""` | no |
| <a name="input_description"></a> [description](#input\_description) | Description for the resource group | `string` | `"MSR2"` | no |
| <a name="input_dev_linux_password"></a> [dev\_linux\_password](#input\_dev\_linux\_password) | Password for dev Linux hosts | `string` | n/a | yes |
| <a name="input_dev_linux_username"></a> [dev\_linux\_username](#input\_dev\_linux\_username) | Username for Linux hosts | `string` | n/a | yes |
| <a name="input_dev_win_admin_password"></a> [dev\_win\_admin\_password](#input\_dev\_win\_admin\_password) | Password for Windows 10 desktop dev boxes | `string` | n/a | yes |
| <a name="input_dev_win_admin_username"></a> [dev\_win\_admin\_username](#input\_dev\_win\_admin\_username) | Username for Windows 10 desktop dev boxes | `string` | `"devadmin"` | no |
| <a name="input_environment"></a> [environment](#input\_environment) | (Optional) Environment tag of resource group | `string` | `"Development"` | no |
| <a name="input_local_domain_admin_password"></a> [local\_domain\_admin\_password](#input\_local\_domain\_admin\_password) | Local admin password to workstation 1 | `string` | `"localAdmin1"` | no |
| <a name="input_local_domain_admin_username"></a> [local\_domain\_admin\_username](#input\_local\_domain\_admin\_username) | Local admin password to workstation 1 | `string` | `"localadmin"` | no |
| <a name="input_name-prefix"></a> [name-prefix](#input\_name-prefix) | String prefix for resource names | `string` | `""` | no |
| <a name="input_shutdown_ok"></a> [shutdown\_ok](#input\_shutdown\_ok) | Whether the resource can be safely disabled or shutdown | `string` | `"vendor-schedule"` | no |
| <a name="input_ssh_private_key_path"></a> [ssh\_private\_key\_path](#input\_ssh\_private\_key\_path) | Path to SSH private key to use for Linux ssh systems (public and private key must be matching pair) | `string` | n/a | yes |
| <a name="input_ssh_public_key_path"></a> [ssh\_public\_key\_path](#input\_ssh\_public\_key\_path) | Path to SSH public key to use for Linux ssh systems (public and private key must be matching pair) | `string` | n/a | yes |
| <a name="input_win_a_domain_name"></a> [win\_a\_domain\_name](#input\_win\_a\_domain\_name) | Domain name for A | `string` | n/a | yes |
| <a name="input_win_a_netbios_name"></a> [win\_a\_netbios\_name](#input\_win\_a\_netbios\_name) | Netbios name for A | `string` | n/a | yes |
| <a name="input_win_b_domain_name"></a> [win\_b\_domain\_name](#input\_win\_b\_domain\_name) | Domain name for B | `string` | n/a | yes |
| <a name="input_win_b_netbios_name"></a> [win\_b\_netbios\_name](#input\_win\_b\_netbios\_name) | Netbios name for B | `string` | n/a | yes |
| <a name="input_win_c_domain_name"></a> [win\_c\_domain\_name](#input\_win\_c\_domain\_name) | Domain name for C | `string` | n/a | yes |
| <a name="input_win_c_netbios_name"></a> [win\_c\_netbios\_name](#input\_win\_c\_netbios\_name) | Netbios name for C | `string` | n/a | yes |
| <a name="input_win_desk_password"></a> [win\_desk\_password](#input\_win\_desk\_password) | Local admin password to workstation 1 | `string` | `"winDesk1"` | no |
| <a name="input_win_desk_username"></a> [win\_desk\_username](#input\_win\_desk\_username) | Local admin password to workstation 1 | `string` | `"windesk"` | no |
| <a name="input_win_domain_name"></a> [win\_domain\_name](#input\_win\_domain\_name) | Domain name for AD | `string` | n/a | yes |
| <a name="input_win_netbios_name"></a> [win\_netbios\_name](#input\_win\_netbios\_name) | Netbios name for AD, e.g. reddev | `string` | n/a | yes |
| <a name="input_win_srv_admin_password"></a> [win\_srv\_admin\_password](#input\_win\_srv\_admin\_password) | Windows Server admin password | `string` | n/a | yes |
| <a name="input_win_srv_admin_username"></a> [win\_srv\_admin\_username](#input\_win\_srv\_admin\_username) | Windows Server admin username | `string` | n/a | yes |

## Outputs

| Name | Description |
|------|-------------|
| <a name="output_cacert"></a> [cacert](#output\_cacert) | output cert and key separately to simplify redirecting to file used to generate vpn profiles |
| <a name="output_cakey"></a> [cakey](#output\_cakey) | n/a |
| <a name="output_endpointid"></a> [endpointid](#output\_endpointid) | n/a |
| <a name="output_rdp_data"></a> [rdp\_data](#output\_rdp\_data) | n/a |
| <a name="output_vpn"></a> [vpn](#output\_vpn) | n/a |
| <a name="output_windows-credentials"></a> [windows-credentials](#output\_windows-credentials) | n/a |
<!-- END OF PRE-COMMIT-TERRAFORM DOCS HOOK -->
