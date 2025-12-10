# _____________________________________________________________________________________
#
# VICTIM NETWORKING
#
# _____________________________________________________________________________________

locals {
  victim-vpc-name     = "${var.name-prefix}-victim-vpc"
  victim-sub-name     = "${var.name-prefix}-victim-subnet"
  victim-sg-name      = "${var.name-prefix}-victim-sg"
  victim-sg-dmz-name  = "${var.name-prefix}-victim-dmz-sg"
  victim-igw-name     = "${var.name-prefix}-victim-igw"
  victim-nat-name     = "${var.name-prefix}-victim-nat"
  victim-vpn-name     = "${var.name-prefix}-victim-vpn"
  victim-eip-name     = "${var.name-prefix}-victim-nat-eip"
  victim-rtb-name     = "${var.name-prefix}-victim-rtb"
  victim-rtb-pub-name = "${var.name-prefix}-victim-public-rtb"

  victim-cidr-prefix     = "10.55"
  victim-cidr            = "${local.victim-cidr-prefix}.0.0/16"
  victim-srv-ip-prefix   = "${local.victim-cidr-prefix}.3"
  victim-desk-ip-prefix  = "${local.victim-cidr-prefix}.4"
  victim-dmz-ip-prefix   = "${local.victim-cidr-prefix}.2"
  victim-vpc-description = "[${var.round-name}] Victim On-Prem Range"
  victim-sg-description  = "[${var.round-name}] Victim On-Prem Range Security Group"

  victim-servers = {
    vpc-cidr        = "${local.victim-srv-ip-prefix}.0/24"
    subnet-cidr     = "${local.victim-srv-ip-prefix}.0/25"
    sub-name        = "${local.victim-sub-name}-srv"
    sub-description = "[${var.round-name}] Victim On-Prem Servers Subnet"
    sg-description  = "[${var.round-name}] Victim On-Prem Servers Security Group"
  }

  victim-desktops = {
    vpc-cidr        = "${local.victim-desk-ip-prefix}.0/24"
    subnet-cidr     = "${local.victim-desk-ip-prefix}.0/25"
    sub-name        = "${local.victim-sub-name}-desk"
    sub-description = "[${var.round-name}] Victim On-Prem Desktops Subnet"
    sg-description  = "[${var.round-name}] Victim On-Prem Desktops Security Group"
  }

  victim-dmz = {
    vpc-cidr        = "${local.victim-dmz-ip-prefix}.0/24"
    subnet-cidr     = "${local.victim-dmz-ip-prefix}.0/25"
    sub-name        = "${local.victim-sub-name}-dmz"
    sub-description = "[${var.round-name}] Victim DMZ Subnet"
    sg-description  = "[${var.round-name}] Victim DMZ Security Group"
  }

  vpn-sub-ip-prefix = "10.33"
  nat-sub-ip-prefix = "10.99"
  tgw-sub-ip-prefix = "10.199"

  vpn-sub = {
    vpc-cidr        = "${local.vpn-sub-ip-prefix}.0.0/24"
    subnet-cidr     = "${local.vpn-sub-ip-prefix}.0.0/24"
    sub-name        = "${local.victim-sub-name}-vpn"
    sub-description = "[${var.round-name}] VPN Subnet"
    sg-description  = "[${var.round-name}] VPN Security Group"
  }

  nat-sub = {
    vpc-cidr        = "${local.nat-sub-ip-prefix}.0.0/17"
    subnet-cidr     = "${local.nat-sub-ip-prefix}.0.0/19"
    nat-ip          = "${local.nat-sub-ip-prefix}.1.64"
    sub-name        = "${local.victim-sub-name}-nat"
    sub-description = "[${var.round-name}] NAT Subnet"
    sg-description  = "[${var.round-name}] NAT Security Group"
  }

  tgw-sub = {
    sub-cidr        = "${local.tgw-sub-ip-prefix}.199.0/24"
    sub-name        = "${local.victim-sub-name}-tgw"
    sub-description = "[${var.round-name}] Victim On-Prem TGW Subnet"
    sg-description  = "[${var.round-name}] Victim On-Prem TGW Security Group"
  }

  # prefix lists
  # to simplify managing/updating lists, simple array
  # format is [CIDR, Description]
  victim_prefix_list_entries = [
    [local.victim-servers.vpc-cidr, local.victim-servers.sub-description],
    [local.victim-desktops.vpc-cidr, local.victim-desktops.sub-description],
    [local.victim-dmz.vpc-cidr, local.victim-dmz.sub-description],
    [local.vpn-sub.vpc-cidr, local.vpn-sub.sub-description],
    [local.tgw-sub.sub-cidr, local.tgw-sub.sub-description],
  ]

  vpn_prefix_list_entries = [
    [local.vpn-sub.vpc-cidr, local.vpn-sub.sub-description],
  ]

}

