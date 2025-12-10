# _____________________________________________________________________________________
# protections VICTIM NETWORKING
# _____________________________________________________________________________________

locals {
  cloud-vpc-name-protections         = "cloud-vpc-protections"
  cloud-sub-name-protections         = "cloud-subnet-protections"
  cloud-sg-name-protections          = "cloud-sg-protections"
  cloud-private-rtb-name-protections = "cloud-rtb-private-protections"
  cloud-public-rtb-name-protections  = "cloud-rtb-public-protections"

  cloud-vpc-protections-description = "[${var.name-prefix}] Victim Protections Cloud Range"
  cloud-sub-protections-description = "[${var.name-prefix}] Victim Protections Cloud Subnet"
  cloud-sg-protections-description  = "[${var.name-prefix}] Victim Protections Cloud Security Group"

  cloud-ip-protections-prefix  = "10.115"
  cloud-sub-protections-prefix = "${local.cloud-ip-protections-prefix}.5"

  cloud-cidr-protections     = "${local.cloud-ip-protections-prefix}.0.0/16"
  cloud-sub-protections-cidr = "${local.cloud-sub-protections-prefix}.0/24"

  cloud-protections-tgw = {
    sub-cidr        = "${local.cloud-ip-protections-prefix}.199.0/24"
    sub-name        = "${local.cloud-sub-name-protections}-tgw"
    sub-description = "[${var.round-name}] Victim Protections Cloud TGW Subnet"
    sg-description  = "[${var.round-name}] Victim Protections Cloud TGW Security Group"
  }
  cloud_protections_prefix_list_entries = [{
    cidr        = local.cloud-cidr-protections
    description = "Victim Protections Cloud CIDR"
    }
  ]
}

######################################################################################
# Prefix List for protections Victim Cloud protections account
######################################################################################
resource "aws_ec2_managed_prefix_list" "cloud-victim-cloud-protections" {
  provider       = aws.cloud-protections
  name           = "Victim Cloud prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.cloud_protections_prefix_list_entries)

  dynamic "entry" {
    for_each = local.cloud_protections_prefix_list_entries

    content {
      cidr        = entry.value.cidr
      description = entry.value.description
    }
  }
  tags = merge(local.victim-cloud-tags, {
    Name        = "Victim Cloud Prefix List [Protections Account]",
    Owner       = "Victim Cloud Protections",
    Account     = "Victim Cloud Protections",
    Description = "Prefix list for the Victim Cloud Protections VPC (10.115.0.0/16)"
  })
}

######################################################################################
# Prefix List for protections Victim Cloud detections account
######################################################################################
resource "aws_ec2_managed_prefix_list" "detections-cloud-victim-cloud-protections" {
  provider       = aws.cloud-detections
  name           = "Victim Cloud prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.cloud_protections_prefix_list_entries)

  dynamic "entry" {
    for_each = local.cloud_protections_prefix_list_entries

    content {
      cidr        = entry.value.cidr
      description = entry.value.description
    }
  }
  tags = merge(module.cloud-defaults.tags, {
    Name        = "Victim Cloud Prefix List [Protections Account]",
    Owner       = "Victim Cloud Protections",
    Account     = "Victim Cloud Protections",
    Description = "Prefix list for the Victim Cloud Protections VPC (10.115.0.0/16)"
  })
}
######################################################################################
# Prefix List for protections Victim Cloud for base account
######################################################################################
resource "aws_ec2_managed_prefix_list" "main-victim-cloud-protections" {
  provider       = aws.default
  name           = "Victim Cloud prefix list for routing [${var.vendor}]"
  address_family = "IPv4"
  max_entries    = length(local.cloud_protections_prefix_list_entries)

  dynamic "entry" {
    for_each = local.cloud_protections_prefix_list_entries

    content {
      cidr        = entry.value.cidr
      description = entry.value.description
    }
  }
  tags = merge(module.defaults.tags, {
    Name        = "Victim Cloud Prefix List [Protections Account]",
    Owner       = "Victim Cloud Protections",
    Account     = "Victim Cloud Protections",
    Description = "Prefix list for the Victim Cloud Protections VPC (10.115.0.0/16)"
  })
}

