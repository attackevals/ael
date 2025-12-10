# [DETECTIONS] Vendor account (detections)
resource "aws_organizations_account" "detections" {
  provider          = aws.cloud
  parent_id         = data.aws_organizations_organizational_unit.cloud-ou-vendors.id
  name              = "enterpriseround7-detections"
  email             = var.account-detections-email
  role_name         = "CrossAccountAccess"
  close_on_deletion = false

  lifecycle {
    ignore_changes  = [role_name, email]
    prevent_destroy = true
  }
  tags = module.cloud-defaults.tags
}

# IAM Identity Center -------------------------------------------------

# [DETECTIONS] Assign account to OrgAdmins
resource "aws_ssoadmin_account_assignment" "assign-to-detections-orgadmins" {
  provider           = aws.cloud
  instance_arn       = local.cloud_sso_instance_arn
  permission_set_arn = data.aws_ssoadmin_permission_set.sso-permission-set-admins.arn # AdministratorAccess

  principal_id   = data.aws_identitystore_group.orgadmins.group_id # OrgAdmins
  principal_type = "GROUP"

  target_id   = aws_organizations_account.detections.id
  target_type = "AWS_ACCOUNT"
}
