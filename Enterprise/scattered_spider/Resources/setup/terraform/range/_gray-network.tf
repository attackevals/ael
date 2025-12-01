# _____________________________________________________________________________________
#
# gray NETWORKING
#
# _____________________________________________________________________________________

locals {
  neutral-vpc-name         = "${var.name-prefix}-neutral-vpc"
  neutral-sub-name-support = "${var.name-prefix}-neutral-sub-support"
  neutral-sub-name-re1     = "${var.name-prefix}-neutral-sub-redirect1"
  neutral-sub-name-re2     = "${var.name-prefix}-neutral-sub-redirect2"
  neutral-sub-name-re3     = "${var.name-prefix}-neutral-sub-redirect3"
  neutral-sub-name-re4     = "${var.name-prefix}-neutral-sub-redirect4"
  neutral-sg-name          = "${var.name-prefix}-neutral-sg"
  neutral-igw-name         = "${var.name-prefix}-neutral-igw"
  neutral-nat-name         = "${var.name-prefix}-neutral-nat"
  neutral-eip-name         = "${var.name-prefix}-neutral-nat-eip"
  neutral-rtb-name         = "${var.name-prefix}-neutral-rtb"
  neutral-tgw-name         = "${var.name-prefix}-neutral-tgw"

  neutral-vpc-description     = "[${var.round-name}] Neutral Range"
  neutral-sub-description     = "[${var.round-name}] Neutral Subnet"
  neutral-sg-description      = "[${var.round-name}] Neutral Security Group"
  neutral-nat-description     = "[${var.round-name}] Neutral NAT"
  neutral-tgw-sub-description = "[${var.round-name}] Neutral TGW Subnet"

  neutral-support-prefix      = "12.78"
  neutral-support-cidr        = "${local.neutral-support-prefix}.0.0/16"
  neutral-support-description = "Neutral Support (Unmanaged WKS, DNS)"

  neutral-redirect1-prefix      = "86.234" # Changed for ER7 (in use)
  neutral-redirect1-cidr        = "${local.neutral-redirect1-prefix}.0.0/16"
  neutral-redirect1-description = "Neutral Redirect 1"

  neutral-redirect2-prefix      = "49.67" # Changed for ER7
  neutral-redirect2-cidr        = "${local.neutral-redirect2-prefix}.0.0/16"
  neutral-redirect2-description = "Neutral Redirect 2"

  neutral-redirect3-prefix      = "191.44" # Changed for ER7
  neutral-redirect3-cidr        = "${local.neutral-redirect3-prefix}.0.0/16"
  neutral-redirect3-description = "Neutral Redirect 3"

  neutral-redirect4-prefix      = "78.31" # Changed for ER7
  neutral-redirect4-cidr        = "${local.neutral-redirect4-prefix}.0.0/16"
  neutral-redirect4-description = "Neutral Redirect 4"

  neutral-tgw-prefix      = "153.73"
  neutral-tgw-cidr        = "${local.neutral-tgw-prefix}.199.0/24"
  neutral-tgw-description = "Neutral TGW"

  neutral_prefix_list_entries = [
    [local.neutral-support-cidr, local.neutral-support-description],
    [local.neutral-redirect1-cidr, local.neutral-redirect1-description],
    [local.neutral-redirect2-cidr, local.neutral-redirect2-description],
    [local.neutral-redirect3-cidr, local.neutral-redirect3-description],
    [local.neutral-redirect4-cidr, local.neutral-redirect4-description],
    [local.neutral-tgw-cidr, local.neutral-tgw-description],
  ]

  neutral_cidrs = {
    support   = local.neutral-support-cidr
    redirect1 = local.neutral-redirect1-cidr
    redirect2 = local.neutral-redirect2-cidr
    redirect3 = local.neutral-redirect3-cidr
    redirect4 = local.neutral-redirect4-cidr
    tgw       = local.neutral-tgw-cidr
  }
}

######################################################################################
# Prefix List for neutral
######################################################################################
resource "aws_ec2_managed_prefix_list" "neutral" {
  name           = "Neutral prefix list - [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.neutral_prefix_list_entries)

  dynamic "entry" {
    for_each = local.neutral_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }
  tags = merge(local.neutral-tags, { Name = "Neutral - [${var.vendor}]" })
}

######################################################################################
# Prefix List for neutral - cloud detections (switch to shared prefix list)
######################################################################################
resource "aws_ec2_managed_prefix_list" "neutral-cloud" {
  provider = aws.cloud-detections

  name           = "Neutral prefix list - [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.neutral_prefix_list_entries)

  dynamic "entry" {
    for_each = local.neutral_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }
  tags = merge(local.neutral-tags, { Name = "Neutral Cloud - [${var.vendor}]" })
}

