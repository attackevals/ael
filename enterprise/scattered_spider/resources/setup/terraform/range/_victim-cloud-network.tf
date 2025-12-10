# _____________________________________________________________________________________
# VICTIM NETWORKING
# _____________________________________________________________________________________

locals {
  cloud-vpc-name         = "${var.name-prefix}-cloud-vpc"
  cloud-sub-name         = "${var.name-prefix}-cloud-subnet"
  cloud-sg-name          = "${var.name-prefix}-cloud-sg"
  cloud-private-rtb-name = "${var.name-prefix}-cloud-rtb-private"
  cloud-public-rtb-name  = "${var.name-prefix}-cloud-rtb-public"
  cloud-igw-name         = "${var.name-prefix}-cloud-igw"
  cloud-nat-name         = "${var.name-prefix}-cloud-nat"
  cloud-eip-name         = "${var.name-prefix}-cloud-nat-eip"
  #   cloud-vpn-name = "${var.name-prefix}-cloud-vpn"

  cloud-vpc-description = "[${var.round-name}] Victim Cloud Range"
  cloud-sub-description = "[${var.round-name}] Victim Cloud Subnet"
  cloud-sg-description  = "[${var.round-name}] Victim Cloud Security Group"

  cloud-ip-prefix  = "10.212"
  cloud-sub-prefix = "${local.cloud-ip-prefix}.3"

  cloud-cidr     = "${local.cloud-ip-prefix}.0.0/16"
  cloud-sub-cidr = "${local.cloud-sub-prefix}.0/24"

  cloud-tgw = {
    sub-cidr        = "${local.cloud-ip-prefix}.199.0/24"
    sub-name        = "${local.cloud-sub-name}-tgw"
    sub-description = "[${var.round-name}] Victim Cloud TGW Subnet"
    sg-description  = "[${var.round-name}] Victim Cloud TGW Security Group"
  }

  cloud-nat = {
    ip              = "${local.cloud-ip-prefix}.1.64"
    sub-cidr        = "${local.cloud-ip-prefix}.2.0/24"
    sub-name        = "${local.cloud-sub-name}-nat"
    sub-description = "[${var.round-name}] Victim Cloud NAT Subnet"
    sg-description  = "[${var.round-name}] Victim Cloud NAT Security Group"
  }

  cloud_prefix_list_entries = [{
    cidr        = local.cloud-cidr
    description = "Victim Cloud CIDR"
    }
  ]
}

######################################################################################
# Prefix List for Victim Cloud
######################################################################################
resource "aws_ec2_managed_prefix_list" "victim-cloud" {
  name           = "Victim Cloud prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.cloud_prefix_list_entries)

  dynamic "entry" {
    for_each = local.cloud_prefix_list_entries
    content {
      cidr        = entry.value.cidr
      description = entry.value.description
    }
  }
  tags = merge(local.victim-cloud-tags, { Name = "Victim Cloud Prefix List" })
}

######################################################################################
# Prefix List for Victim Cloud
######################################################################################
resource "aws_ec2_managed_prefix_list" "cloud-victim-cloud" {
  provider       = aws.cloud-detections
  name           = "Victim Cloud prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.cloud_prefix_list_entries)

  dynamic "entry" {
    for_each = local.cloud_prefix_list_entries
    content {
      cidr        = entry.value.cidr
      description = entry.value.description
    }
  }
  tags = merge(local.victim-cloud-tags, { Name = "Victim Cloud Prefix List" })
}

######################################################################################
# [VICTIM CLOUD] VPC
######################################################################################
resource "aws_vpc" "cloud" {
  provider             = aws.cloud-detections
  cidr_block           = local.cloud-cidr # shares on-prem CIDR
  enable_dns_hostnames = true
  enable_dns_support   = true

  tags = merge(local.victim-cloud-tags, {
    Name        = local.cloud-vpc-name,
    Description = local.cloud-vpc-description
  })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

resource "aws_vpc_dhcp_options" "dns_resolver_cloud" {
  provider            = aws.cloud-detections
  domain_name_servers = [module.dns-srv1.ip]
}

resource "aws_vpc_dhcp_options_association" "dns_resolver_cloud" {
  provider        = aws.cloud-detections
  vpc_id          = aws_vpc.cloud.id
  dhcp_options_id = aws_vpc_dhcp_options.dns_resolver_cloud.id
}

### VICTIM - Cloud Subnet
resource "aws_subnet" "cloud" {
  provider          = aws.cloud-detections
  vpc_id            = aws_vpc.cloud.id
  cidr_block        = local.cloud-sub-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.victim-cloud-tags, { Name = local.cloud-sub-name, Description = local.cloud-sub-description })
}

