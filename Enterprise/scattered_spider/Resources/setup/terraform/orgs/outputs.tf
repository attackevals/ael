# Accounts ------------------------------------------------------------------
# Victim cloud AWS account
output "account-cloud-management" {
  value = data.aws_caller_identity.cloud
}

# Victim cloud AWS account -- IAM Identity Center ARN
output "sso-arn-cloud-management" {
  value = local.cloud_sso_instance_arn
}

# Detections vendor account
output "account-detections" {
  value = aws_organizations_account.detections
}

# Protections vendor account
output "account-protections" {
  value = aws_organizations_account.protections
}

# Account-group assignments -------------------------------------------------
output "assign-to-detections-orgadmins" {
  value = aws_ssoadmin_account_assignment.assign-to-detections-orgadmins
}

output "assign-to-protections-orgadmins" {
  value = aws_ssoadmin_account_assignment.assign-to-protections-orgadmins
}

# Groups --------------------------------------------------------------------
# Existing AdministratorAccess group
output "group-orgadmins" {
  value = data.aws_identitystore_group.orgadmins
}

# Instructions --------------------------------------------------------------
output "assume-role-instructions" {
  value = {
    instructions : "In AWS console as the management account, use the following information to switch into the new vendor account"
    console-url : local.cloud_console_access_url
    account-id-detections : aws_organizations_account.detections.id
    account-id-protections : aws_organizations_account.protections.id
    iam-role-name-detections : aws_organizations_account.detections.role_name
    iam-role-name-protections : aws_organizations_account.protections.role_name
  }
}
