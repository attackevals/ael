# _____________________________________________________________________________________
#
# External Benevolent VMS
#   * (1) Kali C2 Server
#   * (1) Windows Server 2022 Jumpbox
#   * (1) Ubuntu Redirector
#   * (1) External DNS Server
#
# _____________________________________________________________________________________

locals {

  external-benevolent-basenames = {
    redirect1       = "${local.scenario-prefixes.external-benevolent}-redirect-srv1"
    redirect2       = "${local.scenario-prefixes.external-benevolent}-redirect-srv2"
    redirect3       = "${local.scenario-prefixes.external-benevolent}-redirect-srv3"
    redirect4       = "${local.scenario-prefixes.external-benevolent}-redirect-srv4"
    remote-worker-1 = "${local.scenario-prefixes.external-benevolent}-remote-worker-srv1"
    remote-worker-2 = "${local.scenario-prefixes.external-benevolent}-remote-worker-srv2"
    choco-mirror    = "${local.scenario-prefixes.external-benevolent}-choco-mirror-srv1"
  }

  external-benevolent-ips = {
    redirect1-ip1 = "${local.ext-benev-redirect1-prefix}.222.9"
    redirect1-ip2 = "${local.ext-benev-redirect1-prefix}.222.11"
    redirect1-ip3 = "${local.ext-benev-redirect1-prefix}.201.33"
    redirect1-ip4 = "${local.ext-benev-redirect1-prefix}.11.99"
    redirect1-ip5 = "${local.ext-benev-redirect1-prefix}.35.185"
    redirect1-ip6 = "${local.ext-benev-redirect1-prefix}.97.244"

    redirect2-ip1 = "${local.ext-benev-redirect2-prefix}.12.9"
    redirect2-ip2 = "${local.ext-benev-redirect2-prefix}.12.21"
    redirect2-ip3 = "${local.ext-benev-redirect2-prefix}.12.54"
    redirect2-ip4 = "${local.ext-benev-redirect2-prefix}.12.59"
    redirect2-ip5 = "${local.ext-benev-redirect2-prefix}.12.94"

    redirect3-ip1 = "${local.ext-benev-redirect3-prefix}.44.9"
    redirect3-ip2 = "${local.ext-benev-redirect3-prefix}.44.44"
    redirect3-ip3 = "${local.ext-benev-redirect3-prefix}.44.96"
    redirect3-ip4 = "${local.ext-benev-redirect3-prefix}.44.169"
    redirect3-ip5 = "${local.ext-benev-redirect3-prefix}.44.199"
    redirect3-ip6 = "${local.ext-benev-redirect3-prefix}.44.224"

    redirect4-ip1 = "${local.ext-benev-redirect4-prefix}.97.9"
    redirect4-ip2 = "${local.ext-benev-redirect4-prefix}.97.65"
    redirect4-ip3 = "${local.ext-benev-redirect4-prefix}.97.72"
    redirect4-ip4 = "${local.ext-benev-redirect4-prefix}.97.151"
    redirect4-ip5 = "${local.ext-benev-redirect4-prefix}.97.203"
    redirect4-ip6 = "${local.ext-benev-redirect4-prefix}.97.230"
    redirect4-ip7 = "${local.ext-benev-redirect4-prefix}.97.251"

    remote-worker-1-ip1 = "${local.ext-benev-redirect3-prefix}.44.201"
    remote-worker-2-ip1 = "${local.ext-benev-redirect2-prefix}.12.74"

    choco-mirror-ip1 = "${local.ext-benev-redirect2-prefix}.12.121"
  }

  external-benevolent-vms = {
    redirect-srv1 = {
      vm-name  = "${var.name-prefix}-${local.external-benevolent-basenames.redirect1}"
      hostname = "paladin"
    }
    redirect-srv2 = {
      vm-name  = "${var.name-prefix}-${local.external-benevolent-basenames.redirect2}"
      hostname = "asbestosboy"
    }
    redirect-srv3 = {
      vm-name  = "${var.name-prefix}-${local.external-benevolent-basenames.redirect3}"
      hostname = "hindsightlad"
    }
    redirect-srv4 = {
      vm-name  = "${var.name-prefix}-${local.external-benevolent-basenames.redirect4}"
      hostname = "doorman"
    }
    remote-worker-srv1 = {
      vm-name  = "${var.name-prefix}-${local.external-benevolent-basenames.remote-worker-1}"
      hostname = "eyescream"
    }
    remote-worker-srv2 = {
      vm-name  = "${var.name-prefix}-${local.external-benevolent-basenames.remote-worker-2}"
      hostname = "obnoxio"
    }
    choco-mirror-srv1 = {
      vm-name  = "${var.name-prefix}-${local.external-benevolent-basenames.choco-mirror}"
      hostname = "looppedal"
    }
  }
  ext-benev-security-groups-to-apply = tolist([aws_security_group.attacker.id, aws_security_group.allow-outbound-attacker.id])

}

