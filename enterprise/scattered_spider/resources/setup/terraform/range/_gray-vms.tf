# _____________________________________________________________________________________
#
# External Benevolent VMS
#   * (1) External DNS Server
#   * (1) Windows 11 Unmanaged Desktop
#   * (1) Ubuntu Server - Tactical RMM
#   * (1) Ubuntu Server - Choco Mirror
#   * (2) Ubuntu Server - Postfix
#   * (1) Ubuntu Server - Remote Desktop Application (RustDesk)
#   * (1) Ubuntu Server - Remote Desktop Application (wstunnel)
#
# _____________________________________________________________________________________

locals {

  neutral-basenames = {
    dns-srv1        = "${local.context.neutral}-${local.machine-names.dns-srv1}"
    unmanaged-desk1 = "${local.context.neutral}-${local.machine-names.unmanaged-desk1}"
    rustdesk-srv1   = "${local.context.neutral}-${local.machine-names.rustdesk-srv1}"
    wstunnel-srv1   = "${local.context.neutral}-${local.machine-names.wstunnel-srv1}"
    postfix-srv1    = "${local.context.neutral}-${local.machine-names.postfix-srv1}"
    postfix-srv2    = "${local.context.neutral}-${local.machine-names.postfix-srv2}"
    postfix-srv3    = "${local.context.neutral}-${local.machine-names.postfix-srv3}"
  }

  neutral-ips = {
    dns-srv1        = "${local.neutral-support-prefix}.0.60"
    unmanaged-desk1 = "${local.neutral-support-prefix}.44.201"
    rmm-srv1        = "${local.neutral-support-prefix}.0.91"
    choco-srv1      = "${local.neutral-support-prefix}.0.92"
    rustdesk-srv1   = "${local.neutral-support-prefix}.0.93"
    wstunnel-srv1   = "${local.neutral-support-prefix}.0.94"
    postfix-srv1    = "${local.neutral-support-prefix}.0.95"
    postfix-srv2    = "${local.neutral-support-prefix}.13.111"
    postfix-srv3    = "${local.neutral-support-prefix}.13.115"
  }

  neutral-vms = {
    dns-srv1 = {
      vm-name  = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.dns-srv1}"
      hostname = "twins"
    }
    unmanaged-desk1 = {
      vm-name  = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.unmanaged-desk1}"
      hostname = "casterlyrock"
    }
    rmm-srv1 = {
      vm-name  = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.rmm-srv1}"
      hostname = "volantis"
    }
    choco-srv1 = {
      vm-name  = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.choco-srv1}"
      hostname = "dorne"
    }
    postfix-srv1 = {
      vm-name  = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.postfix-srv1}"
      hostname = "hull"
    }
    postfix-srv2 = {
      vm-name  = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.postfix-srv2}"
      hostname = "lorath"
    }
    postfix-srv3 = {
      vm-name  = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.postfix-srv3}"
      hostname = "pentos"
    }
    rustdesk-srv1 = {
      vm-name  = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.rustdesk-srv1}"
      hostname = "stonehedge"
    }
    wstunnel-srv1 = {
      vm-name  = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.wstunnel-srv1}"
      hostname = "sunspear"
    }
  }
}

######################################################################################
## [EXTERNAL] DNS Server - (1)
######################################################################################
module "dns-srv1" {
  scope             = local.scope.internal
  source            = "../modules/aws/base-vm"
  description       = local.description.dns
  platform          = local.platform.linux-ubuntu
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-small
  availability_zone = var.aws-region-az

  name               = local.neutral-vms.dns-srv1.vm-name
  hostname           = local.neutral-vms.dns-srv1.hostname
  subnet_id          = aws_subnet.neutral-support.id
  private_ip         = local.neutral-ips.dns-srv1
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.mail-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.support-tags
}

resource "ansible_host" "dns-srv1" {
  name   = "dns-srv1"
  groups = toset([local.scope.internal, local.context.support, local.ansible_groups.linux-ubuntu, local.ansible_groups.dns])

  variables = {
    ansible_host = module.dns-srv1.ip

    hostname = module.dns-srv1.hostname
    fqdn     = module.dns-srv1.hostname
  }
}

