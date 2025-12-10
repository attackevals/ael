# _____________________________________________________________________________________
#
# Protections VMs
#   * (1) Windows Server 2022 AD/DC
#   * (1) Windows Remote Access Server
#   * (1) Windows Exchange Server
#   * (1) Ubuntu MDM Server
#   * (3) Windows 11 Admin Workstation
#
# _____________________________________________________________________________________

locals {

  b3-basenames = {
    addc-srv1    = "${local.context.b3}-${local.machine-names.addc-srv1}"
    access-srv1  = "${local.context.b3}-${local.machine-names.access-srv1}"
    mail-srv1    = "${local.context.b3}-${local.machine-names.mail-srv1}"
    mdm-srv1     = "${local.context.b3}-${local.machine-names.mdm-srv1}"
    file-srv1    = "${local.context.b3}-${local.machine-names.file-srv1}"
    file-srv2    = "${local.context.b3}-${local.machine-names.file-srv2}"
    win11-desk1  = "${local.context.b3}-${local.machine-names.win11-desk1}"
    win11-desk2  = "${local.context.b3}-${local.machine-names.win11-desk2}"
    win11-desk3  = "${local.context.b3}-${local.machine-names.win11-desk3}"
    win11-desk4  = "${local.context.b3}-${local.machine-names.win11-desk4}"
    win-jumpbox1 = "${local.context.b3}-${local.machine-names.win-jumpbox1}"
    tmt-srv1     = "${local.context.b3}-${local.machine-names.tmt-srv1}" # (Optional)
  }

  b3-ips = {
    addc-srv1    = "${local.b3-srv-ip-prefix}.100"
    access-srv1  = "${local.b3-srv-ip-prefix}.101"
    mail-srv1    = "${local.b3-srv-ip-prefix}.102"
    mdm-srv1     = "${local.b3-srv-ip-prefix}.103"
    file-srv1    = "${local.b3-srv-ip-prefix}.105"
    file-srv2    = "${local.b3-srv-ip-prefix}.106"
    win11-desk1  = "${local.b3-desk-ip-prefix}.101"
    win11-desk2  = "${local.b3-desk-ip-prefix}.102"
    win11-desk3  = "${local.b3-desk-ip-prefix}.103"
    win11-desk4  = "${local.b3-desk-ip-prefix}.104"
    win-jumpbox1 = "${local.b3-srv-ip-prefix}.125" # Not vendor visible
    tmt-srv1     = "${local.b3-srv-ip-prefix}.99"  # (Optional)
  }

  b3-vms = {
    addc-srv1 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.addc-srv1}"
      hostname = "eyrie"
    }
    access-srv1 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.access-srv1}"
      hostname = "blacktyde"
    }
    mail-srv1 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.mail-srv1}"
      hostname = "godswood"
    }
    mdm-srv1 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.mdm-srv1}"
      hostname = "knights"
    }
    file-srv1 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.file-srv1}"
      hostname = "redfort"
    }
    file-srv2 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.file-srv2}"
      hostname = "heartshome"
    }
    win11-desk1 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.win11-desk1}"
      hostname = "runestone"
    }
    win11-desk2 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.win11-desk2}"
      hostname = "tentowers"
    }
    win11-desk3 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.win11-desk3}"
      hostname = "bitterbridge"
    }
    win11-desk4 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.win11-desk4}"
      hostname = "ironoaks"
    }
    win-jumpbox1 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.win-jumpbox1}"
      hostname = "dreadfort"
    }
    # (Optional traffic mirror)
    tmt-srv1 = {
      vm-name  = "${var.name-prefix}-${local.b3-basenames.tmt-srv1}"
      hostname = "breakwater"
    }
  }

}

