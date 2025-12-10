# _____________________________________________________________________________________
#
# VICTIM CLOUD VMs
#   * (1) Ubuntu Airbyte Server
#   * (1) Ubuntu GitLab Server
#   * (1) Ubuntu Wekan Server
# _____________________________________________________________________________________

locals {

  cloud-basenames = {
    wekan-srv1   = "${local.context.cloud}-${local.machine-names.wekan-srv1}"
    gitlab-srv1  = "${local.context.cloud}-${local.machine-names.gitlab-srv1}"
    airbyte-srv1 = "${local.context.cloud}-${local.machine-names.airbyte-srv1}"
    tmt-srv1     = "${local.context.cloud}-${local.machine-names.tmt-srv1}" # (Optional)
  }

  cloud-ips = {
    wekan-srv1   = "${local.cloud-sub-prefix}.105"
    gitlab-srv1  = "${local.cloud-sub-prefix}.106"
    airbyte-srv1 = "${local.cloud-sub-prefix}.107"
    tmt-srv1     = "${local.cloud-sub-prefix}.10" # (Optional)
  }

  cloud-vms = {
    wekan-srv1 = {
      vm-name     = "${var.name-prefix}-${local.cloud-basenames.wekan-srv1}"
      hostname    = "dragonpit"
      description = "wekan"
    }
    gitlab-srv1 = {
      vm-name     = "${var.name-prefix}-${local.cloud-basenames.gitlab-srv1}"
      hostname    = "guildhall"
      description = "gitlab"
    }
    airbyte-srv1 = {
      vm-name     = "${var.name-prefix}-${local.cloud-basenames.airbyte-srv1}"
      hostname    = "rookery"
      description = "airbyte"
    }
    # (Optional traffic mirror)
    tmt-srv1 = {
      vm-name  = "${var.name-prefix}-${local.context.cloud}-${local.cloud-basenames.tmt-srv1}"
      hostname = "bearisland"
    }
  }
}

######################################################################################
## [VICTIM] Ubuntu Wekan Server - (1)
######################################################################################
module "cloud-wekan-srv1" {
  providers = {
    aws = aws.cloud-detections
  }

  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.data
  platform          = local.platform.wekan
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = aws_iam_role.cloudwatch-detections.name
  instance_type     = local.instance-type-ubuntu-victim
  availability_zone = var.aws-region-az

  name               = local.cloud-vms.wekan-srv1.vm-name
  hostname           = local.cloud-vms.wekan-srv1.hostname
  subnet_id          = aws_subnet.cloud.id
  private_ip         = local.cloud-ips.wekan-srv1
  security_group_ids = [aws_security_group.victim-cloud.id]
  key_name           = aws_key_pair.cloud-ssh-key-pair.key_name
  disk_size          = local.disk-size.wekan-srv
  disk_type          = local.standard-disk
  tags               = local.victim-cloud-tags
}

# Ansible Inventory host resource.
resource "ansible_host" "cloud-wekan-srv1" {
  name   = "cloud-wekan-srv1"
  groups = toset([local.scope.victim, local.context.cloud, local.ansible_groups.linux-ubuntu, local.ansible_groups.linux-docker, local.ansible_groups.linux-cloud])

  variables = {
    ansible_host = module.cloud-wekan-srv1.ip

    # Custom vars that we might use in roles/tasks.
    hostname = module.cloud-wekan-srv1.hostname
    fqdn     = module.cloud-wekan-srv1.hostname
  }
}

######################################################################################
## [VICTIM] Ubuntu Airbyte Server - (1)
######################################################################################
module "cloud-airbyte-srv1" {
  providers = {
    aws = aws.cloud-detections
  }

  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.airbyte
  platform          = local.platform.airbyte-srv
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = aws_iam_role.cloudwatch-detections.name
  instance_type     = local.instance-type-ubuntu-victim
  availability_zone = var.aws-region-az

  name               = local.cloud-vms.airbyte-srv1.vm-name
  hostname           = local.cloud-vms.airbyte-srv1.hostname
  subnet_id          = aws_subnet.cloud.id
  private_ip         = local.cloud-ips.airbyte-srv1
  security_group_ids = [aws_security_group.victim-cloud.id]
  key_name           = aws_key_pair.cloud-ssh-key-pair.key_name
  disk_size          = local.disk-size.airbyte-srv
  disk_type          = local.standard-disk
  tags               = local.victim-cloud-tags
}

resource "ansible_host" "cloud-airbyte-srv1" {
  name   = "cloud-airbyte-srv1"
  groups = toset([local.scope.victim, local.context.cloud, local.ansible_groups.linux-ubuntu, local.ansible_groups.linux-docker, local.ansible_groups.linux-cloud])

  variables = {
    ansible_host = module.cloud-airbyte-srv1.ip

    hostname = module.cloud-airbyte-srv1.hostname
    fqdn     = module.cloud-airbyte-srv1.hostname
  }
}

######################################################################################
## [VICTIM] Ubuntu Gitlab Server - (1)
######################################################################################
module "cloud-gitlab-srv1" {
  providers = {
    aws = aws.cloud-detections
  }

  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.gitlab
  platform          = local.platform.gitlab
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = aws_iam_role.cloudwatch-detections.name
  instance_type     = local.instance-type-ubuntu-victim
  availability_zone = var.aws-region-az

  name               = local.cloud-vms.gitlab-srv1.vm-name
  hostname           = local.cloud-vms.gitlab-srv1.hostname
  subnet_id          = aws_subnet.cloud.id
  private_ip         = local.cloud-ips.gitlab-srv1
  security_group_ids = [aws_security_group.victim-cloud.id]
  key_name           = aws_key_pair.cloud-ssh-key-pair.key_name
  disk_size          = local.disk-size.gitlab-srv
  disk_type          = local.standard-disk
  tags               = local.victim-cloud-tags
}

resource "ansible_host" "cloud-gitlab-srv1" {
  name   = "cloud-gitlab-srv1"
  groups = toset([local.scope.victim, local.context.cloud, local.ansible_groups.linux-ubuntu, local.ansible_groups.linux-docker, local.ansible_groups.linux-cloud])
  variables = {
    ansible_host = module.cloud-gitlab-srv1.ip

    hostname = module.cloud-gitlab-srv1.hostname
    fqdn     = module.cloud-gitlab-srv1.hostname
  }
}
