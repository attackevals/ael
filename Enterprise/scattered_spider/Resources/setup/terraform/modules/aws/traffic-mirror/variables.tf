############################################################################
### Internal
############################################################################
variable "name" {
  type        = string
  description = "[REQUIRED] Base name of resource"
}

variable "description" {
  type        = string
  description = "[REQUIRED] Description tag of resource group"
}

variable "tags" {
  type        = map(string)
  description = "[REQUIRED] Tags to attach to resources"
}

#############################################################################
#### Networking
#############################################################################

variable "target_nic_id" {
  type        = any
  description = "[OPTIONAL] The ID of target network interface to mirror traffic to send mirrored traffic. One of target_gateway_load_balancer_endpoint_id or target_nic_id MUST be set."
  default     = null
}

variable "interface_ids" {
  type        = list(any)
  description = "[REQUIRED] Source network interfaces (List of IDs) to create mirror sessions. Target will be target_nic_id. (max 10)"
  #   validation {
  #     condition     = length(var.interface_ids) <= 10
  #     error_message = "The number of instance IDs cannot exceed 10."
  #   }
}

variable "availability_zone" {
  type        = string
  description = "[REQUIRED] Availability zone to place AWS resource"
  default     = "us-east-1a"
}

variable "session_number" {
  type        = number
  description = "[OPTIONAL] Session number to use. Defaults to 1"
  default     = 1
}

variable "target_gateway_load_balancer_endpoint_id" {
  type        = string
  description = "[OPTIONAL] Gateway Load Balancer Endpoint (GWLBe) ID to target. One of target_gateway_load_balancer_endpoint_id, target_network_load_balancer_arn, or target_nic_id MUST be set."
  default     = null
}

variable "target_network_load_balancer_arn" {
  type        = string
  description = "[OPTIONAL] Network Load Balancer ARN to target. One of target_gateway_load_balancer_endpoint_id, target_network_load_balancer_arn, or target_nic_id MUST be set."
  default     = null
}

resource "null_resource" "target_check" {
  lifecycle {
    precondition {
      condition = (
        (var.target_nic_id != null ? 1 : 0) +
        (var.target_gateway_load_balancer_endpoint_id != null ? 1 : 0) +
        (var.target_network_load_balancer_arn != null ? 1 : 0)
      ) == 1
      error_message = "Exactly one of target_nic_id, target_gateway_load_balancer_endpoint_id, or target_network_load_balancer_arn must be set."
    }
  }
}
