# _____________________________________________________________________________________
#
# VICTIM VMS
#   * (1) Windows Server 2022 AD/DC
#   * (1) Ubuntu KVM Server
#   * (1) MacOS Desktops
#   * (2) Windows 11 Desktops
# _____________________________________________________________________________________

locals {

  victim-protections-basenames = {
    addc-srv1   = "${local.scenario-prefixes.protections}-addc-srv1"
    file-srv1   = "${local.scenario-prefixes.protections}-file-srv1"
    mail-srv1   = "${local.scenario-prefixes.protections}-mail-srv1"
    win11-desk1 = "${local.scenario-prefixes.protections}-win11-desk1"
    win11-desk2 = "${local.scenario-prefixes.protections}-win11-desk2"
    macos-desk1 = "${local.scenario-prefixes.protections}-macos-desk1"
    linux-srv1  = "${local.scenario-prefixes.protections}-linux-srv1"
    tmt-srv1    = "${local.scenario-prefixes.protections}-tmt-srv1" # (Optional)
  }

  victim-protections-ips = {
    addc-srv1   = "${local.victim-protections-ip-prefix}.15.10"
    file-srv1   = "${local.victim-protections-ip-prefix}.15.15"
    mail-srv1   = "${local.victim-protections-ip-prefix}.15.20"
    win11-desk1 = "${local.victim-protections-ip-prefix}.25.61"
    win11-desk2 = "${local.victim-protections-ip-prefix}.25.62"
    linux-srv1  = "${local.victim-protections-ip-prefix}.25.65"
    macos-desk1 = "${local.victim-protections-ip-prefix}.25.70"
    tmt-srv1    = "${local.victim-protections-ip-prefix}.35.10" # (Optional)
  }

  victim-protections-vms = {
    addc-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-protections-basenames.addc-srv1}"
      hostname = "blackpink"
    }
    file-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-protections-basenames.file-srv1}"
      hostname = "asix"
    }
    win11-desk1 = {
      vm-name  = "${var.name-prefix}-${local.victim-protections-basenames.win11-desk1}"
      hostname = "bts"
    }
    win11-desk2 = {
      vm-name  = "${var.name-prefix}-${local.victim-protections-basenames.win11-desk2}"
      hostname = "exo"
    }
    macos-desk1 = {
      vm-name  = "${var.name-prefix}-${local.victim-protections-basenames.macos-desk1}"
      hostname = "itzy"
    }
    mail-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-protections-basenames.mail-srv1}"
      hostname = "vixx"
    }
    linux-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-protections-basenames.linux-srv1}"
      hostname = "lisa"
    }
    # (Optional traffic mirror)
    tmt-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-protections-basenames.linux-srv1}"
      hostname = "kimchi"
    }
  }
  victim-protections-security-groups-to-apply = [aws_security_group.victim-protections.id, aws_security_group.allow-outbound-victim.id]
}

######################################################################################
## [AD/DC] Windows Server 2022 AD/DC - (1)
######################################################################################

module "protections-addc-srv1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm-win"
  description       = local.description.addc
  platform          = local.platform.win-srv
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-large
  availability_zone = var.aws-region-az

  name               = local.victim-protections-vms.addc-srv1.vm-name
  hostname           = local.victim-protections-vms.addc-srv1.hostname
  subnet_id          = aws_subnet.victim-protections.id
  private_ip         = local.victim-protections-ips.addc-srv1
  security_group_ids = local.victim-protections-security-groups-to-apply
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  # Windows AD/DC only
  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## [VICTIM] File Server - (1)
######################################################################################

module "protections-file-srv1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.file
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-large
  availability_zone = var.aws-region-az

  name               = local.victim-protections-vms.file-srv1.vm-name
  hostname           = local.victim-protections-vms.file-srv1.hostname
  subnet_id          = aws_subnet.victim-protections.id
  private_ip         = local.victim-protections-ips.file-srv1
  security_group_ids = tolist(local.victim-protections-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}
