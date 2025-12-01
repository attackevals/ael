# Outbound prefix list reference (in AWS)
data "aws_ec2_managed_prefix_list" "outbound" {
  name = "er6-egress-allowed-dev-list"
}

locals {
  shared-tgw-name = "${var.name-prefix}-tgw"
  shared-tgw-desc = "TGW ${var.name-prefix}"

  shared-tgwa-victim-name            = "${local.shared-tgw-name}-tgwa-victim"
  shared-tgwa-cloud-name             = "${local.shared-tgw-name}-tgwa-cloud"
  shared-tgwa-cloud-protections-name = "${local.shared-tgw-name}-tgwa-cloud-protections"
  shared-tgwa-neutral-name           = "${local.shared-tgw-name}-tgwa-neutral"
  shared-tgwa-b3-name                = "${local.shared-tgw-name}-tgwa-b3"

  machine-names = {
    # victim
    addc-srv1   = "addc-srv1"
    file-srv1   = "file-srv1"
    file-srv2   = "file-srv2"
    mail-srv1   = "mail-srv1"
    access-srv1 = "access-srv1"
    mfa-srv1    = "mfa-srv1"
    mdm-srv1    = "mdm-srv1"
    rd-srv1     = "rd-srv1"
    win11-desk1 = "win11-desk1"
    win11-desk2 = "win11-desk2"
    win11-desk3 = "win11-desk3"
    win11-desk4 = "win11-desk4"

    # Victim (Preflights)
    preflights-box1 = "preflights-box1"

    # Victim VPN (Not Vendor Visible)
    win-jumpbox1 = "win-jumpbox1"

    # victim-dmz
    guacamole-srv1 = "dmz-guac-srv1"

    # cloud
    wekan-srv1   = "wekan-srv1"
    gitlab-srv1  = "gitlab-srv1"
    airbyte-srv1 = "airbyte-srv1"

    # attacker
    kali1    = "kali1"
    idp-srv1 = "idp-srv1"

    # neutral
    unmanaged-desk1 = "unmanaged-desk1"
    dns-srv1        = "dns-srv1"
    rmm-srv1        = "rmm-srv1"
    choco-srv1      = "choco-srv1"
    postfix-srv1    = "postfix-srv1"
    postfix-srv2    = "postfix-srv2"
    postfix-srv3    = "postfix-srv3"
    rustdesk-srv1   = "rustdesk-srv1"
    wstunnel-srv1   = "wstunnel-srv1"

    # neutral redirect
    redirect1 = "redirect-srv1"
    redirect2 = "redirect-srv2"
    redirect3 = "redirect-srv3"
    redirect4 = "redirect-srv4"

    # traffic mirror (optional)
    tmt-srv1 = "tmt-srv1"
  }
}

resource "aws_vpc_dhcp_options" "dns_resolver" {
  domain_name_servers = [module.dns-srv1.ip]
}

resource "aws_ec2_transit_gateway" "main" {
  description                        = local.shared-tgw-desc
  auto_accept_shared_attachments     = "enable"
  security_group_referencing_support = "enable"

  # Add DNS support and VPN ECMP support
  dns_support      = "enable"
  vpn_ecmp_support = "enable"

  tags = merge(module.defaults.tags, { Name = local.shared-tgw-name })
}

resource "aws_ec2_transit_gateway_vpc_attachment" "victim-tgwa" {
  subnet_ids         = [aws_subnet.victim-tgw.id]
  transit_gateway_id = aws_ec2_transit_gateway.main.id
  vpc_id             = aws_vpc.victim.id
  tags               = merge(module.defaults.tags, { Name = local.shared-tgwa-victim-name })
  lifecycle {
    create_before_destroy = true
  }
}

resource "aws_ec2_transit_gateway_vpc_attachment" "neutral-tgwa" {
  subnet_ids         = [aws_subnet.neutral-tgw.id]
  transit_gateway_id = aws_ec2_transit_gateway.main.id
  vpc_id             = aws_vpc.attacker.id
  tags               = merge(module.defaults.tags, { Name = local.shared-tgwa-neutral-name })
  lifecycle {
    create_before_destroy = true
  }
}

resource "aws_ec2_transit_gateway_vpc_attachment" "b3-tgwa" {
  subnet_ids         = [aws_subnet.b3-tgw.id]
  transit_gateway_id = aws_ec2_transit_gateway.main.id
  vpc_id             = aws_vpc.b3.id
  tags               = merge(module.defaults.tags, { Name = local.shared-tgwa-b3-name })
  lifecycle {
    create_before_destroy = true
  }
}

# share tgw via RAM with other account
resource "aws_ram_resource_share" "tgw" {
  name                      = "${var.name-prefix}-tgw-share"
  allow_external_principals = true
  tags                      = merge(module.defaults.tags, { Name = local.shared-tgw-name })
}

resource "aws_ram_resource_association" "tgw" {
  resource_arn       = aws_ec2_transit_gateway.main.arn
  resource_share_arn = aws_ram_resource_share.tgw.id
}

