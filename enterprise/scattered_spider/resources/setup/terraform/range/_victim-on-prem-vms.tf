# _____________________________________________________________________________________
#
# VICTIM ON-PREM VMs
#   * (1) Windows Server 2022 AD/DC
#   * (2) Windows Server 2022 File Server
#   * (1) Windows Server 2022 Mail Server
#   * (1) Windows Server 2022 Remote Access Server
#   * (1) Ubuntu MFA Server
#   * (4) Windows 11 Desktops
#   * (1) Ubuntu Desktop
# _____________________________________________________________________________________


locals {
  victim-on-prem-basenames = {
    addc-srv1   = "${local.context.onprem}-${local.machine-names.addc-srv1}"
    file-srv1   = "${local.context.onprem}-${local.machine-names.file-srv1}"
    file-srv2   = "${local.context.onprem}-${local.machine-names.file-srv2}"
    mail-srv1   = "${local.context.onprem}-${local.machine-names.mail-srv1}"
    access-srv1 = "${local.context.onprem}-${local.machine-names.access-srv1}"
    mfa-srv1    = "${local.context.onprem}-${local.machine-names.mfa-srv1}"

    win11-desk1 = "${local.context.onprem}-${local.machine-names.win11-desk1}"
    win11-desk2 = "${local.context.onprem}-${local.machine-names.win11-desk2}"
    win11-desk3 = "${local.context.onprem}-${local.machine-names.win11-desk3}"
    win11-desk4 = "${local.context.onprem}-${local.machine-names.win11-desk4}"

    tmt-srv1        = "${local.context.onprem}-${local.machine-names.tmt-srv1}"        # (Optional)
    preflights-box1 = "${local.context.onprem}-${local.machine-names.preflights-box1}" # (Internal)
  }

  victim-on-prem-ips = {
    addc-srv1   = "${local.victim-srv-ip-prefix}.100"
    file-srv1   = "${local.victim-srv-ip-prefix}.103"
    file-srv2   = "${local.victim-srv-ip-prefix}.105"
    mail-srv1   = "${local.victim-srv-ip-prefix}.101"
    access-srv1 = "${local.victim-srv-ip-prefix}.104"
    mfa-srv1    = "${local.victim-srv-ip-prefix}.102"

    win11-desk1 = "${local.victim-desk-ip-prefix}.101"
    win11-desk2 = "${local.victim-desk-ip-prefix}.102"
    win11-desk3 = "${local.victim-desk-ip-prefix}.103"
    win11-desk4 = "${local.victim-desk-ip-prefix}.104"

    tmt-srv1        = "${local.victim-srv-ip-prefix}.99"   # (Optional)
    preflights-box1 = "${local.victim-desk-ip-prefix}.120" # (Internal)
  }

  victim-on-prem-vms = {
    addc-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.addc-srv1}"
      hostname = "redkeep"
    }
    file-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.file-srv1}"
      hostname = "citadel"
    }
    file-srv2 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.file-srv2}"
      hostname = "conclave"
    }
    mail-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.mail-srv1}"
      hostname = "sept"
    }
    access-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.access-srv1}"
      hostname = "dragongate"
    }
    mfa-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.mfa-srv1}"
      hostname = "citywatch"
    }

    win11-desk1 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.win11-desk1}"
      hostname = "fleabottom"
    }
    win11-desk2 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.win11-desk2}"
      hostname = "fishmonger"
    }
    win11-desk3 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.win11-desk3}"
      hostname = "harrenhal"
    }
    win11-desk4 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.win11-desk4}"
      hostname = "stepstones"
    }
    # (Optional traffic mirror)
    tmt-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.tmt-srv1}"
      hostname = "riverrow"
    }
    # (Internal vendor validation)
    preflights-box1 = {
      vm-name  = "${var.name-prefix}-${local.victim-on-prem-basenames.preflights-box1}"
      hostname = "preflights"
    }
  }
}

######################################################################################
## [VICTIM] Windows Server 2022 AD/DC - (1)
######################################################################################
module "on-prem-addc-srv1" {
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

