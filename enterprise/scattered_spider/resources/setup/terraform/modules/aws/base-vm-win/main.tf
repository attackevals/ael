resource "aws_iam_instance_profile" "main-arn" {
  name_prefix = "${var.name}-"
  role        = var.iam_role
  tags        = var.tags
}

# create vm
resource "aws_instance" "main" {
  ami           = var.ami_id
  instance_type = var.instance_type
  key_name      = var.key_name

  subnet_id              = var.subnet_id
  private_ip             = var.private_ip
  secondary_private_ips  = var.private_ips
  vpc_security_group_ids = var.security_group_ids
  source_dest_check      = false

  get_password_data           = var.get_password_data
  user_data                   = local.win_user_data
  user_data_replace_on_change = true

  # add iam profile if not using default (i.e. provided credential)
  iam_instance_profile = aws_iam_instance_profile.main-arn.name

  root_block_device {
    volume_size = var.disk_size
    volume_type = var.disk_type

    delete_on_termination = true

    tags = merge(var.tags, {
      Name        = "${var.name}-vol"
      Description = var.description
      Hostname    = var.hostname
    })
  }

  lifecycle {
    ignore_changes = [
      #       ami,
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

resource "aws_ec2_tag" "eni_name" {
  resource_id = aws_instance.main.primary_network_interface_id
  key         = "Name"
  value       = "${var.name}-eni"
}
