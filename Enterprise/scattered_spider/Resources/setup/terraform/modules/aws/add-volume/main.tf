resource "aws_ebs_volume" "main" {
  size              = var.disk_size
  type              = var.disk_type
  tags              = merge(var.tags, { Name = var.name })
  availability_zone = var.availability_zone
}

resource "aws_volume_attachment" "additional_volume_attachment" {
  volume_id   = aws_ebs_volume.main.id
  device_name = var.device_name
  instance_id = var.instance_id
}
