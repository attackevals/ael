module "amis" {
  source = "../modules/aws/amis"
}

module "amis-cloud" {
  providers  = { aws = aws.cloud-detections }
  source     = "../modules/aws/amis"
  ami_owners = [local.account_id]
}

# ami_ids
locals {}
