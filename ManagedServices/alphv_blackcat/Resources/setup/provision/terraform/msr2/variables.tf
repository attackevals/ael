# Terraform variables

variable "name-prefix" {
  type        = string
  description = "String prefix for resource names"
  default     = ""
}

variable "aws-bucket-name" {
  type        = string
  description = "Name of S3 bucket for managing Terraform state across vendor ranges. NOTE: Add to backend.tf!"
}

variable "aws-region" {
  type        = string
  description = "AWS region to use, defaults to us-east-1"
  default     = "us-east-1"
}

variable "aws-region-az" {
  type        = string
  description = "AWS availability zone to use, defaults to us-east-1a"
  default     = "us-east-1a"
}

variable "description" {
  type        = string
  description = "Description for the resource group"
  default     = "MSR2"
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

variable "category" {
  type        = string
  description = "(Optional) Category of resource group, (similar to name, used for ansible automation)"
  default     = ""
}

variable "shutdown_ok" {
  type        = string
  description = "Whether the resource can be safely disabled or shutdown"
  default     = "vendor-schedule"
}

#####################################################################################
# default credentials
#####################################################################################

variable "dev_win_admin_username" {
  type        = string
  description = "Username for Windows 10 desktop dev boxes"
  default     = "devadmin"
}

variable "dev_win_admin_password" {
  type        = string
  description = "Password for Windows 10 desktop dev boxes"
}

variable "dev_linux_username" {
  type        = string
  description = "Username for Linux hosts"
}

variable "dev_linux_password" {
  type        = string
  description = "Password for dev Linux hosts"
}

variable "win_srv_admin_username" {
  type        = string
  description = "Windows Server admin username"
}

variable "win_srv_admin_password" {
  type        = string
  description = "Windows Server admin password"
}

variable "win_domain_name" {
  type        = string
  description = "Domain name for AD"
}

variable "win_netbios_name" {
  type        = string
  description = "Netbios name for AD, e.g. reddev"
}

variable "local_domain_admin_username" {
  type        = string
  description = "Local admin password to workstation 1"
  default     = "localadmin"
}

variable "local_domain_admin_password" {
  type        = string
  description = "Local admin password to workstation 1"
  default     = "localAdmin1"
}

variable "win_desk_username" {
  type        = string
  description = "Local admin password to workstation 1"
  default     = "windesk"
}

variable "win_desk_password" {
  type        = string
  description = "Local admin password to workstation 1"
  default     = "winDesk1"
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

variable "charge_code" {
  type        = string
  description = "Billing reference"
}