######################################################################################
## [Protections] Windows Server 2022 AD/DC - (1)
######################################################################################
module "b3-addc-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm-win"
  description       = local.description.addc
  platform          = local.platform.win-srv
  snapshot_required = true
  get_password_data = false

  ami_id            = module.amis.windows-server-2022
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-large
  availability_zone = var.aws-region-az

  name               = local.b3-vms.addc-srv1.vm-name
  hostname           = local.b3-vms.addc-srv1.hostname
  subnet_id          = aws_subnet.b3-servers.id
  private_ip         = local.b3-ips.addc-srv1
  security_group_ids = tolist(local.b3-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  # Windows AD/DC only
  #  active_directory_netbios_name = var.win_netbios_name
  #  active_directory_domain_name  = var.win_domain_name
  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.b3-tags
}

resource "ansible_host" "b3-addc-srv1" {
  name   = "b3-addc-srv1"
  groups = toset([local.scope.victim, local.context.b3, local.ansible_groups.win-addc, local.ansible_groups.win-server, local.ansible_groups.domain-vale])

  variables = {
    ansible_host = module.b3-addc-srv1.ip
    hostname     = module.b3-addc-srv1.hostname
    fqdn         = module.b3-addc-srv1.hostname
  }
}

######################################################################################
## [Protections] Remote Access Server - (1)
######################################################################################
module "b3-access-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.access
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id            = module.amis.windows-server-2022
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-large
  availability_zone = var.aws-region-az

  name               = local.b3-vms.access-srv1.vm-name
  hostname           = local.b3-vms.access-srv1.hostname
  subnet_id          = aws_subnet.b3-servers.id
  private_ip         = local.b3-ips.access-srv1
  security_group_ids = tolist(local.b3-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.access-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.b3-tags
}

resource "ansible_host" "b3-access-srv1" {
  name   = "b3-access-srv1"
  groups = toset([local.scope.victim, local.context.b3, local.ansible_groups.win-server, local.ansible_groups.win-rds-servers, local.ansible_groups.domain-vale])

  variables = {
    ansible_host = module.b3-access-srv1.ip
    hostname     = module.b3-access-srv1.hostname
    fqdn         = module.b3-access-srv1.hostname
  }
}

######################################################################################
## [Protections] File Server - (1)
######################################################################################
module "b3-file-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.file
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id            = module.amis.windows-server-2022
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-large
  availability_zone = var.aws-region-az

  name               = local.b3-vms.file-srv1.vm-name
  hostname           = local.b3-vms.file-srv1.hostname
  subnet_id          = aws_subnet.b3-servers.id
  private_ip         = local.b3-ips.file-srv1
  security_group_ids = tolist(local.b3-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.file-srv-disk-1
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.b3-tags
}

resource "ansible_host" "b3-file-srv1" {
  name   = "b3-file-srv1"
  groups = toset([local.scope.victim, local.context.b3, local.ansible_groups.win-server, local.ansible_groups.win-file, local.ansible_groups.domain-vale])

  variables = {
    ansible_host = module.b3-file-srv1.ip
    hostname     = module.b3-file-srv1.hostname
    fqdn         = module.b3-file-srv1.hostname
  }
}

# below is only needed when deploying from base windows ami
# resource "aws_ebs_volume" "b3-file-srv1-add-volumes" {
#   count             = 4
#   availability_zone = var.aws-region-az
#   size              = local.disk-size.file-srv-disk-3
#   type              = local.standard-disk
# }
#
# resource "aws_volume_attachment" "b3-file-srv1-volume-attachments" {
#   count       = 4
#   instance_id = module.b3-file-srv1.instance.id
#   volume_id   = aws_ebs_volume.b3-file-srv1-add-volumes[count.index].id
#   device_name = "/dev/xvd${element(["b", "c", "d", "e", "f", "g", "h", "i", "j", "k"], count.index)}"
# }

######################################################################################
## [Protections] File Server - (2)
######################################################################################
module "b3-file-srv2" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.file
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id            = module.amis.windows-server-2022
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-large
  availability_zone = var.aws-region-az

  name               = local.b3-vms.file-srv2.vm-name
  hostname           = local.b3-vms.file-srv2.hostname
  subnet_id          = aws_subnet.b3-servers.id
  private_ip         = local.b3-ips.file-srv2
  security_group_ids = tolist(local.b3-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.file-srv-disk-1
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.b3-tags
}

