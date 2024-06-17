######################################################################################
## PEERING CONNECTION
######################################################################################
resource "aws_vpc_peering_connection" "range-to-red-pcx" {
  vpc_id      = aws_vpc.alpha.id
  peer_vpc_id = aws_vpc.beta.id
  auto_accept = true
  accepter { allow_remote_vpc_dns_resolution = true }
  requester { allow_remote_vpc_dns_resolution = true }
  tags = merge(module.defaults.tags, { Name = local.name.pcx })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}

# --------------------------------------------------------------------------------------
# Security Group
# --------------------------------------------------------------------------------------
#module "sgs" {
#  source            = "../../modules/aws/security-group"
#  vpc_id            = aws_vpc.beta.id
#  name_prefix       = var.name-prefix
#  ingress_whitelist = local.ingress_whitelist
#  #  alpha_ingress_whitelist = local.alpha_ingress_whitelist
#  #  beta_ingress_whitelist  = local.beta_ingress_whitelist
#  tags              = module.defaults.tags
#}


######################################################################################
## [Later] AlterNAT
######################################################################################

#module "alternat_instances" {
#  source = "git::https://github.com/1debit/alternat.git//modules/terraform-aws-alternat?ref=v0.3.3"
#
#  alternat_image_uri  = "0123456789012.dkr.ecr.us-east-1.amazonaws.com/alternat-functions-lambda"
#  alternat_image_tag  = "v0.3.3"
#  lambda_package_type = "Zip"
#
#  ingress_security_group_ids = [
#    module.sg1.id,
#    module.sg2.id,
#    module.sg3.id,
#    module.sg4.id,
#    module.sg5.id,
#  ]
#
#  vpc_id      = aws_vpc.vpc1.id
#  vpc_az_maps = var.aws-region-az
#  tags        = merge(module.defaults.tags, { Name = "${var.name-prefix}-alternat-gw" })
#}