######################################################################################
# Prefix List for neutral - cloud protections (switch to shared prefix list)
######################################################################################
resource "aws_ec2_managed_prefix_list" "neutral-cloud-protections" {
  provider = aws.cloud-protections

  name           = "Neutral prefix list - [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.neutral_prefix_list_entries)

  dynamic "entry" {
    for_each = local.neutral_prefix_list_entries
    content {
      cidr        = entry.value[0]
      description = entry.value[1]
    }
  }
  tags = merge(local.neutral-tags, { Name = "Neutral Cloud - [${var.vendor}]" })
}

######################################################################################
# VPC
######################################################################################
resource "aws_vpc_ipv4_cidr_block_association" "neutral_cidrs" {
  for_each   = local.neutral_cidrs
  vpc_id     = aws_vpc.attacker.id
  cidr_block = each.value

  lifecycle {
    create_before_destroy = true
  }
}

# ------------------------------------------------------------------------------------
## SUBNET: Main
resource "aws_subnet" "neutral-support" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.neutral_cidrs["support"].vpc_id
  cidr_block        = local.neutral-support-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.neutral-tags, { Name = local.neutral-sub-name-support, Description = local.neutral-sub-description })
}

resource "aws_subnet" "neutral-redirect1" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.neutral_cidrs["redirect1"].vpc_id
  cidr_block        = local.neutral-redirect1-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.neutral-tags, { Name = local.neutral-sub-name-re1, Description = local.neutral-sub-description })
}

resource "aws_subnet" "neutral-redirect2" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.neutral_cidrs["redirect2"].vpc_id
  cidr_block        = local.neutral-redirect2-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.neutral-tags, { Name = local.neutral-sub-name-re2, Description = local.neutral-sub-description })
}

resource "aws_subnet" "neutral-redirect3" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.neutral_cidrs["redirect3"].vpc_id
  cidr_block        = local.neutral-redirect3-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.neutral-tags, { Name = local.neutral-sub-name-re3, Description = local.neutral-sub-description })
}

resource "aws_subnet" "neutral-redirect4" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.neutral_cidrs["redirect4"].vpc_id
  cidr_block        = local.neutral-redirect4-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.neutral-tags, { Name = local.neutral-sub-name-re4, Description = local.neutral-sub-description })
}

resource "aws_subnet" "neutral-tgw" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.neutral_cidrs["tgw"].vpc_id
  cidr_block        = local.neutral-tgw-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.neutral-tags, { Name = local.neutral-tgw-name, Description = local.neutral-tgw-sub-description })
}

resource "aws_route_table_association" "neutral-redirect1" {
  subnet_id      = aws_subnet.neutral-redirect1.id
  route_table_id = aws_route_table.neutral-rtb.id
}

resource "aws_route_table_association" "neutral-redirect2" {
  subnet_id      = aws_subnet.neutral-redirect2.id
  route_table_id = aws_route_table.neutral-rtb.id
}

resource "aws_route_table_association" "neutral-redirect3" {
  subnet_id      = aws_subnet.neutral-redirect3.id
  route_table_id = aws_route_table.neutral-rtb.id
}

resource "aws_route_table_association" "neutral-redirect4" {
  subnet_id      = aws_subnet.neutral-redirect4.id
  route_table_id = aws_route_table.neutral-rtb.id
}

resource "aws_route_table_association" "neutral-support" {
  subnet_id      = aws_subnet.neutral-support.id
  route_table_id = aws_route_table.neutral-rtb.id
}

resource "aws_route_table_association" "neutral-tgw" {
  subnet_id      = aws_subnet.neutral-tgw.id
  route_table_id = aws_route_table.neutral-rtb.id
}


# --------------------------------------------------------------------------------------
# Route tables
# --------------------------------------------------------------------------------------
# Private (default)
resource "aws_route_table" "neutral-rtb" {
  vpc_id = aws_vpc.attacker.id

  dynamic "route" {
    for_each = {
      victim      = aws_ec2_managed_prefix_list.victim.id
      attacker    = aws_ec2_managed_prefix_list.attacker.id
      b3          = aws_ec2_managed_prefix_list.b3.id
      cloud       = aws_ec2_managed_prefix_list.victim-cloud.id
      protections = aws_ec2_managed_prefix_list.main-victim-cloud-protections.id
    }
    content {
      destination_prefix_list_id = route.value
      transit_gateway_id         = aws_ec2_transit_gateway.main.id
    }
  }

  # ALL TRAFFIC --> Attacker NAT
  route {
    cidr_block     = local.egress-cidr
    nat_gateway_id = aws_nat_gateway.attacker-nat.id
  }

  tags = merge(local.neutral-tags, {
    Name = local.neutral-rtb-name
  })
}

# ######################################################################################
# # SECURITY GROUP
# ######################################################################################
# Not needed currently, uses aws_security_group.attacker resource
# can re-add if needed, but removed for time being to avoid confusion