resource "ansible_host" "b3-file-srv2" {
  name   = "b3-file-srv2"
  groups = toset([local.scope.victim, local.context.b3, local.ansible_groups.win-server, local.ansible_groups.win-file, local.ansible_groups.domain-vale])

  variables = {
    ansible_host = module.b3-file-srv2.ip
    hostname     = module.b3-file-srv2.hostname
    fqdn         = module.b3-file-srv2.hostname
  }
}

# Should only be uncommented if not using AMIs/snapshots
# module "b3-file-srv2-add-volume" {
#   source            = "../modules/aws/add-volume"
#   name              = "${local.b3-vms.file-srv2.vm-name}-vol"
#   instance_id       = module.b3-file-srv2.instance.id
#   device_name       = "xvdf"
#   disk_size         = local.disk-size.file-srv-disk-2
#   disk_type         = local.standard-disk
#   availability_zone = var.aws-region-az
#   tags              = local.victim-onprem-tags
# }

######################################################################################
## [Protections] Mail Server - (1)
######################################################################################
module "b3-mail-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.mail
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id            = module.amis.windows-server-2022
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-large
  availability_zone = var.aws-region-az

  name               = local.b3-vms.mail-srv1.vm-name
  hostname           = local.b3-vms.mail-srv1.hostname
  subnet_id          = aws_subnet.b3-servers.id
  private_ip         = local.b3-ips.mail-srv1
  security_group_ids = tolist(local.b3-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.mail-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.b3-tags
}

resource "ansible_host" "b3-mail-srv1" {
  name   = "b3-mail-srv1"
  groups = toset([local.scope.victim, local.context.b3, local.ansible_groups.win-mail, local.ansible_groups.win-server, local.ansible_groups.domain-vale])

  variables = {
    ansible_host = module.b3-mail-srv1.ip
    hostname     = module.b3-mail-srv1.hostname
    fqdn         = module.b3-mail-srv1.hostname
  }
}

######################################################################################
## [Protections] Ubuntu MDM Server - (1)
######################################################################################
module "b3-mdm-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.mdm
  platform          = local.platform.mdm
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-ubuntu-victim
  availability_zone = var.aws-region-az

  name               = local.b3-vms.mdm-srv1.vm-name
  hostname           = local.b3-vms.mdm-srv1.hostname
  subnet_id          = aws_subnet.b3-servers.id
  private_ip         = local.b3-ips.mdm-srv1
  security_group_ids = tolist(local.b3-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.mdm-srv
  disk_type          = local.standard-disk
  tags               = local.b3-tags
}

resource "ansible_host" "b3-mdm-srv1" {
  name   = "b3-mdm-srv1"
  groups = toset([local.scope.victim, local.context.b3, local.ansible_groups.linux-ubuntu, local.ansible_groups.linux-docker, local.ansible_groups.mdm, local.ansible_groups.domain-vale])

  variables = {
    ansible_host = module.b3-mdm-srv1.ip
    hostname     = module.b3-mdm-srv1.hostname
    fqdn         = module.b3-mdm-srv1.hostname
  }
}

######################################################################################
## [Protections] Windows 11 Desktops - (3)
######################################################################################
module "b3-win11-desk1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.win11-desk
  platform          = local.platform.win11-desk
  snapshot_required = true
  get_password_data = false

  ami_id            = module.amis.windows-desktop-11
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-win11
  availability_zone = var.aws-region-az

  name               = local.b3-vms.win11-desk1.vm-name
  hostname           = local.b3-vms.win11-desk1.hostname
  subnet_id          = aws_subnet.b3-desktops.id
  private_ip         = local.b3-ips.win11-desk1
  security_group_ids = tolist(local.b3-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.b3-tags
}

resource "ansible_host" "b3-win11-desk1" {
  name   = "b3-win11-desk1"
  groups = toset([local.scope.victim, local.context.b3, local.ansible_groups.win-client, local.ansible_groups.domain-vale])

  variables = {
    ansible_host = module.b3-win11-desk1.ip
    hostname     = module.b3-win11-desk1.hostname
    fqdn         = module.b3-win11-desk1.hostname
  }
}

