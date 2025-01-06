## resource group
module "defaults" {
  source      = "../modules/aws/defaults"
  name        = var.name-prefix
  description = var.description
  category    = var.category
  charge_code = var.charge_code
  created     = tostring(try(timestamp()))
  updated     = tostring(try(timestamp()))
  environment = var.environment
  region      = var.aws-region
  vendor      = "enterpriseround6"
  provisioner = "terraform"
  shutdown_ok = var.shutdown_ok
  performer   = var.name-prefix
}

## ssh keys
resource "aws_key_pair" "ssh-key-pair" {
  key_name   = var.name-prefix
  public_key = file(var.ssh_public_key_path)
  tags       = merge(module.defaults.tags, { Name : "${var.name-prefix}-ssh-key-pair" })
  lifecycle { ignore_changes = [tags, tags_all] }
}