# ------------------------------------------------------------------------------------
# [VICTIM] Prefix Lists
# ------------------------------------------------------------------------------------
resource "aws_ec2_managed_prefix_list" "victim" {
  name           = "Victim prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.victim_prefix_list_entries)

  dynamic "entry" {
    for_each = local.victim_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }

  tags = merge(local.victim-onprem-tags, {
    Name = "Victim On-Prem Prefix List"
  })
}

######################################################################################
# Prefix List for onprem - cloud detections
######################################################################################
resource "aws_ec2_managed_prefix_list" "onprem-cloud" {
  provider = aws.cloud-detections

  name           = "Victim prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.victim_prefix_list_entries)

  dynamic "entry" {
    for_each = local.victim_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }

  tags = merge(local.victim-onprem-tags, {
    Name = "Victim On-Prem Prefix List"
  })
}

######################################################################################
# Prefix List for onprem - cloud protections
######################################################################################
resource "aws_ec2_managed_prefix_list" "onprem-cloud-protections" {
  provider = aws.cloud-protections

  name           = "Victim prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.victim_prefix_list_entries)

  dynamic "entry" {
    for_each = local.victim_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }

  tags = merge(local.victim-onprem-tags, {
    Name = "Victim On-Prem Prefix List"
  })
}

resource "aws_ec2_managed_prefix_list" "vpn" {
  name           = "VPN prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.vpn_prefix_list_entries)

  dynamic "entry" {
    for_each = local.vpn_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }

  tags = merge(local.victim-onprem-tags, {
    Name = "VPN Prefix List"
  })
}

######################################################################################
# [VICTIM] VPC
######################################################################################
resource "aws_vpc" "victim" {
  cidr_block           = local.victim-cidr
  enable_dns_hostnames = true
  enable_dns_support   = true

  tags = merge(local.victim-onprem-tags, {
    Name        = local.victim-vpc-name,
    Description = local.victim-vpc-description
  })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

resource "aws_vpc_dhcp_options_association" "dns_resolver_victim" {
  vpc_id          = aws_vpc.victim.id
  dhcp_options_id = aws_vpc_dhcp_options.dns_resolver.id
}

# associate CIDRs with VPC
resource "aws_vpc_ipv4_cidr_block_association" "nat-cidr" {
  cidr_block = local.nat-sub.vpc-cidr
  vpc_id     = aws_vpc.victim.id
}

resource "aws_vpc_ipv4_cidr_block_association" "vpn-cidr" {
  cidr_block = local.vpn-sub.vpc-cidr
  vpc_id     = aws_vpc.victim.id
}

resource "aws_vpc_ipv4_cidr_block_association" "tgw-cidr" {
  cidr_block = local.tgw-sub.sub-cidr
  vpc_id     = aws_vpc.victim.id
}

# ------------------------------------------------------------------------------------
## [VICTIM] SUBNETS
### VICTIM - On-Prem Subnet 1 - Servers
resource "aws_subnet" "victim-servers" {
  vpc_id            = aws_vpc.victim.id
  cidr_block        = local.victim-servers.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.victim-onprem-tags, { Name = local.victim-servers.sub-name, Description = local.victim-servers.sub-description })
}

### VICTIM - On-Prem Subnet 2 - Desktops
resource "aws_subnet" "victim-desktops" {
  vpc_id            = aws_vpc.victim.id
  cidr_block        = local.victim-desktops.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.victim-onprem-tags, { Name = local.victim-desktops.sub-name, Description = local.victim-desktops.sub-description })
}