module "b3-win11-desk2" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.win11-desk
  platform          = local.platform.win11-desk
  snapshot_required = true
  get_password_data = false

  ami_id            = module.amis.windows-desktop-11
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-win11
  availability_zone = var.aws-region-az

  name               = local.b3-vms.win11-desk2.vm-name
  hostname           = local.b3-vms.win11-desk2.hostname
  subnet_id          = aws_subnet.b3-desktops.id
  private_ip         = local.b3-ips.win11-desk2
  security_group_ids = tolist(local.b3-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.b3-tags
}

resource "ansible_host" "b3-win11-desk2" {
  name   = "b3-win11-desk2"
  groups = toset([local.scope.victim, local.context.b3, local.ansible_groups.win-client, local.ansible_groups.domain-vale])

  variables = {
    ansible_host = module.b3-win11-desk2.ip
    hostname     = module.b3-win11-desk2.hostname
    fqdn         = module.b3-win11-desk2.hostname
  }
}

module "b3-win11-desk3" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.win11-desk
  platform          = local.platform.win11-desk
  snapshot_required = true
  get_password_data = false

  ami_id            = module.amis.windows-desktop-11
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-win11
  availability_zone = var.aws-region-az

  name               = local.b3-vms.win11-desk3.vm-name
  hostname           = local.b3-vms.win11-desk3.hostname
  subnet_id          = aws_subnet.b3-desktops.id
  private_ip         = local.b3-ips.win11-desk3
  security_group_ids = tolist(local.b3-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.b3-tags
}

resource "ansible_host" "b3-win11-desk3" {
  name   = "b3-win11-desk3"
  groups = toset([local.scope.victim, local.context.b3, local.ansible_groups.win-client, local.ansible_groups.domain-vale])

  variables = {
    ansible_host = module.b3-win11-desk3.ip
    hostname     = module.b3-win11-desk3.hostname
    fqdn         = module.b3-win11-desk3.hostname
  }
}

module "b3-win11-desk4" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.win11-desk
  platform          = local.platform.win11-desk
  snapshot_required = true
  get_password_data = false

  ami_id            = module.amis.windows-desktop-11
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-win11
  availability_zone = var.aws-region-az

  name               = local.b3-vms.win11-desk4.vm-name
  hostname           = local.b3-vms.win11-desk4.hostname
  subnet_id          = aws_subnet.b3-desktops.id
  private_ip         = local.b3-ips.win11-desk4
  security_group_ids = tolist(local.b3-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.b3-tags
}

resource "ansible_host" "b3-win11-desk4" {
  name   = "b3-win11-desk4"
  groups = toset([local.scope.victim, local.context.b3, local.ansible_groups.win-client, local.ansible_groups.domain-vale])

  variables = {
    ansible_host = module.b3-win11-desk4.ip
    hostname     = module.b3-win11-desk4.hostname
    fqdn         = module.b3-win11-desk4.hostname
  }
}

######################################################################################
## [INTERNAL] Windows Server 2022 Jumpbox - (1)
######################################################################################
module "b3-win-jumpbox1" {
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

  name               = local.b3-vms.win-jumpbox1.vm-name
  hostname           = local.b3-vms.win-jumpbox1.hostname
  subnet_id          = aws_subnet.b3-servers.id
  private_ip         = local.b3-ips.win-jumpbox1
  security_group_ids = tolist(local.b3-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk
  key_name           = aws_key_pair.ssh-key-pair.key_name

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.attacker-tags
}

resource "ansible_host" "b3-win-jumpbox1" {
  name   = "b3-win-jumpbox1"
  groups = toset([local.scope.attacker, local.context.attacker, local.ansible_groups.win-server, local.ansible_groups.red-jumpbox-servers])

  variables = {
    ansible_host = module.b3-win-jumpbox1.ip
    hostname     = module.b3-win-jumpbox1.hostname
    fqdn         = module.b3-win-jumpbox1.hostname
  }
}
