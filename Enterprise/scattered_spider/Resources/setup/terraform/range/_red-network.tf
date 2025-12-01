# _____________________________________________________________________________________
#
# INTERNAL NETWORKING
#
# _____________________________________________________________________________________

locals {
  attacker-vpc-name    = "${var.name-prefix}-red-vpc"
  attacker-sub-name    = "${var.name-prefix}-red-sub"
  attacker-sub-name-re = "${var.name-prefix}-red-sub-redirect"
  attacker-sg-name     = "${var.name-prefix}-red-sg"
  attacker-igw-name    = "${var.name-prefix}-red-igw"
  attacker-nat-name    = "${var.name-prefix}-red-nat"
  attacker-eip-name    = "${var.name-prefix}-red-nat-eip"
  attacker-rtb-name    = "${var.name-prefix}-red-rtb"

  attacker-vpc-description = "[${var.round-name}] Red Range"
  attacker-sub-description = "[${var.round-name}] Red Subnet"
  attacker-sg-description  = "[${var.round-name}] Red Security Group"
  internal-nat-description = "[${var.round-name}] Red NAT"

  attacker-ip-prefix    = "174.3"                              # CHANGED FOR MSR3
  attacker-vpc-cidr     = "${local.attacker-ip-prefix}.0.0/16" # 174.3.0.0/16
  attacker-subnet-cidr  = "${local.attacker-ip-prefix}.0.0/24" # 174.3.0.0/24
  attacker-nat-ip       = "${local.attacker-ip-prefix}.1.64"   # 174.3.1.64
  attacker-nat-sub-cidr = "${local.attacker-ip-prefix}.2.0/24" # 174.3.2.0/24

  attacker-redirect-prefix = "86.123"                                   # CHANGED FOR MSR3
  attacker-redirect-cidr   = "${local.attacker-redirect-prefix}.0.0/16" # 86.123.0.0/16
  attacker-redirect-ip     = "${local.attacker-redirect-prefix}.0.0"    # 86.123.0.0

  attacker_prefix_list_entries = [
    [local.attacker-vpc-cidr, local.attacker-vpc-description],
    [local.neutral-support-cidr, local.neutral-support-description],
    [local.neutral-redirect1-cidr, local.neutral-redirect1-description],
    [local.neutral-redirect2-cidr, local.neutral-redirect2-description],
    [local.neutral-redirect3-cidr, local.neutral-redirect3-description],
    [local.neutral-redirect4-cidr, local.neutral-redirect4-description],

  ]
}

######################################################################################
# Prefix List for Attacker
######################################################################################
resource "aws_ec2_managed_prefix_list" "attacker" {
  name           = "Attacker prefix list - [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.attacker_prefix_list_entries)

  dynamic "entry" {
    for_each = local.attacker_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }

  tags = merge(local.attacker-tags, {
    Name = "Attacker Prefix List - [${var.vendor}]"
  })
}

######################################################################################
# Prefix List for Attacker (Cloud Detections Account)
######################################################################################
resource "aws_ec2_managed_prefix_list" "attacker-cloud" {
  provider = aws.cloud-detections

  name           = "Attacker prefix list - [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.attacker_prefix_list_entries)

  dynamic "entry" {
    for_each = local.attacker_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }

  tags = merge(local.attacker-tags, {
    Name = "Attacker Prefix List - [${var.vendor}]"
  })
}

######################################################################################
# Prefix List for Attacker (Cloud Protections Account)
######################################################################################
resource "aws_ec2_managed_prefix_list" "attacker-cloud-protections" {
  provider = aws.cloud-protections

  name           = "Attacker prefix list - [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.attacker_prefix_list_entries)

  dynamic "entry" {
    for_each = local.attacker_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }

  tags = merge(local.attacker-tags, {
    Name = "Attacker Prefix List - [${var.vendor}]"
  })
}


