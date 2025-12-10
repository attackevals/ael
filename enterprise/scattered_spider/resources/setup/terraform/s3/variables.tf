# Terraform variables
variable "name-prefix" {
  type        = string
  description = "String prefix for resource names"
}

# variable "aws-bucket-name" {
#   type        = string
#   description = "Name of S3 bucket for managing Terraform state across vendor ranges"
# }

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
  description = "String charge code for MSR3 resources"
}

variable "round-name" {
  type        = string
  description = "Uppercase description of the resource group, e.g., ER6, MSR3"
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
