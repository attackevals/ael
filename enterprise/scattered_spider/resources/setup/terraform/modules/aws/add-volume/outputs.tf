output "volume" {
  value = aws_ebs_volume.main
}

output "name" {
  value = var.name
}

output "tags" {
  value = var.tags
}
