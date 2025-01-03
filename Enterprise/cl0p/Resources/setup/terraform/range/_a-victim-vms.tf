# _____________________________________________________________________________________
#
# VICTIM VMS
#   * (1) Windows Server 2022 AD/DC
#   * (1) Ubuntu KVM Server
#   * (1) MacOS Desktops
#   * (2) Windows 11 Desktops
# _____________________________________________________________________________________

locals {

  victim-a-basenames = {
    addc-srv1   = "${local.scenario-prefixes.a}-addc-srv1"
    file-srv1   = "${local.scenario-prefixes.a}-file-srv1"
    mail-srv1   = "${local.scenario-prefixes.a}-mail-srv1"
    win11-desk1 = "${local.scenario-prefixes.a}-win11-desk1"
    win11-desk2 = "${local.scenario-prefixes.a}-win11-desk2"
    macos-desk1 = "${local.scenario-prefixes.a}-macos-desk1"
    tmt-srv1    = "${local.scenario-prefixes.a}-tmt-srv1" # (Optional)
  }

  victim-a-ips = {
    addc-srv1   = "${local.victim-a-ip-prefix}.3.100"
    file-srv1   = "${local.victim-a-ip-prefix}.3.105"
    mail-srv1   = "${local.victim-a-ip-prefix}.3.110"
    win11-desk1 = "${local.victim-a-ip-prefix}.4.21"
    win11-desk2 = "${local.victim-a-ip-prefix}.4.22"
    macos-desk1 = "${local.victim-a-ip-prefix}.4.50"
    tmt-srv1    = "${local.victim-a-ip-prefix}.5.10" # (Optional)
  }

  victim-a-vms = {
    addc-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-a-basenames.addc-srv1}"
      hostname = "vault713"
    }
    file-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-a-basenames.file-srv1}"
      hostname = "azkaban"
    }
    win11-desk1 = {
      vm-name  = "${var.name-prefix}-${local.victim-a-basenames.win11-desk1}"
      hostname = "diagonalley"
    }
    win11-desk2 = {
      vm-name  = "${var.name-prefix}-${local.victim-a-basenames.win11-desk2}"
      hostname = "gobbledgook"
    }
    macos-desk1 = {
      vm-name  = "${var.name-prefix}-${local.victim-a-basenames.macos-desk1}"
      hostname = "hogshead"
    }
    mail-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-a-basenames.mail-srv1}"
      hostname = "hangleton"
    }
    # (Optional traffic mirror)
    tmt-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-a-basenames.tmt-srv1}"
      hostname = "floo"
    }
  }
  ### INTERNET ON
  victim-a-security-groups-to-apply = tolist([aws_security_group.victim-a.id, aws_security_group.allow-outbound-victim.id])

  ### INTERNET OFF
  # victim-a-security-groups-to-apply = tolist([aws_security_group.victim-a.id, aws_security_group.block-outbound-victim.id])
}

######################################################################################
## [AD/DC] Windows Server 2022 AD/DC - (1)
## TODO: Potentially 2 if we need Exchange
######################################################################################

module "a-addc-srv1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm-win"
  description       = local.description.addc
  platform          = local.platform.win-srv
  snapshot_required = true
  get_password_data = false

  ami_id            = module.defaults.ami-windows-server-2022
  instance_type     = local.aws-vm-size-large
  availability_zone = var.aws-region-az

  name               = local.victim-a-vms.addc-srv1.vm-name
  hostname           = local.victim-a-vms.addc-srv1.hostname
  subnet_id          = aws_subnet.victim-a.id
  private_ip         = local.victim-a-ips.addc-srv1
  security_group_ids = tolist(local.victim-a-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  # Windows AD/DC only
  #  active_directory_netbios_name = var.win_netbios_name
  #  active_directory_domain_name  = var.win_domain_name
  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## [VICTIM] File Server - (1)
######################################################################################

module "a-file-srv1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.file
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id            = module.defaults.ami-windows-server-2022
  instance_type = local.aws-vm-size-large

  availability_zone = var.aws-region-az

  name               = local.victim-a-vms.file-srv1.vm-name
  hostname           = local.victim-a-vms.file-srv1.hostname
  subnet_id          = aws_subnet.victim-a.id
  private_ip         = local.victim-a-ips.file-srv1
  security_group_ids = tolist(local.victim-a-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

## Should only be uncommented if not using AMIs/snapshots
# module "a-file-srv1-add-volume" {
#   source            = "../modules/aws/add-volume"
#   name              = "${local.victim-a-vms.file-srv1.vm-name}-vol"
#   instance_id       = module.a-file-srv1.instance.id
#   device_name       = "xvdf"
#   disk_size         = local.disk-size.file-srv-disk-2
#   disk_type         = local.standard-disk
#   availability_zone = var.aws-region-az
#   tags              = module.defaults.tags
# }

######################################################################################
## [VICTIM] Mail Server - (1)
######################################################################################

module "a-mail-srv1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.mail
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id = module.defaults.ami-windows-server-2022
  instance_type = local.aws-vm-size-large

  availability_zone = var.aws-region-az

  name               = local.victim-a-vms.mail-srv1.vm-name
  hostname           = local.victim-a-vms.mail-srv1.hostname
  subnet_id          = aws_subnet.victim-a.id
  private_ip         = local.victim-a-ips.mail-srv1
  security_group_ids = tolist(local.victim-a-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}


######################################################################################
## [VICTIM] MacOS Desktops - (1)

# NOTE: MacOS instances must run on dedicated hardware, which AWS charges for >=24 hours
######################################################################################
module "a-macos-desk1" {
  count             = var.deploy_mac_hosts == "yes" ? 1 : 0
  scope             = "victim"
  source            = "../modules/aws/base-vm-macos"
  description       = local.description.macos-desk
  platform          = local.platform.macos-desk
  snapshot_required = true
  get_password_data = false

  ami_id            = module.defaults.ami-macos-arm-sonoma
  instance_type     = local.aws-vm-size-mac-m1
  availability_zone = var.aws-region-az

  name               = local.victim-a-vms.macos-desk1.vm-name
  hostname           = local.victim-a-vms.macos-desk1.hostname
  subnet_id          = aws_subnet.victim-a.id
  private_ip         = local.victim-a-ips.macos-desk1
  security_group_ids = tolist(local.victim-a-security-groups-to-apply)
  disk_size          = local.disk-size.macos-desk
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## [VICTIM] Windows 11 Desktops - (2)
# REPLACED WITH Windows Server OS Image, replace with your custom AWS Windows 11 AMI
######################################################################################

module "a-win11-desk1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.win11-desk
  platform          = local.platform.win11-desk
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-win11
  availability_zone = var.aws-region-az

  name               = local.victim-a-vms.win11-desk1.vm-name
  hostname           = local.victim-a-vms.win11-desk1.hostname
  subnet_id          = aws_subnet.victim-a.id
  private_ip         = local.victim-a-ips.win11-desk1
  security_group_ids = tolist(local.victim-a-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

module "a-win11-desk2" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.win11-desk
  platform          = local.platform.win11-desk
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-win11
  availability_zone = var.aws-region-az

  name               = local.victim-a-vms.win11-desk2.vm-name
  hostname           = local.victim-a-vms.win11-desk2.hostname
  subnet_id          = aws_subnet.victim-a.id
  private_ip         = local.victim-a-ips.win11-desk2
  security_group_ids = tolist(local.victim-a-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk


  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}
