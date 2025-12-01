variable "allowed_access_groups" {
  type        = list(string)
  description = "List of Access group IDs to allow access. Leave empty to allow all groups"
  default     = []
}

variable "allowed_cidr_ranges" {
  type        = list(string)
  description = "List of CIDR ranges from which access is allowed"
  default     = []
}

variable "authentication_type" {
  description = "The type of client authentication to be used. Specify certificate-authentication to use certificate-based authentication, directory-service-authentication to use Active Directory authentication, or federated-authentication to use Federated Authentication via SAML 2.0."
  default     = "certificate-authentication"
}

variable "cidr" {
  type        = string
  description = "Network CIDR to use for clients"
}

variable "dns_servers" {
  type        = list(string)
  description = "List of DNS Servers"
  default     = []
}

variable "enable_self_service_portal" {
  type        = bool
  default     = false
  description = "Specify whether to enable the self-service portal for the Client VPN endpoint"
}

variable "lambda_function_arn" {
  type        = string
  description = "AWSClientVPN Lambda function ARN"
  default     = "arn:aws:lambda:us-east-1:756935708346:function:AWSClientVPN-AuthByScope"
}

variable "logs_retention" {
  type        = number
  description = "Retention in days for CloudWatch Log Group"
  default     = 365
}

variable "name" {
  type        = string
  description = "Name prefix for the resources of this stack"
}

variable "organization_name" {
  description = "Name of organization to use in private certificate"
}

variable "security_group_id" {
  description = "Security group id to use instead of the default created"
  default     = null
}

variable "split_tunnel" {
  description = "With split_tunnel false, all client traffic will go through the VPN."
  default     = true
}

variable "subnet_ids" {
  type        = list(string)
  description = "Subnet ID to associate clients (each subnet passed will create an VPN association - costs involved)"
}

variable "tags" {
  type        = map(string)
  description = "Tags to attach to resources"
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

variable "vpc_id" {
  type        = string
  description = "VPC Id to create resources"
}
