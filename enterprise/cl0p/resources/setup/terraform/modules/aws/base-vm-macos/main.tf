## Mac EC2 instance
### Mac dedicated host
resource "aws_ec2_host" "macos-host" {
  instance_type     = var.instance_type
  availability_zone = var.availability_zone

  tags = merge({ Name : var.name }, var.tags)
}

# create vm
resource "aws_instance" "main" {
  ami           = var.ami_id
  instance_type = var.instance_type
  key_name      = var.key_name
  host_id       = aws_ec2_host.macos-host.id

  subnet_id              = var.subnet_id
  private_ip             = var.private_ip
  secondary_private_ips  = var.private_ips
  vpc_security_group_ids = var.security_group_ids
  source_dest_check      = false

  get_password_data = var.get_password_data
  #  user_data                   = local.is_windows ? local.win_user_data : local.lin_user_data
  #  user_data_replace_on_change = false

  iam_instance_profile = var.iam_instance_profile

  root_block_device {
    volume_size           = var.disk_size
    volume_type           = var.disk_type
    delete_on_termination = true

    tags = merge(var.tags, {
      Name        = "${var.name}-vol"
      Description = var.description
      Hostname    = var.hostname
    })
  }

  lifecycle {
    ignore_changes = [
      user_data,
      tags,
      tags_all,
      root_block_device,
    ]
  }

  tags = merge(var.tags, {
    Name             = var.name
    Hostname         = var.hostname
    Description      = var.description
    SnapshotRequired = var.snapshot_required
    Scope            = var.scope
  })
}

# ENI tagging
resource "aws_ec2_tag" "eni_name" {
  resource_id = aws_instance.main.primary_network_interface_id
  key         = "Name"
  value       = "${var.name}-eni"
}