  name               = local.victim-on-prem-vms.addc-srv1.vm-name
  hostname           = local.victim-on-prem-vms.addc-srv1.hostname
  subnet_id          = aws_subnet.victim-servers.id
  private_ip         = local.victim-on-prem-ips.addc-srv1
  security_group_ids = tolist(local.victim-on-prem-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  # Windows AD/DC only
  #  active_directory_netbios_name = var.win_netbios_name
  #  active_directory_domain_name  = var.win_domain_name
  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.victim-onprem-tags
}

resource "ansible_host" "on-prem-addc-srv1" {
  name   = "on-prem-addc-srv1"
  groups = toset([local.scope.victim, local.context.onprem, local.ansible_groups.win-addc, local.ansible_groups.win-server, local.ansible_groups.domain-kingslanding])

  variables = {
    ansible_host = module.on-prem-addc-srv1.ip

    hostname = module.on-prem-addc-srv1.hostname
    fqdn     = module.on-prem-addc-srv1.hostname
  }
}

######################################################################################
## [VICTIM] File Server - (2)
######################################################################################
module "on-prem-file-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.file
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id            = module.amis.windows-server-2022
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-medium
  availability_zone = var.aws-region-az

  name               = local.victim-on-prem-vms.file-srv1.vm-name
  hostname           = local.victim-on-prem-vms.file-srv1.hostname
  subnet_id          = aws_subnet.victim-servers.id
  private_ip         = local.victim-on-prem-ips.file-srv1
  security_group_ids = tolist(local.victim-on-prem-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.file-srv-disk-1
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.victim-onprem-tags
}

resource "ansible_host" "on-prem-file-srv1" {
  name   = "on-prem-file-srv1"
  groups = toset([local.scope.victim, local.context.onprem, local.ansible_groups.win-server, local.ansible_groups.win-file, local.ansible_groups.domain-kingslanding])

  variables = {
    ansible_host = module.on-prem-file-srv1.ip

    hostname = module.on-prem-file-srv1.hostname
    fqdn     = module.on-prem-file-srv1.hostname
  }
}

# Should only be uncommented if not using AMIs/snapshots
# module "on-prem-file-srv1-add-volume" {
#   source            = "../modules/aws/add-volume"
#   name              = "${local.victim-on-prem-vms.file-srv1.vm-name}-vol"
#   instance_id       = module.on-prem-file-srv1.instance.id
#   device_name       = "xvdf"
#   disk_size         = local.disk-size.file-srv-disk-2
#   disk_type         = local.standard-disk
#   availability_zone = var.aws-region-az
#   tags              = local.victim-onprem-tags
# }

module "on-prem-file-srv2" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.file
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id            = module.amis.windows-server-2022
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-medium
  availability_zone = var.aws-region-az

  name               = local.victim-on-prem-vms.file-srv2.vm-name
  hostname           = local.victim-on-prem-vms.file-srv2.hostname
  subnet_id          = aws_subnet.victim-servers.id
  private_ip         = local.victim-on-prem-ips.file-srv2
  security_group_ids = tolist(local.victim-on-prem-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.file-srv-disk-1
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.victim-onprem-tags
}

resource "aws_ebs_volume" "on-prem-file-srv2-disks" {
  count             = 5
  availability_zone = var.aws-region-az
  size              = local.disk-size.file-srv-disk-3
  type              = local.standard-disk
}

resource "aws_volume_attachment" "on-prem-file-srv2-disks_attachment" {
  count       = 5
  instance_id = module.on-prem-file-srv2.instance.id
  volume_id   = aws_ebs_volume.on-prem-file-srv2-disks[count.index].id
  device_name = "/dev/xvd${element(["b", "c", "d", "e", "f", "g", "h", "i", "j", "k"], count.index)}"
}

resource "ansible_host" "on-prem-file-srv2" {
  name   = "on-prem-file-srv2"
  groups = toset([local.scope.victim, local.context.onprem, local.ansible_groups.win-server, local.ansible_groups.win-file-multi, local.ansible_groups.win-file, local.ansible_groups.domain-kingslanding])

  variables = {
    ansible_host = module.on-prem-file-srv2.ip
    hostname     = module.on-prem-file-srv2.hostname
    fqdn         = module.on-prem-file-srv2.hostname
  }
}

######################################################################################
## [VICTIM] Mail Server - (1)
######################################################################################
module "on-prem-mail-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.mail
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id            = module.amis.windows-server-2022
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-large
  availability_zone = var.aws-region-az

