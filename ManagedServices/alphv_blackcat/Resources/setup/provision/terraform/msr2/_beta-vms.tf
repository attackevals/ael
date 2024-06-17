#####################################################################################
# [VMs] Red LAN
#####################################################################################

### [Red - Linux VM Server] Kali Linux
module "red-kali1" {
  source            = "../../modules/aws/base-vm"
  description       = "[RED] Kali Attack Platform VM"
  platform          = "kali"
  snapshot_required = false
  scope             = "internal"

  ami_id            = module.defaults.ami-linux-kali
  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.red.kali
  hostname           = local.name.host.red.kali
  subnet_id          = aws_subnet.support_red.id
  private_ip         = local.ip.red.kali
  private_ips        = local.ip.red.kali-list
  security_group_ids = tolist([aws_security_group.beta.id])
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk_size.red.kali
  disk_type          = local.standard-disk
  tags               = module.defaults.tags
}


#####################################################################################
# [VMs] Support - Servers
#   * Jumpbox
#   * DNS
#   * Proxy
#   * Redirector for A
#   * Redirector for B
#   * Postfix
#####################################################################################
### [Support - Windows VM Server] Jumpbox
module "support-jumpbox-srv1" {
  source            = "../../modules/aws/base-vm-win"
  description       = "[SUPPORT] Windows Jumpbox Server"
  platform          = "windows-server"
  snapshot_required = false
  scope             = "internal"

  ami_id            = module.defaults.ami-windows-server-2022
  get_password_data = false

  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.support.jumpbox
  hostname           = local.name.host.support.jumpbox
  subnet_id          = aws_subnet.support_b.id
  private_ip         = local.ip.support.jumpbox
  private_ips        = local.ip.support.jumpbox-list
  security_group_ids = tolist([aws_security_group.beta.id])
  disk_size          = local.disk_size.support.jumpbox
  disk_type          = local.standard-disk
  key_name           = aws_key_pair.ssh-key-pair.key_name

  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

#### [Support - Linux VM Server] DNS
module "support-dns-srv1" {
  source            = "../../modules/aws/base-vm"
  description       = "[SUPPORT] Ubuntu DNS Server"
  platform          = "ubuntu-jammy"
  snapshot_required = false
  scope             = "internal"

  ami_id            = module.defaults.ami-linux-ubuntu-jammy
  instance_type     = module.defaults.aws-vm-size.small
  availability_zone = var.aws-region-az

  name               = local.name.vm.support.dns
  hostname           = local.name.host.support.dns
  subnet_id          = aws_subnet.support_b.id
  private_ip         = local.ip.support.dns
  security_group_ids = tolist([aws_security_group.beta.id])
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk_size.support.dns
  disk_type          = local.standard-disk
  tags               = module.defaults.tags
}

### [Support - Linux VM Server] Postfix
module "support-pf-srv1" {
  source            = "../../modules/aws/base-vm"
  description       = "[SUPPORT] Ubuntu Postfix Server"
  platform          = "ubuntu-jammy"
  snapshot_required = false
  scope             = "internal"

  ami_id            = module.defaults.ami-linux-ubuntu-jammy
  instance_type     = module.defaults.aws-vm-size.small
  availability_zone = var.aws-region-az

  name               = local.name.vm.support.postfix
  hostname           = local.name.host.support.postfix
  subnet_id          = aws_subnet.support_b.id
  private_ip         = local.ip.support.postfix
  security_group_ids = tolist([aws_security_group.beta.id])
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk_size.support.postfix
  disk_type          = local.standard-disk
  tags               = module.defaults.tags
}

# --------------------------------------------------------------------------------------
# Redirectors
# A/MenuPass split across 2 VMs due to 3 NIC/VM limit
# --------------------------------------------------------------------------------------

#### [Support - Linux VM Server] Redirector for A [1/2]
module "support-redirect-srv1" {
  source            = "../../modules/aws/base-vm"
  description       = "[SUPPORT] Redirect Ubuntu Server (1)"
  platform          = "ubuntu-jammy"
  snapshot_required = false
  scope             = "internal"

  ami_id             = module.defaults.ami-linux-ubuntu-jammy
  name               = local.name.vm.support.redirect1
  hostname           = local.name.host.support.redirect1
  subnet_id          = aws_subnet.support_a.id
  private_ip         = local.ip.support.redirect-a1
  private_ips        = local.ip.support.redirect-a1-list
  security_group_ids = tolist([aws_security_group.beta.id])
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk_size.support.redirect1
  disk_type          = local.standard-disk
  instance_type      = local.aws-vm-size-small
  availability_zone  = var.aws-region-az
  tags               = module.defaults.tags
}

#### [Support - Linux VM Server] Redirector for A [2/2]
module "support-redirect-srv2" {
  source            = "../../modules/aws/base-vm"
  description       = "[SUPPORT] Redirect Ubuntu Server (2)"
  platform          = "ubuntu-jammy"
  snapshot_required = false
  scope             = "internal"

  ami_id             = module.defaults.ami-linux-ubuntu-jammy
  name               = local.name.vm.support.redirect2
  hostname           = local.name.host.support.redirect2
  subnet_id          = aws_subnet.support_a.id
  private_ip         = local.ip.support.redirect-a2
  private_ips        = local.ip.support.redirect-a2-list
  security_group_ids = tolist([aws_security_group.beta.id])
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk_size.support.redirect1
  disk_type          = local.standard-disk
  instance_type      = local.aws-vm-size-small
  availability_zone  = var.aws-region-az
  tags               = module.defaults.tags
}

#### [Support - Linux VM Server] Redirector for B
module "support-redirect-srv3" {
  source            = "../../modules/aws/base-vm"
  description       = "[SUPPORT] Redirect Ubuntu Server (3)"
  platform          = "ubuntu-jammy"
  snapshot_required = false
  scope             = "internal"

  ami_id             = module.defaults.ami-linux-ubuntu-jammy
  name               = local.name.vm.support.redirect3
  hostname           = local.name.host.support.redirect3
  subnet_id          = aws_subnet.support_b.id
  private_ip         = local.ip.support.redirect-b
  private_ips        = local.ip.support.redirect-b-list
  security_group_ids = tolist([aws_security_group.beta.id])
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk_size.support.redirect3
  disk_type          = local.standard-disk
  instance_type      = local.aws-vm-size-small
  availability_zone  = var.aws-region-az
  tags               = module.defaults.tags
}
