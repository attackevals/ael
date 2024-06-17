############################################################
/* AMIs
Capture and define all commonly used AMIs here, even if they
are currently unused.
*/
############################################################
#------------------------------
# Windows Images
#------------------------------

# Desktop AMI currently hardcoded to use Windows Server 2022 image.
# Please replace with search query to your private Windows 11 AMI.
data "aws_ami" "windows-desktop-11" {
  most_recent = false
  owners      = ["amazon"]
  filter {
    name   = "name"
    values = ["Windows_Server-2022-English-Full-Base-2024.01.16"]
  }
}

## Windows Server 2022 -- pinned to October 11 release
data "aws_ami" "windows-server-2022" {
  most_recent = false
  owners      = ["amazon"]
  filter {
    name   = "name"
    values = ["Windows_Server-2022-English-Full-Base-2024.01.16"]
  }
}

data "aws_ami" "windows-sql-server-2022" {
  most_recent = false
  owners      = ["amazon"]
  filter {
    name   = "name"
    values = ["Windows_Server-2022-English-Full-SQL_2022_Standard-2024.01.16"]
  }
}


#------------------------------
# Linux Images
#------------------------------

# Ubuntu 20.04 (Focal)
data "aws_ami" "ubuntu-focal" {
  most_recent = false
  owners      = ["099720109477"] # Canonical (posted on Ubuntu official docs, https://help.ubuntu.com/community/EC2StartersGuide)
  filter {
    name   = "name"
    values = ["ubuntu/images/hvm-ssd/ubuntu-focal-20.04-amd64-server-20221014"]
  }
  # add filter for hvm virtualization type (vs pv)
  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }
}

# Ubuntu 22.04 (Jammy)
data "aws_ami" "ubuntu-jammy" {
  most_recent = false
  owners      = ["099720109477"] # Canonical (posted on Ubuntu official docs, https://help.ubuntu.com/community/EC2StartersGuide)
  filter {
    name   = "name"
    values = ["ubuntu/images/hvm-ssd/ubuntu-jammy-22.04-amd64-server-20230919"]
  }
  # add filter for hvm virtualization type (vs pv)
  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }
  # add filter for arch
  filter {
    name   = "architecture"
    values = ["x86_64"]
  }
}

data "aws_ami" "kali" {
  most_recent = true
  owners      = ["679593333241"] # Kali Organization (https://www.kali.org/docs/cloud/aws/)
  # add filter for name
  filter {
    name   = "name"
    values = ["kali-*2023*"]
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
