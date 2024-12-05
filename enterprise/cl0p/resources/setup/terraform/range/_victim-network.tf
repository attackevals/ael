# _____________________________________________________________________________________
#
# VICTIM NETWORKING
#
# _____________________________________________________________________________________

locals {
  victim-vpc-name              = "${var.name-prefix}-vpc"
  victim-sub-name              = "${var.name-prefix}-subnet"
  victim-sg-name               = "${var.name-prefix}-sg"
  victim-igw-name              = "${var.name-prefix}-igw"
  victim-nat-name              = "${var.name-prefix}-nat"
  victim-vpn-name              = "${var.name-prefix}-vpn"
  victim-eip-name              = "${var.name-prefix}-nat-eip"
  victim-rtb-name              = "${var.name-prefix}-rtb"
  victim-a-ip-prefix           = "10.55"
  victim-b-ip-prefix           = "10.111"
  victim-protections-ip-prefix = "10.222"
  vpn-sub-ip-prefix            = "10.33"
  nat-sub-ip-prefix            = "10.99"

  victim-vpc-description = "[ER6] Victim Range"
  victim-a = {
    vpc-cidr        = "${local.victim-a-ip-prefix}.0.0/17"
    subnet-cidr     = "${local.victim-a-ip-prefix}.0.0/19"
    sub-description = "[Scenario A] Victim Subnet"
    sg-description  = "[Scenario A] Victim Security Group"
    sub-name        = "Scenario A subnet"
  }

  victim-b = {
    vpc-cidr        = "${local.victim-b-ip-prefix}.0.0/17"
    subnet-cidr     = "${local.victim-b-ip-prefix}.0.0/19"
    sub-description = "[Scenario B] Victim Subnet"
    sg-description  = "[Scenario B] Victim Security Group"
    sub-name        = "Scenario B subnet"
  }

  victim-protections = {
    vpc-cidr        = "${local.victim-protections-ip-prefix}.0.0/17"
    subnet-cidr     = "${local.victim-protections-ip-prefix}.0.0/19"
    sub-description = "[Scenario Protections] Victim Subnet"
    sg-description  = "[Scenario Protections] Victim Security Group"
    sub-name        = "Scenario Protections subnet"
  }

  vpn-sub = {
    vpc-cidr        = "${local.vpn-sub-ip-prefix}.0.0/17"
    subnet-cidr     = "${local.vpn-sub-ip-prefix}.0.0/19"
    sub-description = "[VPN] Subnet"
    sg-description  = "[VPN] Security Group"
    sub-name        = "VPN subnet"

  }

  nat-sub = {
    vpc-cidr        = "${local.nat-sub-ip-prefix}.0.0/17"
    subnet-cidr     = "${local.nat-sub-ip-prefix}.0.0/19"
    sub-description = "[NAT] Subnet"
    sg-description  = "[NAT] Security Group"
    nat-ip          = "${local.nat-sub-ip-prefix}.1.64"
    sub-name        = "NAT subnet"

  }
}

####
# outbound prefix list reference
####
data "aws_ec2_managed_prefix_list" "outbound" {
  name = "er6-egress-allowed-dev-list"
}

######################################################################################
# Prefix List for Victim
######################################################################################
resource "aws_ec2_managed_prefix_list" "victim" {
  name           = "Victim prefix list for routing"
  address_family = "IPv4"
  max_entries    = 5

  entry {
    cidr        = local.victim-a.vpc-cidr
    description = "Victim A"
  }

  entry {
    cidr        = local.victim-b.vpc-cidr
    description = "Victim B"
  }

  entry {
    cidr        = local.victim-protections.vpc-cidr
    description = "Victim Protections"
  }
  entry {
    cidr        = local.vpn-sub.vpc-cidr
    description = "VPN"
  }

  tags = merge(module.defaults.tags, { Name = "Victim Prefix List" })
}

