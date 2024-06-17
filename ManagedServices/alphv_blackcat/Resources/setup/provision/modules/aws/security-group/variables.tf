##################
# Module requirements
##################

variable "name_prefix" {
  type        = string
  description = "Name of VPC to apply group policy"
}

variable "description" {
  type        = string
  description = "Description for the default security group applied to all range resources"
  default     = "Default Security Group - RDP, ICMP, SSH, WinRM"
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

variable "vpc_id" {
  type        = string
  description = "ID of the VPC where the security group will be applied"
  default     = null
}

#variable "alpha_ingress_whitelist" {
#  type        = list(string)
#  description = "List of VPC CIDRs to apply global ingress security group policies to"
#}
#
#variable "beta_ingress_whitelist" {
#  type        = list(string)
#  description = "List of VPC CIDRs to apply global ingress security group policies to"
#}

variable "ingress_whitelist" {
  type        = list(string)
  description = "List of VPC CIDRs to apply global ingress security group policies to"
}

variable "egress_whitelist" {
  type        = list(string)
  description = "List of VPC CIDRs to apply global egress security group policies to"
  default     = ["0.0.0.0/0"]
}

##################
# Policies
##################

variable "policy_list" {
  description = "List of group policies (objects) to apply to the instance. See security-group/variables.tf for options."
  type = list(object({
    name        = string
    description = string
    ingress_rules = object({
      from_port = number
      to_port   = number
      protocol  = string
    })
  }))
  # TODO: fix or delete
  default = null
}

variable "rdp-sg" {
  description = "Allows RDP access to instance"

  type = object({
    name        = string
    description = string
    ingress_rules = object({
      from_port = number
      to_port   = number
      protocol  = string
    })
  })
  default = {
    name        = "rdp-sg"
    description = "Allows RDP access to instance"
    ingress_rules = {
      from_port = 3389
      to_port   = 3389
      protocol  = "tcp"
    }
  }
}

variable "xssh-sg" {
  description = "Allows SSH access to instance"

  type = object({
    name        = string
    description = string
    ingress_rules = object({
      from_port = number
      to_port   = number
      protocol  = string
    })
  })
  default = {
    name        = "xssh-sg"
    description = "Allows SSH access to instance"
    ingress_rules = {
      from_port = 22
      to_port   = 22
      protocol  = "tcp"
    }
  }
}

variable "winrm-sg" {
  description = "Allows WinRM access to instance"

  type = object({
    name        = string
    description = string
    ingress_rules = object({
      from_port = number
      to_port   = number
      protocol  = string
    })
  })

  default = {
    name        = "winrm-sg"
    description = "Allows WinRM access to instance"
    ingress_rules = {
      from_port = 5985
      to_port   = 5986
      protocol  = "tcp"
    }
  }
}

variable "http-sg" {
  description = "Allows HTTP access to instance"
  type = object({
    name        = string
    description = string
    ingress_rules = object({
      from_port = number
      to_port   = number
      protocol  = string
    })
  })
  default = {
    name        = "http-sg"
    description = "Allows HTTP access to instance"
    ingress_rules = {
      from_port = 80
      to_port   = 80
      protocol  = "tcp"
    }
  }
}

variable "icmp-sg" {
  description = "Allows ICMP/pinging the instance"
  type = object({
    name        = string
    description = string
    ingress_rules = object({
      from_port = number
      to_port   = number
      protocol  = string
    })
  })
  default = {
    name        = "icmp-sg"
    description = "Allows HTTP access to instance"
    ingress_rules = {
      from_port = 8
      to_port   = 0
      protocol  = "icmp"
    }
  }
}
