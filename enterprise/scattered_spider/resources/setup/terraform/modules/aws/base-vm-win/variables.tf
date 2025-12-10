############################################################################
### Internal
############################################################################
variable "name" {
  type        = string
  description = "[REQUIRED] Base name of resource"
}

variable "tags" {
  type        = map(string)
  description = "[REQUIRED] Tags to attach to resources"
}

variable "description" {
  type        = string
  description = "[REQUIRED] Description tag of resource group"
}

variable "scope" {
  type        = string
  description = "[REQUIRED] Indicates the type of resource to filter for range resetting (victim, internal, etc.)"
}

variable "snapshot_required" {
  type        = bool
  description = "[REQUIRED] True if instance is a victim and should be snapshotted; false otherwise (e.g., support proxy)."
}

variable "hostname" {
  type        = string
  description = "[REQUIRED] Hostname to use for system"
}

variable "platform" {
  type        = string
  description = "If passed, will locate and run platform-specific setup scripts for the VM (user_data)"
  default     = null
}

############################################################################
### Networking
############################################################################

variable "subnet_id" {
  type        = string
  description = "[REQUIRED] ID of subnet to place on VM"
}

variable "private_ip" {
  type        = string
  description = "[REQUIRED] Private IP address to associate with the instance in a VPC"
}

variable "private_ips" {
  type        = list(string)
  description = "List of static IPs to assign to nics. NOTE: length of list will determine the number of NICs created. e.g., if you specify 3 IPs, then 3 NICs will be created."
  default     = null
}

variable "availability_zone" {
  type        = string
  description = "[REQUIRED] Availability zone to place AWS resource"
}

variable "security_group_ids" {
  type        = list(string)
  description = "[REQUIRED] List of aws_security_group object IDs [aws_security_group.[object_name].id]"
}

############################################################################
### Instance Creation
############################################################################

variable "ami_id" {
  type        = string
  description = "[REQUIRED] AMI ID for aws_instance"
}

variable "instance_type" {
  type        = string
  description = "[REQUIRED] AWS instance type (e.g., \"t3.medium\")"
}

variable "key_name" {
  type        = string
  description = "[REQUIRED] Key pair used to establish connection"
}

variable "ssh_private_key_path" {
  type        = string
  description = "Filepath to private ssh key (RSA)"
  default     = null
}

variable "iam_role" {
  type        = string
  description = "Name of policy role to assume, e.g., CloudWatchAgentServerRole"
}

############################################################################
### Managed Disk
############################################################################

variable "disk_size" {
  type        = string
  description = "[REQUIRED] Size (in GiB) of disk to create)"
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

variable "admin_username" {
  type        = string
  description = "Admin username for VM"
}

variable "admin_password" {
  type        = string
  description = "Admin password for VM"
}

variable "get_password_data" {
  type        = bool
  description = "Should the instance retrieve password data from AWS"
  default     = false
}

variable "active_directory_domain_name" {
  type        = string
  description = "Domain name for active directory, e.g. reddev.local"
  default     = "unknown.local"
}