### VICTIM - Cloud Subnet
resource "aws_subnet" "cloud-nat" {
  provider          = aws.cloud-detections
  vpc_id            = aws_vpc.cloud.id
  cidr_block        = local.cloud-nat.sub-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.victim-cloud-tags, { Name = local.cloud-nat.sub-name, Description = local.cloud-nat.sub-description })
}

### VICTIM - Cloud TGW Attachment Subnet
resource "aws_subnet" "cloud-tgw" {
  provider          = aws.cloud-detections
  vpc_id            = aws_vpc.cloud.id
  cidr_block        = local.cloud-tgw.sub-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.victim-cloud-tags, { Name = "${local.cloud-tgw.sub-name}-tgw", Description = local.cloud-tgw.sub-description })
}

# ------------------------------------------------------------------------------------
# INTERNET GATEWAY
resource "aws_internet_gateway" "cloud-igw" {
  provider = aws.cloud-detections
  vpc_id   = aws_vpc.cloud.id
  tags     = merge(local.victim-cloud-tags, local.victim-internet-tags, { Name = local.cloud-igw-name })
}
# ------------------------------------------------------------------------------------
# NAT GATEWAY
resource "aws_eip" "cloud-nat-eip" {
  provider = aws.cloud-detections
  # NOTE: Do not use network_interface to associate aws_eip to aws_nat_gateway
  associate_with_private_ip = local.cloud-nat.ip

  public_ipv4_pool = "amazon"
  domain           = "vpc"
  depends_on       = [aws_internet_gateway.cloud-igw]
  tags             = merge(local.victim-cloud-tags, { Name = local.cloud-eip-name })
}
# Create NAT gateway
resource "aws_nat_gateway" "cloud-nat" {
  provider      = aws.cloud-detections
  allocation_id = aws_eip.cloud-nat-eip.id
  subnet_id     = aws_subnet.cloud-nat.id
  depends_on    = [aws_internet_gateway.cloud-igw]
  tags          = merge(local.victim-cloud-tags, { Name = local.cloud-nat-name })
}

# --------------------------------------------------------------------------------------
# Route tables
# --------------------------------------------------------------------------------------
# Private (default)
resource "aws_route_table" "cloud-rtb" {
  provider = aws.cloud-detections
  vpc_id   = aws_vpc.cloud.id

  dynamic "route" {
    for_each = {
      attacker-cloud    = aws_ec2_managed_prefix_list.attacker-cloud.id
      neutral-cloud     = aws_ec2_managed_prefix_list.neutral-cloud.id
      onprem-cloud      = aws_ec2_managed_prefix_list.onprem-cloud.id
      b3-cloud          = aws_ec2_managed_prefix_list.b3-cloud.id
      protections-cloud = aws_ec2_managed_prefix_list.detections-cloud-victim-cloud-protections.id
    }
    content {
      destination_prefix_list_id = route.value
      transit_gateway_id         = aws_ec2_transit_gateway.main.id
    }
  }

  # ALL TRAFFIC --> NAT
  route {
    cidr_block     = local.egress-cidr
    nat_gateway_id = aws_nat_gateway.cloud-nat.id
  }
  tags       = merge(local.victim-cloud-tags, { Name = local.cloud-private-rtb-name })
  depends_on = [aws_ec2_transit_gateway_vpc_attachment.cloud-tgwa]
}

