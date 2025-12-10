# IAM Identity Center (cloud management account)
data "aws_ssoadmin_instances" "sso-instances" {
  provider = aws.cloud
}

# Get permission set for Admins
data "aws_ssoadmin_permission_set" "sso-permission-set-admins" {
  provider     = aws.cloud
  instance_arn = local.cloud_sso_instance_arn
  name         = "AdministratorAccess"
}

data "aws_identitystore_group" "orgadmins" {
  provider          = aws.cloud
  identity_store_id = var.cloud-identity-store-id

  alternate_identifier {
    unique_attribute {
      attribute_path  = "DisplayName"
      attribute_value = "OrgAdmins"
    }
  }
}
