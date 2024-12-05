# _____________________________________________________________________________________
#
# ext-benev NETWORKING
#
# _____________________________________________________________________________________

locals {
  ext-benev-vpc-name    = "${var.name-prefix}-benev-vpc"
  ext-benev-sub-name    = "${var.name-prefix}-benev-sub"
  ext-benev-sub-name-re = "${var.name-prefix}-benev-sub-redirect"
  ext-benev-sg-name     = "${var.name-prefix}-benev-sg"
  ext-benev-igw-name    = "${var.name-prefix}-benev-igw"
  ext-benev-nat-name    = "${var.name-prefix}-benev-nat"
  ext-benev-eip-name    = "${var.name-prefix}-benev-nat-eip"
  ext-benev-rtb-name    = "${var.name-prefix}-benev-rtb"

  ext-benev-vpc-description = "ER6 Benevolent Range"
  ext-benev-sub-description = "ER6 Benevolent Subnet"
  ext-benev-sg-description  = "ER6 Benevolent Security Group"
  ext-benev-nat-description = "ER6 Benevolent NAT"

  ext-benev-redirect1-prefix = "14.121"
  ext-benev-redirect1-cidr   = "${local.ext-benev-redirect1-prefix}.0.0/16"
  ext-benev-redirect1-ip1    = "${local.ext-benev-redirect1-prefix}.222.9"
  ext-benev-redirect1-ip2    = "${local.ext-benev-redirect1-prefix}.222.11"
  ext-benev-redirect1-ip3    = "${local.ext-benev-redirect1-prefix}.201.33"

  ext-benev-redirect2-prefix = "27.21"
  ext-benev-redirect2-cidr   = "${local.ext-benev-redirect2-prefix}.12.0/24"
  ext-benev-redirect2-ip1    = "${local.ext-benev-redirect2-prefix}.12.9"
  ext-benev-redirect2-ip2    = "${local.ext-benev-redirect2-prefix}.12.21"
  ext-benev-redirect2-ip3    = "${local.ext-benev-redirect2-prefix}.12.54"
  ext-benev-redirect2-ip4    = "${local.ext-benev-redirect2-prefix}.12.94"

  ext-benev-redirect3-prefix = "199.88"
  ext-benev-redirect3-cidr   = "${local.ext-benev-redirect3-prefix}.44.0/24"


  ext-benev-redirect4-prefix = "36.43"
  ext-benev-redirect4-cidr   = "${local.ext-benev-redirect4-prefix}.0.0/16"

}

######################################################################################
# Prefix List for Ext-Benevolent
######################################################################################
resource "aws_ec2_managed_prefix_list" "ext-benev" {
  name           = "External benevolent prefix list for routing"
  address_family = "IPv4"
  max_entries    = 5

  entry {
    cidr        = local.ext-benev-redirect1-cidr
    description = "Benevolent Redirect 1"
  }

  entry {
    cidr        = local.ext-benev-redirect2-cidr
    description = "Benevolent Redirect 2"
  }

  entry {
    cidr        = local.ext-benev-redirect3-cidr
    description = "Benevolent Redirect 3"
  }

  entry {
    cidr        = local.ext-benev-redirect4-cidr
    description = "Benevolent Redirect 4"
  }

  tags = merge(module.defaults.tags, { Name = "External Benevolent Prefix List" })
}

######################################################################################
# VPC
######################################################################################
# resource "aws_vpc" "ext-benev" {
#   cidr_block           = local.ext-benev-redirect1-cidr
#   enable_dns_hostnames = true
#   enable_dns_support   = true
#   tags                 = merge(module.defaults.tags, { Name = local.ext-benev-vpc-name, Description = local.ext-benev-vpc-description })
#   lifecycle {
#     ignore_changes = [tags, tags_all]
#   }
# }

resource "aws_vpc_ipv4_cidr_block_association" "ext-benev-redirect1-cidr" {
  vpc_id     = aws_vpc.attacker.id
  cidr_block = local.ext-benev-redirect1-cidr
}

resource "aws_vpc_ipv4_cidr_block_association" "ext-benev-redirect2-cidr" {
  vpc_id     = aws_vpc.attacker.id
  cidr_block = local.ext-benev-redirect2-cidr
}

resource "aws_vpc_ipv4_cidr_block_association" "ext-benev-redirect3-cidr" {
  vpc_id     = aws_vpc.attacker.id
  cidr_block = local.ext-benev-redirect3-cidr
}

resource "aws_vpc_ipv4_cidr_block_association" "ext-benev-redirect4-cidr" {
  vpc_id     = aws_vpc.attacker.id
  cidr_block = local.ext-benev-redirect4-cidr
}


# ------------------------------------------------------------------------------------
## SUBNET: Main
resource "aws_subnet" "ext-benev-redirect1" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.ext-benev-redirect1-cidr.vpc_id
  cidr_block        = local.ext-benev-redirect1-cidr
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.ext-benev-sub-name, Description = local.ext-benev-sub-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

resource "aws_subnet" "ext-benev-redirect2" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.ext-benev-redirect2-cidr.vpc_id
  cidr_block        = local.ext-benev-redirect2-cidr
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.ext-benev-sub-name, Description = local.ext-benev-sub-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

resource "aws_subnet" "ext-benev-redirect3" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.ext-benev-redirect3-cidr.vpc_id
  cidr_block        = local.ext-benev-redirect3-cidr
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.ext-benev-sub-name, Description = local.ext-benev-sub-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

resource "aws_subnet" "ext-benev-redirect4" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.ext-benev-redirect4-cidr.vpc_id
  cidr_block        = local.ext-benev-redirect4-cidr
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.ext-benev-sub-name, Description = local.ext-benev-sub-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

# --------------------------------------------------------------------------------------
# Route tables
# --------------------------------------------------------------------------------------
# Private (default)
# resource "aws_default_route_table" "ext-benev-rtb" {
#   default_route_table_id = aws_vpc.ext-benev.default_route_table_id
#
#   route {
#     cidr_block                = aws_vpc.victim.cidr_block
#     vpc_peering_connection_id = aws_vpc_peering_connection.victim2benevolent-pcx.id
#   }
#
#   # ALL TRAFFIC --> Attacker NAT
#   route {
#     cidr_block     = local.egress-cidr
#     nat_gateway_id = aws_nat_gateway.attacker-nat.id
#   }
#   tags = merge(module.defaults.tags, { Name = local.ext-benev-rtb-name })
# }

######################################################################################
# SECURITY GROUP
######################################################################################
# resource "aws_security_group" "ext-benev" {
#   name_prefix = local.ext-benev-sg-name
#   description = local.ext-benev-sg-description
#   vpc_id      = aws_vpc.ext-benev.id
#
#   ingress {
#     from_port = 0
#     to_port   = 0
#     protocol  = "-1"
#     self      = true
#   }
#
#   # allow traffic from within VPC
#   egress {
#     from_port   = 0
#     to_port     = 0
#     protocol    = "-1"
#     cidr_blocks = ["0.0.0.0/0"]
#   }
#
#   lifecycle {
#     create_before_destroy = true
#     ignore_changes        = [tags, tags_all, egress]
#   }
#   depends_on = [aws_vpc.ext-benev]
#   tags = merge(module.defaults.tags, {
#     Name        = local.ext-benev-sg-name
#     Description = local.ext-benev-sg-description
#   })
# }
