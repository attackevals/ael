# _____________________________________________________________________________________
#
# Scenario B.3 NETWORKING
#
# _____________________________________________________________________________________

locals {
  b3-vpc-name     = "${var.name-prefix}-b3-vpc"
  b3-sub-name     = "${var.name-prefix}-b3-subnet"
  b3-sg-name      = "${var.name-prefix}-b3-sg"
  b3-sg-dmz-name  = "${var.name-prefix}-b3-dmz-sg"
  b3-igw-name     = "${var.name-prefix}-b3-igw"
  b3-nat-name     = "${var.name-prefix}-b3-nat"
  b3-eip-name     = "${var.name-prefix}-b3-nat-eip"
  b3-rtb-name     = "${var.name-prefix}-b3-rtb"
  b3-rtb-pub-name = "${var.name-prefix}-b3-public-rtb"

  b3-cidr-prefix     = "10.26"
  b3-cidr            = "${local.b3-cidr-prefix}.0.0/16"
  b3-srv-ip-prefix   = "${local.b3-cidr-prefix}.3"
  b3-desk-ip-prefix  = "${local.b3-cidr-prefix}.4"
  b3-dmz-ip-prefix   = "${local.b3-cidr-prefix}.5"
  b3-vpc-description = "[${var.round-name}] Scenario B.3 Range"
  b3-sg-description  = "[${var.round-name}] Scenario B.3 Range Security Group"

  b3-servers = {
    vpc-cidr        = "${local.b3-srv-ip-prefix}.0/24"
    subnet-cidr     = "${local.b3-srv-ip-prefix}.0/25"
    sub-name        = "${local.b3-sub-name}-srv"
    sub-description = "[${var.round-name}] Scenario B.3 Servers Subnet"
    sg-description  = "[${var.round-name}] Scenario B.3 Servers Security Group"
    sg-name         = "${local.b3-sg-name}-jump"
  }

  b3-dmz = {
    vpc-cidr        = "${local.b3-dmz-ip-prefix}.0/24"
    subnet-cidr     = "${local.b3-dmz-ip-prefix}.0/25"
    sub-name        = "${local.b3-sub-name}-dmz"
    sub-description = "[${var.round-name}] Scenario B.3 DMZ Subnet"
    sg-description  = "[${var.round-name}] Scenario B.3 DMZ Security Group"
  }

  b3-desktops = {
    vpc-cidr        = "${local.b3-desk-ip-prefix}.0/24"
    subnet-cidr     = "${local.b3-desk-ip-prefix}.0/25"
    sub-name        = "${local.b3-sub-name}-desk"
    sub-description = "[${var.round-name}] Scenario B.3 Desktops Subnet"
    sg-description  = "[${var.round-name}] Scenario B.3 Desktops Security Group"
  }

  b3-nat-sub-ip-prefix = "10.203"
  b3-tgw-sub-ip-prefix = "10.233"

  b3-nat-sub = {
    vpc-cidr        = "${local.b3-nat-sub-ip-prefix}.0.0/17"
    subnet-cidr     = "${local.b3-nat-sub-ip-prefix}.0.0/19"
    nat-ip          = "${local.b3-nat-sub-ip-prefix}.1.64"
    sub-name        = "${local.b3-sub-name}-nat"
    sub-description = "[${var.round-name}] Scenario B.3 NAT Subnet"
    sg-description  = "[${var.round-name}] Scenario B.3 NAT Security Group"
  }

  b3-tgw-sub = {
    sub-cidr        = "${local.b3-tgw-sub-ip-prefix}.199.0/24"
    sub-name        = "${local.b3-sub-name}-tgw"
    sub-description = "[${var.round-name}] Victim B.3 TGW Subnet"
    sg-description  = "[${var.round-name}] Victim B.3 TGW Security Group"
  }

  # prefix lists
  b3_prefix_list_entries = [
    [local.b3-servers.vpc-cidr, local.b3-servers.sub-description],
    [local.b3-dmz.vpc-cidr, local.b3-dmz.sub-description],
    [local.b3-desktops.vpc-cidr, local.b3-desktops.sub-description],
    [local.b3-tgw-sub.sub-cidr, local.b3-tgw-sub.sub-description]
  ]

}

# ------------------------------------------------------------------------------------
# [Scenario B.3] Prefix Lists
# ------------------------------------------------------------------------------------
resource "aws_ec2_managed_prefix_list" "b3" {
  name           = "Scenario B.3 prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.b3_prefix_list_entries)

  dynamic "entry" {
    for_each = local.b3_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }

  tags = merge(local.b3-tags, {
    Name = "Scenario B.3 Prefix List"
  })
}