######################################################################################
## [EXTERNAL] Unmanaged Windows 11 Desktop - (1)
######################################################################################
module "unmanaged-win11-desk1" {
  scope             = local.scope.attacker # Currently only accessible to red team
  source            = "../modules/aws/base-vm"
  description       = local.description.win11-desk
  platform          = local.platform.win11-desk
  snapshot_required = true
  get_password_data = false

  ami_id            = module.amis.windows-desktop-11
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-win11
  availability_zone = var.aws-region-az

  name               = local.neutral-vms.unmanaged-desk1.vm-name
  hostname           = local.neutral-vms.unmanaged-desk1.hostname
  subnet_id          = aws_subnet.neutral-support.id
  private_ip         = local.neutral-ips.unmanaged-desk1
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  disk_size          = local.disk-size.win11-desk
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  key_name             = aws_key_pair.ssh-key-pair.key_name
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.attacker-tags # Currently only accessible to red team
}

resource "ansible_host" "unmanaged-win11-desk1" {
  name   = "unmanaged-win11-desk1"
  groups = toset([local.scope.victim, local.context.attacker, local.ansible_groups.win-client])

  variables = {
    ansible_host = module.unmanaged-win11-desk1.ip

    hostname = module.unmanaged-win11-desk1.hostname
    fqdn     = module.unmanaged-win11-desk1.hostname
  }
}

######################################################################################
## [NEUTRAL] Ubuntu Choco Mirror (1)
######################################################################################
module "choco-srv1" {
  scope             = local.scope.internal
  source            = "../modules/aws/base-vm"
  description       = local.description.choco
  platform          = local.platform.linux-ubuntu
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-small
  availability_zone = var.aws-region-az

  name               = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.choco-srv1}"
  hostname           = local.neutral-vms.choco-srv1.hostname
  subnet_id          = aws_subnet.neutral-support.id
  private_ip         = local.neutral-ips.choco-srv1
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.mail-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.support-tags
}

resource "ansible_host" "choco-srv1" {
  name   = "choco-srv1"
  groups = toset([local.scope.internal, local.context.support, local.ansible_groups.linux-ubuntu, local.ansible_groups.choco, local.ansible_groups.linux-docker])

  variables = {
    ansible_host = module.choco-srv1.ip

    hostname = module.choco-srv1.hostname
    fqdn     = module.choco-srv1.hostname
  }
}

######################################################################################
## [EXTERNAL] Ubuntu Tactical RMM
######################################################################################
module "rmm-srv1" {
  scope             = local.scope.internal
  source            = "../modules/aws/base-vm"
  description       = local.description.rmm
  platform          = local.platform.rmm
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-small
  availability_zone = var.aws-region-az

  name               = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.rmm-srv1}"
  hostname           = local.neutral-vms.rmm-srv1.hostname
  subnet_id          = aws_subnet.neutral-support.id
  private_ip         = local.neutral-ips.rmm-srv1
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.rmm-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.support-tags
}

resource "ansible_host" "rmm-srv1" {
  name   = "rmm-srv1"
  groups = toset([local.scope.internal, local.context.support, local.ansible_groups.linux-ubuntu, local.ansible_groups.rmm])

  variables = {
    ansible_host = module.rmm-srv1.ip

    hostname = module.rmm-srv1.hostname
    fqdn     = module.rmm-srv1.hostname
  }
}

######################################################################################
## [EXTERNAL] Ubuntu Server - Postfix 1
######################################################################################
module "postfix-srv1" {
  scope             = local.scope.internal
  source            = "../modules/aws/base-vm"
  description       = local.description.postfix
  platform          = local.platform.postfix
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-extra-small
  availability_zone = var.aws-region-az

  name               = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.postfix-srv1}"
  hostname           = local.neutral-vms.postfix-srv1.hostname
  subnet_id          = aws_subnet.neutral-support.id
  private_ip         = local.neutral-ips.postfix-srv1
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.postfix-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.support-tags
}

resource "ansible_host" "postfix-srv1" {
  name   = "postfix-srv1"
  groups = toset([local.scope.internal, local.context.support, local.ansible_groups.linux-ubuntu, local.ansible_groups.postfix])

  variables = {
    ansible_host = module.postfix-srv1.ip

    hostname = module.postfix-srv1.hostname
    fqdn     = module.postfix-srv1.hostname
  }
}

######################################################################################
## [EXTERNAL] Ubuntu Server - Postfix 2
######################################################################################
module "postfix-srv2" {
  scope             = local.scope.internal
  source            = "../modules/aws/base-vm"
  description       = local.description.postfix
  platform          = local.platform.postfix
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-extra-small
  availability_zone = var.aws-region-az