  name               = local.victim-on-prem-vms.mail-srv1.vm-name
  hostname           = local.victim-on-prem-vms.mail-srv1.hostname
  subnet_id          = aws_subnet.victim-servers.id
  private_ip         = local.victim-on-prem-ips.mail-srv1
  security_group_ids = tolist(local.victim-on-prem-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.mail-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.victim-onprem-tags
}

resource "ansible_host" "on-prem-mail-srv1" {
  name   = "on-prem-mail-srv1"
  groups = toset([local.scope.victim, local.context.onprem, local.ansible_groups.win-mail, local.ansible_groups.win-server, local.ansible_groups.domain-kingslanding])

  variables = {
    ansible_host = module.on-prem-mail-srv1.ip
    hostname     = module.on-prem-mail-srv1.hostname
    fqdn         = module.on-prem-mail-srv1.hostname
  }
}

######################################################################################
## [VICTIM] Remote Access Server - (1)
######################################################################################
module "on-prem-access-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.access
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id            = module.amis.windows-server-2022
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-large
  availability_zone = var.aws-region-az

  name               = local.victim-on-prem-vms.access-srv1.vm-name
  hostname           = local.victim-on-prem-vms.access-srv1.hostname
  subnet_id          = aws_subnet.victim-servers.id
  private_ip         = local.victim-on-prem-ips.access-srv1
  security_group_ids = tolist(local.victim-on-prem-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.access-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.victim-onprem-tags
}

resource "ansible_host" "on-prem-access-srv1" {
  name   = "on-prem-access-srv1"
  groups = toset([local.scope.victim, local.context.onprem, local.ansible_groups.win-server, local.ansible_groups.win-rds-servers, local.ansible_groups.domain-kingslanding])

  variables = {
    ansible_host = module.on-prem-access-srv1.ip

    hostname = module.on-prem-access-srv1.hostname
    fqdn     = module.on-prem-access-srv1.hostname
  }
}

######################################################################################
## [VICTIM] Ubuntu MFA Server - (1)
######################################################################################
module "on-prem-mfa-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.mfa
  platform          = local.platform.mfa
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-ubuntu-victim
  availability_zone = var.aws-region-az

  name               = local.victim-on-prem-vms.mfa-srv1.vm-name
  hostname           = local.victim-on-prem-vms.mfa-srv1.hostname
  subnet_id          = aws_subnet.victim-servers.id
  private_ip         = local.victim-on-prem-ips.mfa-srv1
  security_group_ids = tolist(local.victim-on-prem-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.mfa-srv
  disk_type          = local.standard-disk
  tags               = local.victim-onprem-tags
}

resource "ansible_host" "on-prem-mfa-srv1" {
  name   = "on-prem-mfa-srv1"
  groups = toset([local.scope.victim, local.context.onprem, local.ansible_groups.linux-ubuntu, local.ansible_groups.linux-docker, local.ansible_groups.mfa, local.ansible_groups.domain-kingslanding])

  variables = {
    ansible_host = module.on-prem-mfa-srv1.ip
    hostname     = module.on-prem-mfa-srv1.hostname
    fqdn         = module.on-prem-mfa-srv1.hostname
    aws_provider = local.account-detections-id
  }
}

######################################################################################
## [VICTIM] Windows 11 Desktops - (4)
######################################################################################
module "on-prem-win11-desk1" {
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

  name               = local.victim-on-prem-vms.win11-desk1.vm-name
  hostname           = local.victim-on-prem-vms.win11-desk1.hostname
  subnet_id          = aws_subnet.victim-desktops.id
  private_ip         = local.victim-on-prem-ips.win11-desk1
  security_group_ids = tolist(local.victim-on-prem-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.victim-onprem-tags
}

resource "ansible_host" "on-prem-win11-desk1" {
  name   = "on-prem-win11-desk1"
  groups = toset([local.scope.victim, local.context.onprem, local.ansible_groups.win-client, local.ansible_groups.domain-kingslanding])

  variables = {
    ansible_host = module.on-prem-win11-desk1.ip

    hostname = module.on-prem-win11-desk1.hostname
    fqdn     = module.on-prem-win11-desk1.hostname
  }
}

module "on-prem-win11-desk2" {
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

