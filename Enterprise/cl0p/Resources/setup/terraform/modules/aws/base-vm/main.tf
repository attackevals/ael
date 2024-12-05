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

  associate_public_ip_address = var.associate_public_ip

  # TODO: Get this working with user_data across platforms
  get_password_data           = var.get_password_data
  user_data                   = local.is_windows ? local.win_user_data : local.lin_user_data
  user_data_replace_on_change = false

  # add iam profile if not using default (i.e. provided credential)
  iam_instance_profile = var.iam_instance_profile

  # add storage if not using default (i.e. EBS)
  # root_block_device {
  #   volume_size = var.disk_size
  # }

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
      #       ami,
      user_data,
      tags,
      tags_all,
      root_block_device,
    ]
  }

  ### may incorporate (ricky)
  # instance_market_options {
  #   market_type  = var.market_type
  #   spot_options = var.spot_options
  # }
  # maintenance_options { auto_recovery = var.auto_recovery }
  # metadata_options { }

  tags = merge(var.tags, {
    Name             = var.name
    Hostname         = var.hostname
    Description      = var.description
    SnapshotRequired = var.snapshot_required
    Scope            = var.scope
  })
}

# TODO: for_each won't work, but we need tagging for at least the ENI
resource "aws_ec2_tag" "eni_name" {
  resource_id = aws_instance.main.primary_network_interface_id
  key         = "Name"
  value       = "${var.name}-eni"
}

############################################################################
### Alternative way of creating Disk, more messy and less integrated
### Keeping for the sake of Snapshot conflicts
############################################################################

# Create Manage Disk
# resource "aws_ebs_volume" "main" {
#   availability_zone    = var.availability_zone
#   size                 = var.volume_size
#   snapshot_id          = var.snapshot_id
#   type                 = var.volume_type
# }

# resource "aws_volume_attachment" "main" {
#   device_name          = "${var.name}-disk1"
#   volume_id            = aws_ebs_volume.main.id
#   instance_id          = aws_instance.main.id
# }

## To Create Snapshots
# resource "aws_ebs_snapshot" "main" {
#   volume_id = aws_instance.main.ebs_block_device.volume_id

# }
