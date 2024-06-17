# Configure the AWS Provider
provider "aws" {
  region = var.aws-region
}

# template for using terraform cloud
#  cloud {
#    organization = "##ORG##"
#    workspaces {
#      name = "##WORKSPACE##"
#    }
#  }
