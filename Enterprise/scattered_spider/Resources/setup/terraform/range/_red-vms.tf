# _____________________________________________________________________________________
#
# INTERNAL VMS
#   * (1) Kali C2 Server
#   * (1) Attacker IDP server
#
# _____________________________________________________________________________________

locals {
  # Resource names
  attacker-basenames = {
    kali1        = "${local.context.attacker}-${local.machine-names.kali1}"
    idp-srv1     = "${local.context.attacker}-${local.machine-names.idp-srv1}"
    win-jumpbox1 = "${local.context.attacker}-${local.machine-names.win-jumpbox1}"
  }

  # IP addresses
  attacker-ip = {
    kali1 = "${local.attacker-ip-prefix}.0.70"
    kali1-list = [
      "${local.attacker-ip-prefix}.0.71",
      "${local.attacker-ip-prefix}.0.72"
    ]
    # 12.78.110.37
    win-jumpbox1 = "${local.neutral-support-prefix}.110.37"
    win-jumpbox1-list = [
      "${local.neutral-support-prefix}.110.38",
      "${local.neutral-support-prefix}.110.39"
    ]
    idp-srv1 = "${local.attacker-ip-prefix}.0.100"
  }

  # AWS Names and Hostnames
  attacker-vms = {
    kali1 = {
      vm-name  = "${var.name-prefix}-${local.attacker-basenames.kali1}"
      hostname = "driftmark"
    }
    win-jumpbox1 = {
      vm-name  = "${var.name-prefix}-${local.attacker-basenames.win-jumpbox1}"
      hostname = "jumpbox"
    }
    idp-srv1 = {
      vm-name  = "${var.name-prefix}-${local.attacker-basenames.idp-srv1}"
      hostname = "braavos"
    }
  }
}

######################################################################################
## [INTERNAL] Kali
######################################################################################
module "red-kali1" {
  scope             = local.scope.attacker
  source            = "../modules/aws/base-vm"
  description       = local.description.kali
  platform          = local.platform.kali
  snapshot_required = false

  ami_id            = module.amis.linux-kali
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-large
  availability_zone = var.aws-region-az

  name               = local.attacker-vms.kali1.vm-name
  hostname           = local.attacker-vms.kali1.hostname
  subnet_id          = aws_subnet.attacker.id
  private_ip         = local.attacker-ip.kali1
  private_ips        = local.attacker-ip.kali1-list
  security_group_ids = tolist(local.attacker-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.kali
  disk_type          = local.standard-disk
  tags               = local.attacker-tags
}

resource "ansible_host" "red-kali1" {
  name   = "red-kali1"
  groups = toset([local.scope.attacker, local.context.attacker, local.ansible_groups.kali])

  variables = {
    ansible_host = module.red-kali1.ip

    hostname = module.red-kali1.hostname
    fqdn     = module.red-kali1.hostname

    gitlab_pat_secret_name = local.gitlab-pat-secret-name
  }
}

######################################################################################
## [INTERNAL] Ubuntu IDP Server - (1)
######################################################################################
module "idp-srv1" {
  scope             = local.scope.attacker
  source            = "../modules/aws/base-vm"
  description       = local.description.idp
  platform          = local.platform.idp-srv
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-medium
  availability_zone = var.aws-region-az

  name               = local.attacker-vms.idp-srv1.vm-name
  hostname           = local.attacker-vms.idp-srv1.hostname
  subnet_id          = aws_subnet.attacker.id
  private_ip         = local.attacker-ip.idp-srv1
  security_group_ids = tolist(local.attacker-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.idp-srv
  disk_type          = local.standard-disk
  tags               = local.attacker-tags
}

resource "ansible_host" "red-idp-srv1" {
  name   = "red-idp-srv1"
  groups = [local.scope.attacker, local.context.attacker, local.ansible_groups.linux-docker, local.ansible_groups.linux-ubuntu]

  variables = {
    ansible_host = module.idp-srv1.ip

    hostname = module.idp-srv1.hostname
    fqdn     = module.idp-srv1.hostname
  }
}

######################################################################################
## [INTERNAL] Windows Server 2022 Jumpbox - (1)
######################################################################################
module "win-jumpbox1" {
  scope             = local.scope.attacker
  source            = "../modules/aws/base-vm"
  description       = local.description.win-jumpbox1
  platform          = local.platform.win-srv
  snapshot_required = true
  get_password_data = false

  ami_id            = module.amis.windows-server-2022
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-large
  availability_zone = var.aws-region-az

  name               = local.attacker-vms.win-jumpbox1.vm-name
  hostname           = local.attacker-vms.win-jumpbox1.hostname
  subnet_id          = aws_subnet.neutral-support.id
  private_ip         = local.attacker-ip.win-jumpbox1
  private_ips        = local.attacker-ip.win-jumpbox1-list
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk
  key_name           = aws_key_pair.ssh-key-pair.key_name

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.attacker-tags
}

resource "ansible_host" "red-win-jumpbox1" {
  name   = "red-win-jumpbox1"
  groups = toset([local.scope.attacker, local.context.attacker, local.ansible_groups.win-server, local.ansible_groups.red-jumpbox-servers])

  variables = {
    ansible_host = module.win-jumpbox1.ip

    hostname = module.win-jumpbox1.hostname
    fqdn     = module.win-jumpbox1.hostname
  }
}
