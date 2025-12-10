# _____________________________________________________________________________________
#
# External Benevolent VMS
#   * (3/4) Ubuntu Redirectors
#
# _____________________________________________________________________________________

locals {
  redirect-basenames = {
    redirect1 = "${local.context.support}-${local.machine-names.redirect1}"
    redirect2 = "${local.context.support}-${local.machine-names.redirect2}"
    redirect3 = "${local.context.support}-${local.machine-names.redirect3}"
    redirect4 = "${local.context.support}-${local.machine-names.redirect4}"
  }

  redirect-ips = {
    redirect1-ip1 = "${local.neutral-redirect1-prefix}.12.9"    # Redirect-1's IP
    redirect1-ip2 = "${local.neutral-redirect1-prefix}.232.55"  # pasteme.io
    redirect1-ip3 = "${local.neutral-redirect1-prefix}.15.38"   # kingslanding-rmm.com
    redirect1-ip4 = "${local.neutral-redirect1-prefix}.99.108"  # ramsgate.com
    redirect1-ip5 = "${local.neutral-redirect1-prefix}.54.133"  # evals-preflights.org
    redirect1-ip6 = "${local.neutral-redirect1-prefix}.158.146" # sso-kingslanding.net

    redirect2-ip1 = "${local.neutral-redirect2-prefix}.12.9"  # Redirect-2's IP
    redirect2-ip2 = "${local.neutral-redirect2-prefix}.12.21" # karhold.com (ftp server on kali box)
    redirect2-ip3 = "${local.neutral-redirect2-prefix}.12.54" # livesso.com
    redirect2-ip4 = "${local.neutral-redirect2-prefix}.12.59" # mailstreamnet.com
    redirect2-ip5 = "${local.neutral-redirect2-prefix}.12.94" # edupronewscd.com (VScode tunnel handler)

    redirect3-ip1 = "${local.neutral-redirect3-prefix}.44.9"   # Redirect-3's IP
    redirect3-ip2 = "${local.neutral-redirect3-prefix}.44.44"  # pentos-analysis.org
    redirect3-ip3 = "${local.neutral-redirect3-prefix}.44.96"  # faithoftheseven.com
    redirect3-ip4 = "${local.neutral-redirect3-prefix}.44.169" # essos-news.com
    redirect3-ip5 = "${local.neutral-redirect3-prefix}.44.199" # karstarkland.net
    redirect3-ip6 = "${local.neutral-redirect3-prefix}.44.224" # casterlystone.com

    redirect4-ip1 = "${local.neutral-redirect4-prefix}.97.9"   # Redirect-4's IP
    redirect4-ip2 = "${local.neutral-redirect4-prefix}.97.65"  # goldcloaks.org
    redirect4-ip3 = "${local.neutral-redirect4-prefix}.97.72"  # shadowbinders.net
    redirect4-ip4 = "${local.neutral-redirect4-prefix}.97.151" # ironborn.io
    redirect4-ip5 = "${local.neutral-redirect4-prefix}.97.203"
    redirect4-ip6 = "${local.neutral-redirect4-prefix}.97.230"
    redirect4-ip7 = "${local.neutral-redirect4-prefix}.97.251"
  }

  redirect-vms = {
    redirect-srv1 = {
      vm-name  = "${var.name-prefix}-${local.redirect-basenames.redirect1}"
      hostname = "stormlands"
    }
    redirect-srv2 = {
      vm-name  = "${var.name-prefix}-${local.redirect-basenames.redirect2}"
      hostname = "stark"
    }
    redirect-srv3 = {
      vm-name  = "${var.name-prefix}-${local.redirect-basenames.redirect3}"
      hostname = "bolton"
    }
    redirect-srv4 = {
      vm-name  = "${var.name-prefix}-${local.redirect-basenames.redirect4}"
      hostname = "greyjoy"
    }
  }
}