#### [Ubuntu Server] Redirector

module "redirect-srv1" {
  source            = "../modules/aws/base-vm"
  description       = "${local.description.redirect} (1)"
  platform          = local.platform.redirect
  snapshot_required = false
  scope             = local.scopes.internal

  ami_id = module.defaults.ami-linux-ubuntu-jammy

  name               = local.external-benevolent-vms.redirect-srv1.vm-name
  hostname           = local.external-benevolent-vms.redirect-srv1.hostname
  subnet_id          = aws_subnet.ext-benev-redirect1.id
  private_ip         = local.external-benevolent-ips.redirect1-ip1
  private_ips        = tolist([local.external-benevolent-ips.redirect1-ip2, local.external-benevolent-ips.redirect1-ip3])
  security_group_ids = tolist(local.ext-benev-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.redirect
  disk_type          = local.standard-disk
  instance_type      = local.aws-vm-size-small
  availability_zone  = var.aws-region-az
  tags               = module.defaults.tags
}

resource "aws_network_interface" "redirect1-b" {

  subnet_id         = aws_subnet.ext-benev-redirect1.id
  private_ips       = tolist([local.external-benevolent-ips.redirect1-ip4, local.external-benevolent-ips.redirect1-ip5, local.external-benevolent-ips.redirect1-ip6])
  security_groups   = tolist(local.ext-benev-security-groups-to-apply)
  source_dest_check = false

  attachment {
    instance     = module.redirect-srv1.instance.id
    device_index = 1
  }
  tags = module.defaults.tags
}

#### [Ubuntu Server] Redirector

module "redirect-srv2" {
  source            = "../modules/aws/base-vm"
  description       = "${local.description.redirect} (2)"
  platform          = local.platform.redirect
  snapshot_required = false
  scope             = local.scopes.internal

  ami_id = module.defaults.ami-linux-ubuntu-jammy

  name               = local.external-benevolent-vms.redirect-srv2.vm-name
  hostname           = local.external-benevolent-vms.redirect-srv2.hostname
  subnet_id          = aws_subnet.ext-benev-redirect2.id
  private_ip         = local.external-benevolent-ips.redirect2-ip1
  private_ips        = tolist([local.external-benevolent-ips.redirect2-ip2, local.external-benevolent-ips.redirect2-ip3])
  security_group_ids = tolist(local.ext-benev-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.redirect
  disk_type          = local.standard-disk
  instance_type      = local.aws-vm-size-small
  availability_zone  = var.aws-region-az
  tags               = module.defaults.tags
}
resource "aws_network_interface" "redirect2-b" {

  subnet_id         = aws_subnet.ext-benev-redirect2.id
  private_ips       = tolist([local.external-benevolent-ips.redirect2-ip4, local.external-benevolent-ips.redirect2-ip5])
  security_groups   = tolist(local.ext-benev-security-groups-to-apply)
  source_dest_check = false

  attachment {
    instance     = module.redirect-srv2.instance.id
    device_index = 1
  }
  tags = module.defaults.tags
}


#### [Ubuntu Server] Redirector
module "redirect-srv3" {
  source            = "../modules/aws/base-vm"
  description       = "${local.description.redirect} (3)"
  platform          = local.platform.redirect
  snapshot_required = false
  scope             = local.scopes.internal

  ami_id = module.defaults.ami-linux-ubuntu-jammy

  name               = local.external-benevolent-vms.redirect-srv3.vm-name
  hostname           = local.external-benevolent-vms.redirect-srv3.hostname
  subnet_id          = aws_subnet.ext-benev-redirect3.id
  private_ip         = local.external-benevolent-ips.redirect3-ip1
  private_ips        = tolist([local.external-benevolent-ips.redirect3-ip2, local.external-benevolent-ips.redirect3-ip3])
  security_group_ids = tolist(local.ext-benev-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.redirect
  disk_type          = local.standard-disk
  instance_type      = local.aws-vm-size-small
  availability_zone  = var.aws-region-az
  tags               = module.defaults.tags
}

resource "aws_network_interface" "redirect3-b" {

  subnet_id         = aws_subnet.ext-benev-redirect3.id
  private_ips       = tolist([local.external-benevolent-ips.redirect3-ip4, local.external-benevolent-ips.redirect3-ip5, local.external-benevolent-ips.redirect3-ip6])
  security_groups   = tolist(local.ext-benev-security-groups-to-apply)
  source_dest_check = false

  attachment {
    instance     = module.redirect-srv3.instance.id
    device_index = 1
  }
  tags = module.defaults.tags
}

#### [Ubuntu Server] Redirector
module "redirect-srv4" {
  source            = "../modules/aws/base-vm"
  description       = "${local.description.redirect} (4)"
  platform          = local.platform.redirect
  snapshot_required = false
  scope             = local.scopes.internal

