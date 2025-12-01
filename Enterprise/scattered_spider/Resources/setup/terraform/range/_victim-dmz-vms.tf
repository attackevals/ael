# _____________________________________________________________________________________
#
# DMZ VMs
#   * (1) Ubuntu Guacamole Server
# _____________________________________________________________________________________

locals {

  victim-dmz-basenames = {
    guacamole-srv1 = "${local.context.onprem}-${local.machine-names.guacamole-srv1}"
    tmt-srv1       = "${local.context.onprem}-${local.machine-names.tmt-srv1}" # (Optional)
  }

  victim-dmz-ips = {
    guacamole-srv1 = "${local.victim-dmz-ip-prefix}.100"
    tmt-srv1       = "${local.victim-dmz-ip-prefix}.99" # (Optional)
  }

  victim-dmz-vms = {
    guacamole-srv1 = {
      vm-name  = "${var.name-prefix}-${local.victim-dmz-basenames.guacamole-srv1}"
      hostname = "kingswood"
    }
    # (Optional traffic mirror)
    tmt-srv1 = {
      vm-name  = "${var.name-prefix}-${local.context.onprem}-${local.victim-dmz-basenames.tmt-srv1}"
      hostname = "moatcailin"
    }
  }

}


######################################################################################
# SECURITY GROUP
######################################################################################
resource "aws_security_group" "victim-dmz" {
  name_prefix = local.victim-sg-name
  description = local.victim-dmz.sg-description
  vpc_id      = aws_vpc.victim.id

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
      aws_ec2_managed_prefix_list.neutral.id,
      aws_ec2_managed_prefix_list.victim.id
    ]
  }

  lifecycle {
    create_before_destroy = true
  }
  depends_on = [aws_vpc.victim]
  tags = merge(local.victim-onprem-tags, {
    Name        = local.victim-sg-dmz-name
    Description = local.victim-dmz.sg-description
  })
}

######################################################################################
## [VICTIM] Ubuntu Guacamole Server - (1)
######################################################################################
module "dmz-guacamole-srv1" {
  scope             = local.scope.victim
  source            = "../modules/aws/base-vm"
  description       = local.description.guacamole
  platform          = local.platform.guacamole-srv
  snapshot_required = true

  ami_id            = module.amis.linux-ubuntu-noble
  iam_role          = local.roles.cwagent-policy
  instance_type     = local.instance-type-ubuntu-victim
  availability_zone = var.aws-region-az

  name               = local.victim-dmz-vms.guacamole-srv1.vm-name
  hostname           = local.victim-dmz-vms.guacamole-srv1.hostname
  subnet_id          = aws_subnet.victim-dmz.id
  private_ip         = local.victim-dmz-ips.guacamole-srv1
  security_group_ids = tolist(local.victim-dmz-security-groups-to-apply)
  key_name           = aws_key_pair.ssh-key-pair.key_name
  disk_size          = local.disk-size.guacamole-srv
  disk_type          = local.standard-disk
  tags               = local.victim-onprem-tags
}

resource "ansible_host" "dmz-guacamole-srv1" {
  name   = "dmz-guacamole-srv1"
  groups = [local.scope.victim, local.context.onprem, local.ansible_groups.linux-docker, local.ansible_groups.linux-ubuntu, local.ansible_groups.guacamole]

  variables = {
    ansible_host = module.dmz-guacamole-srv1.ip

    hostname = module.dmz-guacamole-srv1.hostname
    fqdn     = module.dmz-guacamole-srv1.hostname
  }
}
