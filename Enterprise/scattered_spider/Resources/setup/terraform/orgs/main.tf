module "cloud-defaults" {
  providers   = { aws = aws.cloud }
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