### VICTIM - DMZ Subnet
resource "aws_subnet" "victim-dmz" {
  #   vpc_id            = aws_vpc_ipv4_cidr_block_association.dmz-cidr.vpc_id ## <-- recommended per: https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet - Subnets In Secondary VPC CIDR Blocks section
  vpc_id            = aws_vpc.victim.id
  cidr_block        = local.victim-dmz.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.victim-onprem-tags, { Name = local.victim-dmz.sub-name, Description = local.victim-dmz.sub-description })
}

## VICTIM - NAT Subnet
resource "aws_subnet" "victim-nat" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.nat-cidr.vpc_id ## <-- recommended per: https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet - Subnets In Secondary VPC CIDR Blocks section
  cidr_block        = local.nat-sub.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.victim-onprem-tags, { Name = local.nat-sub.sub-name, Description = local.nat-sub.sub-description })
}

## VICTIM - VPN (& CLIENT)
resource "aws_subnet" "vpn" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.vpn-cidr.vpc_id ## <-- recommended per: https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet - Subnets In Secondary VPC CIDR Blocks section
  cidr_block        = local.vpn-sub.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.victim-onprem-tags, { Name = local.vpn-sub.sub-name, Description = local.vpn-sub.sub-description })
}

## VICTIM - TGW Attachment Subnet
resource "aws_subnet" "victim-tgw" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.tgw-cidr.vpc_id
  cidr_block        = local.tgw-sub.sub-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.victim-onprem-tags, { Name = local.tgw-sub.sub-name, Description = local.tgw-sub.sub-description })
}

# ------------------------------------------------------------------------------------
# INTERNET GATEWAY
resource "aws_internet_gateway" "victim-igw" {
  vpc_id = aws_vpc.victim.id
  tags   = merge(local.victim-onprem-tags, local.victim-internet-tags, { Name = local.victim-igw-name, Internet = local.scope.victim })
}

# ------------------------------------------------------------------------------------
# NAT GATEWAY
resource "aws_eip" "victim-nat-eip" {
  # NOTE: Do not use network_interface to associate aws_eip to aws_nat_gateway
  associate_with_private_ip = local.nat-sub.nat-ip

  public_ipv4_pool = "amazon"
  domain           = "vpc"
  depends_on       = [aws_internet_gateway.victim-igw]
  tags             = merge(local.victim-onprem-tags, { Name = local.victim-eip-name })
}

# Create NAT gateway
resource "aws_nat_gateway" "victim-nat" {
  allocation_id = aws_eip.victim-nat-eip.id
  subnet_id     = aws_subnet.victim-nat.id
  depends_on    = [aws_internet_gateway.victim-igw]
  tags          = merge(local.victim-onprem-tags, { Name = local.victim-nat-name, Internet = local.scope.victim })
}

# --------------------------------------------------------------------------------------
# Route tables
# --------------------------------------------------------------------------------------
# Private (default)
resource "aws_route_table" "victim-rtb" {
  vpc_id = aws_vpc.victim.id

  # For victim-rtb
  dynamic "route" {
    for_each = {
      attacker          = aws_ec2_managed_prefix_list.attacker.id
      neutral           = aws_ec2_managed_prefix_list.neutral.id
      victim-cloud      = aws_ec2_managed_prefix_list.victim-cloud.id
      b3                = aws_ec2_managed_prefix_list.b3.id
      protections-cloud = aws_ec2_managed_prefix_list.main-victim-cloud-protections.id
    }
    content {
      destination_prefix_list_id = route.value
      transit_gateway_id         = aws_ec2_transit_gateway.main.id
    }
  }

  # ALL TRAFFIC --> NAT
  route {
    cidr_block     = local.egress-cidr
    nat_gateway_id = aws_nat_gateway.victim-nat.id
  }
  tags = merge(local.victim-onprem-tags, { Name = local.victim-rtb-name })
}