######################################################################################
# VPC
######################################################################################
resource "aws_vpc" "attacker" {
  cidr_block           = local.attacker-vpc-cidr
  enable_dns_hostnames = true
  enable_dns_support   = true
  tags                 = merge(local.attacker-tags, { Name = local.attacker-vpc-name, Description = local.attacker-vpc-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

resource "aws_vpc_ipv4_cidr_block_association" "attacker-redirect-cidr" {
  vpc_id     = aws_vpc.attacker.id
  cidr_block = local.attacker-redirect-cidr
}

resource "aws_vpc_dhcp_options_association" "dns_resolver_red" {
  vpc_id          = aws_vpc.attacker.id
  dhcp_options_id = aws_vpc_dhcp_options.dns_resolver.id
}

# ------------------------------------------------------------------------------------
## SUBNET: Main
resource "aws_subnet" "attacker" {
  vpc_id            = aws_vpc.attacker.id
  cidr_block        = local.attacker-subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.attacker-tags, { Name = local.attacker-sub-name, Description = local.attacker-sub-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

## SUBNET: NAT
resource "aws_subnet" "attacker-nat" {
  vpc_id            = aws_vpc.attacker.id
  cidr_block        = local.attacker-nat-sub-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.attacker-tags, { Name = local.attacker-nat-name, Description = local.internal-nat-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

# ------------------------------------------------------------------------------------
# INTERNET GATEWAY
resource "aws_internet_gateway" "attacker-igw" {
  vpc_id = aws_vpc.attacker.id
  tags   = merge(local.attacker-tags, { Name = local.attacker-igw-name })
}

# ------------------------------------------------------------------------------------
# NAT GATEWAY
resource "aws_eip" "attacker-nat-eip" {
  # NOTE: Do not use network_interface to associate aws_eip to aws_nat_gateway
  associate_with_private_ip = local.attacker-nat-ip

  public_ipv4_pool = "amazon"
  domain           = "vpc"
  depends_on       = [aws_internet_gateway.attacker-igw]
  tags             = merge(local.attacker-tags, { Name = local.attacker-eip-name })
}

# Create NAT gateway
resource "aws_nat_gateway" "attacker-nat" {
  allocation_id = aws_eip.attacker-nat-eip.id
  subnet_id     = aws_subnet.attacker-nat.id
  depends_on    = [aws_internet_gateway.attacker-igw]
  tags          = merge(local.attacker-tags, { Name = local.attacker-nat-name })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

# --------------------------------------------------------------------------------------
# Route tables
# --------------------------------------------------------------------------------------
# Private (default)
resource "aws_route_table" "attacker-rtb" {
  vpc_id = aws_vpc.attacker.id

  # For victim-rtb
  dynamic "route" {
    for_each = {
      attacker     = aws_ec2_managed_prefix_list.victim.id
      victim-cloud = aws_ec2_managed_prefix_list.victim-cloud.id
      b3           = aws_ec2_managed_prefix_list.b3.id
      protections  = aws_ec2_managed_prefix_list.main-victim-cloud-protections.id
    }
    content {
      destination_prefix_list_id = route.value
      transit_gateway_id         = aws_ec2_transit_gateway.main.id
    }
  }

  # ALL TRAFFIC --> NAT
  route {
    cidr_block     = local.egress-cidr
    nat_gateway_id = aws_nat_gateway.attacker-nat.id
  }
  tags = merge(local.attacker-tags, { Name = local.attacker-rtb-name })
}

# Public
resource "aws_route_table" "attacker-rtb-public" {
  vpc_id = aws_vpc.attacker.id

  # For victim-rtb
  dynamic "route" {
    for_each = {
      victim       = aws_ec2_managed_prefix_list.victim.id
      victim-cloud = aws_ec2_managed_prefix_list.victim-cloud.id
      b3           = aws_ec2_managed_prefix_list.b3.id
      protections  = aws_ec2_managed_prefix_list.main-victim-cloud-protections.id
    }
    content {
      destination_prefix_list_id = route.value
      transit_gateway_id         = aws_ec2_transit_gateway.main.id
    }
  }

  # NAT TRAFFIC --> IGW
  route {
    cidr_block = local.egress-cidr
    gateway_id = aws_internet_gateway.attacker-igw.id
  }
}

# --------------------------------------------------------------------------------------
# Route table associations (subnet-level)
# --------------------------------------------------------------------------------------
# private
resource "aws_route_table_association" "attacker-rtba" {
  subnet_id      = aws_subnet.attacker.id
  route_table_id = aws_route_table.attacker-rtb.id
}

# PUBLIC **
resource "aws_route_table_association" "attacker-nat-rtba" {
  subnet_id      = aws_subnet.attacker-nat.id
  route_table_id = aws_route_table.attacker-rtb-public.id
}

######################################################################################
# SECURITY GROUP
######################################################################################
resource "aws_security_group" "attacker" {
  name_prefix = local.attacker-sg-name
  description = local.attacker-sg-description
  vpc_id      = aws_vpc.attacker.id

  tags = merge(local.attacker-tags, {
    Name        = local.attacker-sg-name
    Description = local.attacker-sg-description
  })
}

# locals specific to sg rules
locals {
  attacker_allowed_prefix_lists = [
    aws_ec2_managed_prefix_list.victim-cloud.id,
    aws_ec2_managed_prefix_list.neutral.id,
    aws_ec2_managed_prefix_list.victim.id,
    aws_ec2_managed_prefix_list.b3.id,
    aws_ec2_managed_prefix_list.main-victim-cloud-protections.id
  ]
  attacker_external_security_groups = [
    aws_ec2_managed_prefix_list.victim-cloud.id,
    aws_security_group.victim-on-prem.id,
    aws_security_group.victim-dmz.id,
    aws_security_group.b3-dmz.id,
    aws_security_group.b3.id,
    aws_ec2_managed_prefix_list.main-victim-cloud-protections.id

  ]
}

resource "aws_vpc_security_group_ingress_rule" "attacker_allowed_self_ingress" {
  security_group_id = aws_security_group.attacker.id

  description                  = "Allow all traffic within self security group"
  from_port                    = -1
  to_port                      = -1
  ip_protocol                  = "-1"
  referenced_security_group_id = aws_security_group.attacker.id
}

resource "aws_vpc_security_group_ingress_rule" "attacker_prefix_list_ingress" {
  count = length(local.attacker_allowed_prefix_lists)

  security_group_id = aws_security_group.attacker.id
  description       = "Allow all inbound traffic from prefix list ${local.attacker_allowed_prefix_lists[count.index]}"
  from_port         = -1
  to_port           = -1
  ip_protocol       = "-1"
  prefix_list_id    = local.attacker_allowed_prefix_lists[count.index]
}

resource "aws_vpc_security_group_egress_rule" "attacker_egress" {
  security_group_id = aws_security_group.attacker.id
  description       = "Allow all outbound traffic"
  from_port         = -1
  to_port           = -1
  ip_protocol       = "-1"
  cidr_ipv4         = "0.0.0.0/0"
}