# ------------------------------------------------------------------------------------
# [Scenario B.3] Prefix Lists (cloud detections)
# ------------------------------------------------------------------------------------
resource "aws_ec2_managed_prefix_list" "b3-cloud" {
  provider       = aws.cloud-detections
  name           = "Scenario B.3 prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.b3_prefix_list_entries)

  dynamic "entry" {
    for_each = local.b3_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }

  tags = merge(local.b3-tags, {
    Name = "Scenario B.3 Prefix List"
  })
}

# ------------------------------------------------------------------------------------
# [Scenario B.3] Prefix Lists (cloud detections)
# ------------------------------------------------------------------------------------
resource "aws_ec2_managed_prefix_list" "b3-cloud-protections" {
  provider       = aws.cloud-protections
  name           = "Scenario B.3 prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.b3_prefix_list_entries)

  dynamic "entry" {
    for_each = local.b3_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }

  tags = merge(local.b3-tags, {
    Name = "Scenario B.3 Prefix List Victim Cloud Protections"
  })
}

######################################################################################
# [Protections] VPC
######################################################################################
resource "aws_vpc" "b3" {
  cidr_block           = local.b3-cidr
  enable_dns_hostnames = true
  enable_dns_support   = true

  tags = merge(local.b3-tags, {
    Name        = local.b3-vpc-name,
    Description = local.b3-vpc-description
  })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

# associate CIDRs with VPC
resource "aws_vpc_ipv4_cidr_block_association" "b3-nat-cidr" {
  cidr_block = local.b3-nat-sub.vpc-cidr
  vpc_id     = aws_vpc.b3.id
}

resource "aws_vpc_ipv4_cidr_block_association" "b3-tgw-cidr" {
  cidr_block = local.b3-tgw-sub.sub-cidr
  vpc_id     = aws_vpc.b3.id
}

# ------------------------------------------------------------------------------------
## [Protections] SUBNETS
### Scenario B.3 Subnet 1 - Servers
resource "aws_subnet" "b3-servers" {
  vpc_id            = aws_vpc.b3.id
  cidr_block        = local.b3-servers.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.b3-tags, { Name = local.b3-servers.sub-name, Description = local.b3-servers.sub-description })
}

### Scenario B.3 - DMZ Subnet
resource "aws_subnet" "b3-dmz" {
  #   vpc_id            = aws_vpc_ipv4_cidr_block_association.dmz-cidr.vpc_id ## <-- recommended per: https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet - Subnets In Secondary VPC CIDR Blocks section
  vpc_id            = aws_vpc.b3.id
  cidr_block        = local.b3-dmz.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.b3-tags, { Name = local.b3-sub-name, Description = local.b3-dmz.sub-description })
}

### Scenario B.3 - Desktops Subnet
resource "aws_subnet" "b3-desktops" {
  vpc_id            = aws_vpc.b3.id
  cidr_block        = local.b3-desktops.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.b3-tags, { Name = local.b3-desktops.sub-name, Description = local.b3-desktops.sub-description })
}

## Scenario B.3 - NAT Subnet
resource "aws_subnet" "b3-nat" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.b3-nat-cidr.vpc_id ## <-- recommended per: https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet - Subnets In Secondary VPC CIDR Blocks section
  cidr_block        = local.b3-nat-sub.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.b3-tags, { Name = local.b3-nat-name, Description = local.b3-nat-sub.sub-description })
}

## Scenario B.3 - TGW Attachment Subnet
resource "aws_subnet" "b3-tgw" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.b3-tgw-cidr.vpc_id
  cidr_block        = local.b3-tgw-sub.sub-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.b3-tags, { Name = local.b3-tgw-sub.sub-name, Description = local.b3-tgw-sub.sub-description })
}

# ------------------------------------------------------------------------------------
# INTERNET GATEWAY
resource "aws_internet_gateway" "b3-igw" {
  vpc_id = aws_vpc.b3.id
  tags   = merge(local.b3-tags, local.b3-internet-tags, { Name = local.b3-igw-name, Internet = local.scope.victim })

}

# ------------------------------------------------------------------------------------
# NAT GATEWAY
resource "aws_eip" "b3-nat-eip" {
  # NOTE: Do not use network_interface to associate aws_eip to aws_nat_gateway
  associate_with_private_ip = local.b3-nat-sub.nat-ip

  public_ipv4_pool = "amazon"
  domain           = "vpc"
  depends_on       = [aws_internet_gateway.b3-igw]
  tags             = merge(local.b3-tags, { Name = local.b3-eip-name })
}

# Create NAT gateway
resource "aws_nat_gateway" "b3-nat" {
  allocation_id = aws_eip.b3-nat-eip.id
  subnet_id     = aws_subnet.b3-nat.id
  depends_on    = [aws_internet_gateway.b3-igw]
  tags          = merge(local.b3-tags, { Name = local.b3-nat-name, Internet = local.scope.victim })
}