  ami_id = module.defaults.ami-linux-ubuntu-jammy

  name               = local.external-benevolent-vms.redirect-srv4.vm-name
  hostname           = local.external-benevolent-vms.redirect-srv4.hostname
  subnet_id          = aws_subnet.ext-benev-redirect4.id
  private_ip         = local.external-benevolent-ips.redirect4-ip1
  private_ips        = tolist([local.external-benevolent-ips.redirect4-ip2, local.external-benevolent-ips.redirect4-ip3])
  security_group_ids = tolist(local.ext-benev-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.redirect
  disk_type          = local.standard-disk
  instance_type      = local.aws-vm-size-small
  availability_zone  = var.aws-region-az
  tags               = module.defaults.tags
}


resource "aws_network_interface" "redirect4-b" {

  subnet_id         = aws_subnet.ext-benev-redirect4.id
  private_ips       = tolist([local.external-benevolent-ips.redirect4-ip4, local.external-benevolent-ips.redirect4-ip5])
  security_groups   = tolist(local.ext-benev-security-groups-to-apply)
  source_dest_check = false

  attachment {
    instance     = module.redirect-srv4.instance.id
    device_index = 1
  }
  tags = module.defaults.tags
}

resource "aws_network_interface" "redirect4-c" {

  subnet_id         = aws_subnet.ext-benev-redirect4.id
  private_ips       = tolist([local.external-benevolent-ips.redirect4-ip6, local.external-benevolent-ips.redirect4-ip7])
  security_groups   = tolist(local.ext-benev-security-groups-to-apply)
  source_dest_check = false

  attachment {
    instance     = module.redirect-srv4.instance.id
    device_index = 2
  }
  tags = module.defaults.tags
}

######################################################################################
## Remote Worker 1
######################################################################################

module "remote-worker-srv1" {
  scope             = local.scopes.internal
  source            = "../modules/aws/base-vm-win"
  description       = "${local.description.remote-worker} (1)"
  platform          = local.platform.win-srv
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.external-benevolent-vms.remote-worker-srv1.vm-name
  hostname           = local.external-benevolent-vms.remote-worker-srv1.hostname
  subnet_id          = aws_subnet.ext-benev-redirect3.id ## Remote Worker 1 sits on Redirect 3 subnet
  private_ip         = local.external-benevolent-ips.remote-worker-1-ip1
  security_group_ids = tolist(local.ext-benev-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

######################################################################################
## Remote Worker 2
######################################################################################

module "remote-worker-srv2" {
  scope             = local.scopes.internal
  source            = "../modules/aws/base-vm-win"
  description       = "${local.description.remote-worker} (2)"
  platform          = local.platform.win-srv
  snapshot_required = true
  get_password_data = false

  ami_id = module.defaults.ami-windows-server-2022

  instance_type     = local.aws-vm-size-medium
  availability_zone = var.aws-region-az

  name               = local.external-benevolent-vms.remote-worker-srv2.vm-name
  hostname           = local.external-benevolent-vms.remote-worker-srv2.hostname
  subnet_id          = aws_subnet.ext-benev-redirect2.id ## Remote Worker 2 sits on Redirect 2 subnet
  private_ip         = local.external-benevolent-ips.remote-worker-2-ip1
  security_group_ids = tolist(local.ext-benev-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.addc-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = module.defaults.tags
}

module "choco-mirror-srv1" {
  scope             = local.scopes.support
  source            = "../modules/aws/base-vm"
  description       = local.description.choco-mirror
  platform          = local.platform.choco-mirror
  snapshot_required = true

  ami_id = module.defaults.ami-linux-ubuntu-jammy

  instance_type     = local.aws-vm-size-small
  availability_zone = var.aws-region-az

  subnet_id          = aws_subnet.ext-benev-redirect2.id
  name               = local.external-benevolent-vms.choco-mirror-srv1.vm-name
  hostname           = local.external-benevolent-vms.choco-mirror-srv1.hostname
  private_ip         = local.external-benevolent-ips.choco-mirror-ip1
  security_group_ids = tolist(local.ext-benev-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.choco-mirror
  disk_type          = local.standard-disk
  tags               = module.defaults.tags
}
