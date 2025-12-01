# initialize defaults
module "defaults" {
  providers   = { aws = aws.default }
  source      = "../modules/aws/defaults"
  name        = "${var.name-prefix}-s3"
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

# initialize defaults for cloud detections
module "cloud-defaults-detections" {
  providers   = { aws = aws.cloud-detections }
  source      = "../modules/aws/defaults"
  name        = "${var.name-prefix}-s3"
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

# initialize defaults for cloud protections
module "cloud-defaults-protections" {
  providers   = { aws = aws.cloud-protections }
  source      = "../modules/aws/defaults"
  name        = "${var.name-prefix}-s3"
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
