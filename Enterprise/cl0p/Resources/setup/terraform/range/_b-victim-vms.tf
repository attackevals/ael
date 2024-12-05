# _____________________________________________________________________________________
#
# VICTIM VMS
#   * (1) Windows Server 2022 AD/DC --- TODO: maybe (2)
#   * (1) Ubuntu KVM Server
#   * (1) MacOS Desktops
#   * (2) Windows 11 Desktops
# _____________________________________________________________________________________

locals {

  victim-b-basenames = {
    addc-srv1   = "${local.scenario-prefixes.b}-addc-srv1"
    kvm-srv1    = "${local.scenario-prefixes.b}-kvm-srv1"
    win11-desk1 = "${local.scenario-prefixes.b}-win11-desk1"
    win11-desk2 = "${local.scenario-prefixes.b}-win11-desk2"
    file-srv1   = "${local.scenario-prefixes.b}-file-srv1"
    mail-srv1   = "${local.scenario-prefixes.b}-mail-srv1"
    tmt-srv1    = "${local.scenario-prefixes.b}-tmt-srv1" # (Optional)
  }

  victim-b-ips = {
    addc-srv1   = "${local.victim-b-ip-prefix}.8.30"
    kvm-srv1    = "${local.victim-b-ip-prefix}.8.40"
    file-srv1   = "${local.victim-b-ip-prefix}.8.50"
    mail-srv1   = "${local.victim-b-ip-prefix}.8.60"
    win11-desk1 = "${local.victim-b-ip-prefix}.9.201"
    win11-desk2 = "${local.victim-b-ip-prefix}.9.202"
    tmt-srv1    = "${local.victim-a-ip-prefix}.10.10" # (Optional)
  }

  victim-b-vms = {
    addc-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-b-basenames.addc-srv1}"
      hostname = "vault12"
    }
    kvm-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-b-basenames.kvm-srv1}"
      hostname = "horcrux"
    }
    win11-desk1 = {
      vm-name  = "${var.name-prefix}-${local.victim-b-basenames.win11-desk1}"
      hostname = "knockturnalley"
    }
    win11-desk2 = {
      vm-name  = "${var.name-prefix}-${local.victim-b-basenames.win11-desk2}"
      hostname = "quirrell"
    }
    file-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-b-basenames.file-srv1}"
      hostname = "gauntshack"
    }
    mail-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-b-basenames.mail-srv1}"
      hostname = "zonkos"
    }
    # (Optional traffic mirror)
    tmt-srv1 = {
      vm-name  = "${var.name-prefix}-${local.scenario-prefixes.b}-${local.victim-b-basenames.tmt-srv1}"
      hostname = "polyjuice"
    }
  }
  victim-b-security-groups-to-apply = tolist([aws_security_group.victim-b.id, aws_security_group.allow-outbound-victim.id])
}

######################################################################################
## [AD/DC] Windows Server 2022 AD/DC - (1)
## TODO: Potentially 2 if we need Exchange
######################################################################################

module "b-addc-srv1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm-win"
  description       = local.description.addc
  platform          = local.platform.win-srv
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-large
  availability_zone = var.aws-region-az

  name               = local.victim-b-vms.addc-srv1.vm-name
  hostname           = local.victim-b-vms.addc-srv1.hostname
  subnet_id          = aws_subnet.victim-b.id
  private_ip         = local.victim-b-ips.addc-srv1
  security_group_ids = tolist(local.victim-b-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## [VICTIM] File Server - (1)
######################################################################################

module "b-file-srv1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.file
  platform          = local.platform.win-srv
  snapshot_required = true

  ami_id        = module.defaults.ami-windows-server-2022
  instance_type = local.aws-vm-size-large

  availability_zone = var.aws-region-az

  name               = local.victim-b-vms.file-srv1.vm-name
  hostname           = local.victim-b-vms.file-srv1.hostname
  subnet_id          = aws_subnet.victim-b.id
  private_ip         = local.victim-b-ips.file-srv1
  security_group_ids = tolist(local.victim-b-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

# # Should only be uncommented if not using AMIs/snapshots
module "b-file-srv1-add-volume" {
  source            = "../modules/aws/add-volume"
  name              = "${local.victim-b-vms.file-srv1.vm-name}-vol"
  instance_id       = module.b-file-srv1.instance.id
  device_name       = "xvdf"
  disk_size         = local.disk-size.file-srv-disk-2
  disk_type         = local.standard-disk
  availability_zone = var.aws-region-az
  tags              = module.defaults.tags
}

######################################################################################
## [VICTIM] Mail Server - (1)
######################################################################################

module "b-mail-srv1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.mail
  platform          = local.platform.win-srv
  snapshot_required = true

  #   ami_id = module.defaults.ami-windows-server-2022
  ami_id        = local.er6prod5.zonkos
  instance_type = local.aws-vm-size-large

  availability_zone = var.aws-region-az

  name               = local.victim-b-vms.mail-srv1.vm-name
  hostname           = local.victim-b-vms.mail-srv1.hostname
  subnet_id          = aws_subnet.victim-b.id
  private_ip         = local.victim-b-ips.mail-srv1
  security_group_ids = tolist(local.victim-b-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## [VICTIM] Ubuntu KVM Server - (1)
######################################################################################
module "b-kvm-srv1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.kvm
  platform          = local.platform.kvm
  snapshot_required = true

  ami_id = module.defaults.ami-linux-ubuntu-jammy

  instance_type     = local.aws-vm-size-large
  availability_zone = var.aws-region-az

  name               = local.victim-b-vms.kvm-srv1.vm-name
  hostname           = local.victim-b-vms.kvm-srv1.hostname
  subnet_id          = aws_subnet.victim-b.id
  private_ip         = local.victim-b-ips.kvm-srv1
  security_group_ids = tolist(local.victim-b-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  tags = module.defaults.tags
}

######################################################################################
## [VICTIM] Windows 11 Desktops - (2)
# REPLACED WITH Windows Server OS Image, replace with your custom AWS Windows 11 AMI
######################################################################################

module "b-win11-desk1" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.win11-desk
  platform          = local.platform.win11-desk
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-win11
  availability_zone = var.aws-region-az

  name               = local.victim-b-vms.win11-desk1.vm-name
  hostname           = local.victim-b-vms.win11-desk1.hostname
  subnet_id          = aws_subnet.victim-b.id
  private_ip         = local.victim-b-ips.win11-desk1
  security_group_ids = tolist(local.victim-b-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

module "b-win11-desk2" {
  scope             = "victim"
  source            = "../modules/aws/base-vm"
  description       = local.description.win11-desk
  platform          = local.platform.win11-desk
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-win11
  availability_zone = var.aws-region-az

  name               = local.victim-b-vms.win11-desk2.vm-name
  hostname           = local.victim-b-vms.win11-desk2.hostname
  subnet_id          = aws_subnet.victim-b.id
  private_ip         = local.victim-b-ips.win11-desk2
  security_group_ids = tolist(local.victim-b-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk


  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}
