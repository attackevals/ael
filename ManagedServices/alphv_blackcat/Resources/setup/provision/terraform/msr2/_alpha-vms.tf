######################################################################################
## [VMs] Servers
#   * [Windows] AD/DC
#   * [Windows] Exchange
#   * [Windows] IIS
######################################################################################

#### [A - Windows VM Server] AD/DC
module "a-ad-srv1" {
  source            = "../../modules/aws/base-vm-win"
  description       = "[A] Windows Server AD"
  platform          = "windows-server"
  snapshot_required = true
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-server-2022
  get_password_data = false


  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.a.ad
  hostname           = local.name.host.a.ad
  subnet_id          = aws_subnet.a_srv.id
  private_ip         = local.ip.a.ad
  security_group_ids = tolist([aws_security_group.alpha.id])
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk_size.a.ad
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

#### [A - Windows VM Server] Exchange
module "a-ex-srv1" {
  source            = "../../modules/aws/base-vm-win"
  description       = "[A] Windows Exchange Server"
  platform          = "windows-server"
  snapshot_required = true
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-server-2022
  get_password_data = false

  instance_type     = local.aws-vm-size-large
  availability_zone = var.aws-region-az

  name               = local.name.vm.a.ex
  hostname           = local.name.host.a.ex
  subnet_id          = aws_subnet.a_srv.id
  private_ip         = local.ip.a.ex
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.a.ex
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

#### [A - Windows VM Server] IIS
module "a-iis-srv1" {
  source            = "../../modules/aws/base-vm-win"
  description       = "[A] Windows IIS Server"
  platform          = "windows-server"
  snapshot_required = true
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-server-2022
  get_password_data = false

  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.a.iis
  hostname           = local.name.host.a.iis
  subnet_id          = aws_subnet.a_srv.id
  private_ip         = local.ip.a.iis
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.a.iis
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## [VMs] Desktops
#   * [Windows] Bastion 1
#   * [Windows] Generic 2
#   * [Windows] Generic 3
######################################################################################

# [A - Windows VM Desktop] Generic 1
module "a-desk1" {
  source            = "../../modules/aws/base-vm"
  description       = "[A] Windows Desktop Victim VM on domain"
  platform          = "windows-desk"
  snapshot_required = true
  get_password_data = false
  scope             = "victim"

  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  ami_id             = module.defaults.ami-windows-desktop-11
  name               = local.name.vm.a.desk1
  hostname           = local.name.host.a.desk1
  subnet_id          = aws_subnet.a_desk.id
  private_ip         = local.ip.a.desk1
  private_ips        = local.ip.a.desk1-list
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.a.desk1
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

## [A - Windows VM Desktop] Generic 2
module "a-desk2" {
  source            = "../../modules/aws/base-vm"
  description       = "[A] Windows Desktop Victim VM on domain"
  platform          = "windows-desk"
  snapshot_required = true
  get_password_data = false
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-desktop-11
  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.a.desk2
  hostname           = local.name.host.a.desk2
  subnet_id          = aws_subnet.a_desk.id
  private_ip         = local.ip.a.desk2
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.a.desk2
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

#### [A - Windows VM Desktop] Generic 3
module "a-desk3" {
  source            = "../../modules/aws/base-vm"
  description       = "[A] Windows Desktop Victim VM on domain"
  platform          = "windows-desk"
  snapshot_required = true
  get_password_data = false
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-desktop-11
  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.a.desk3
  hostname           = local.name.host.a.desk3
  subnet_id          = aws_subnet.a_desk.id
  private_ip         = local.ip.a.desk3
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.a.desk3
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

#######################################################################################
### [VMs] Victim LAN - Servers
##   * [Windows] AD/DC
##   * [Windows] Microsoft Exchange
##   * [Windows] File Server
##   * [Windows] SQL Server
##   * [Linux]   KVM Server
#######################################################################################

## [B - VM Server] Windows AD/DC
module "b-ad-srv1" {
  source            = "../../modules/aws/base-vm-win"
  description       = "[B] Windows AD Server"
  platform          = "windows-server-addc"
  snapshot_required = true
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-server-2022
  get_password_data = false

  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.b.ad
  hostname           = local.name.host.b.ad
  subnet_id          = aws_subnet.b_combined.id
  private_ip         = local.ip.b.ad
  security_group_ids = tolist([aws_security_group.alpha.id])
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk_size.b.ad
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

### [B - Windows VM Server] Microsoft Exchange
module "b-ex-srv1" {
  source            = "../../modules/aws/base-vm-win"
  description       = "[B] Windows Exchange Server"
  platform          = "windows-server"
  snapshot_required = true
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-server-2022
  get_password_data = false

  instance_type     = local.aws-vm-size-large
  availability_zone = var.aws-region-az

