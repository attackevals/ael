######################################################################################
## VPC
######################################################################################
resource "aws_vpc" "beta" {
  cidr_block           = local.cidr.beta
  enable_dns_hostnames = true
  enable_dns_support   = true
  tags                 = merge(module.defaults.tags, { Name = local.name.beta.vpc })
}

resource "aws_vpc_ipv4_cidr_block_association" "redirect-a-cidr" {
  vpc_id     = aws_vpc.beta.id
  cidr_block = local.cidr.redirect-a
}

resource "aws_vpc_ipv4_cidr_block_association" "redirect-b-cidr" {
  vpc_id     = aws_vpc.beta.id
  cidr_block = local.cidr.redirect-b
}

resource "aws_vpc_ipv4_cidr_block_association" "beta-nat" {
  vpc_id     = aws_vpc.beta.id
  cidr_block = local.cidr.beta-nat
}

# Create Internet Gateway to BETA VPC
resource "aws_internet_gateway" "beta-igw" {
  vpc_id = aws_vpc.beta.id
  tags   = merge(module.defaults.tags, { Name = local.name.beta.igw })
}

######################################################################################
## NAT GATEWAY
######################################################################################
# Create Elastic IP
resource "aws_eip" "beta-nat-eip" {
  associate_with_private_ip = local.ip.support.nat-private

  public_ipv4_pool = "amazon"
  domain           = "vpc"
  depends_on       = [aws_internet_gateway.beta-igw]
  tags             = merge(module.defaults.tags, { Name = local.name.beta.eip })
}

# Create NAT gateway
resource "aws_nat_gateway" "beta-nat-gw" {
  allocation_id = aws_eip.beta-nat-eip.id
  subnet_id     = aws_subnet.beta_nat.id
  depends_on    = [aws_internet_gateway.beta-igw]
  tags          = merge(module.defaults.tags, { Name = local.name.beta.nat })
}

######################################################################################
## SUBNETS - 4 (red, support, nat)
######################################################################################
# [RED/SUPPORT] red
resource "aws_subnet" "support_red" {
  vpc_id            = aws_vpc.beta.id
  cidr_block        = local.cidr.support-red
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.name.subnet.red.main })
}
# [RED/SUPPORT] redirect-a
resource "aws_subnet" "support_a" {
  vpc_id            = aws_vpc.beta.id
  cidr_block        = local.cidr.redirect-a
  availability_zone = var.aws-region-az
  depends_on        = [aws_vpc_ipv4_cidr_block_association.redirect-a-cidr]
  tags              = merge(module.defaults.tags, { Name = local.name.subnet.support.redirect-a })
}
# [RED/SUPPORT] redirect-b
resource "aws_subnet" "support_b" {
  vpc_id            = aws_vpc.beta.id
  cidr_block        = local.cidr.redirect-b
  availability_zone = var.aws-region-az
  depends_on        = [aws_vpc_ipv4_cidr_block_association.redirect-b-cidr]
  tags              = merge(module.defaults.tags, { Name = local.name.subnet.support.redirect-b })
}
# [RED/SUPPORT] nat
resource "aws_subnet" "beta_nat" {
  map_public_ip_on_launch = true

  vpc_id            = aws_vpc.beta.id
  cidr_block        = local.cidr.support-nat
  availability_zone = var.aws-region-az
  depends_on        = [aws_internet_gateway.beta-igw, aws_vpc_ipv4_cidr_block_association.redirect-b-cidr]
  tags              = merge(module.defaults.tags, { Name = local.name.subnet.support.nat })
}
# --------------------------------------------------------------------------------------
# Routes (via peering connections)
# --------------------------------------------------------------------------------------
resource "aws_default_route_table" "beta-rtb" {
  default_route_table_id = aws_vpc.beta.default_route_table_id

  # ALPHA --> BETA
  route {
    cidr_block                = local.cidr.alpha
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  route {
    cidr_block                = local.cidr.alpha-a
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  route {
    cidr_block                = local.cidr.alpha-b
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  route {
    cidr_block                = local.cidr.alpha-c
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  route {
    cidr_block                = local.cidr.alpha-nat
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }

  # ALL TRAFFIC --> BETA NAT
  route {
    cidr_block     = local.cidr.all_egress
    nat_gateway_id = aws_nat_gateway.beta-nat-gw.id
  }
  tags = merge(module.defaults.tags, { Name = local.name.beta.rtb })
}

# Public (to gateway)
resource "aws_route_table" "beta-public-rtb" {
  vpc_id = aws_vpc.beta.id
  # BETA --> ALPHA
  route {
    cidr_block                = local.cidr.alpha
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  route {
    cidr_block                = local.cidr.alpha-a
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  route {
    cidr_block                = local.cidr.alpha-b
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  route {
    cidr_block                = local.cidr.alpha-c
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }
  route {
    cidr_block                = local.cidr.alpha-nat
    vpc_peering_connection_id = aws_vpc_peering_connection.range-to-red-pcx.id
  }

  # NAT TRAFFIC --> ALPHA IGW
  route {
    cidr_block = local.cidr.all_egress
    gateway_id = aws_internet_gateway.beta-igw.id
  }
}

# --------------------------------------------------------------------------------------
# Route table association
# --------------------------------------------------------------------------------------
# [RED/SUPPORT] private
resource "aws_route_table_association" "rtba_red" {
  subnet_id      = aws_subnet.support_red.id
  route_table_id = aws_vpc.beta.default_route_table_id
}
# [RED/SUPPORT] private
resource "aws_route_table_association" "rtba_redirect_a" {
  subnet_id      = aws_subnet.support_a.id
  route_table_id = aws_vpc.beta.default_route_table_id
}
# [RED/SUPPORT] private
resource "aws_route_table_association" "rtba_redirect_b" {
  subnet_id      = aws_subnet.support_b.id
  route_table_id = aws_vpc.beta.default_route_table_id
}
# [RED/SUPPORT] PUBLIC **
resource "aws_route_table_association" "rtba_support_nat" {
  subnet_id      = aws_subnet.beta_nat.id
  route_table_id = aws_route_table.beta-public-rtb.id
}