######################################################################################
# VPC
######################################################################################
resource "aws_vpc" "victim" {
  cidr_block           = local.victim-a.vpc-cidr
  enable_dns_hostnames = true
  enable_dns_support   = true

  tags = merge(module.defaults.tags, { Name = local.victim-vpc-name, Description = local.victim-vpc-description }, local.cost-tag)
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

# associate all CIDRs with VPC

resource "aws_vpc_ipv4_cidr_block_association" "b_cidr" {
  cidr_block = local.victim-b.vpc-cidr
  vpc_id     = aws_vpc.victim.id
}

resource "aws_vpc_ipv4_cidr_block_association" "protections_cidr" {
  cidr_block = local.victim-protections.vpc-cidr
  vpc_id     = aws_vpc.victim.id
}

resource "aws_vpc_ipv4_cidr_block_association" "nat_cidr" {
  cidr_block = local.nat-sub.vpc-cidr
  vpc_id     = aws_vpc.victim.id
}

resource "aws_vpc_ipv4_cidr_block_association" "vpn_cidr" {
  cidr_block = local.vpn-sub.vpc-cidr
  vpc_id     = aws_vpc.victim.id
}




# ------------------------------------------------------------------------------------
## SUBNET

### VICTIMS - A
resource "aws_subnet" "victim-a" {
  vpc_id            = aws_vpc.victim.id
  cidr_block        = local.victim-a.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.victim-sub-name, Description = local.victim-a.sub-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

### VICTIMS - B
resource "aws_subnet" "victim-b" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.b_cidr.vpc_id ## <-- recommended per: https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet - Subnets In Secondary VPC CIDR Blocks section
  cidr_block        = local.victim-b.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.victim-sub-name, Description = local.victim-b.sub-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

### VICTIMS - PROTECTIONS
resource "aws_subnet" "victim-protections" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.protections_cidr.vpc_id ## <-- recommended per: https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet - Subnets In Secondary VPC CIDR Blocks section
  cidr_block        = local.victim-protections.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.victim-sub-name, Description = local.victim-protections.sub-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

## SUBNET NAT
resource "aws_subnet" "victim-nat" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.nat_cidr.vpc_id ## <-- recommended per: https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet - Subnets In Secondary VPC CIDR Blocks section
  cidr_block        = local.nat-sub.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.victim-nat-name, Description = local.nat-sub.sub-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

## SUBNET VPN (& CLIENT)
resource "aws_subnet" "vpn" {
  vpc_id            = aws_vpc_ipv4_cidr_block_association.vpn_cidr.vpc_id ## <-- recommended per: https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/subnet - Subnets In Secondary VPC CIDR Blocks section
  cidr_block        = local.vpn-sub.subnet-cidr
  availability_zone = var.aws-region-az
  tags              = merge(module.defaults.tags, { Name = local.victim-vpn-name, Description = local.vpn-sub.sub-description })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

## openvpn
module "openvpn-client" {
  source            = "../../modules/aws/vpn-client"
  name              = var.name-prefix
  organization_name = local.vendor
  cidr              = local.openvpn-cidr

  target_subnet_id  = aws_subnet.vpn.id
  subnet_ids        = [aws_subnet.vpn.id]
  vpc_id            = aws_vpc.victim.id
  security_group_id = aws_security_group.victim-a.id

  allowed_cidr_ranges = [
    local.victim-a.vpc-cidr,
    local.victim-b.vpc-cidr,
    local.victim-protections.vpc-cidr,
    aws_vpc.attacker.cidr_block,
    aws_subnet.attacker-nat.cidr_block,
    aws_subnet.ext-benev-redirect1.cidr_block,
    aws_subnet.ext-benev-redirect2.cidr_block,
    aws_subnet.ext-benev-redirect3.cidr_block,
    aws_subnet.ext-benev-redirect4.cidr_block,
    local.attacker-redirect-cidr
  ]
  target_cidr_blocks = [aws_vpc.attacker.cidr_block,
    local.attacker-redirect-cidr,
    local.ext-benev-redirect1-cidr,
    local.ext-benev-redirect2-cidr,
    local.ext-benev-redirect3-cidr,
  local.ext-benev-redirect4-cidr]

  split_tunnel = true
  depends_on   = [aws_security_group.victim-a]
  tags         = merge(module.defaults.tags, { Name : local.victim-vpn-name })
}

## create security group ingress rule, allowing traffic tagged with vpn security group access to victim
resource "aws_vpc_security_group_ingress_rule" "main" {
  security_group_id            = aws_security_group.victim-a.id
  referenced_security_group_id = module.openvpn-client.vpn_security_group_id
  ip_protocol                  = "-1"
}

# ------------------------------------------------------------------------------------
# INTERNET GATEWAY
resource "aws_internet_gateway" "victim-igw" {
  vpc_id = aws_vpc.victim.id
  tags   = merge(module.defaults.tags, { Name = local.victim-igw-name })
}

# ------------------------------------------------------------------------------------
# NAT GATEWAY
resource "aws_eip" "victim-nat-eip" {
  # NOTE: Do not use network_interface to associate aws_eip to aws_nat_gateway
  associate_with_private_ip = local.nat-sub.nat-ip

  public_ipv4_pool = "amazon"
  domain           = "vpc"
  depends_on       = [aws_internet_gateway.victim-igw]
  tags             = merge(module.defaults.tags, { Name = local.victim-eip-name })
}

# Create NAT gateway
resource "aws_nat_gateway" "victim-nat" {
  allocation_id = aws_eip.victim-nat-eip.id
  subnet_id     = aws_subnet.victim-nat.id
  depends_on    = [aws_internet_gateway.victim-igw]
  tags          = merge(module.defaults.tags, { Name = local.victim-nat-name })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

# --------------------------------------------------------------------------------------
# Route tables
# --------------------------------------------------------------------------------------
# Private (default)
resource "aws_default_route_table" "victim-rtb" {
  default_route_table_id = aws_vpc.victim.default_route_table_id

  route {
    destination_prefix_list_id = aws_ec2_managed_prefix_list.attacker.id
    vpc_peering_connection_id  = aws_vpc_peering_connection.red2victim-pcx.id
  }

  route {
    destination_prefix_list_id = aws_ec2_managed_prefix_list.ext-benev.id
    vpc_peering_connection_id  = aws_vpc_peering_connection.red2victim-pcx.id
  }

  # ALL TRAFFIC --> NAT
  route {
    cidr_block     = local.egress-cidr
    nat_gateway_id = aws_nat_gateway.victim-nat.id
  }
  tags = merge(module.defaults.tags, { Name = local.victim-rtb-name })
}

# Public
resource "aws_route_table" "victim-rtb-public" {
  vpc_id = aws_vpc.victim.id

  route {
    destination_prefix_list_id = aws_ec2_managed_prefix_list.attacker.id
    vpc_peering_connection_id  = aws_vpc_peering_connection.red2victim-pcx.id
  }

  route {
    destination_prefix_list_id = aws_ec2_managed_prefix_list.ext-benev.id
    vpc_peering_connection_id  = aws_vpc_peering_connection.red2victim-pcx.id
  }

  # NAT TRAFFIC --> IGW
  route {
    cidr_block = local.egress-cidr
    gateway_id = aws_internet_gateway.victim-igw.id
  }
}

# --------------------------------------------------------------------------------------
# Route table associations (subnet-level)
# --------------------------------------------------------------------------------------
# private
resource "aws_route_table_association" "rtba-victim" {
  subnet_id      = aws_subnet.victim-a.id
  route_table_id = aws_vpc.victim.default_route_table_id
}

# PUBLIC **
resource "aws_route_table_association" "rtba-victim-nat" {
  subnet_id      = aws_subnet.victim-nat.id
  route_table_id = aws_route_table.victim-rtb-public.id
}

# private
resource "aws_route_table_association" "rtba-vpn" {
  subnet_id      = aws_subnet.vpn.id
  route_table_id = aws_vpc.victim.default_route_table_id
}


######################################################################################
# SECURITY GROUP
######################################################################################
resource "aws_security_group" "victim-a" {
  name_prefix = local.victim-sg-name
  description = local.victim-a.sg-description
  vpc_id      = aws_vpc.victim.id

  ingress {
    from_port = 0
    to_port   = 0
    protocol  = "-1"
    self      = true
  }

  ingress {
    from_port = 0
    to_port   = 0
    protocol  = "-1"
    prefix_list_ids = [
      aws_ec2_managed_prefix_list.attacker.id,
      aws_ec2_managed_prefix_list.ext-benev.id,
      aws_ec2_managed_prefix_list.victim.id
    ]
  }


  # allow traffic from within VPC

  lifecycle {
    create_before_destroy = true
    ignore_changes        = [tags, tags_all]
  }
  depends_on = [aws_vpc.victim]
  tags = merge(module.defaults.tags, {
    Name        = local.victim-sg-name
    Description = local.victim-a.sg-description
  })
}

resource "aws_security_group" "victim-b" {
  name_prefix = local.victim-sg-name
  description = local.victim-b.sg-description
  vpc_id      = aws_vpc.victim.id

  ingress {
    from_port = 0
    to_port   = 0
    protocol  = "-1"
    self      = true
  }

  ingress {
    from_port = 0
    to_port   = 0
    protocol  = "-1"
    prefix_list_ids = [
      aws_ec2_managed_prefix_list.attacker.id,
      aws_ec2_managed_prefix_list.ext-benev.id,
      aws_ec2_managed_prefix_list.victim.id
    ]
  }


  # allow traffic from internal VPC SG
  ingress {
    from_port       = 0
    to_port         = 0
    security_groups = [aws_security_group.attacker.id]
    protocol        = "-1"
  }

  # allow traffic from within VPC

  lifecycle {
    #     create_before_destroy = true
    ignore_changes = [tags, tags_all]
  }
  depends_on = [aws_vpc.victim]
  tags = merge(module.defaults.tags, {
    Name        = local.victim-sg-name
    Description = local.victim-b.sg-description
  })
}

resource "aws_security_group" "victim-protections" {
  name_prefix = local.victim-sg-name
  description = local.victim-protections.sg-description
  vpc_id      = aws_vpc.victim.id

  ingress {
    from_port = 0
    to_port   = 0
    protocol  = "-1"
    self      = true
  }

  ingress {
    from_port = 0
    to_port   = 0
    protocol  = "-1"
    prefix_list_ids = [
      aws_ec2_managed_prefix_list.attacker.id,
      aws_ec2_managed_prefix_list.ext-benev.id,
      aws_ec2_managed_prefix_list.victim.id
    ]
  }

  lifecycle {
    #     create_before_destroy = true
    ignore_changes = [tags, tags_all]
  }
  depends_on = [aws_vpc.victim]
  tags = merge(module.defaults.tags, {
    Name        = local.victim-sg-name
    Description = local.victim-protections.sg-description
  })
}

# resource "aws_security_group_rule" "outbound-lockdown-sgva" {
#   from_port = 0
#   to_port   = 0
#   protocol  = "-1"
#
#   # block internet except for jfrog, splunk, wazuh
#   prefix_list_ids = [data.aws_ec2_managed_prefix_list.outbound.id]
#
#   security_group_id = aws_security_group.victim-a.id
#   type              = "egress"
# }
# resource "aws_security_group_rule" "outbound-lockdown-sgvb" {
#   from_port = 0
#   to_port   = 0
#   protocol  = "-1"
#
#   # block internet except for jfrog, splunk, wazuh
#   prefix_list_ids = [data.aws_ec2_managed_prefix_list.outbound.id]
#
#   # open internet
#   # cidr_blocks = ["0.0.0.0/0"]
#
#   security_group_id = aws_security_group.victim-b.id
#   type              = "egress"
# }
#
# resource "aws_security_group_rule" "outbound-lockdown-sgvp" {
#   from_port = 0
#   to_port   = 0
#   protocol  = "-1"
#
#   # block internet except for jfrog, splunk, wazuh
#   prefix_list_ids = [data.aws_ec2_managed_prefix_list.outbound.id]
#
#   # open internet
#   # cidr_blocks = ["0.0.0.0/0"]
#
#   security_group_id = aws_security_group.victim-protections.id
#   type              = "egress"
# }
#
# resource "aws_security_group_rule" "outbound-lockdown-sgvred" {
#   from_port = 0
#   to_port   = 0
#   protocol  = "-1"
#
#   # block internet except for jfrog, splunk, wazuh
#   prefix_list_ids = [data.aws_ec2_managed_prefix_list.outbound.id]
#   security_group_id = aws_security_group.attacker.id
#   type              = "egress"
# }

resource "aws_security_group" "allow-outbound-victim" {
  name_prefix = "${local.victim-sg-name}-outbound-victim-allow"
  description = local.victim-a.sg-description
  vpc_id      = aws_vpc.victim.id
}

resource "aws_security_group" "block-outbound-victim" {
  name_prefix = "${local.victim-sg-name}-outbound-victim-block"
  description = local.victim-a.sg-description
  vpc_id      = aws_vpc.victim.id
}

resource "aws_security_group" "allow-outbound-attacker" {
  name_prefix = "${local.attacker-sg-name}-outbound-attacker-allow"
  description = local.attacker-sg-description
  vpc_id      = aws_vpc.attacker.id
}

resource "aws_security_group" "block-outbound-attacker" {
  name_prefix = "${local.attacker-sg-name}-outbound-attacker-block"
  description = local.attacker-sg-description
  vpc_id      = aws_vpc.attacker.id
}

resource "aws_security_group_rule" "outbound-allow-victim" {
  from_port = 0
  to_port   = 0
  protocol  = "-1"

  # block internet except for jfrog, splunk, wazuh
  cidr_blocks       = ["0.0.0.0/0"]
  security_group_id = aws_security_group.allow-outbound-victim.id
  type              = "egress"
}

resource "aws_security_group_rule" "outbound-allow-attacker" {
  from_port = 0
  to_port   = 0
  protocol  = "-1"

  # block internet except for jfrog, splunk, wazuh
  cidr_blocks       = ["0.0.0.0/0"]
  security_group_id = aws_security_group.allow-outbound-attacker.id
  type              = "egress"
}

resource "aws_security_group_rule" "outbound-lockdown-victim" {
  from_port = 0
  to_port   = 0
  protocol  = "-1"

  # block internet except for jfrog, splunk, wazuh
  prefix_list_ids   = [data.aws_ec2_managed_prefix_list.outbound.id]
  security_group_id = aws_security_group.block-outbound-victim.id
  type              = "egress"
}

resource "aws_security_group_rule" "outbound-lockdown-attacker" {
  from_port = 0
  to_port   = 0
  protocol  = "-1"

  # block internet except for jfrog, splunk, wazuh
  prefix_list_ids   = [data.aws_ec2_managed_prefix_list.outbound.id]
  security_group_id = aws_security_group.block-outbound-attacker.id
  type              = "egress"
}