######################################################################################
## [EXTERNAL] Ubuntu Redirectors
######################################################################################
module "redirect-srv1" {
  source            = "../modules/aws/base-vm"
  description       = "${local.description.redirect} (1)"
  platform          = local.platform.redirect
  snapshot_required = false
  scope             = local.scope.internal

  ami_id   = module.amis.linux-ubuntu-noble
  iam_role = local.roles.cwagent-policy

  name               = local.redirect-vms.redirect-srv1.vm-name
  hostname           = local.redirect-vms.redirect-srv1.hostname
  subnet_id          = aws_subnet.neutral-redirect1.id
  private_ip         = local.redirect-ips.redirect1-ip1
  private_ips        = tolist([local.redirect-ips.redirect1-ip2, local.redirect-ips.redirect1-ip3])
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.redirect
  disk_type          = local.standard-disk
  instance_type      = local.instance-type-extra-small
  availability_zone  = var.aws-region-az
  tags               = local.support-tags
}

resource "ansible_host" "redirect-srv1" {
  name   = "redirect-srv1"
  groups = toset([local.scope.internal, local.context.onprem, local.ansible_groups.redirectors, local.ansible_groups.linux-ubuntu])

  variables = {
    ansible_host = module.redirect-srv1.ip

    hostname = module.redirect-srv1.hostname
    fqdn     = module.redirect-srv1.hostname
  }
}

resource "aws_network_interface" "redirect1-b" {

  subnet_id         = aws_subnet.neutral-redirect1.id
  private_ips       = tolist([local.redirect-ips.redirect1-ip4, local.redirect-ips.redirect1-ip5, local.redirect-ips.redirect1-ip6])
  security_groups   = tolist(local.neutral-security-groups-to-apply)
  source_dest_check = false

  attachment {
    instance     = module.redirect-srv1.instance.id
    device_index = 1
  }
  tags = local.support-tags
}

module "redirect-srv2" {
  source            = "../modules/aws/base-vm"
  description       = "${local.description.redirect} (2)"
  platform          = local.platform.redirect
  snapshot_required = false
  scope             = local.scope.internal

  ami_id   = module.amis.linux-ubuntu-noble
  iam_role = local.roles.cwagent-policy

  name               = local.redirect-vms.redirect-srv2.vm-name
  hostname           = local.redirect-vms.redirect-srv2.hostname
  subnet_id          = aws_subnet.neutral-redirect2.id
  private_ip         = local.redirect-ips.redirect2-ip1
  private_ips        = tolist([local.redirect-ips.redirect2-ip2, local.redirect-ips.redirect2-ip3])
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.redirect
  disk_type          = local.standard-disk
  instance_type      = local.instance-type-extra-small
  availability_zone  = var.aws-region-az
  tags               = local.support-tags
}

resource "ansible_host" "redirect-srv2" {
  name   = "redirect-srv2"
  groups = toset([local.scope.internal, local.context.onprem, local.ansible_groups.redirectors, local.ansible_groups.linux-ubuntu])

  variables = {
    ansible_host = module.redirect-srv2.ip

    hostname = module.redirect-srv2.hostname
    fqdn     = module.redirect-srv2.hostname
  }
}

resource "aws_network_interface" "redirect2-b" {

  subnet_id         = aws_subnet.neutral-redirect2.id
  private_ips       = tolist([local.redirect-ips.redirect2-ip4, local.redirect-ips.redirect2-ip5])
  security_groups   = tolist(local.neutral-security-groups-to-apply)
  source_dest_check = false

  attachment {
    instance     = module.redirect-srv2.instance.id
    device_index = 1
  }
  tags = local.support-tags
}

module "redirect-srv3" {
  source            = "../modules/aws/base-vm"
  description       = "${local.description.redirect} (3)"
  platform          = local.platform.redirect
  snapshot_required = false
  scope             = local.scope.internal

  ami_id   = module.amis.linux-ubuntu-noble
  iam_role = local.roles.cwagent-policy

