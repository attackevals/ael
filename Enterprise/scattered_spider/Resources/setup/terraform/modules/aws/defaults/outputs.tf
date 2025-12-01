output "name" {
  value = var.name
}

output "tags" {
  value = aws_resourcegroups_group.defaults.tags
}

output "category" {
  value = var.category
}

output "environment" {
  value = var.environment
}

output "region" {
  value = var.region
}

output "provisioner" {
  value = var.provisioner
}

output "charge_code" {
  value = var.charge_code
}

output "performer" {
  value = var.performer
}

output "owner" {
  value = var.owner
}
