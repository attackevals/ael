locals {
  name_query = {
    windows-desktop-11 : "Windows_Server-2022-English-Full-Base*" # TODO: Change to your Win11 AMI name
    windows-server-2022 : "Windows_Server-2022-English-Full-Base*"
    linux-kali : "kali-*2025*"
    linux-ubuntu-noble : "ubuntu/images/hvm-ssd-gp3/ubuntu-noble-24.04-amd64-server-*"
  }

  owners = {
    windows-desktop-11 : "amazon" # TODO: Change to "self"
    windows-server-2022 : "amazon"
    linux-kali : "679593333241"         # Kali Organization (https://www.kali.org/docs/cloud/aws/)
    linux-ubuntu-noble : "099720109477" # Canonical
  }
}
