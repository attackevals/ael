# Provided for Terraform state management in S3.
# Replace the suffix in the "bucket" attribute to a random value.
# See Terraform docs for overview of dynamodb and versioning, https://developer.hashicorp.com/terraform/language/settings/backends/s3.

# for aws, terraform locking is done via dynamodb (aws nosql db service)
# dynamodb must be created with "LockID" as partition key (String type)

### Vendors Bucket
data "aws_s3_bucket" "vendors" {
  bucket = var.aws-bucket-name
}

## To recreate
#resource "aws_s3_bucket" "vendors" {
#  bucket = var.aws-bucket-name
#  lifecycle {
#    ignore_changes = [bucket]
#  }
#}

### Create folder for individual vendor
resource "aws_s3_object" "msr2" {

  bucket = data.aws_s3_bucket.vendors.id
  key    = "msr2/"

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
  name         = "msr2-lock"
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
    #     bucket = "YOUR_BUCKET_HERE"
    region = "us-east-1"
    key    = "msr2/msr2.tfstate"
  }
}
