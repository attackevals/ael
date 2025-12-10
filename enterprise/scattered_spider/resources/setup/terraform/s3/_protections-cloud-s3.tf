locals {
  protections_cloud_directories = {
    "Finance Department" = "sensitive"
    "Human Resources"    = "confidential"
    "IT Department"      = "secret"
  }
}

resource "aws_s3_bucket" "protections_cloud" {
  provider = aws.cloud-protections
  bucket   = "${var.name-prefix}-storage"

  tags = merge(local.victim-cloud-tags, {
    Name        = "Cloud S3",
    Owner       = "Cloud Protections",
    Account     = "Cloud Protections",
    Description = "S3 Bucket for storing sensitive files in the Cloud Protections account"
  })

  lifecycle {
    prevent_destroy = true
  }
}

resource "aws_s3_object" "protections_cloud_folders" {
  for_each = local.protections_cloud_directories

  provider = aws.cloud-protections
  bucket   = aws_s3_bucket.protections_cloud.id
  key      = "${each.key}/"

  content = ""
  tags = {
    Classification = each.value
  }
}

output "protections_cloud_bucket_name" {
  value       = aws_s3_bucket.protections_cloud.bucket
  description = "Name of the S3 bucket used by the cloud protections account"
}
