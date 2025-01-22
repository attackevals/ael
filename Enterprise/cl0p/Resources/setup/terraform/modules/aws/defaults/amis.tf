############################################################
/* AMIs
Capture and define all commonly used AMIs here, even if they
are currently unused.
*/
############################################################
#------------------------------
# Windows Images
#------------------------------
data "aws_ami" "windows-desktop-11" {
  most_recent = true
  owners      = ["self"]
  filter {
    name   = "tag:Name"
    values = ["Windows11"]
  }
  filter {
    name   = "tag:version"
    values = ["prod"]
  }
}

## Windows Server 2022 -- pinned to October 11 release
data "aws_ami" "windows-server-2022" {
  most_recent = false
  owners      = ["amazon"]
  filter {
    name   = "name"
    values = ["Windows_Server-2022-English-Full-Base-2025.01.15"]
  }
}

data "aws_ami" "windows-sql-server-2022" {
  most_recent = false
  owners      = ["amazon"]
  filter {
    name   = "name"
    values = ["Windows_Server-2022-English-Full-SQL_2022_Standard-2025.01.15"]
  }
}

#------------------------------
# MacOS Images
#------------------------------
# [static] macOS x86_64 Ventura 13.6.4 AMI
data "aws_ami" "macos-intel-ventura" {
  owners = ["amazon"]
  filter {
    name   = "name"
    values = ["amzn-ec2-macos-13.6.4-20240207-222134"]
  }
  # add filter for hvm virtualization type (vs pv)
  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }
}

# [static] macOS arm64 Sonoma 14.3 AMI
data "aws_ami" "macos-arm-sonoma" {
  owners = ["amazon"]
  filter {
    name   = "name"
    values = ["amzn-ec2-macos-14.3-20240208-140501-arm64"]
  }
  # add filter for hvm virtualization type (vs pv)
  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }
}

# [static] macOS arm64 Ventura 13.6.4 AMI
data "aws_ami" "macos-arm-ventura" {
  owners = ["amazon"]
  filter {
    name   = "name"
    values = ["amzn-ec2-macos-13.6.4-20240208-182310-arm64"]
  }
  # add filter for hvm virtualization type (vs pv)
  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }
}

# [latest] macOS Monterey 12 AMI
data "aws_ami" "macos-arm-monterrey" {
  most_recent = true
  owners      = ["amazon"]
  filter {
    name   = "name"
    values = ["amzn-ec2-macos-12*"]
  }
}

# [latest] macOS Big Sur 11 AMI
data "aws_ami" "macos-arm-bigsur" {
  most_recent = true
  owners      = ["amazon"]
  filter {
    name   = "name"
    values = ["amzn-ec2-macos-11*"]
  }
}

# [latest] macOS Catalina 10.5 AMI
data "aws_ami" "macos-arm-catalina" {
  most_recent = true
  owners      = ["amazon"]
  filter {
    name   = "name"
    values = ["amzn-ec2-macos-10.15*"]
  }
}

# [latest] macOS Mojave 10.4 AMI
data "aws_ami" "macos-arm-mojave" {
  most_recent = true
  owners      = ["amazon"]
  filter {
    name   = "name"
    values = ["amzn-ec2-macos-10.14*"]
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
    values = ["ubuntu/images/hvm-ssd/ubuntu-focal-20.04-amd64-server-20250111"]
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
    values = ["ubuntu/images/hvm-ssd/ubuntu-jammy-22.04-amd64-server-20240223"]
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
  owners      = ["679593333241"] # Kali Organization (correlated by looking up owner of AMIs via process referenced by Kali, https://www.kali.org/docs/cloud/aws/)
  # add filter for name
  filter {
    name   = "name"
    values = ["kali-*2024.4*"]
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
