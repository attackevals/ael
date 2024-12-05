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

  attacker-vpc-description = "ER6 Red Range"
  attacker-sub-description = "ER6 Red Subnet"
  attacker-sg-description  = "ER6 Red Security Group"
  internal-nat-description = "ER6 Red NAT"

  attacker-ip-prefix    = "223.246"
  attacker-vpc-cidr     = "${local.attacker-ip-prefix}.0.0/16" # 223.246.0.0/16
  attacker-subnet-cidr  = "${local.attacker-ip-prefix}.0.0/24" # 223.246.0.0/24
  attacker-nat-ip       = "${local.attacker-ip-prefix}.1.64"   # 223.246.1.64
  attacker-nat-sub-cidr = "${local.attacker-ip-prefix}.2.0/24" # 223.246.2.0/24

  attacker-redirect-prefix = "42.53"
  attacker-redirect-cidr   = "${local.attacker-redirect-prefix}.0.0/16" # 42.53.0.0/16
  attacker-redirect-ip     = "${local.attacker-redirect-prefix}.0.0"    # 42.53.0.0

}

######################################################################################
# Prefix List for Ext-Benevolent
######################################################################################
resource "aws_ec2_managed_prefix_list" "attacker" {
  name           = "Attacker Benevolent Prefix List"
  address_family = "IPv4"
  max_entries    = 5

  entry {
    cidr        = local.attacker-vpc-cidr
    description = "Attacker"
  }

  tags = merge(module.defaults.tags, { Name = "Attacker Prefix List" })
}


######################################################################################
# VPC
######################################################################################
resource "aws_vpc" "attacker" {
  cidr_block           = local.attacker-vpc-cidr
  enable_dns_hostnames = true
  enable_dns_support   = true
  tags                 = merge(module.defaults.tags, { Name = local.attacker-vpc-name, Description = local.attacker-vpc-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

resource "aws_vpc_ipv4_cidr_block_association" "attacker-redirect-cidr" {
  vpc_id     = aws_vpc.attacker.id
  cidr_block = local.attacker-redirect-cidr
}


# ------------------------------------------------------------------------------------
## SUBNET: Main
resource "aws_subnet" "attacker" {
  vpc_id            = aws_vpc.attacker.id
  cidr_block        = local.attacker-subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.attacker-sub-name, Description = local.attacker-sub-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

## SUBNET: NAT
resource "aws_subnet" "attacker-nat" {
  vpc_id            = aws_vpc.attacker.id
  cidr_block        = local.attacker-nat-sub-cidr
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.attacker-nat-name, Description = local.internal-nat-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

# ------------------------------------------------------------------------------------
# INTERNET GATEWAY
resource "aws_internet_gateway" "attacker-igw" {
  vpc_id = aws_vpc.attacker.id
  tags   = merge(module.defaults.tags, { Name = local.attacker-igw-name })
}

# ------------------------------------------------------------------------------------
# NAT GATEWAY
resource "aws_eip" "attacker-nat-eip" {
  # NOTE: Do not use network_interface to associate aws_eip to aws_nat_gateway
  associate_with_private_ip = local.attacker-nat-ip

  public_ipv4_pool = "amazon"
  domain           = "vpc"
  depends_on       = [aws_internet_gateway.attacker-igw]
  tags             = merge(module.defaults.tags, { Name = local.attacker-eip-name })
}

# Create NAT gateway
resource "aws_nat_gateway" "attacker-nat" {
  allocation_id = aws_eip.attacker-nat-eip.id
  subnet_id     = aws_subnet.attacker-nat.id
  depends_on    = [aws_internet_gateway.attacker-igw]
  tags          = merge(module.defaults.tags, { Name = local.attacker-nat-name })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

# --------------------------------------------------------------------------------------
# Route tables
# --------------------------------------------------------------------------------------
# Private (default)
resource "aws_default_route_table" "attacker-rtb" {
  default_route_table_id = aws_vpc.attacker.default_route_table_id

  route {
    destination_prefix_list_id = aws_ec2_managed_prefix_list.victim.id
    vpc_peering_connection_id  = aws_vpc_peering_connection.red2victim-pcx.id
  }

  # ALL TRAFFIC --> NAT
  route {
    cidr_block     = local.egress-cidr
    nat_gateway_id = aws_nat_gateway.attacker-nat.id
  }
  tags = merge(module.defaults.tags, { Name = local.attacker-rtb-name })
}

# Public
resource "aws_route_table" "attacker-rtb-public" {
  vpc_id = aws_vpc.attacker.id

  route {
    cidr_block                = aws_vpc.victim.cidr_block
    vpc_peering_connection_id = aws_vpc_peering_connection.red2victim-pcx.id
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
resource "aws_route_table_association" "rtba-attacker" {
  subnet_id      = aws_subnet.attacker.id
  route_table_id = aws_vpc.attacker.default_route_table_id
}

# PUBLIC **
resource "aws_route_table_association" "rtba-attacker-nat" {
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

  ingress {
    from_port = 0
    to_port   = 0
    protocol  = "-1"
    self      = true
  }

  ingress {
    from_port       = 0
    to_port         = 0
    protocol        = "-1"
    prefix_list_ids = [aws_ec2_managed_prefix_list.victim.id, aws_ec2_managed_prefix_list.ext-benev.id, aws_ec2_managed_prefix_list.attacker.id]
  }

  # allow traffic from within VPC
  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  lifecycle {
    create_before_destroy = true
    ignore_changes        = [tags, tags_all, egress]
  }
  depends_on = [aws_vpc.attacker]
  tags = merge(module.defaults.tags, {
    Name        = local.attacker-sg-name
    Description = local.attacker-sg-description
  })
}
