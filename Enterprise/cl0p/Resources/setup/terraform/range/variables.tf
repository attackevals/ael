# Terraform variables
variable "name-prefix" {
  type        = string
  description = "String prefix for resource names"
}

variable "aws-bucket-name" {
  type        = string
  description = "Name of S3 bucket for managing Terraform state across vendor ranges"
}

variable "aws-shared-credentials" {
  type        = list(string)
  description = "AWS credentials to use with Terraform"
}

variable "aws-region" {
  type        = string
  description = "AWS region to use, (default: us-east-1)"
  default     = "us-east-1"
}

variable "aws-region-az" {
  type        = string
  description = "AWS availability zone to use, (default: us-east-1a)"
  default     = "us-east-1a"
}

variable "category" {
  type        = string
  description = "Category of resource group, (similar to name, used for ansible automation)"
}

variable "charge_code" {
  type        = string
  description = "String charge code for ER6 resources"
}

variable "description" {
  type        = string
  description = "Description for the resource group"
  default     = "ER6"
}

variable "environment" {
  type        = string
  description = "(Optional) Environment tag of resource group"
  default     = "Development"
  validation {
    condition     = contains(["Production", "Development"], var.environment)
    error_message = "Valid values: Production, Development."
  }
}

variable "shutdown_ok" {
  type        = string
  description = "Whether the resource can be safely disabled or shutdown"
  default     = "vendor-schedule"
}

#####################################################################################
# ad/exchange
#####################################################################################
variable "win_domain_name" {
  type        = string
  description = "Domain name for AD, e.g. reddev.org"
}

variable "win_netbios_name" {
  type        = string
  description = "Netbios name for AD, e.g. reddev"
}

#####################################################################################
# default credentials
#####################################################################################

variable "dev_win_admin_username" {
  type        = string
  description = "Default username for Windows 11 developer resources"
}

variable "dev_win_admin_password" {
  type        = string
  description = "Default password for Windows 11 developer resources"
}

variable "dev_linux_username" {
  type        = string
  description = "Default developer username for Linux hosts"
}

variable "dev_linux_password" {
  type        = string
  description = "Default developer password for Linux hosts"
}

variable "win_srv_admin_username" {
  type        = string
  description = "Default username for Windows Server administrators"
}

variable "win_srv_admin_password" {
  type        = string
  description = "Default password for Windows Server administrators"
}

variable "local_domain_admin_username" {
  type        = string
  description = "Default username for local domain admins"
}

variable "local_domain_admin_password" {
  type        = string
  description = "Default password for local domain admins"
}

variable "win_desk_username" {
  type        = string
  description = "Default username for Windows 11 Desktops"
}

variable "win_desk_password" {
  type        = string
  description = "Default password for Windows 11 Desktops"
}

# SSH keys for linux systems
variable "ssh_public_key_path" {
  type        = string
  description = "Path to SSH public key to use for Linux ssh systems (public and private key must be matching pair)"
}

variable "ssh_private_key_path" {
  type        = string
  description = "Path to SSH private key to use for Linux ssh systems (public and private key must be matching pair)"
}

variable "deploy_mac_hosts" {
  type        = string
  description = "Should mac hosts be deployed. NOTE: Only the literal lowercase string yes will enable mac deployment."
  default     = "no"
}
