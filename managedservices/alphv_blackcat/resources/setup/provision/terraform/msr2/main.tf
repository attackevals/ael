## resource group
module "defaults" {
  source      = "../../modules/aws/defaults"
  name        = var.name-prefix
  description = var.description
  category    = var.category
  charge_code = var.charge_code
  created     = tostring(try(timestamp()))
  updated     = tostring(try(timestamp()))
  environment = var.environment
  region      = var.aws-region
  vendor      = "msr2"
  provisioner = "terraform"
  shutdown_ok = var.shutdown_ok
}

## ssh keys
resource "aws_key_pair" "ssh-key-pair" {
  key_name   = var.name-prefix
  public_key = file(var.ssh_public_key_path)
  tags       = merge(module.defaults.tags, { Name : "${var.name-prefix}-ssh-key-pair" })
  lifecycle { ignore_changes = [tags, tags_all] }
}

## openvpn
module "openvpn-client" {
  source              = "../../modules/aws/vpn-client"
  name                = var.name-prefix
  organization_name   = local.vendor
  cidr                = local.cidr.openvpn
  subnet_ids          = [aws_subnet.a_vpn.id]
  vpc_id              = aws_vpc.alpha.id
  security_group_id   = aws_security_group.alpha.id
  allowed_cidr_ranges = local.beta_ingress_whitelist
  target_cidr_blocks  = [local.cidr.beta, local.cidr.redirect-a, local.cidr.redirect-b]
  target_subnet_id    = aws_subnet.a_vpn.id
  split_tunnel        = true
  depends_on          = [aws_security_group.alpha]
  tags                = merge(module.defaults.tags, { Name : local.name.vpn })
}

## create security group ingress rule, allowing traffic tagged with vpn security group access to alpha
resource "aws_vpc_security_group_ingress_rule" "main" {
  security_group_id            = aws_security_group.alpha.id
  referenced_security_group_id = module.openvpn-client.vpn_security_group_id
  ip_protocol                  = "-1"
}

# MAIN - snapshots to use
locals {

  ##################################################################################################################
  # IN msr2 SCOPE
  ##################################################################################################################
  # SUBSIDIARY A HOSTS

  # SUBSIDIARY B HOSTS

  # CONTRACTOR HOST

  ##################################################################################################################
  # OUT OF msr2 SCOPE
  ##################################################################################################################
  # TRAFFIC HOSTS

  # RED TEAM HOSTS

  # SUPPORT HOSTS
}
