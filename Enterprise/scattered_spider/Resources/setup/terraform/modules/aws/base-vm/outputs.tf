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

output "subnet_id" {
  description = "Subnet ID"
  value       = aws_instance.main.subnet_id
}

output "security_groups" {
  description = "Security Groups associated"
  value       = aws_instance.main.vpc_security_group_ids
}

output "network_interface_id" {
  description = "Network interface ID"
  value       = aws_instance.main.primary_network_interface_id
}

output "network_interface" {
  description = "Network interface"
  value       = aws_instance.main.network_interface
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

output "hostname" {
  description = "Hostname of the instance"
  value       = var.hostname
}