######################################################################################
# [VICTIM CLOUD protections] VPC
######################################################################################
resource "aws_vpc" "cloud-protections" {
  provider             = aws.cloud-protections
  cidr_block           = local.cloud-cidr-protections
  enable_dns_hostnames = true
  enable_dns_support   = true

  tags = merge(local.victim-cloud-tags, {
    Name        = local.cloud-vpc-name-protections,
    Description = local.cloud-vpc-protections-description
  })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

### VICTIM - Cloud protections Subnet
resource "aws_subnet" "cloud-protections" {
  provider          = aws.cloud-protections
  vpc_id            = aws_vpc.cloud-protections.id
  cidr_block        = local.cloud-sub-protections-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.victim-cloud-tags, { Name = local.cloud-sub-name-protections, Description = local.cloud-sub-protections-description })
}

# VICTIM - Cloud protections TGW Attachment Subnet
resource "aws_subnet" "cloud-tgw-protections" {
  provider          = aws.cloud-protections
  vpc_id            = aws_vpc.cloud-protections.id
  cidr_block        = local.cloud-protections-tgw.sub-cidr
  availability_zone = var.aws-region-az
  tags              = merge(local.victim-cloud-tags, { Name = "${local.cloud-protections-tgw.sub-name}-tgw", Description = local.cloud-protections-tgw.sub-description })
}

# --------------------------------------------------------------------------------------
# Route tables
# --------------------------------------------------------------------------------------
# Private (default)
resource "aws_route_table" "cloud-rtb-protections" {
  provider = aws.cloud-protections
  vpc_id   = aws_vpc.cloud-protections.id

  dynamic "route" {
    for_each = {
      attacker    = aws_ec2_managed_prefix_list.attacker-cloud-protections.id
      neutral     = aws_ec2_managed_prefix_list.neutral-cloud-protections.id
      victim      = aws_ec2_managed_prefix_list.onprem-cloud-protections.id
      protections = aws_ec2_managed_prefix_list.cloud-victim-cloud-protections.id
    }
    content {
      destination_prefix_list_id = route.value
      transit_gateway_id         = aws_ec2_transit_gateway.main.id
    }
  }

  # Route internet traffic through TGW to B3
  route {
    cidr_block         = local.egress-cidr
    transit_gateway_id = aws_ec2_transit_gateway.main.id
  }
  tags = merge(local.victim-cloud-tags, { Name = local.cloud-private-rtb-name-protections })
}

# --------------------------------------------------------------------------------------
# Route table associations (subnet-level)
# --------------------------------------------------------------------------------------
# private
resource "aws_route_table_association" "cloud-rtba-protections" {
  provider       = aws.cloud-protections
  subnet_id      = aws_subnet.cloud-protections.id
  route_table_id = aws_route_table.cloud-rtb-protections.id
}

## tgw attachment
resource "aws_route_table_association" "cloud-tgw-rtba-protections" {
  provider       = aws.cloud-protections
  subnet_id      = aws_subnet.cloud-tgw-protections.id
  route_table_id = aws_route_table.cloud-rtb-protections.id
}

######################################################################################
# SECURITY GROUP
######################################################################################
resource "aws_security_group" "victim-cloud-protections" {
  provider    = aws.cloud-protections
  name        = local.cloud-sg-name-protections
  description = local.cloud-sg-protections-description
  vpc_id      = aws_vpc.cloud-protections.id

  depends_on = [aws_vpc.cloud-protections]
  tags = merge(local.victim-cloud-tags, {
    Name        = local.cloud-sg-name-protections
    Description = local.cloud-sg-protections-description
  })
}

# locals specific to protections sg rules
locals {
  cloud_protections_allowed_prefix_lists = [
    aws_ec2_managed_prefix_list.attacker-cloud-protections.id,
    aws_ec2_managed_prefix_list.neutral-cloud-protections.id,
    aws_ec2_managed_prefix_list.onprem-cloud-protections.id,
    aws_ec2_managed_prefix_list.cloud-victim-cloud-protections.id,
    aws_ec2_managed_prefix_list.b3-cloud-protections.id,
  ]
}

resource "aws_vpc_security_group_ingress_rule" "cloud_allowed_self_ingress_protections" {
  provider          = aws.cloud-protections
  security_group_id = aws_security_group.victim-cloud-protections.id

  description                  = "Allow all traffic within self security group"
  from_port                    = -1
  to_port                      = -1
  ip_protocol                  = "-1"
  referenced_security_group_id = aws_security_group.victim-cloud-protections.id
}

resource "aws_vpc_security_group_ingress_rule" "cloud_prefix_list_ingress_protections" {
  provider = aws.cloud-protections
  count    = length(local.cloud_protections_allowed_prefix_lists)

  security_group_id = aws_security_group.victim-cloud-protections.id
  description       = "Allow all inbound traffic from prefix list ${local.cloud_protections_allowed_prefix_lists[count.index]}"
  from_port         = -1
  to_port           = -1
  ip_protocol       = "-1"
  prefix_list_id    = local.cloud_protections_allowed_prefix_lists[count.index]
}