resource "aws_ram_principal_association" "tgw" {
  principal          = data.aws_caller_identity.cloud-detections.account_id
  resource_share_arn = aws_ram_resource_share.tgw.id
}

resource "aws_ram_principal_association" "tgw-protections" {
  principal          = data.aws_caller_identity.cloud-protections.account_id
  resource_share_arn = aws_ram_resource_share.tgw.id
}

# cross-org sharing: accept ram share in cloud-detections (second) account
resource "aws_ram_resource_share_accepter" "tgw" {
  provider  = aws.cloud-detections
  share_arn = aws_ram_principal_association.tgw.resource_share_arn
}

# cross-org sharing: accept ram share in cloud-protections (third) account
resource "aws_ram_resource_share_accepter" "tgw-protections" {
  provider  = aws.cloud-protections
  share_arn = aws_ram_principal_association.tgw-protections.resource_share_arn
}

resource "time_sleep" "wait_for_tgw_share" {
  depends_on = [
    aws_ram_principal_association.tgw,
    aws_ram_resource_association.tgw,
    aws_ram_resource_share_accepter.tgw
  ]
  create_duration = "30s"
}

resource "time_sleep" "wait_for_tgw_protections_share" {
  depends_on = [
    aws_ram_principal_association.tgw-protections,
    aws_ram_resource_association.tgw,
    aws_ram_resource_share_accepter.tgw-protections
  ]
  create_duration = "30s"
}

# Create VPC attachment in second account (cloud detections)
resource "aws_ec2_transit_gateway_vpc_attachment" "cloud-tgwa" {
  provider = aws.cloud-detections

  depends_on = [
    aws_ram_principal_association.tgw,
    aws_ram_resource_association.tgw,
    aws_ram_resource_share_accepter.tgw,
    time_sleep.wait_for_tgw_share,
  ]

  subnet_ids         = [aws_subnet.cloud-tgw.id]
  transit_gateway_id = aws_ec2_transit_gateway.main.id
  vpc_id             = aws_vpc.cloud.id
  tags               = merge(module.defaults.tags, { Name = local.shared-tgwa-cloud-name })

  lifecycle {
    create_before_destroy = true
  }
}

# Create VPC attachment in third account (cloud protections)
resource "aws_ec2_transit_gateway_vpc_attachment" "cloud-protections-tgwa" {
  provider = aws.cloud-protections

  depends_on = [
    aws_ram_principal_association.tgw-protections,
    aws_ram_resource_association.tgw,
    aws_ram_resource_share_accepter.tgw-protections,
    time_sleep.wait_for_tgw_protections_share,
  ]

  subnet_ids         = [aws_subnet.cloud-protections.id]
  transit_gateway_id = aws_ec2_transit_gateway.main.id
  vpc_id             = aws_vpc.cloud-protections.id
  tags               = merge(module.defaults.tags, { Name = local.shared-tgwa-cloud-protections-name })

  lifecycle {
    create_before_destroy = true
  }
}

## openvpn
module "openvpn-client" {
  source            = "../modules/aws/vpn-client"
  name              = var.name-prefix
  organization_name = var.vendor
  cidr              = local.openvpn-cidr

  target_subnet_id  = aws_subnet.vpn.id
  subnet_ids        = [aws_subnet.vpn.id]
  vpc_id            = aws_vpc.victim.id
  security_group_id = aws_security_group.victim-on-prem.id

  allowed_cidr_ranges = tolist(toset([
    # Victim
    local.cloud-cidr,
    local.victim-cidr,
    local.b3-cidr,

    # Red/Gray
    aws_vpc.attacker.cidr_block,
    local.attacker-redirect-cidr,
    local.neutral-support-cidr,
    aws_subnet.neutral-redirect1.cidr_block,
    aws_subnet.neutral-redirect2.cidr_block,
    aws_subnet.neutral-redirect3.cidr_block,
    aws_subnet.neutral-redirect4.cidr_block,
  ]))

  target_cidr_blocks = tolist(toset([
    # Victim
    local.cloud-cidr,
    local.b3-cidr,
    local.cloud-cidr-protections,

    # Red/Gray
    aws_vpc.attacker.cidr_block,
    local.attacker-redirect-cidr,
    local.neutral-support-cidr,
    aws_subnet.neutral-redirect1.cidr_block,
    aws_subnet.neutral-redirect2.cidr_block,
    aws_subnet.neutral-redirect3.cidr_block,
    aws_subnet.neutral-redirect4.cidr_block,
    local.cloud-cidr-protections,
  ]))

  split_tunnel = true
  depends_on   = [aws_security_group.victim-on-prem]
  tags         = merge(module.defaults.tags, { Name : local.victim-vpn-name })
}

# module issue - if output, terraform will consider this a dependency to include in destroy plan
output "vpn-assoc" {
  value = module.openvpn-client.vpn_network_assoc
}

## create security group ingress rule to allow traffic tagged with vpn security group access to victim
resource "aws_vpc_security_group_ingress_rule" "main" {
  security_group_id            = aws_security_group.victim-on-prem.id
  referenced_security_group_id = module.openvpn-client.vpn_security_group_id
  ip_protocol                  = "-1"
}
