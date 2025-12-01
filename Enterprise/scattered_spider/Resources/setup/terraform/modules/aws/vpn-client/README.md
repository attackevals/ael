# `vpn-client`
<!--- Based on https://github.com/DNXLabs/terraform-aws-client-vpn --->

This Terraform module installs a client vpn. The following resources will be created:

- VPN Endpoint - Provides an AWS Client VPN endpoint for OpenVPN clients.
- Provides network associations for AWS Client VPN endpoints
- Generate AWS Certificate Manager(ACM) certificates

<!--- BEGIN_TF_DOCS --->

## Inputs

| Name | Description | Type | Default | Required |
|------|-------------|------|---------|:--------:|
| allowed\_access\_groups | List of Access group IDs to allow access. Leave empty to allow all groups | `list(string)` | `[]` | no |
| allowed\_cidr\_ranges | List of CIDR ranges from which access is allowed | `list(string)` | `[]` | no |
| authentication\_saml\_provider\_arn | (Optional) The ARN of the IAM SAML identity provider if type is federated-authentication. | `any` | `null` | no |
| authentication\_type | The type of client authentication to be used. Specify certificate-authentication to use certificate-based authentication, directory-service-authentication to use Active Directory authentication, or federated-authentication to use Federated Authentication via SAML 2.0. | `string` | `"certificate-authentication"` | no |
| cidr | Network CIDR to use for clients | `any` | n/a | yes |
| dns\_servers | List of DNS Servers | `list(string)` | `[]` | no |
| enable\_self\_service\_portal | Specify whether to enable the self-service portal for the Client VPN endpoint | `bool` | `false` | no |
| logs\_retention | Retention in days for CloudWatch Log Group | `number` | `365` | no |
| name | Name prefix for the resources of this stack | `any` | n/a | yes |
| organization\_name | Name of organization to use in private certificate | `string` | `"ACME, Inc"` | no |
| security\_group\_id | Optional security group id to use instead of the default created | `string` | `""` | no |
| split\_tunnel | With split\_tunnel false, all client traffic will go through the VPN. | `bool` | `true` | no |
| subnet\_ids | Subnet ID to associate clients (each subnet passed will create an VPN association - costs involved) | `list(string)` | n/a | yes |
| tags | Extra tags to attach to resources | `map(string)` | `{}` | no |
| vpc\_id | VPC Id to create resources | `string` | n/a | yes |

## Outputs

| Name | Description |
|------|-------------|
| security\_group\_id | n/a |
| vpn\_ca\_cert | n/a |
| vpn\_ca\_key | n/a |
| vpn\_client\_cert | n/a |
| vpn\_client\_key | n/a |
| vpn\_endpoint\_id | n/a |
| vpn\_server\_cert | n/a |
| vpn\_server\_key | n/a |

<!--- END_TF_DOCS --->
