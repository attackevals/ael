locals {
  ####################################################################################
  # RDP Profiles
  ####################################################################################
  # RDP names (outputs.tf)
  rdp_out = {
    # scenario on-prem (victim-on-prem, 10 hosts)
    on-prem-addc-srv1   = ["${local.context.onprem}-${local.machine-names.addc-srv1}-${local.victim-on-prem-vms.addc-srv1.hostname}", local.victim-on-prem-ips.addc-srv1]
    on-prem-file-srv1   = ["${local.context.onprem}-${local.machine-names.file-srv1}-${local.victim-on-prem-vms.file-srv1.hostname}", local.victim-on-prem-ips.file-srv1]
    on-prem-file-srv2   = ["${local.context.onprem}-${local.machine-names.file-srv2}-${local.victim-on-prem-vms.file-srv2.hostname}", local.victim-on-prem-ips.file-srv2]
    on-prem-mail-srv1   = ["${local.context.onprem}-${local.machine-names.mail-srv1}-${local.victim-on-prem-vms.mail-srv1.hostname}", local.victim-on-prem-ips.mail-srv1]
    on-prem-access-srv1 = ["${local.context.onprem}-${local.machine-names.access-srv1}-${local.victim-on-prem-vms.access-srv1.hostname}", local.victim-on-prem-ips.access-srv1]
    on-prem-mfa-srv1    = ["${local.context.onprem}-${local.machine-names.mfa-srv1}-${local.victim-on-prem-vms.mfa-srv1.hostname}", local.victim-on-prem-ips.mfa-srv1]
    on-prem-win11-desk1 = ["${local.context.onprem}-${local.machine-names.win11-desk1}-${local.victim-on-prem-vms.win11-desk1.hostname}", local.victim-on-prem-ips.win11-desk1]
    on-prem-win11-desk2 = ["${local.context.onprem}-${local.machine-names.win11-desk2}-${local.victim-on-prem-vms.win11-desk2.hostname}", local.victim-on-prem-ips.win11-desk2]
    on-prem-win11-desk3 = ["${local.context.onprem}-${local.machine-names.win11-desk3}-${local.victim-on-prem-vms.win11-desk3.hostname}", local.victim-on-prem-ips.win11-desk3]
    on-prem-win11-desk4 = ["${local.context.onprem}-${local.machine-names.win11-desk4}-${local.victim-on-prem-vms.win11-desk4.hostname}", local.victim-on-prem-ips.win11-desk4]

    # scenario dmz (victim-dmz, 1 host)
    dmz-guacamole-srv1 = ["${local.context.onprem}-${local.machine-names.guacamole-srv1}-${local.victim-dmz-vms.guacamole-srv1.hostname}", local.victim-dmz-ips.guacamole-srv1]

    # scenario cloud (victim-cloud, 3 hosts)
    cloud-airbyte-srv1 = ["${local.context.cloud}-${local.machine-names.airbyte-srv1}-${local.cloud-vms.airbyte-srv1.hostname}", local.cloud-ips.airbyte-srv1]
    cloud-wekan-srv1   = ["${local.context.cloud}-${local.machine-names.wekan-srv1}-${local.cloud-vms.wekan-srv1.hostname}", local.cloud-ips.wekan-srv1]
    cloud-gitlab-srv1  = ["${local.context.cloud}-${local.machine-names.gitlab-srv1}-${local.cloud-vms.gitlab-srv1.hostname}", local.cloud-ips.gitlab-srv1]

    # scenario B.3 dmz (victim-dmz, 2 host)
    b3-dmz-guacamole-srv1 = ["${local.context.b3}-${local.machine-names.guacamole-srv1}-${local.b3-dmz-vms.guacamole-srv1.hostname}", local.b3-dmz-ips.guacamole-srv1]
    b3-dmz-mfa-srv1       = ["${local.context.b3}-${local.machine-names.mfa-srv1}-${local.b3-dmz-vms.mfa-srv1.hostname}", local.b3-dmz-ips.mfa-srv1]

    # scenario B.3 (victim, 4 hosts)
    b3-addc-srv1   = ["${local.context.b3}-${local.machine-names.addc-srv1}-${local.b3-vms.addc-srv1.hostname}", local.b3-ips.addc-srv1]
    b3-access-srv1 = ["${local.context.b3}-${local.machine-names.access-srv1}-${local.b3-vms.access-srv1.hostname}", local.b3-ips.access-srv1]
    b3-mdm-srv1    = ["${local.context.b3}-${local.machine-names.mdm-srv1}-${local.b3-vms.mdm-srv1.hostname}", local.b3-ips.mdm-srv1]
    b3-mail-srv1   = ["${local.context.b3}-${local.machine-names.mail-srv1}-${local.b3-vms.mail-srv1.hostname}", local.b3-ips.mail-srv1]
    b3-win11-desk1 = ["${local.context.b3}-${local.machine-names.win11-desk1}-${local.b3-vms.win11-desk1.hostname}", local.b3-ips.win11-desk1]
    b3-win11-desk2 = ["${local.context.b3}-${local.machine-names.win11-desk2}-${local.b3-vms.win11-desk2.hostname}", local.b3-ips.win11-desk2]
    b3-win11-desk3 = ["${local.context.b3}-${local.machine-names.win11-desk3}-${local.b3-vms.win11-desk3.hostname}", local.b3-ips.win11-desk3]
    b3-win11-desk4 = ["${local.context.b3}-${local.machine-names.win11-desk4}-${local.b3-vms.win11-desk4.hostname}", local.b3-ips.win11-desk4]
    b3-file-srv1   = ["${local.context.b3}-${local.machine-names.file-srv1}-${local.b3-vms.file-srv1.hostname}", local.b3-ips.file-srv1]
    b3-file-srv2   = ["${local.context.b3}-${local.machine-names.file-srv2}-${local.b3-vms.file-srv2.hostname}", local.b3-ips.file-srv2]

    # internal neutral (gray, 7 hosts)
    dns1            = ["${local.context.neutral}-${local.machine-names.dns-srv1}-${local.neutral-vms.dns-srv1.hostname}", local.neutral-ips.dns-srv1]
    unmanaged-desk1 = ["${local.context.neutral}-${local.machine-names.unmanaged-desk1}-${local.neutral-vms.unmanaged-desk1.hostname}", local.neutral-ips.unmanaged-desk1]
    rmm-srv1        = ["${local.context.neutral}-${local.machine-names.rmm-srv1}-${local.neutral-vms.rmm-srv1.hostname}", local.neutral-ips.rmm-srv1]
    choco-srv1      = ["${local.context.neutral}-${local.machine-names.choco-srv1}-${local.neutral-vms.choco-srv1.hostname}", local.neutral-ips.choco-srv1]
    postfix-srv1    = ["${local.context.neutral}-${local.machine-names.postfix-srv1}-${local.neutral-vms.postfix-srv1.hostname}", local.neutral-ips.postfix-srv1]
    postfix-srv2    = ["${local.context.neutral}-${local.machine-names.postfix-srv2}-${local.neutral-vms.postfix-srv2.hostname}", local.neutral-ips.postfix-srv2]
    postfix-srv3    = ["${local.context.neutral}-${local.machine-names.postfix-srv3}-${local.neutral-vms.postfix-srv3.hostname}", local.neutral-ips.postfix-srv3]
    rustdesk-srv1   = ["${local.context.neutral}-${local.machine-names.rustdesk-srv1}-${local.neutral-vms.rustdesk-srv1.hostname}", local.neutral-ips.rustdesk-srv1]
    wstunnel-srv1   = ["${local.context.neutral}-${local.machine-names.wstunnel-srv1}-${local.neutral-vms.wstunnel-srv1.hostname}", local.neutral-ips.wstunnel-srv1]

    # internal neutral (gray-redirect, 1 host)
    redirect1 = ["${local.context.neutral}-${local.machine-names.redirect1}-${local.redirect-vms.redirect-srv1.hostname}", local.redirect-ips.redirect1-ip1]
    redirect2 = ["${local.context.neutral}-${local.machine-names.redirect2}-${local.redirect-vms.redirect-srv2.hostname}", local.redirect-ips.redirect2-ip1]
    redirect3 = ["${local.context.neutral}-${local.machine-names.redirect3}-${local.redirect-vms.redirect-srv3.hostname}", local.redirect-ips.redirect3-ip1]
    redirect4 = ["${local.context.neutral}-${local.machine-names.redirect4}-${local.redirect-vms.redirect-srv4.hostname}", local.redirect-ips.redirect4-ip1]

    # attacker (red, 3 hosts)
    kali1                = ["${local.context.attacker}-${local.machine-names.kali1}-${local.attacker-vms.kali1.hostname}", local.attacker-ip.kali1]
    idp-srv1             = ["${local.context.attacker}-${local.machine-names.idp-srv1}-${local.attacker-vms.idp-srv1.hostname}", local.attacker-ip.idp-srv1]
    win-jump             = ["${local.context.attacker}-${local.machine-names.win-jumpbox1}-${local.attacker-vms.win-jumpbox1.hostname}", local.attacker-ip.win-jumpbox1]
    protections-jumpbox1 = ["${local.context.attacker}-${local.machine-names.win-jumpbox1}-${local.b3-vms.win-jumpbox1.hostname}", local.b3-ips.win-jumpbox1]
  }
}

