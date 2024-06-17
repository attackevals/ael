output "name" {
  value = var.name
}

output "tags" {
  value = aws_resourcegroups_group.defaults.tags
}

output "aws-vm-size" {
  value = var.aws-vm-size
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

### AMIs
output "ami-windows-desktop-11" {
  value = data.aws_ami.windows-desktop-11.id
}

output "ami-windows-server-2022" {
  value = data.aws_ami.windows-server-2022.id
}

output "ami-windows-sql-server-2022" {
  value = data.aws_ami.windows-sql-server-2022.id
}

output "ami-linux-kali" {
  value = data.aws_ami.kali.id
}

output "ami-linux-ubuntu-focal" {
  value = data.aws_ami.ubuntu-focal.id
}

output "ami-linux-ubuntu-jammy" {
  value = data.aws_ami.ubuntu-jammy.id
}
