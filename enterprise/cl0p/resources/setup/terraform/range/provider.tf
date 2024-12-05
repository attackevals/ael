# Configure the AWS Provider
provider "aws" {
  shared_credentials_files = var.aws-shared-credentials
  region                   = var.aws-region
}

# template for using terraform cloud
#  cloud {
#    organization = "##ORG##"
#    workspaces {
#      name = "##WORKSPACE##"
#    }
#  }
