## resource group
module "defaults" {
  source      = "../modules/aws/defaults"
  name        = var.name-prefix
  description = var.round-name
  category    = var.category
  charge_code = var.charge-code
  created     = tostring(try(timestamp()))
  environment = var.environment
  owner       = var.round-name
  performer   = var.name-prefix
  provisioner = "terraform"
  region      = var.aws-region
  shutdown_ok = var.shutdown_ok
  updated     = tostring(try(timestamp()))
  vendor      = "enterpriseround7"
}

module "cloud-defaults" {
  providers = {
    aws = aws.cloud-detections
  }
  source      = "../modules/aws/defaults"
  name        = var.name-prefix
  description = var.round-name
  category    = var.category
  charge_code = var.charge-code
  created     = tostring(try(timestamp()))
  environment = var.environment
  owner       = var.round-name
  performer   = var.name-prefix
  provisioner = "terraform"
  region      = var.aws-region
  shutdown_ok = var.shutdown_ok
  updated     = tostring(try(timestamp()))
  vendor      = "enterpriseround7"
}

## ssh keys
resource "aws_key_pair" "ssh-key-pair" {
  key_name   = var.name-prefix
  public_key = file(var.ssh_public_key_path)
  tags       = merge(module.defaults.tags, { Name : "${var.name-prefix}-ssh-key-pair" })
  lifecycle { ignore_changes = [tags, tags_all] }
}

resource "aws_key_pair" "cloud-ssh-key-pair" {
  provider   = aws.cloud-detections
  key_name   = var.name-prefix
  public_key = file(var.ssh_public_key_path)
  tags       = merge(module.cloud-defaults.tags, { Name : "${var.name-prefix}-ssh-key-pair" })
  lifecycle { ignore_changes = [tags, tags_all] }
}

# [onprem/preflights] vendor validation
resource "aws_key_pair" "ssh-key-pair-vendor" {
  key_name   = "${var.name-prefix}-vendor"
  public_key = file(basename(var.ssh_public_key_path_vendor))
  tags       = merge(module.defaults.tags, { Name : "${var.name-prefix}-ssh-key-pair-vendor" })
  lifecycle { ignore_changes = [tags, tags_all] }
}

data "aws_caller_identity" "current" {}

data "aws_caller_identity" "cloud-detections" {
  provider = aws.cloud-detections
}

data "aws_caller_identity" "cloud-protections" {
  provider = aws.cloud-protections
}

locals {
  account_id                   = data.aws_caller_identity.current.account_id
  account_id_cloud_detections  = data.aws_caller_identity.cloud-detections.account_id
  account_id_cloud_protections = data.aws_caller_identity.cloud-protections.account_id
}

## needed for ansible inv.
locals {
  b1_prefix = md5("${var.name-prefix}")
}
output "b1_prefix" {
  value = local.b1_prefix
}