  name               = local.redirect-vms.redirect-srv3.vm-name
  hostname           = local.redirect-vms.redirect-srv3.hostname
  subnet_id          = aws_subnet.neutral-redirect3.id
  private_ip         = local.redirect-ips.redirect3-ip1
  private_ips        = tolist([local.redirect-ips.redirect3-ip2, local.redirect-ips.redirect3-ip3])
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.redirect
  disk_type          = local.standard-disk
  instance_type      = local.instance-type-extra-small
  availability_zone  = var.aws-region-az
  tags               = local.support-tags
}

resource "ansible_host" "redirect-srv3" {
  name   = "redirect-srv3"
  groups = toset([local.scope.internal, local.context.onprem, local.ansible_groups.redirectors, local.ansible_groups.linux-ubuntu])

  variables = {
    ansible_host = module.redirect-srv3.ip

    hostname = module.redirect-srv3.hostname
    fqdn     = module.redirect-srv3.hostname
  }
}

resource "aws_network_interface" "redirect3-b" {

  subnet_id         = aws_subnet.neutral-redirect3.id
  private_ips       = tolist([local.redirect-ips.redirect3-ip4, local.redirect-ips.redirect3-ip5, local.redirect-ips.redirect3-ip6])
  security_groups   = tolist(local.neutral-security-groups-to-apply)
  source_dest_check = false

  attachment {
    instance     = module.redirect-srv3.instance.id
    device_index = 1
  }
  tags = local.support-tags
}

module "redirect-srv4" {
  source            = "../modules/aws/base-vm"
  description       = "${local.description.redirect} (4)"
  platform          = local.platform.redirect
  snapshot_required = false
  scope             = local.scope.internal

  ami_id   = module.amis.linux-ubuntu-noble
  iam_role = local.roles.cwagent-policy

  name               = local.redirect-vms.redirect-srv4.vm-name
  hostname           = local.redirect-vms.redirect-srv4.hostname
  subnet_id          = aws_subnet.neutral-redirect4.id
  private_ip         = local.redirect-ips.redirect4-ip1
  private_ips        = tolist([local.redirect-ips.redirect4-ip2, local.redirect-ips.redirect4-ip3])
  security_group_ids = tolist(local.neutral-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.redirect
  disk_type          = local.standard-disk
  instance_type      = local.instance-type-extra-small
  availability_zone  = var.aws-region-az
  tags               = local.support-tags
}

resource "ansible_host" "redirect-srv4" {
  name   = "redirect-srv4"
  groups = toset([local.scope.internal, local.context.onprem, local.ansible_groups.redirectors, local.ansible_groups.linux-ubuntu])

  variables = {
    ansible_host = module.redirect-srv4.ip

    hostname = module.redirect-srv4.hostname
    fqdn     = module.redirect-srv4.hostname
  }
}

resource "aws_network_interface" "redirect4-b" {

  subnet_id         = aws_subnet.neutral-redirect4.id
  private_ips       = tolist([local.redirect-ips.redirect4-ip4, local.redirect-ips.redirect4-ip5, local.redirect-ips.redirect4-ip6, local.redirect-ips.redirect4-ip7])
  security_groups   = tolist(local.neutral-security-groups-to-apply)
  source_dest_check = false

  attachment {
    instance     = module.redirect-srv4.instance.id
    device_index = 1
  }
  tags = local.support-tags
}
#
# resource "aws_network_interface" "redirect4-c" {
#
#   subnet_id         = aws_subnet.neutral-redirect4.id
#   private_ips       = tolist([local.redirect-ips.redirect4-ip6, local.redirect-ips.redirect4-ip7])
#   security_groups   = tolist(local.neutral-security-groups-to-apply)
#   source_dest_check = false
#
#   attachment {
#     instance     = module.redirect-srv4.instance.id
#     device_index = 2
#   }
#   tags = local.support-tags
# }