# Public
resource "aws_route_table" "cloud-rtb-public" {
  provider = aws.cloud-detections
  vpc_id   = aws_vpc.cloud.id

  dynamic "route" {
    for_each = {
      attacker-cloud    = aws_ec2_managed_prefix_list.attacker-cloud.id
      neutral-cloud     = aws_ec2_managed_prefix_list.neutral-cloud.id
      onprem-cloud      = aws_ec2_managed_prefix_list.onprem-cloud.id
      b3-cloud          = aws_ec2_managed_prefix_list.b3-cloud.id
      protections-cloud = aws_ec2_managed_prefix_list.detections-cloud-victim-cloud-protections.id
    }
    content {
      destination_prefix_list_id = route.value
      transit_gateway_id         = aws_ec2_transit_gateway.main.id
    }
  }

  # NAT TRAFFIC --> CLOUD --> IGW
  route {
    cidr_block = local.egress-cidr
    gateway_id = aws_internet_gateway.cloud-igw.id
  }
  tags       = merge(local.victim-cloud-tags, local.victim-internet-tags, { Name = local.cloud-public-rtb-name })
  depends_on = [aws_ec2_transit_gateway_vpc_attachment.cloud-tgwa]
}

# --------------------------------------------------------------------------------------
# Route table associations (subnet-level)
# --------------------------------------------------------------------------------------
# private
resource "aws_route_table_association" "cloud-rtba" {
  provider       = aws.cloud-detections
  subnet_id      = aws_subnet.cloud.id
  route_table_id = aws_route_table.cloud-rtb.id
}

# PUBLIC **
resource "aws_route_table_association" "cloud-nat-rtba" {
  provider       = aws.cloud-detections
  subnet_id      = aws_subnet.cloud-nat.id
  route_table_id = aws_route_table.cloud-rtb-public.id
}

## tgw attachment
resource "aws_route_table_association" "cloud-tgw-rtba" {
  provider       = aws.cloud-detections
  subnet_id      = aws_subnet.cloud-tgw.id
  route_table_id = aws_route_table.cloud-rtb.id
}

######################################################################################
# SECURITY GROUP
######################################################################################
resource "aws_security_group" "victim-cloud" {
  provider    = aws.cloud-detections
  name_prefix = local.cloud-sg-name
  description = local.cloud-sg-description
  vpc_id      = aws_vpc.cloud.id

  depends_on = [aws_vpc.cloud]
  tags = merge(local.victim-cloud-tags, {
    Name        = local.cloud-sg-name
    Description = local.cloud-sg-description
  })
}

# locals specific to sg rules
locals {
  cloud_allowed_prefix_lists = [
    aws_ec2_managed_prefix_list.attacker-cloud.id,
    aws_ec2_managed_prefix_list.neutral-cloud.id,
    aws_ec2_managed_prefix_list.onprem-cloud.id,
    aws_ec2_managed_prefix_list.b3-cloud.id,
    aws_ec2_managed_prefix_list.detections-cloud-victim-cloud-protections.id
  ]
}

resource "aws_vpc_security_group_ingress_rule" "cloud_allowed_self_ingress" {
  provider          = aws.cloud-detections
  security_group_id = aws_security_group.victim-cloud.id

  description                  = "Allow all traffic within self security group"
  from_port                    = -1
  to_port                      = -1
  ip_protocol                  = "-1"
  referenced_security_group_id = aws_security_group.victim-cloud.id
}

resource "aws_vpc_security_group_ingress_rule" "cloud_prefix_list_ingress" {
  provider = aws.cloud-detections
  count    = length(local.cloud_allowed_prefix_lists)

  security_group_id = aws_security_group.victim-cloud.id
  description       = "Allow all inbound traffic from prefix list ${local.cloud_allowed_prefix_lists[count.index]}"
  from_port         = -1
  to_port           = -1
  ip_protocol       = "-1"
  prefix_list_id    = local.cloud_allowed_prefix_lists[count.index]
  depends_on        = [aws_security_group.victim-cloud]
}

resource "aws_vpc_security_group_egress_rule" "cloud_allow_all_outbound" {
  provider          = aws.cloud-detections
  security_group_id = aws_security_group.victim-cloud.id
  description       = "Allow all outbound traffic"
  ip_protocol       = "-1"
  cidr_ipv4         = "0.0.0.0/0"
  from_port         = -1
  to_port           = -1
  depends_on        = [aws_security_group.victim-cloud]
}
