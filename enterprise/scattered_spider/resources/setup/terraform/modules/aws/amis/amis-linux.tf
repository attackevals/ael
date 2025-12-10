# Ubuntu Noble
data "aws_ami" "ubuntu-noble" {
  most_recent = true
  owners      = [local.owners.linux-ubuntu-noble]

  filter {
    name   = "name"
    values = [local.name_query.linux-ubuntu-noble]
  }
}

# Kali Linux
data "aws_ami" "kali" {
  most_recent = true
  owners      = [local.owners.linux-kali]
  # add filter for name
  filter {
    name   = "name"
    values = [local.name_query.linux-kali]
  }
  # add filter for arch
  filter {
    name   = "architecture"
    values = ["x86_64"]
  }
  # add filter for hvm virt type
  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }
}

########################################################################################################################
# Outputs

output "linux-kali" {
  value = data.aws_ami.kali.id
}

output "linux-ubuntu-noble" {
  value = data.aws_ami.ubuntu-noble.id
}
