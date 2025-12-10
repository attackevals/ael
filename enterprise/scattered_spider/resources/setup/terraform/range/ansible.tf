
# ansible specific creations
# ansible groups organized as so:
#
# linux -|
#        |- linux_ubuntu
#        |- linux_kali
#        |- linux_cloud

# windows -|
#          |- windows_servers
#          |- windows_desktop

# concept is parent group (linux/windows) holds the general info, and any specific configuration,
# and then the child group holds the specific configuration for that subgroup (e.g. username).

# linux groups
resource "ansible_group" "linux" {
  name     = "linux"
  children = [local.ansible_groups.linux-ubuntu, local.ansible_groups.kali, local.ansible_groups.linux-cloud, local.ansible_groups.linux-docker]
  variables = {
    ansible_ssh_private_key_file = var.ssh_private_key_path
  }
}

resource "ansible_group" "linux_preflights" {
  name = local.ansible_groups.preflights
}

resource "ansible_group" "linux_ubuntu" {
  name = local.ansible_groups.linux-ubuntu
  variables = {
    ansible_user = "ubuntu"
  }
}

resource "ansible_group" "linux_kali" {
  name = local.ansible_groups.kali
  variables = {
    ansible_user = "kali"
  }
}

resource "ansible_group" "linux_cloud" {
  name = local.ansible_groups.linux-cloud
  variables = {
    ansible_user         = "ubuntu"
    cloud_management_arn = "arn:aws:iam::${local.account-cloud-management-id}:role/CrossAccountAccess"
    detections_acct_arn  = "arn:aws:iam::${local.account-detections-id}:role/CrossAccountAccess"
    protections_acct_arn = "arn:aws:iam::${local.account-protections-id}:role/CrossAccountAccess"
  }
}

# windows groups
resource "ansible_group" "windows" {
  name = "windows"
  children = [
    local.ansible_groups.win-client,
    local.ansible_groups.win-server,
    local.ansible_groups.win-file,
    local.ansible_groups.win-addc,
    local.ansible_groups.win-rds-servers,
    local.ansible_groups.red-jumpbox-servers
  ]
  variables = {
    ansible_ssh_private_key_file         = var.ssh_private_key_path
    ansible_user                         = var.win_srv_admin_username
    ansible_password                     = var.win_srv_admin_password
    ansible_port                         = 5986
    ansible_connection                   = "winrm"
    ansible_winrm_server_cert_validation = "ignore"
    ansible_winrm_operation_timeout_sec  = 120
    ansible_winrm_read_timeout_sec       = 240
  }
}

resource "ansible_group" "windows_desktops" {
  name = local.ansible_groups.win-client
  variables = {
    ansible_user          = var.win_srv_admin_username
    ansible_password      = var.win_srv_admin_password
    ansible_become_method = "runas"
    ansible_become_user   = var.win_srv_admin_username
    ansible_become_pass   = var.win_srv_admin_password
  }
}

resource "ansible_group" "windows_servers" {
  name = local.ansible_groups.win-server
  variables = {
    ansible_user          = var.win_srv_admin_username
    ansible_password      = var.win_srv_admin_password
    ansible_become_method = "runas"
    ansible_become_user   = var.win_srv_admin_username
    ansible_become_pass   = var.win_srv_admin_password
  }
}

resource "ansible_group" "windows_rds_servers" {
  name = local.ansible_groups.win-rds-servers
}

resource "ansible_group" "red_jumpbox_servers" {
  name = local.ansible_groups.red-jumpbox-servers
}

resource "ansible_group" "windows_file_servers" {
  name = local.ansible_groups.win-file
}

resource "ansible_group" "windows_domain_controllers" {
  name = local.ansible_groups.win-addc
}

# domain groups
# kingslanding
resource "ansible_group" "domain_kingslanding" {
  name = local.ansible_groups.domain-kingslanding
  variables = {
    domain_dns_domain_name = "kingslanding.net"
    domain_short_name      = "kingslanding"
  }
}

# kingslanding
resource "ansible_group" "domain_vale" {
  name = local.ansible_groups.domain-vale
  variables = {
    domain_dns_domain_name = "vale.net"
    domain_short_name      = "vale"
  }
}
