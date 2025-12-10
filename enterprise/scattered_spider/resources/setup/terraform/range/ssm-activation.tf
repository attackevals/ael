locals {
  # Set this to true only when you want to force a new activation
  force_new_activation = false

  # Optional: Set a fixed expiration date (RFC3339 format)
  # fixed_expiration_date = "2025-12-31T23:59:59Z"
}

module "ssm_activation" {
  providers = { aws = aws.cloud-detections }
  source    = "../modules/aws/ssm-activation"

  name_prefix        = "${var.name-prefix}-ssm"
  registration_limit = 100
  expiration_days    = 15

  # Use a fixed expiration date if provided, otherwise use expiration_days
  # fixed_expiration_date = try(local.fixed_expiration_date, "")

  # Add a trigger to force new activation when needed
  force_new_activation = local.force_new_activation
}

# Store the activation credentials in AWS SSM Parameter Store for Ansible to retrieve
resource "aws_ssm_parameter" "activation_id" {
  provider = aws.cloud-detections
  name     = "/${var.name-prefix}/ssm/activation_id"
  type     = "String"
  value    = module.ssm_activation.activation_id
}

resource "aws_ssm_parameter" "activation_code" {
  provider = aws.cloud-detections
  name     = "/${var.name-prefix}/ssm/activation_code"
  type     = "SecureString"
  value    = module.ssm_activation.activation_code
}

# create aws ssm association to gather inventory for all instances
resource "aws_ssm_association" "inventory" {
  provider = aws.cloud-detections
  name     = "AWS-GatherSoftwareInventory"
  targets {
    key    = "InstanceIds"
    values = ["*"]
  }
  parameters = {
    applications                = "Enabled"
    awsComponents               = "Enabled"
    customInventory             = "Enabled"
    instanceDetailedInformation = "Enabled"
    networkConfig               = "Enabled"
    services                    = "Enabled"
    windowsRoles                = "Enabled"
    windowsUpdates              = "Enabled"
  }

  schedule_expression = "rate(30 minutes)"
}
