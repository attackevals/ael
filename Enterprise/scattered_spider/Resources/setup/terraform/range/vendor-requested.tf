
# place to put any resources explicitly requested by vendors
# e.g. extra VMs for tooling, networking, etc

#module "baget-srv1" {
#  scope             = "dev"
#  source            = "../modules/aws/base-vm"
#  description       = "[VICTIM] Mirror server"
#  platform          = "ubuntu"
#  snapshot_required = true
#
#  ami_id            = module.amis.linux-ubuntu-jammy
#  instance_type     = local.aws-vm-size-large
#  availability_zone = var.aws-region-az
#
#  subnet_id          = aws_subnet.victim.id
#  name               = "${var.name-prefix}-baget-mirror"
#  hostname           = local.hostname.baget
#  private_ip         = "10.100.0.150"
#  security_group_ids = tolist([aws_security_group.victim.id])
#  key_name           = aws_key_pair.ssh-key-pair.key_name
#  disk_size          = 300
#  disk_type          = local.standard-disk
#  tags               = module.defaults.tags
#}