# --------------------------------------------------------------------------------------
# Route tables
# --------------------------------------------------------------------------------------
# Private (default)
resource "aws_route_table" "b3-rtb" {
  vpc_id = aws_vpc.b3.id

  dynamic "route" {
    for_each = {
      attacker     = aws_ec2_managed_prefix_list.attacker.id
      neutral      = aws_ec2_managed_prefix_list.neutral.id
      victim-cloud = aws_ec2_managed_prefix_list.victim-cloud.id
      victim       = aws_ec2_managed_prefix_list.victim.id
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
    nat_gateway_id = aws_nat_gateway.b3-nat.id
  }
  tags = merge(local.b3-tags, { Name = local.b3-rtb-name })
}

# Public
resource "aws_route_table" "b3-rtb-public" {
  vpc_id = aws_vpc.b3.id

  dynamic "route" {
    for_each = {
      attacker     = aws_ec2_managed_prefix_list.attacker.id
      neutral      = aws_ec2_managed_prefix_list.neutral.id
      victim-cloud = aws_ec2_managed_prefix_list.victim-cloud.id
      victim       = aws_ec2_managed_prefix_list.victim.id
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
    gateway_id = aws_internet_gateway.b3-igw.id
  }

  tags = merge(local.b3-tags, { Name = local.b3-rtb-pub-name, Internet = local.scope.victim })
}

# --------------------------------------------------------------------------------------
# Route table associations (subnet-level)
# --------------------------------------------------------------------------------------
# servers - private
resource "aws_route_table_association" "b3-srv-rtba" {
  subnet_id      = aws_subnet.b3-servers.id
  route_table_id = aws_route_table.b3-rtb.id
}

# desktops - private
resource "aws_route_table_association" "b3-desk-rtba" {
  subnet_id      = aws_subnet.b3-desktops.id
  route_table_id = aws_route_table.b3-rtb.id
}

# dmz
resource "aws_route_table_association" "b3-dmz-rtba" {
  subnet_id      = aws_subnet.b3-dmz.id
  route_table_id = aws_route_table.b3-rtb.id
}

# tgw - private
resource "aws_route_table_association" "b3-tgw-rtba" {
  subnet_id      = aws_subnet.b3-tgw.id
  route_table_id = aws_route_table.b3-rtb.id
}

# PUBLIC **
resource "aws_route_table_association" "b3-nat-rtba" {
  subnet_id      = aws_subnet.b3-nat.id
  route_table_id = aws_route_table.b3-rtb-public.id
}

######################################################################################
# SECURITY GROUP
######################################################################################
resource "aws_security_group" "b3" {
  name_prefix = local.b3-sg-name
  description = local.b3-sg-description
  vpc_id      = aws_vpc.b3.id

  lifecycle {
    create_before_destroy = true
  }

  tags = merge(local.b3-tags, {
    Name        = local.b3-sg-name
    Description = local.b3-sg-description
  })
  depends_on = [aws_vpc.b3]
}

resource "aws_vpc_security_group_ingress_rule" "b3_self_ingress" {
  security_group_id            = aws_security_group.b3.id
  description                  = "Allow all inbound traffic from self"
  from_port                    = -1
  to_port                      = -1
  ip_protocol                  = "-1"
  referenced_security_group_id = aws_security_group.b3.id
}

resource "aws_vpc_security_group_ingress_rule" "b3_dmz_ingress" {
  security_group_id            = aws_security_group.b3.id
  description                  = "Allow all inbound traffic from dmz"
  from_port                    = -1
  to_port                      = -1
  ip_protocol                  = "-1"
  referenced_security_group_id = aws_security_group.b3-dmz.id
}

locals {
  b3_allowed_prefix_lists = [
    aws_ec2_managed_prefix_list.attacker.id,
    aws_ec2_managed_prefix_list.neutral.id,
    aws_ec2_managed_prefix_list.victim.id,
    aws_ec2_managed_prefix_list.victim-cloud.id,
    aws_ec2_managed_prefix_list.main-victim-cloud-protections.id
  ]
}

resource "aws_vpc_security_group_ingress_rule" "b3_prefix_list_ingress" {
  count = length(local.b3_allowed_prefix_lists)

  security_group_id = aws_security_group.b3.id
  description       = "Allow all inbound traffic from prefix list ${local.b3_allowed_prefix_lists[count.index]}"
  from_port         = -1
  to_port           = -1
  ip_protocol       = "-1"
  prefix_list_id    = local.b3_allowed_prefix_lists[count.index]
}

resource "aws_vpc_security_group_egress_rule" "b3_allow_all_outbound" {
  security_group_id = aws_security_group.b3.id
  description       = "Allow all outbound traffic"
  ip_protocol       = "-1"
  cidr_ipv4         = "0.0.0.0/0"
  from_port         = -1
  to_port           = -1
}
