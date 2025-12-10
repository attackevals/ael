# _____________________________________________________________________________________
#
# Protections DMZ VMs
#   * (1) Ubuntu Guacamole Server
# _____________________________________________________________________________________

locals {

  b3-dmz-basenames = {
    guacamole-srv1 = "${local.context.b3}-${local.machine-names.guacamole-srv1}"
    mfa-srv1       = "${local.context.b3}-${local.machine-names.mfa-srv1}"
    tmt-srv1       = "${local.context.b3}-${local.machine-names.tmt-srv1}" # (Optional)
  }

  b3-dmz-ips = {
    guacamole-srv1 = "${local.b3-dmz-ip-prefix}.100"
    mfa-srv1       = "${local.b3-dmz-ip-prefix}.101"
    tmt-srv1       = "${local.b3-dmz-ip-prefix}.99" # (Optional)
  }

  b3-dmz-vms = {
    guacamole-srv1 = {
      vm-name  = "${var.name-prefix}-${local.b3-dmz-basenames.guacamole-srv1}"
      hostname = "eastgate"
    }
    mfa-srv1 = {
      vm-name  = "${var.name-prefix}-${local.b3-dmz-basenames.mfa-srv1}"
      hostname = "bloodygate"
    }
    # (Optional traffic mirror)
    tmt-srv1 = {
      vm-name  = "${var.name-prefix}-${local.context.b3}-${local.b3-dmz-basenames.tmt-srv1}"
      hostname = "moatcailin"
    }
  }

}


######################################################################################
# SECURITY GROUP
######################################################################################
resource "aws_security_group" "b3-dmz" {
  name_prefix = local.b3-sg-name
  description = local.b3-dmz.sg-description
  vpc_id      = aws_vpc.b3.id

  # self rule
  ingress {
    from_port = 0
    to_port   = 0
    protocol  = "-1"
    self      = true
  }

  # prefix lists rule
  ingress {
    from_port = 0
    to_port   = 0
    protocol  = "-1"
    prefix_list_ids = [
      aws_ec2_managed_prefix_list.attacker.id,
      aws_ec2_managed_prefix_list.b3.id,
      aws_ec2_managed_prefix_list.main-victim-cloud-protections.id,
      aws_ec2_managed_prefix_list.neutral.id,
      aws_ec2_managed_prefix_list.victim-cloud.id,
      aws_ec2_managed_prefix_list.victim.id,
      aws_ec2_managed_prefix_list.vpn.id,
    ]
  }

  lifecycle {
    create_before_destroy = true
  }
  depends_on = [aws_vpc.b3]
  tags = merge(local.b3-tags, {
    Name        = local.b3-sg-dmz-name
    Description = local.b3-dmz.sg-description
  })
}

######################################################################################
## [Scenario B.3] Ubuntu Guacamole Server - (1)
######################################################################################
module "b3-dmz-guacamole-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.guacamole
  platform          = local.platform.guacamole-srv
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-ubuntu-victim
  availability_zone = var.aws-region-az

  name               = local.b3-dmz-vms.guacamole-srv1.vm-name
  hostname           = local.b3-dmz-vms.guacamole-srv1.hostname
  subnet_id          = aws_subnet.b3-dmz.id
  private_ip         = local.b3-dmz-ips.guacamole-srv1
  security_group_ids = tolist(local.b3-dmz-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.guacamole-srv
  disk_type          = local.standard-disk
  tags               = local.b3-tags
}

resource "ansible_host" "b3-dmz-guacamole-srv1" {
  name   = "b3-dmz-guacamole-srv1"
  groups = [local.scope.victim, local.context.b3, local.ansible_groups.linux-docker, local.ansible_groups.linux-ubuntu, local.ansible_groups.guacamole, local.ansible_groups.domain-vale]

  variables = {
    ansible_host = module.b3-dmz-guacamole-srv1.ip
    hostname     = module.b3-dmz-guacamole-srv1.hostname
    fqdn         = module.b3-dmz-guacamole-srv1.hostname
  }
}

######################################################################################
## [B.3] Ubuntu MFA Server - (1)
######################################################################################
module "b3-dmz-mfa-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.mfa
  platform          = local.platform.mfa
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-ubuntu-victim
  availability_zone = var.aws-region-az

  name               = local.b3-dmz-vms.mfa-srv1.vm-name
  hostname           = local.b3-dmz-vms.mfa-srv1.hostname
  subnet_id          = aws_subnet.b3-dmz.id
  private_ip         = local.b3-dmz-ips.mfa-srv1
  security_group_ids = tolist(local.b3-dmz-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.mfa-srv
  disk_type          = local.standard-disk
  tags               = local.b3-tags
}

resource "ansible_host" "b3-dmz-mfa-srv1" {
  name   = "b3-dmz-mfa-srv1"
  groups = toset([local.scope.victim, local.context.b3, local.ansible_groups.linux-ubuntu, local.ansible_groups.linux-docker, local.ansible_groups.mfa, local.ansible_groups.domain-vale])

  variables = {
    ansible_host = module.b3-dmz-mfa-srv1.ip
    hostname     = module.b3-dmz-mfa-srv1.hostname
    fqdn         = module.b3-dmz-mfa-srv1.hostname
    aws_provider = local.account-protections-id
  }
}
