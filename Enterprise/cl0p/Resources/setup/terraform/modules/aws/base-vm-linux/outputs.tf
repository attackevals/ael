output "instance" {
  value = aws_instance.main
}

output "name" {
  value = var.name
}

output "ip" {
  description = "Main IP address provisioned."
  value       = aws_instance.main.private_ip
}

output "ips" {
  description = "Secondary IP addresses provisioned."
  value       = aws_instance.main.secondary_private_ips
}

output "tags" {
  value = var.tags
}

output "is_windows" {
  value = local.is_windows
}

output "Windows_Admin_Password" {
  value = local.password_data
}