resource "aws_vpc_security_group_egress_rule" "cloud_allow_all_outbound_protections" {
  provider          = aws.cloud-protections
  security_group_id = aws_security_group.victim-cloud-protections.id
  description       = "Allow all outbound traffic"
  ip_protocol       = "-1"
  cidr_ipv4         = "0.0.0.0/0"
  from_port         = -1
  to_port           = -1
}

# --------------------------------------------------------------------------------------
# TGW Route Tables for consolidating internet access through B3
# --------------------------------------------------------------------------------------
# Create dedicated route table for cloud-protections attachment
resource "aws_ec2_transit_gateway_route_table" "cloud-protections" {
  transit_gateway_id = aws_ec2_transit_gateway.main.id
  tags = merge(module.defaults.tags, {
    Name = "${var.name-prefix}-tgw-rtb-cloud-protections"
  })
}

# Associate cloud-protections attachment with dedicated route table
resource "aws_ec2_transit_gateway_route_table_association" "cloud-protections" {
  transit_gateway_attachment_id  = aws_ec2_transit_gateway_vpc_attachment.cloud-protections-tgwa.id
  transit_gateway_route_table_id = aws_ec2_transit_gateway_route_table.cloud-protections.id
  replace_existing_association   = true
}

# Route internet traffic (0.0.0.0/0) from cloud-protections to B3 VPC
resource "aws_ec2_transit_gateway_route" "cloud-protections-internet" {
  destination_cidr_block         = "0.0.0.0/0"
  transit_gateway_attachment_id  = aws_ec2_transit_gateway_vpc_attachment.b3-tgwa.id
  transit_gateway_route_table_id = aws_ec2_transit_gateway_route_table.cloud-protections.id
}

# this is routing at the tgw level, not the attachment level, so the routes need to reference which attachment
# to send traffic to, which is why we have to do this for every prefix list.
# in effect, we are recreating the default route table for all traffic, except for internet traffic, which should be
# sent to the protections (b3) attachment. We have to explicitly define all these relationships.
# Return traffic to the cloud protections network will already be routed back through, so no additional work required.
resource "aws_ec2_transit_gateway_prefix_list_reference" "cloud-protections-plr-neutral" {
  prefix_list_id                 = aws_ec2_managed_prefix_list.neutral.id
  transit_gateway_route_table_id = aws_ec2_transit_gateway_route_table.cloud-protections.id
  transit_gateway_attachment_id  = aws_ec2_transit_gateway_vpc_attachment.neutral-tgwa.id
}

resource "aws_ec2_transit_gateway_prefix_list_reference" "cloud-protections-plr-attacker" {
  prefix_list_id                 = aws_ec2_managed_prefix_list.attacker.id
  transit_gateway_route_table_id = aws_ec2_transit_gateway_route_table.cloud-protections.id
  transit_gateway_attachment_id  = aws_ec2_transit_gateway_vpc_attachment.neutral-tgwa.id
}

resource "aws_ec2_transit_gateway_prefix_list_reference" "cloud-protections-plr-victim" {
  prefix_list_id                 = aws_ec2_managed_prefix_list.victim.id
  transit_gateway_route_table_id = aws_ec2_transit_gateway_route_table.cloud-protections.id
  transit_gateway_attachment_id  = aws_ec2_transit_gateway_vpc_attachment.victim-tgwa.id
}

resource "aws_ec2_transit_gateway_prefix_list_reference" "cloud-protections-plr-victim-cloud" {
  prefix_list_id                 = aws_ec2_managed_prefix_list.victim-cloud.id
  transit_gateway_route_table_id = aws_ec2_transit_gateway_route_table.cloud-protections.id
  transit_gateway_attachment_id  = aws_ec2_transit_gateway_vpc_attachment.cloud-tgwa.id
}

resource "aws_ec2_transit_gateway_prefix_list_reference" "cloud-protections-plr-protections" {
  prefix_list_id                 = aws_ec2_managed_prefix_list.main-victim-cloud-protections.id
  transit_gateway_route_table_id = aws_ec2_transit_gateway_route_table.cloud-protections.id
  transit_gateway_attachment_id  = aws_ec2_transit_gateway_vpc_attachment.cloud-protections-tgwa.id
}
