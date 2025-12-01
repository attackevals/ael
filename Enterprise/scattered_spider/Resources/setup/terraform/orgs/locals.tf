locals {
  cloud_console_access_url = "https://${var.cloud-identity-store-id}.awsapps.com/start/#"
  cloud_sso_instance_arn   = one(data.aws_ssoadmin_instances.sso-instances.arns) # SSO with existing permission sets
}
