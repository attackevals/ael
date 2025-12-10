variable "name_prefix" {
  description = "Prefix for resource names"
  type        = string
}

variable "registration_limit" {
  description = "Maximum number of managed instances that can be registered using this activation"
  type        = number
  default     = 100
}

variable "expiration_days" {
  description = "Number of days before the activation expires"
  type        = number
  default     = 30
}

variable "fixed_expiration_date" {
  description = "Fixed expiration date in RFC3339 format (e.g., '2023-12-31T23:59:59Z'). If provided, overrides expiration_days."
  type        = string
  default     = ""
}

variable "force_new_activation" {
  description = "Set to true to force creation of a new activation"
  type        = bool
  default     = false
}
