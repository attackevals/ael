locals {


  openvpn-cidr = "172.16.0.0/16"

  scopes = {
    internal = "internal"
    red      = "red"
    support  = "support"
    victim   = "victim"
  }

  # [Instance Type]
  aws-vm-size-extra-small = "t3.small"   #  2 vCPUS
  aws-vm-size-small       = "t3.medium"  #  2 vCPUS
  aws-vm-size-medium      = "t3.large"   #  2 vCPUS
  aws-vm-size-large       = "t3.xlarge"  #  4 vCPUs
  aws-vm-size-xlarge      = "t3.2xlarge" #  8 vCPUS
  aws-vm-size-mac-m1      = "mac2.metal" # 12 vCPUs (6 physical)
  aws-vm-size-win11       = "t3.xlarge"  #  4 vCPUs

  # [Disk - Volume]
  standard-disk = "gp3"

  scenario-prefixes = {
    a                   = "sa"
    attacker            = "red"
    b                   = "sb"
    external-benevolent = "eb"
    protections         = "sp"
  }


  platform = {
    choco-mirror = "ubuntu-jammy"
    kali         = "kali"
    kvm          = "ubuntu-jammy"
    macos-desk   = "macos"
    redirect     = "ubuntu-jammy"
    win-srv      = "windows-server"
    win11-desk   = "windows-desk"
  }

  description = {
    addc          = "[VICTIM] Windows Server 2022 AD/DC"
    choco-mirror  = "[SUPPORT] Chocolatey Mirror"
    dns           = "[SUPPORT] Ubuntu DNS Server"
    mail          = "[VICTIM] Windows Server Exchange Server"
    file          = "[VICTIM] Windows Server File Server"
    kali          = "[RED] Kali Attack Platform VM"
    kvm           = "[VICTIM] Ubuntu KVM Server"
    linux         = "[VICTIM] Ubuntu Server"
    macos-desk    = "[VICTIM] MacOS Desktop VM"
    postfix       = "[SUPPORT] Ubuntu Postfix Server"
    redirect      = "[SUPPORT] Ubuntu Redirector"
    remote-worker = "[RED] Remote Worker"
    win-jumpbox1  = "[RED] Windows Attack Platform VM"
    win11-desk    = "[VICTIM] Windows Desktop VM"
  }

  disk-size = {
    addc-srv        = "127"
    choco-mirror    = "127"
    dns             = "127"
    kali            = "30"
    kvm             = "127"
    macos-desk      = "127"
    postfix         = "127"
    redirect        = "30"
    traffic-mirror  = "30"
    win-jumpbox     = "127"
    win11-desk      = "127"
    file-srv-disk-2 = "100"
  }

  ########################################################################
  # static locals
  ########################################################################
  # merge individual settings to default tags to change automated behavior
  set = {
    machine-off = { ShutdownOk = "turn-off" }
  }

  # Allow for billing to be categorized by vendor. Have to use tags already associated with
  # cost allocation.
  cost-tag = { Performer = var.name-prefix }

  vendor      = module.defaults.tags["Vendor"]
  egress-cidr = "0.0.0.0/0"

}