  name               = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.postfix-srv2}"
  hostname           = local.neutral-vms.postfix-srv2.hostname
  subnet_id          = aws_subnet.neutral-support.id
  private_ip         = local.neutral-ips.postfix-srv2
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.postfix-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.support-tags
}

resource "ansible_host" "postfix-srv2" {
  name   = "postfix-srv2"
  groups = toset([local.scope.internal, local.context.support, local.ansible_groups.linux-ubuntu, local.ansible_groups.postfix])

  variables = {
    ansible_host = module.postfix-srv2.ip

    hostname = module.postfix-srv2.hostname
    fqdn     = module.postfix-srv2.hostname
  }
}

######################################################################################
## [EXTERNAL] Ubuntu Server - Postfix 3
######################################################################################
module "postfix-srv3" {
  scope             = local.scope.internal
  source            = "../modules/aws/base-vm"
  description       = local.description.postfix
  platform          = local.platform.postfix
  snapshot_required = true

  ami_id = module.amis.linux-ubuntu-noble

  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-small
  availability_zone = var.aws-region-az

  name               = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.postfix-srv3}"
  hostname           = local.neutral-vms.postfix-srv3.hostname
  subnet_id          = aws_subnet.neutral-support.id
  private_ip         = local.neutral-ips.postfix-srv3
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.postfix-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.support-tags
}

resource "ansible_host" "postfix-srv3" {
  name   = "postfix-srv3"
  groups = toset([local.scope.internal, local.context.support, local.ansible_groups.linux-ubuntu, local.ansible_groups.postfix])

  variables = {
    ansible_host = module.postfix-srv3.ip

    hostname = module.postfix-srv3.hostname
    fqdn     = module.postfix-srv3.hostname
  }
}

######################################################################################
## [EXTERNAL] Ubuntu Server - Remote Desktop Application (RustDesk)
######################################################################################
module "rustdesk-srv1" {
  scope             = local.scope.internal
  source            = "../modules/aws/base-vm"
  description       = local.description.rustdesk
  platform          = local.platform.rustdesk
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-small
  availability_zone = var.aws-region-az

  name               = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.rustdesk-srv1}"
  hostname           = local.neutral-vms.rustdesk-srv1.hostname
  subnet_id          = aws_subnet.neutral-support.id
  private_ip         = local.neutral-ips.rustdesk-srv1
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.rustdesk-srv
  disk_type          = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.support-tags
}

resource "ansible_host" "rustdesk-srv1" {
  name   = "rustdesk-srv1"
  groups = toset([local.scope.internal, local.context.support, local.ansible_groups.linux-ubuntu, local.ansible_groups.rustdesk])

  variables = {
    ansible_host = module.rustdesk-srv1.ip

    hostname = module.rustdesk-srv1.hostname
    fqdn     = module.rustdesk-srv1.hostname
  }
}

######################################################################################
## [EXTERNAL] Ubuntu Server - Remote Desktop Application (wstunnel)
######################################################################################
module "wstunnel-srv1" {
  scope             = local.scope.internal
  source            = "../modules/aws/base-vm"
  description       = local.description.wstunnel
  platform          = local.platform.wstunnel
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-small
  availability_zone = var.aws-region-az

  name               = "${var.name-prefix}-${local.context.neutral}-${local.machine-names.wstunnel-srv1}"
  hostname           = local.neutral-vms.wstunnel-srv1.hostname
  subnet_id          = aws_subnet.neutral-support.id
  private_ip         = local.neutral-ips.wstunnel-srv1
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.wstunnel-srv

  disk_type = local.standard-disk

  admin_username       = var.win_srv_admin_username
  admin_password       = var.win_srv_admin_password
  ssh_private_key_path = var.ssh_private_key_path
  tags                 = local.support-tags
}

resource "ansible_host" "wstunnel-srv1" {
  name   = "wstunnel-srv1"
  groups = toset([local.scope.internal, local.context.support, local.ansible_groups.linux-ubuntu, local.ansible_groups.wstunnel])

  variables = {
    ansible_host = module.wstunnel-srv1.ip

    hostname = module.wstunnel-srv1.hostname
    fqdn     = module.wstunnel-srv1.hostname
  }
}