  name               = local.name.vm.b.ex
  hostname           = local.name.host.b.ex
  subnet_id          = aws_subnet.b_combined.id
  private_ip         = local.ip.b.ex
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.b.ex
  disk_type          = local.standard-disk
  key_name           = aws_key_pair.ssh-key-pair.key_name

  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

### [B - Windows VM Server] File Server
module "b-file-srv1" {
  source            = "../../modules/aws/base-vm-win"
  description       = "[B] Windows File Server"
  platform          = "windows-server"
  snapshot_required = true
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-server-2022
  get_password_data = false

  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.b.file
  hostname           = local.name.host.b.file
  subnet_id          = aws_subnet.b_combined.id
  private_ip         = local.ip.b.file
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.b.file
  disk_type          = local.standard-disk
  key_name           = aws_key_pair.ssh-key-pair.key_name

  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

module "b-file-srv1-add-volume" {
  source            = "../../modules/aws/add-volume"
  name              = "${local.name.vm.b.file}-vol"
  instance_id       = module.b-file-srv1.instance.id
  device_name       = "xvdf"
  disk_size         = local.disk_size.b.file-vol
  disk_type         = local.standard-disk
  availability_zone = var.aws-region-az
  tags              = module.defaults.tags
}

### [B - Windows VM Server] SQL Server
module "b-sql-srv1" {
  source            = "../../modules/aws/base-vm-win"
  description       = "[B] Windows SQL Server"
  platform          = "windows-server"
  snapshot_required = true
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-sql-server-2022
  get_password_data = false

  instance_type     = module.defaults.aws-vm-size.large
  availability_zone = var.aws-region-az

  name               = local.name.vm.b.sql
  hostname           = local.name.host.b.sql
  subnet_id          = aws_subnet.b_combined.id
  private_ip         = local.ip.b.sql
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.b.sql
  disk_type          = local.standard-disk
  key_name           = aws_key_pair.ssh-key-pair.key_name

  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

### [B - Linux VM Server] KVM Server
module "b-kvm-srv1" {
  source            = "../../modules/aws/base-vm"
  description       = "[B] Linux KVM Server"
  platform          = "ubuntu-jammy"
  snapshot_required = true
  scope             = "victim"

  ami_id = module.defaults.ami-linux-ubuntu-jammy
  #   instance_type     = module.defaults.aws-vm-size.baremetal
  # NOTE: Switch to baremetal type above to use KVM to launch VMs
  # by default uses standard EC2 instance to reduce cost.
  instance_type     = module.defaults.aws-vm-size.medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.b.kvm
  hostname           = local.name.host.b.kvm
  subnet_id          = aws_subnet.b_combined.id
  private_ip         = local.ip.b.kvm
  security_group_ids = tolist([aws_security_group.alpha.id])
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk_size.b.kvm
  disk_type          = local.standard-disk
  tags               = module.defaults.tags
}

#######################################################################################
### [VMs] Victim LAN - Desktops
##   * [Windows] Local Admin 1
##   * [Windows] Generic 2
##   * [Windows] Generic 3
#######################################################################################

### [B - Windows VM Desktop] Generic 1 (Local Admin)
module "b-desk1" {
  source            = "../../modules/aws/base-vm"
  description       = "[B] Windows Desktop Victim VM on domain"
  platform          = "windows-desk"
  snapshot_required = true
  get_password_data = false
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-desktop-11
  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.b.desk1
  hostname           = local.name.host.b.desk1
  subnet_id          = aws_subnet.b_combined.id
  private_ip         = local.ip.b.desk1
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.b.desk1
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

### [B - Windows VM Desktop] Generic 2
module "b-desk2" {
  source            = "../../modules/aws/base-vm"
  description       = "[B] Windows Desktop Victim VM on domain"
  platform          = "windows-desk"
  snapshot_required = true
  get_password_data = false
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-desktop-11
  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.b.desk2
  hostname           = local.name.host.b.desk2
  subnet_id          = aws_subnet.b_combined.id
  private_ip         = local.ip.b.desk2
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.b.desk2
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

### [B - Windows VM Desktop] Generic 3
module "b-desk3" {
  source            = "../../modules/aws/base-vm"
  description       = "[B] Windows Desktop Victim VM on domain"
  platform          = "windows-desk"
  snapshot_required = true
  get_password_data = false
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-desktop-11
  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.b.desk3
  hostname           = local.name.host.b.desk3
  subnet_id          = aws_subnet.b_combined.id
  private_ip         = local.ip.b.desk3
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.b.desk3
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## [VMs] Victim LAN - Desktop
#   * [Windows] Generic 1
######################################################################################

# [C - Windows VM Desktop] | Generic 1
module "c-desk1" {
  source            = "../../modules/aws/base-vm"
  description       = "[C] Windows Desktop Victim VM on domain"
  platform          = "windows-desk"
  snapshot_required = true
  get_password_data = false
  scope             = "victim"

  ami_id            = module.defaults.ami-windows-desktop-11
  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.name.vm.c.desk1
  hostname           = local.name.host.c.desk1
  subnet_id          = aws_subnet.c_desk.id
  private_ip         = local.ip.c.desk1
  security_group_ids = tolist([aws_security_group.alpha.id])
  disk_size          = local.disk_size.c.desk1
  disk_type          = local.standard-disk

  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}
