# ----------------------------------------------------------------------------------------------------------------------
# aws.default is the provider for the main account (on-prem emulation)
# aws.cloud is the provider for the cloud account, which creates two AWS child accounts (cloud emulation)
# ----------------------------------------------------------------------------------------------------------------------

# (1) Configure default account AWS Provider
provider "aws" {
  alias                    = "default"
  profile                  = "evals"
  region                   = var.aws-region
  shared_credentials_files = var.aws-shared-credentials
}

# (2) Configure cloud account AWS Provider
provider "aws" {
  alias                    = "cloud"
  profile                  = "evals"
  region                   = var.aws-region
  shared_credentials_files = var.aws-shared-credentials
  assume_role {
    role_arn     = "arn:aws:iam::${var.account-cloud-management-id}:role/CrossAccountAccess"
    session_name = "terraform-enterpriseround7-cloud-mgmt"
  }
}

data "aws_caller_identity" "default" {}
data "aws_caller_identity" "cloud" { provider = aws.cloud }
