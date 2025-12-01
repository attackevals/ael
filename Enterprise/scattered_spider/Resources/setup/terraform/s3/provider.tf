# 1. (aws.default) is our default account/provider
# 2. (aws.cloud) is our secondary account/provider, emulating the victim cloud account environment.
# The aws.cloud account is accessed by using an assumed role from the primary account.

# 1. Configure default account AWS Provider
provider "aws" {
  alias                    = "default"
  profile                  = "evals"
  shared_credentials_files = var.aws-shared-credentials
  region                   = var.aws-region
}

## 2a. Use orgs terraform state to grab vendor account info
data "terraform_remote_state" "orgs" {
  backend = "local"
  config = {
    path = "../orgs/terraform.tfstate"
  }
}

locals {
  account-detections-id       = data.terraform_remote_state.orgs.outputs.account-detections.id
  account-protections-id      = data.terraform_remote_state.orgs.outputs.account-protections.id
  account-cloud-management-id = data.terraform_remote_state.orgs.outputs.account-cloud-management.account_id
}

# 2b. Configure secondary account provider
provider "aws" {
  alias                    = "cloud-detections"
  profile                  = "evals"
  region                   = var.aws-region
  shared_credentials_files = var.aws-shared-credentials
  assume_role {
    role_arn     = "arn:aws:iam::${local.account-cloud-management-id}:role/CrossAccountAccess"
    session_name = "terraform-enterpriseround7-detections-mgmt"
  }
  assume_role {
    role_arn     = "arn:aws:iam::${local.account-detections-id}:role/CrossAccountAccess"
    session_name = "terraform-enterpriseround7-detections"
  }
}

provider "aws" {
  alias                    = "cloud-protections"
  profile                  = "evals"
  region                   = var.aws-region
  shared_credentials_files = var.aws-shared-credentials
  assume_role {
    role_arn     = "arn:aws:iam::${local.account-cloud-management-id}:role/CrossAccountAccess"
    session_name = "terraform-enterpriseround7-protections-mgmt"
  }
  assume_role {
    role_arn     = "arn:aws:iam::${local.account-protections-id}:role/CrossAccountAccess"
    session_name = "terraform-enterpriseround7-protections"
  }
}
