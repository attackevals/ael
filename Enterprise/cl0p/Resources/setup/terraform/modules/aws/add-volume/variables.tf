############################################################################
### Internal
############################################################################
variable "name" {
  type        = string
  description = "[REQUIRED] Base name of resource"
}

variable "availability_zone" {
  type        = string
  description = "[REQUIRED] Availability zone for range resources"
}

variable "tags" {
  type = object({
    Name        = string
    Description = string
    Category    = string
    ChargeCode  = string
    Environment = string
    Region      = string
    Vendor      = string
    Provisioner = string
  })
}

############################################################################
### Managed Disk
############################################################################
variable "device_name" {
  type        = string
  description = "[REQUIRED] Name of disk to create"
}

variable "instance_id" {
  type        = string
  description = "[REQUIRED] ID of instance to attach volume to"
}

variable "disk_size" {
  type        = string
  description = "[REQUIRED] Size (in GiB) of disk to create"
}

variable "disk_type" {
  type        = string
  description = "[REQUIRED] Storage type used for instance (Defaults to gp2)"
}

variable "device_index" {
  type        = string
  description = "[REQUIRED] NICs device index for attachment order"
  default     = 0
}