output "rdp_data" {
  value = tolist([
    # Red
    local.rdp_out.kali1,
    local.rdp_out.idp-srv1,
    local.rdp_out.win-jump,
    local.rdp_out.protections-jumpbox1,

    # Victim on-Prem
    local.rdp_out.on-prem-addc-srv1,
    local.rdp_out.on-prem-file-srv1,
    local.rdp_out.on-prem-file-srv2,
    local.rdp_out.on-prem-mail-srv1,
    local.rdp_out.on-prem-access-srv1,
    local.rdp_out.on-prem-mfa-srv1,
    local.rdp_out.on-prem-win11-desk1,
    local.rdp_out.on-prem-win11-desk2,
    local.rdp_out.on-prem-win11-desk3,
    local.rdp_out.on-prem-win11-desk4,

    # Victim On-Prem Cloud
    local.rdp_out.cloud-airbyte-srv1,
    local.rdp_out.cloud-wekan-srv1,
    local.rdp_out.cloud-gitlab-srv1,

    # Victim On-Prem DMZ
    local.rdp_out.dmz-guacamole-srv1,

    # Victim Protections
    local.rdp_out.b3-addc-srv1,
    local.rdp_out.b3-access-srv1,
    local.rdp_out.b3-file-srv1,
    local.rdp_out.b3-file-srv2,
    local.rdp_out.b3-mail-srv1,
    local.rdp_out.b3-mdm-srv1,
    local.rdp_out.b3-win11-desk1,
    local.rdp_out.b3-win11-desk2,
    local.rdp_out.b3-win11-desk3,
    local.rdp_out.b3-win11-desk4,

    # Victim Protections DMZ
    local.rdp_out.b3-dmz-guacamole-srv1,
    local.rdp_out.b3-dmz-mfa-srv1,

    # Victim Neutral
    local.rdp_out.dns1,
    local.rdp_out.redirect1,
    local.rdp_out.redirect2,
    local.rdp_out.redirect3,
    local.rdp_out.redirect4,
    local.rdp_out.unmanaged-desk1,
    local.rdp_out.rmm-srv1,
    local.rdp_out.choco-srv1,
    local.rdp_out.postfix-srv1,
    local.rdp_out.postfix-srv2,
    local.rdp_out.postfix-srv3,
    local.rdp_out.rustdesk-srv1,
    local.rdp_out.wstunnel-srv1,
  ])
}

output "vpn" {
  value = {
    endpoint_id : module.openvpn-client.vpn_endpoint_id,
    security_group_id : module.openvpn-client.vpn_security_group_id,
    dns_name : module.openvpn-client.vpn_dns_name,
    client_cert : module.openvpn-client.vpn_client_cert,
    client_key : nonsensitive(module.openvpn-client.vpn_client_key),
    server_cert : module.openvpn-client.vpn_server_cert,
    server_key : nonsensitive(module.openvpn-client.vpn_server_key),
    ca_cert : module.openvpn-client.vpn_ca_cert,
    ca_key : nonsensitive(module.openvpn-client.vpn_ca_key),
  }
}

# output cert and key separately to simplify redirecting to file
# used to generate vpn profiles
output "cacert" {
  value = module.openvpn-client.vpn_ca_cert
}

output "cakey" {
  value = nonsensitive(module.openvpn-client.vpn_ca_key)
}

output "endpointid" {
  value = module.openvpn-client.vpn_endpoint_id
}
