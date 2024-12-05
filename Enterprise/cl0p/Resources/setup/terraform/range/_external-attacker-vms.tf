# _____________________________________________________________________________________
#
# INTERNAL VMS
#   * (1) Kali C2 Server
#   * (1) Windows Server 2022 Jumpbox
#   * (1) Ubuntu Redirector
#   * (1) External DNS Server
#
# _____________________________________________________________________________________

locals {

  attacker-ip = {
    dns-srv1 = "${local.attacker-ip-prefix}.0.60" # 223.246.
    kali1    = "${local.attacker-ip-prefix}.0.70"
    kali1-list = [
      "${local.attacker-ip-prefix}.0.71",
      "${local.attacker-ip-prefix}.0.72"
    ]
    win-jumpbox1 = "${local.attacker-ip-prefix}.0.90"
    win-jumpbox1-list = [
      "${local.attacker-ip-prefix}.0.91",
      "${local.attacker-ip-prefix}.0.92"
    ]
  }

  attacker-basenames = {
    kali1        = "${local.scenario-prefixes.attacker}-kali1"
    win-jumpbox1 = "${local.scenario-prefixes.attacker}-win-jumpbox1"
    dns-srv1     = "${local.scenario-prefixes.attacker}-dns-srv1"
  }

  attacker-vms = {

    kali1 = {
      vm-name  = "${var.name-prefix}-${local.attacker-basenames.kali1}"
      hostname = "corsair"
    }
    win-jumpbox1 = {
      vm-name  = "${var.name-prefix}-${local.attacker-basenames.win-jumpbox1}"
      hostname = "spitfire"
    }
    dns-srv1 = {
      vm-name  = "${var.name-prefix}-${local.attacker-basenames.dns-srv1}"
      hostname = "ronan"
    }
  }

  attacker-host = {
    kali1        = "corsair"
    win-jumpbox1 = "spitfire"
    dns-srv1     = "ronan"
  }

  attacker-security-groups-to-apply = tolist([aws_security_group.attacker.id, aws_security_group.allow-outbound-attacker.id])
}

######################################################################################
## [INTERNAL] Kali
######################################################################################
module "red-kali1" {
  scope             = local.scopes.red
  source            = "../modules/aws/base-vm"
  description       = local.description.kali
  platform          = local.platform.kali
  snapshot_required = false

  ami_id = module.defaults.ami-linux-kali

  instance_type     = local.aws-vm-size-large
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
  tags               = module.defaults.tags
}

######################################################################################
## [INTERNAL] Windows Server 2022 Jumpbox
######################################################################################
module "win-jumpbox1" {
  scope             = local.scopes.red
  source            = "../modules/aws/base-vm"
  description       = local.description.win-jumpbox1
  platform          = local.platform.win-srv
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-large
  availability_zone = var.aws-region-az

  name               = local.attacker-vms.win-jumpbox1.vm-name
  hostname           = local.attacker-vms.win-jumpbox1.hostname
  subnet_id          = aws_subnet.attacker.id
  private_ip         = local.attacker-ip.win-jumpbox1
  private_ips        = local.attacker-ip.win-jumpbox1-list
  security_group_ids = tolist(local.attacker-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk
  key_name           = aws_key_pair.ssh-key-pair.key_name

  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## [INTERNAL] Misc:
#   * (1) Ubuntu Redirector
#   * (1) External DNS Server

######################################################################################

#### [Ubuntu Server] DNS
module "support-dns-srv1" {
  source            = "../modules/aws/base-vm"
  description       = "[SUPPORT] Ubuntu DNS Server"
  platform          = "ubuntu-jammy"
  snapshot_required = false
  scope             = local.scopes.support

  ami_id = module.defaults.ami-linux-ubuntu-jammy

  instance_type     = local.aws-vm-size-small
  availability_zone = var.aws-region-az

  name               = local.attacker-vms.dns-srv1.vm-name
  hostname           = local.attacker-vms.dns-srv1.hostname
  subnet_id          = aws_subnet.attacker.id
  private_ip         = local.attacker-ip.dns-srv1
  security_group_ids = tolist(local.attacker-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.dns
  disk_type          = local.standard-disk
  tags               = module.defaults.tags
}