## Should only be uncommented if not using AMIs/snapshots
module "protections-file-srv1-add-volume" {
  source            = "../modules/aws/add-volume"
  name              = "${local.victim-protections-vms.file-srv1.vm-name}-vol"
  instance_id       = module.protections-file-srv1.instance.id
  device_name       = "xvdf"
  disk_size         = local.disk-size.file-srv-disk-2
  disk_type         = local.standard-disk
  availability_zone = var.aws-region-az
  tags              = module.defaults.tags
}

######################################################################################
## [VICTIM] Mail Server - (1)
######################################################################################

module "protections-mail-srv1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.mail
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-large
  availability_zone = var.aws-region-az

  name                 = local.victim-protections-vms.mail-srv1.vm-name
  hostname             = local.victim-protections-vms.mail-srv1.hostname
  subnet_id            = aws_subnet.victim-protections.id
  private_ip           = local.victim-protections-ips.mail-srv1
  security_group_ids   = tolist(local.victim-protections-security-groups-to-apply)
  key_name             = aws_key_pair.ssh-key-pair.key_name
  disk_size            = local.disk-size.addc-srv
  disk_type            = local.standard-disk
  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## [VICTIM] MacOS Desktops - (1)

# NOTE: MacOS instances must run on dedicated hardware, which AWS charges for >=24 hours
######################################################################################

module "protections-macos-desk1" {
  count             = var.deploy_mac_hosts == "yes" ? 1 : 0
  scope             = "victim"
  source            = "../modules/aws/base-vm-macos"
  description       = local.description.macos-desk
  platform          = local.platform.macos-desk
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-macos-arm-sonoma

  instance_type     = local.aws-vm-size-mac-m1
  availability_zone = var.aws-region-az

  name               = local.victim-protections-vms.macos-desk1.vm-name
  hostname           = local.victim-protections-vms.macos-desk1.hostname
  subnet_id          = aws_subnet.victim-protections.id
  private_ip         = local.victim-protections-ips.macos-desk1
  security_group_ids = tolist(local.victim-protections-security-groups-to-apply)
  disk_size          = local.disk-size.macos-desk
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## [VICTIM] Windows 11 Desktops - (2)
######################################################################################

module "protections-win11-desk1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.win11-desk
  platform          = local.platform.win11-desk
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-windows-desktop-11

  instance_type     = local.aws-vm-size-win11
  availability_zone = var.aws-region-az

  name               = local.victim-protections-vms.win11-desk1.vm-name
  hostname           = local.victim-protections-vms.win11-desk1.hostname
  subnet_id          = aws_subnet.victim-protections.id
  private_ip         = local.victim-protections-ips.win11-desk1
  security_group_ids = tolist(local.victim-protections-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

module "protections-win11-desk2" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.win11-desk
  platform          = local.platform.win11-desk
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-win11
  availability_zone = var.aws-region-az

  name               = local.victim-protections-vms.win11-desk2.vm-name
  hostname           = local.victim-protections-vms.win11-desk2.hostname
  subnet_id          = aws_subnet.victim-protections.id
  private_ip         = local.victim-protections-ips.win11-desk2
  security_group_ids = local.victim-protections-security-groups-to-apply
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk


  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## [VICTIM] Linux Desktops - (1)
######################################################################################
module "protections-linux-srv1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.linux
  platform          = local.platform.kvm
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-linux-ubuntu-jammy

  instance_type     = local.aws-vm-size-large
  availability_zone = var.aws-region-az

  name                 = local.victim-protections-vms.linux-srv1.vm-name
  hostname             = local.victim-protections-vms.linux-srv1.hostname
  subnet_id            = aws_subnet.victim-protections.id
  private_ip           = local.victim-protections-ips.linux-srv1
  security_group_ids   = tolist(local.victim-protections-security-groups-to-apply)
  disk_size            = local.disk-size.kvm
  disk_type            = local.standard-disk
  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}
