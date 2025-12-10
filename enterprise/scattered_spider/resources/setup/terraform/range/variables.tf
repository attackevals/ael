# Terraform variables
variable "name-prefix" {
  type        = string
  description = "String prefix for resource names"
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

variable "charge-code" {
  type        = string
  description = "String charge code for ER7 resources"
}

variable "round-name" {
  type        = string
  description = "Uppercase description of the resource group, e.g., MSR3, ER7"
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

variable "vendor" {
  type        = string
  description = "Name of vendor to associate to range"
}

variable "aws_trusted_account_id" {
  type        = string
  description = "AWS account ID of the account to use to create a trust relationship"
}

#####################################################################################
# default credentials
#####################################################################################

# SSH keys for linux systems
variable "ssh_public_key_path" {
  type        = string
  description = "Path to SSH public key to use for Linux ssh systems (public and private key must be matching pair)"
}

variable "ssh_private_key_path" {
  type        = string
  description = "Path to SSH private key to use for Linux ssh systems (public and private key must be matching pair)"
}

# SSH key for Preflights box
variable "ssh_public_key_path_vendor" {
  type        = string
  description = "Path to SSH public key to use for preflights Linux box (public and private key must be matching pair)"
}

variable "ssh_private_key_path_vendor" {
  type        = string
  description = "Path to SSH private key to use for preflights Linux box (public and private key must be matching pair)"
}

variable "win_srv_admin_username" {
  type        = string
  description = "Default username for Windows Server administrators"
}

variable "win_srv_admin_password" {
  type        = string
  description = "Default password for Windows Server administrators"
}
