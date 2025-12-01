locals {
  timestamp = timestamp()
}

variable "week" {
  type        = string
  description = "Week of evaluations"
  default     = "init"
}

variable "exec_day" {
  type        = string
  description = "Specifies execution week scenario or day"
  default     = "none"
}

variable "range_status" {
  type        = string
  description = "Cleanliness of range"
  default     = "clean"
}

variable "name" {
  type        = string
  description = "REQUIRED: Unique prefix to use when naming resources"
}

variable "region" {
  type        = string
  description = "AWS region for resources"
}

variable "description" {
  type        = string
  description = "(Optional) Description tag of resource group"
}

variable "environment" {
  type        = string
  description = "(Optional) Environment tag of resource group"
}

variable "category" {
  type        = string
  description = "(Optional) Category of resource group, (similar to name, used for ansible automation)"
}

variable "created" {
  type        = string
  description = "Timestamp of when resource was created"
}

variable "updated" {
  type        = string
  description = "Timestamp of when resource was last updated"
}

variable "charge_code" {
  type        = string
  description = "Charge code for billing, currently defaults to ER6 billing code, set as appropriate"
}

variable "provisioner" {
  type        = string
  description = "Person or tooling that provisions this resource"
}

variable "vendor" {
  type        = string
  description = "Name of vendor"
}

variable "shutdown_ok" {
  type        = string
  description = "This resource can be safely disabled or shutdown"
}

variable "performer" {
  type        = string
  description = "AWS Cost allocation tag for vendor name"
}

variable "owner" {
  type        = string
  description = "AWS Cost allocation tag for owner name. Defaults to ER6 currently."
}
