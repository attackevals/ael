#-----------------------------------------------------------------------------------------------------------------------
# Windows Desktop
#-----------------------------------------------------------------------------------------------------------------------
data "aws_ami" "windows-desktop-11" {
  most_recent = true
  owners      = [local.owners.windows-desktop-11]
  filter {
    name   = "name"
    values = [local.name_query.windows-desktop-11]
  }
}

#-----------------------------------------------------------------------------------------------------------------------
# Windows Server
#-----------------------------------------------------------------------------------------------------------------------
data "aws_ami" "windows-server-2022" {
  most_recent = true
  owners      = [local.owners.windows-server-2022]
  filter {
    name   = "name"
    values = [local.name_query.windows-server-2022]
  }
}

########################################################################################################################
# Outputs
output "windows-desktop-11" {
  value = data.aws_ami.windows-desktop-11.id
}
output "windows-server-2022" {
  value = data.aws_ami.windows-server-2022.id
}
