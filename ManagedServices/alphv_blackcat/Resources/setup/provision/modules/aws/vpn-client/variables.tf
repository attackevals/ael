variable "name" {
  description = "Name prefix for the resources of this stack"
}

variable "cidr" {
  description = "Network CIDR to use for clients"
}

variable "subnet_ids" {
  type        = list(string)
  description = "Subnet ID to associate clients (each subnet passed will create an VPN association - costs involved)"
}

variable "allowed_cidr_ranges" {
  type        = list(string)
  description = "List of CIDR ranges from which access is allowed"
  default     = []
}

variable "allowed_access_groups" {
  type        = list(string)
  description = "List of Access group IDs to allow access. Leave empty to allow all groups"
  default     = []
}

variable "vpc_id" {
  type        = string
  description = "VPC Id to create resources"
}
variable "dns_servers" {
  type        = list(string)
  description = "List of DNS Servers"
  default     = []
}

variable "organization_name" {
  description = "Name of organization to use in private certificate"
}

variable "tags" {
  type        = map(string)
  description = "Tags to attach to resources"
}

variable "logs_retention" {
  description = "Retention in days for CloudWatch Log Group"
  default     = 30 #365
}

variable "authentication_type" {
  description = "The type of client authentication to be used. Specify certificate-authentication to use certificate-based authentication, directory-service-authentication to use Active Directory authentication, or federated-authentication to use Federated Authentication via SAML 2.0."
  default     = "certificate-authentication"
}

#variable "authentication_saml_provider_arn" {
#  description = "(Optional) The ARN of the IAM SAML identity provider if type is federated-authentication."
#  default     = null
#}

variable "split_tunnel" {
  description = "With split_tunnel false, all client traffic will go through the VPN."
  default     = true
}

variable "security_group_id" {
  description = "Optional security group id to use instead of the default created"
  #  default     = ""
}

variable "enable_self_service_portal" {
  type        = bool
  default     = false
  description = "Specify whether to enable the self-service portal for the Client VPN endpoint"
}

variable "target_cidr_blocks" {
  type        = list(string)
  description = "List of target CIDR ranges to route through VPN tunnel"
  default     = null
}

variable "target_subnet_id" {
  type        = string
  description = "Subnet ID to route CIDR traffic to"
  default     = null
}