  name               = local.victim-on-prem-vms.win11-desk2.vm-name
  hostname           = local.victim-on-prem-vms.win11-desk2.hostname
  subnet_id          = aws_subnet.victim-desktops.id
  private_ip         = local.victim-on-prem-ips.win11-desk2
  security_group_ids = tolist(local.victim-on-prem-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk


  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.victim-onprem-tags
}

resource "ansible_host" "on-prem-win11-desk2" {
  name   = "on-prem-win11-desk2"
  groups = toset([local.scope.victim, local.context.onprem, local.ansible_groups.win-client, local.ansible_groups.domain-kingslanding])

  variables = {
    ansible_host = module.on-prem-win11-desk2.ip

    hostname = module.on-prem-win11-desk2.hostname
    fqdn     = module.on-prem-win11-desk2.hostname
  }
}

module "on-prem-win11-desk3" {
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

  name               = local.victim-on-prem-vms.win11-desk3.vm-name
  hostname           = local.victim-on-prem-vms.win11-desk3.hostname
  subnet_id          = aws_subnet.victim-desktops.id
  private_ip         = local.victim-on-prem-ips.win11-desk3
  security_group_ids = tolist(local.victim-on-prem-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk


  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.victim-onprem-tags
}

resource "ansible_host" "on-prem-win11-desk3" {
  name   = "on-prem-win11-desk3"
  groups = toset([local.scope.victim, local.context.onprem, local.ansible_groups.win-client, local.ansible_groups.domain-kingslanding])

  variables = {
    ansible_host = module.on-prem-win11-desk3.ip

    hostname = module.on-prem-win11-desk3.hostname
    fqdn     = module.on-prem-win11-desk3.hostname
  }
}

module "on-prem-win11-desk4" {
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

  name               = local.victim-on-prem-vms.win11-desk4.vm-name
  hostname           = local.victim-on-prem-vms.win11-desk4.hostname
  subnet_id          = aws_subnet.victim-desktops.id
  private_ip         = local.victim-on-prem-ips.win11-desk4
  security_group_ids = tolist(local.victim-on-prem-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk


  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.victim-onprem-tags
}

resource "ansible_host" "on-prem-win11-desk4" {
  name   = "on-prem-win11-desk4"
  groups = toset([local.scope.victim, local.context.onprem, local.ansible_groups.win-client, local.ansible_groups.domain-kingslanding])

  variables = {
    ansible_host = module.on-prem-win11-desk4.ip

    hostname = module.on-prem-win11-desk4.hostname
    fqdn     = module.on-prem-win11-desk4.hostname
  }
}

######################################################################################
## [PREFLIGHTS] Ubuntu Box - (1)
######################################################################################
module "preflights-box1" {
  scope             = local.scope.internal
  source            = "../modules/aws/base-vm"
  description       = local.description.preflights-box
  platform          = local.platform.preflights
  snapshot_required = false

  ami_id            = module.amis.linux-kali
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-small
  availability_zone = var.aws-region-az

  name               = local.victim-on-prem-vms.preflights-box1.vm-name
  hostname           = local.victim-on-prem-vms.preflights-box1.hostname
  subnet_id          = aws_subnet.victim-desktops.id
  private_ip         = local.victim-on-prem-ips.preflights-box1
  security_group_ids = tolist(local.victim-on-prem-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.preflights-box
  disk_type          = local.standard-disk
  tags               = local.victim-onprem-tags
}

resource "ansible_host" "preflights-box1" {
  name   = "preflights-box1"
  groups = toset([local.scope.internal, local.context.onprem, local.ansible_groups.linux-ubuntu, local.ansible_groups.linux-docker, local.ansible_groups.preflights])

  variables = {
    ansible_host           = module.preflights-box1.ip
    ansible_user           = "kali"
    hostname               = module.preflights-box1.hostname
    fqdn                   = module.preflights-box1.hostname
    vendor_ssh_private_key = sensitive(file(basename(var.ssh_private_key_path_vendor)))
    vendor_ssh_public_key  = file(basename(var.ssh_public_key_path_vendor))
  }
}
