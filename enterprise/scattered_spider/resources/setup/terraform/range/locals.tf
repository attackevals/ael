locals {
  openvpn-cidr = "172.16.0.0/16"
  egress-cidr  = "0.0.0.0/0"

  # [Disk - Volume]
  standard-disk = "gp3"

  # [Instance Type]
  instance-type-extra-small   = "t3.small"  #  2 vCPUS
  instance-type-small         = "t3.medium" #  2 vCPUS
  instance-type-medium        = "t3.large"  #  2 vCPUS
  instance-type-large         = "t3.xlarge" #  4 vCPUs
  instance-type-win11         = "t3.xlarge" #  4 vCPUs
  instance-type-ubuntu-victim = "t3.xlarge" #  2 vCPUs

  # default usernames
  username = {
    kali   = "kali"
    win    = "Administrator"
    ubuntu = "ubuntu"
  }

  ansible_groups = {
    win-addc            = "windows_domain_controllers"
    win-server          = "windows_servers"
    win-client          = "windows_desktops"
    win-rds-servers     = "windows_rds_servers"
    domain-kingslanding = "domain_kingslanding"
    domain-vale         = "domain_vale"
    kali                = "linux_kali"
    win-mail            = "windows_mail_servers"
    win-file            = "windows_file_servers"
    win-file-multi      = "windows_file_servers_multi"
    linux-docker        = "linux_docker"
    linux-ubuntu        = "linux_ubuntu"
    linux-cloud         = "linux_cloud"
    redirectors         = "redirectors"
    dns                 = "dns"
    rmm                 = "linux_rmm"
    choco               = "linux_choco"
    mfa                 = "linux_mfa"
    guacamole           = "linux_guacamole"
    airbyte             = "linux_airbyte"
    wekan               = "linux_wekan"
    gitlab              = "linux_gitlab"
    postfix             = "linux_postfix"
    rustdesk            = "linux_rustdesk"
    wstunnel            = "linux_wstunnel"
    mdm                 = "linux_mdm"
    red-jumpbox-servers = "red_jumpbox_servers"
    research-server     = "research_servers"
    preflights          = "preflights"
  }

  scope = {
    victim   = "victim"
    internal = "internal"
    attacker = "red" # Attacker resources

    victim-tag   = { Scope : "victim" } # Vendor has visibility
    attacker-tag = { Scope : "red" }
    internal-tag = { Scope : "internal" } # Vendor lacks visibility
  }

  context = {
    cloud    = "cloud"
    onprem   = "onprem"
    b3       = "b3"
    attacker = "red"     # Attacker resources
    support  = "support" # DNS, redirectors, etc.
    neutral  = "gray"    # e.g., external benevolent
    vendor   = "vendor"

    cloud-tag    = { Context : "cloud" }
    onprem-tag   = { Context : "onprem" }
    b3-tag       = { Context : "b3" }
    attacker-tag = { Context : "red" }
    support-tag  = { Context : "support" }
    neutral-tag  = { Context : "neutral" }
    vendor-tag   = { Context : "vendor" }
  }

  scenario = {
    a  = "a"
    b3 = "b3"
    c  = "c"
  }

  # Tags
  victim-tags   = merge(module.defaults.tags, local.scope.victim-tag)
  internal-tags = merge(module.defaults.tags, local.scope.internal-tag)
  attacker-tags = merge(module.defaults.tags, local.scope.attacker-tag, local.context.attacker-tag)

  victim-onprem-tags = merge(local.victim-tags, local.context.onprem-tag)
  victim-cloud-tags  = merge(local.victim-tags, local.context.cloud-tag)

  b3-tags = merge(local.victim-tags, local.context.b3-tag)

  support-tags = merge(local.internal-tags, local.context.support-tag)
  neutral-tags = merge(local.internal-tags, local.context.neutral-tag)

  victim-internet-tags = { "Internet" = "victim" }
  b3-internet-tags     = { "Internet" = "b3" }

  platform = {
    wekan         = "ubuntu-noble"
    airbyte-srv   = "ubuntu-noble"
    gitlab        = "ubuntu-noble"
    guacamole-srv = "ubuntu-noble"
    idp-srv       = "ubuntu-noble"
    rmm           = "ubuntu-noble"
    kali          = "kali"
    mfa           = "ubuntu-noble"
    mdm           = "ubuntu-noble"
    redirect      = "ubuntu-noble"
    win-srv       = "windows-server"
    win11-desk    = "windows-desk"
    linux-ubuntu  = "ubuntu-noble"
    postfix       = "ubuntu-noble"
    rustdesk      = "ubuntu-noble"
    wstunnel      = "ubuntu-noble"
    postfix       = "ubuntu-noble"
    preflights    = "ubuntu-noble"
  }

  description = {
    kali         = "[RED] Kali Attack Platform VM"
    win-jumpbox1 = "[RED] Windows Attack Platform VM"
    unmanaged    = "[RED]* Windows Unmanaged Workstation"
    idp          = "[RED] Ubuntu Authentik Server"

    dns      = "[GRAY] Ubuntu DNS Server"
    redirect = "[GRAY] Ubuntu Redirector"
    rmm      = "[GRAY] Ubuntu RMM Server"
    choco    = "[GRAY] Ubuntu Choco Server"
    postfix  = "[GRAY] Ubuntu Postfix Server"
    rustdesk = "[GRAY] Ubuntu Rustdesk Server"
    wstunnel = "[GRAY] Ubuntu wstunnel Server"

    addc       = "[VICTIM] Windows Server AD/DC"
    file       = "[VICTIM] Windows Server File Server"
    mail       = "[VICTIM] Windows Server Exchange Server"
    access     = "[VICTIM] Windows Server Remote Access Server"
    win11-desk = "[VICTIM] Windows Desktop VM"
    mfa        = "[VICTIM] Ubuntu MFA Server"
    mdm        = "[VICTIM] Ubuntu MDM Server"
    research   = "[VICTIM] Windows Server Research Server"

    guacamole = "[DMZ] Ubuntu Guacamole Server"

    data    = "[VICTIM CLOUD] Ubuntu Wekan Server"
    gitlab  = "[VICTIM CLOUD] Ubuntu GitLab Server"
    airbyte = "[VICTIM CLOUD] Ubuntu Airbyte Server"

    preflights-box = "[VENDOR/ONPREM] Preflights Server"
  }

  disk-size = {
    win-jumpbox = "127"
    kali        = "30"
    idp-srv     = "127"

    dns            = "127"
    redirect       = "30"
    preflights-box = "127"

    guacamole-srv = "127"
    rmm-srv       = "30"
    postfix-srv   = "30"
    rustdesk-srv  = "127"
    wstunnel-srv  = "127"

    wekan-srv   = "127"
    gitlab-srv  = "127"
    airbyte-srv = "127"

    addc-srv        = "127"
    file-srv-disk-1 = "127"
    file-srv-disk-2 = "100"
    file-srv-disk-3 = "20"
    mail-srv        = "127"
    access-srv      = "127"
    win11-desk      = "127"
    mfa-srv         = "127"
    mdm-srv         = "127"
    rd-srv          = "127"

    traffic-mirror = "30"
  }

  # [IAM Roles]
  roles = {

    # default cloudwatch role
    cwagent-policy = "CloudWatchAgentServerRole"

    # sso roles assumable via authentik
    detections-sso-admin-role   = "kingslanding-sso-admin"
    detections-sso-vendor-role  = "kingslanding-sso-vendor"
    protections-sso-admin-role  = "vale-sso-admin"
    protections-sso-vendor-role = "vale-sso-vendor"

  }
  ########################################################################
  # static locals
  ########################################################################
  # merge individual settings to default tags to change automated behavior
  set = {
    machine-off = { ShutdownOk = "turn-off" }
  }

  # cross account role basename
  cross_account_snapshot_role_name = "CrossAccountSnapshotRole"
}
