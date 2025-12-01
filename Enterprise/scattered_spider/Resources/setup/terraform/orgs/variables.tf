variable "account-cloud-management-id" {
  type        = string
  description = "12-digit AWS Account ID for the cloud management (parent) account"
}

variable "account-detections-email" {
  type        = string
  description = "Email address to associate with the Detections AWS account"
}

variable "account-protections-email" {
  type        = string
  description = "Email address to associate with the Protections AWS account"
}

variable "cloud-identity-store-id" {
  type        = string
  description = "AWS IAM Identity Center Store ID of the pattern 'd-[0-9a-f]{10}'"
}

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
  description = "Uppercase description of the resource group, e.g., ER7, MSR2"
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

variable "provisioner" {
  type        = string
  description = ""
}
