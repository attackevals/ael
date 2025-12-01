---
tags: [infrastructure, enterprise2025, aws, ami, terraform, module]
---

# Enterprise 2025 Platforms

Enterprise Evaluation 2025 uses the following platforms:

1. `linux-ubuntu-noble`  *(Ubuntu 24.04)*
2. `linux-kali`
3. `windows-desktop-11`
4. `windows-server-2022`

Lookups for Amazon Machine Images (AMIs) of these platforms is managed in the `modules/aws/amis` Terraform module.

### Windows 11

!!!

**IMPORTANT**: AWS does not currently provide a pre-built Windows 11 Amazon Machine Image (AMI).
You must provide your own Windows 11 AMI for use in the environment.

Please review the Amazon and Microsoft terms of service to ensure you are compliant with all licensing guidelines.

!!!

#### Build & Import AMI

One method of building and converting a Windows 11 VM into an  AMI uses:

1. [Packer](https://www.packer.io/) to automate the build of a Windows 11 VM from the installation image.
2. Packer to upload the virtual machine file to an AWS S3 bucket.
3. [AWS VM Import](https://aws.amazon.com/blogs/modernizing-with-aws/bringing-your-windows-11-image-to-aws-with-vm-import-export/) to convert the VM to an AMI.

AWS provides additional [documentation on importing a Windows 11 VM](https://aws.amazon.com/blogs/modernizing-with-aws/bringing-your-windows-11-image-to-aws-with-vm-import-export/).

#### Module References

If you completed the import process above, the following references must be changed to create Windows 11 EC2 instances from your new Windows 11 AMI. Please see the referenced files in `modules/aws/amis`.

Depending on how the AMI is named, tagged, or accessed, you can either:

1. Change the reference in `amis-windows.tf` to your hardcoded value:

```python
#### (1) Comment out the Windows 11 data block query

# #-----------------------------------------------------------------------------------------------------------------------
# # Windows Desktop
# #-----------------------------------------------------------------------------------------------------------------------
# data "aws_ami" "windows-desktop-11" {
#   most_recent = true
#   owners      = [local.owners.windows-desktop-11]
#   filter {
#     name   = "name"
#     values = [local.name_query.windows-desktop-11]
#   }
# }

#### (2) Set this output to the Win11 AMI ID under your account
output "windows-desktop-11" {
  value = "ami-0123abcd4567efgh"
}
```

2. Change the `windows-desktop-11` references in `locals.tf`:

```terraform
## in locals.tf

locals {
  name_query = {
    #### (1) Change filter to Win11 AMI name
    windows-desktop-11: "Windows_Server-2022-English-Full-Base*"
    windows-server-2022: "Windows_Server-2022-English-Full-Base*"
    linux-kali: "kali-*2025*"
    linux-ubuntu-noble: "ubuntu/images/hvm-ssd-gp3/ubuntu-noble-24.04-amd64-server-*"
  }

  owners = {
    #### (2) Change owner to "self"
    windows-desktop-11: "amazon"
    windows-server-2022: "amazon"
    linux-kali: "679593333241"
    linux-ubuntu-noble: "099720109477"
  }
}
```
