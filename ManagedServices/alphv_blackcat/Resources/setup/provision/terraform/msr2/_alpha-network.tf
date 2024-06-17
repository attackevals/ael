######################################################################################
## VPC
######################################################################################
resource "aws_vpc" "alpha" {
  cidr_block           = local.cidr.alpha
  enable_dns_hostnames = true
  enable_dns_support   = true
  tags                 = merge(module.defaults.tags, { Name = local.name.alpha.vpc })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }

}

resource "aws_vpc_ipv4_cidr_block_association" "alpha-a" {
  vpc_id     = aws_vpc.alpha.id
  cidr_block = local.cidr.alpha-a
}

resource "aws_vpc_ipv4_cidr_block_association" "alpha-b" {
  vpc_id     = aws_vpc.alpha.id
  cidr_block = local.cidr.alpha-b
}

resource "aws_vpc_ipv4_cidr_block_association" "alpha-c" {
  vpc_id     = aws_vpc.alpha.id
  cidr_block = local.cidr.alpha-c
}

resource "aws_vpc_ipv4_cidr_block_association" "alpha-nat" {
  vpc_id     = aws_vpc.alpha.id
  cidr_block = local.cidr.alpha-nat
}

# Create Internet Gateway on ALPHA VPC
resource "aws_internet_gateway" "alpha-igw" {
  vpc_id = aws_vpc.alpha.id
  tags   = merge(module.defaults.tags, { Name = local.name.alpha.igw })
}

######################################################################################
## NAT GATEWAY
######################################################################################
# Create Elastic IP
resource "aws_eip" "alpha-nat-eip" {
  associate_with_private_ip = local.ip.a.nat-private

  public_ipv4_pool = "amazon"
  domain           = "vpc"
  depends_on       = [aws_internet_gateway.alpha-igw]
  tags             = merge(module.defaults.tags, { Name = local.name.alpha.eip })
}

# Create NAT gateway
resource "aws_nat_gateway" "alpha-nat-gw" {
  allocation_id = aws_eip.alpha-nat-eip.id
  subnet_id     = aws_subnet.alpha_nat.id
  depends_on    = [aws_internet_gateway.alpha-igw]
  tags          = merge(module.defaults.tags, { Name = local.name.alpha.nat })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }

}

######################################################################################
## SUBNETS - 6 subnets total (3 simulated VPCs)
######################################################################################

# [A] server
resource "aws_subnet" "a_srv" {
  vpc_id            = aws_vpc.alpha.id
  cidr_block        = local.cidr.a.srv
  availability_zone = var.aws-region-az
  depends_on        = [aws_vpc_ipv4_cidr_block_association.alpha-a]
  tags              = merge(module.defaults.tags, { Name = local.name.subnet.a.srv })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }

}
# [A] desk
resource "aws_subnet" "a_desk" {
  vpc_id            = aws_vpc.alpha.id
  cidr_block        = local.cidr.a.desk
  availability_zone = var.aws-region-az
  depends_on        = [aws_vpc_ipv4_cidr_block_association.alpha-a]
  tags              = merge(module.defaults.tags, { Name = local.name.subnet.a.desk })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }

}
# [A] vpn
resource "aws_subnet" "a_vpn" {
  vpc_id            = aws_vpc.alpha.id
  cidr_block        = local.cidr.a.vpn
  availability_zone = var.aws-region-az
  depends_on        = [aws_vpc_ipv4_cidr_block_association.alpha-a]
  tags              = merge(module.defaults.tags, { Name = local.name.subnet.a.vpn })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }

}
# [A] nat
resource "aws_subnet" "alpha_nat" {
  map_public_ip_on_launch = true

  vpc_id            = aws_vpc.alpha.id
  cidr_block        = local.cidr.vendor-nat
  availability_zone = var.aws-region-az
  depends_on        = [aws_internet_gateway.alpha-igw, aws_vpc_ipv4_cidr_block_association.alpha-a]
  tags              = merge(module.defaults.tags, { Name = local.name.subnet.a.nat })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }

}

# [B] all (single subnet)
resource "aws_subnet" "b_combined" {
  vpc_id            = aws_vpc.alpha.id
  cidr_block        = local.cidr.b.combined
  availability_zone = var.aws-region-az
  depends_on        = [aws_vpc_ipv4_cidr_block_association.alpha-b]
  tags              = merge(module.defaults.tags, { Name = local.name.subnet.b.combined })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

# [C] desk
resource "aws_subnet" "c_desk" {
  vpc_id            = aws_vpc.alpha.id
  cidr_block        = local.cidr.c.desk
  availability_zone = var.aws-region-az
  depends_on        = [aws_vpc_ipv4_cidr_block_association.alpha-c]
  tags              = merge(module.defaults.tags, { Name = local.name.subnet.c.desk })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

# --------------------------------------------------------------------------------------
# Routes (via peering connections)
# --------------------------------------------------------------------------------------
# Private (default)
resource "aws_default_route_table" "alpha-rtb" {
  default_route_table_id = aws_vpc.alpha.default_route_table_id

  #  # BETA --> ALPHA
  #  route {
  #    cidr_block                = aws_vpc.beta.cidr_block
  #    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  #  }
  # Redirection for A
  route {
    cidr_block                = local.cidr.redirect-a
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  # Redirection for B
  route {
    cidr_block                = local.cidr.redirect-b
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  # Redirection for Red network (Kali)
  route {
    cidr_block                = local.cidr.beta
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }

  # ALL TRAFFIC --> ALPHA NAT
  route {
    cidr_block     = local.cidr.all_egress
    nat_gateway_id = aws_nat_gateway.alpha-nat-gw.id
  }
  tags = merge(module.defaults.tags, { Name = local.name.alpha.rtb })
}

# Public (to gateway)
resource "aws_route_table" "alpha-public-rtb" {
  vpc_id = aws_vpc.alpha.id

  #  # BETA --> ALPHA
  # Redirection for A
  route {
    cidr_block                = local.cidr.redirect-a
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  # Redirection for B
  route {
    cidr_block                = local.cidr.redirect-b
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  route {
    cidr_block                = local.cidr.beta
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }

  # NAT TRAFFIC --> ALPHA IGW
  route {
    cidr_block = local.cidr.all_egress
    gateway_id = aws_internet_gateway.alpha-igw.id
  }
}

# --------------------------------------------------------------------------------------
# Route table associations (subnet-level)
# --------------------------------------------------------------------------------------
# [A] private
resource "aws_route_table_association" "rtba_a_srv" {
  subnet_id      = aws_subnet.a_srv.id
  route_table_id = aws_vpc.alpha.default_route_table_id
}
# [A] private
resource "aws_route_table_association" "rtba_a_desk" {
  subnet_id      = aws_subnet.a_desk.id
  route_table_id = aws_vpc.alpha.default_route_table_id
}
# [A] private
resource "aws_route_table_association" "rtba_a_vpn" {
  subnet_id      = aws_subnet.a_vpn.id
  route_table_id = aws_vpc.alpha.default_route_table_id
}
# [A] PUBLIC **
resource "aws_route_table_association" "rtba_a_nat" {
  subnet_id      = aws_subnet.alpha_nat.id
  route_table_id = aws_route_table.alpha-public-rtb.id
}
# [B] private
resource "aws_route_table_association" "rtba_b_combined" {
  subnet_id      = aws_subnet.b_combined.id
  route_table_id = aws_vpc.alpha.default_route_table_id
}
# [C] private
resource "aws_route_table_association" "rtba_c_desk" {
  subnet_id      = aws_subnet.c_desk.id
  route_table_id = aws_vpc.alpha.default_route_table_id
}
