# Provided for Terraform state management in S3.
# Replace the suffix in the "bucket" attribute to a random value.
# See Terraform docs for overview of dynamodb and versioning, https://developer.hashicorp.com/terraform/language/settings/backends/s3.

# for aws, terraform locking is done via dynamodb (aws nosql db service)
# dynamodb must be created with "LockID" as partition key (String type)

### Vendors Bucket
data "aws_s3_bucket" "vendors" {
  bucket = var.aws-bucket-name
}

### Create folder for individual vendor
resource "aws_s3_object" "enterpriseround6" {

  bucket = data.aws_s3_bucket.vendors.id
  key    = "enterpriseround6/"

  # Creates an empty folder in the S3 bucket (Mac/Linux devs)
  source = "/dev/null"
  acl    = "private"
  lifecycle {
    ignore_changes = [
      bucket
    ]
  }
}

### Create state lock
resource "aws_dynamodb_table" "dynamodb-terraform-state-lock" {
  name         = "enterpriseround6-lock"
  billing_mode = "PAY_PER_REQUEST"

  attribute {
    name = "id"
    type = "S"
  }

  hash_key = "id"
}

### Actual backend
terraform {
  backend "s3" {
    bucket = "er6-vendor-resources"
    region = "us-east-1"
    key    = "enterpriseround6/enterpriseround6.tfstate"
  }
}