# Public
resource "aws_route_table" "victim-rtb-public" {
  vpc_id = aws_vpc.victim.id

  # For victim-rtb
  dynamic "route" {
    for_each = {
      attacker          = aws_ec2_managed_prefix_list.attacker.id
      neutral           = aws_ec2_managed_prefix_list.neutral.id
      victim-cloud      = aws_ec2_managed_prefix_list.victim-cloud.id
      b3                = aws_ec2_managed_prefix_list.b3.id
      protections-cloud = aws_ec2_managed_prefix_list.main-victim-cloud-protections.id
    }
    content {
      destination_prefix_list_id = route.value
      transit_gateway_id         = aws_ec2_transit_gateway.main.id
    }
  }

  # NAT TRAFFIC --> IGW
  route {
    cidr_block = local.egress-cidr
    gateway_id = aws_internet_gateway.victim-igw.id
  }

  tags = merge(local.victim-onprem-tags, { Name = local.victim-rtb-pub-name, Internet = local.scope.victim })
}

# --------------------------------------------------------------------------------------
# Route table associations (subnet-level)
# --------------------------------------------------------------------------------------
# servers - private
resource "aws_route_table_association" "on-prem-victim-srv-rtba" {
  subnet_id      = aws_subnet.victim-servers.id
  route_table_id = aws_route_table.victim-rtb.id
}

# desktop - private
resource "aws_route_table_association" "on-prem-victim-desk-rtba" {
  subnet_id      = aws_subnet.victim-desktops.id
  route_table_id = aws_route_table.victim-rtb.id
}

# dmz - private
resource "aws_route_table_association" "on-prem-dmz-rtba" {
  subnet_id      = aws_subnet.victim-dmz.id
  route_table_id = aws_route_table.victim-rtb.id
}

# tgw - private
resource "aws_route_table_association" "on-prem-tgw-rtba" {
  subnet_id      = aws_subnet.victim-tgw.id
  route_table_id = aws_route_table.victim-rtb.id
}

# vpn - private
resource "aws_route_table_association" "on-prem-vpn-rtba" {
  subnet_id      = aws_subnet.vpn.id
  route_table_id = aws_route_table.victim-rtb.id
}

# PUBLIC **
resource "aws_route_table_association" "on-prem-victim-nat-rtba" {
  subnet_id      = aws_subnet.victim-nat.id
  route_table_id = aws_route_table.victim-rtb-public.id
}

######################################################################################
# SECURITY GROUP
######################################################################################
resource "aws_security_group" "victim-on-prem" {
  name_prefix = local.victim-sg-name
  description = local.victim-sg-description
  vpc_id      = aws_vpc.victim.id

  lifecycle {
    create_before_destroy = true
  }

  tags = merge(local.victim-onprem-tags, {
    Name        = local.victim-sg-name
    Description = local.victim-sg-description
  })
  depends_on = [aws_vpc.victim]
}

resource "aws_vpc_security_group_ingress_rule" "self_ingress" {
  security_group_id            = aws_security_group.victim-on-prem.id
  description                  = "Allow all inbound traffic from self"
  from_port                    = -1
  to_port                      = -1
  ip_protocol                  = "-1"
  referenced_security_group_id = aws_security_group.victim-on-prem.id
}

locals {
  allowed_prefix_lists = [
    aws_ec2_managed_prefix_list.attacker.id,
    aws_ec2_managed_prefix_list.neutral.id,
    aws_ec2_managed_prefix_list.victim.id,
    aws_ec2_managed_prefix_list.victim-cloud.id,
    aws_ec2_managed_prefix_list.vpn.id,
    aws_ec2_managed_prefix_list.b3.id,
    aws_ec2_managed_prefix_list.main-victim-cloud-protections.id,

  ]
}

resource "aws_vpc_security_group_ingress_rule" "prefix_list_ingress" {
  count = length(local.allowed_prefix_lists)

  security_group_id = aws_security_group.victim-on-prem.id
  description       = "Allow all inbound traffic from prefix list ${local.allowed_prefix_lists[count.index]}"
  from_port         = -1
  to_port           = -1
  ip_protocol       = "-1"
  prefix_list_id    = local.allowed_prefix_lists[count.index]
}

resource "aws_vpc_security_group_egress_rule" "allow_all_outbound" {
  security_group_id = aws_security_group.victim-on-prem.id
  description       = "Allow all outbound traffic"
  ip_protocol       = "-1"
  cidr_ipv4         = "0.0.0.0/0"
  from_port         = -1
  to_port           = -1
}
