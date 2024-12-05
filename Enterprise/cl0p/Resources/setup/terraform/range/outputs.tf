locals {
  ####################################################################################
  # RDP Profiles
  ####################################################################################
  # RDP names (outputs.tf)
  rdp_out = {
    # scenario a (victim-a, 5 hosts)
    a-addc-srv1   = ["${local.scenario-prefixes.a}-${local.victim-a-vms.addc-srv1.hostname}-${local.victim-a-basenames.addc-srv1}", local.victim-a-ips.addc-srv1]
    a-win11-desk1 = ["${local.scenario-prefixes.a}-${local.victim-a-vms.win11-desk1.hostname}-${local.victim-a-basenames.win11-desk1}", local.victim-a-ips.win11-desk1]
    a-win11-desk2 = ["${local.scenario-prefixes.a}-${local.victim-a-vms.win11-desk2.hostname}-${local.victim-a-basenames.win11-desk2}", local.victim-a-ips.win11-desk2]
    a-file-srv1   = ["${local.scenario-prefixes.a}-${local.victim-a-vms.file-srv1.hostname}-${local.victim-a-basenames.file-srv1}", local.victim-a-ips.file-srv1]
    a-mail-srv1   = ["${local.scenario-prefixes.a}-${local.victim-a-vms.mail-srv1.hostname}-${local.victim-a-basenames.mail-srv1}", local.victim-a-ips.mail-srv1]
    a-macos-desk1 = ["${local.scenario-prefixes.a}-${local.victim-a-vms.macos-desk1.hostname}-${local.victim-a-basenames.macos-desk1}", local.victim-a-ips.macos-desk1]

    # scenario b (victim-b, 4 hosts)
    b-addc-srv1   = ["${local.scenario-prefixes.b}-${local.victim-b-vms.addc-srv1.hostname}-${local.victim-b-basenames.addc-srv1}", local.victim-b-ips.addc-srv1]
    b-win11-desk1 = ["${local.scenario-prefixes.b}-${local.victim-b-vms.win11-desk1.hostname}-${local.victim-b-basenames.win11-desk1}", local.victim-b-ips.win11-desk1]
    b-win11-desk2 = ["${local.scenario-prefixes.b}-${local.victim-b-vms.win11-desk2.hostname}-${local.victim-b-basenames.win11-desk2}", local.victim-b-ips.win11-desk2]
    b-kvm-srv1    = ["${local.scenario-prefixes.b}-${local.victim-b-vms.kvm-srv1.hostname}-${local.victim-b-basenames.kvm-srv1}", local.victim-b-ips.kvm-srv1]
    b-mail-srv1   = ["${local.scenario-prefixes.b}-${local.victim-b-vms.mail-srv1.hostname}-${local.victim-b-basenames.mail-srv1}", local.victim-b-ips.mail-srv1]
    b-file-srv1   = ["${local.scenario-prefixes.b}-${local.victim-b-vms.file-srv1.hostname}-${local.victim-b-basenames.file-srv1}", local.victim-b-ips.file-srv1]

    # protections (victim-protections, 5 hosts)
    p-addc-srv1   = ["${local.scenario-prefixes.protections}-${local.victim-protections-vms.addc-srv1.hostname}-${local.victim-protections-basenames.addc-srv1}", local.victim-protections-ips.addc-srv1]
    p-win11-desk1 = ["${local.scenario-prefixes.protections}-${local.victim-protections-vms.win11-desk1.hostname}-${local.victim-protections-basenames.win11-desk1}", local.victim-protections-ips.win11-desk1]
    p-win11-desk2 = ["${local.scenario-prefixes.protections}-${local.victim-protections-vms.win11-desk2.hostname}-${local.victim-protections-basenames.win11-desk2}", local.victim-protections-ips.win11-desk2]
    p-file-srv1   = ["${local.scenario-prefixes.protections}-${local.victim-protections-vms.file-srv1.hostname}-${local.victim-protections-basenames.file-srv1}", local.victim-protections-ips.file-srv1]
    p-macos-desk1 = ["${local.scenario-prefixes.protections}-${local.victim-protections-vms.macos-desk1.hostname}-${local.victim-protections-basenames.macos-desk1}", local.victim-protections-ips.macos-desk1]
    p-mail-srv1   = ["${local.scenario-prefixes.protections}-${local.victim-protections-vms.mail-srv1.hostname}-${local.victim-protections-basenames.mail-srv1}", local.victim-protections-ips.mail-srv1]
    p-linux-srv1  = ["${local.scenario-prefixes.protections}-${local.victim-protections-vms.linux-srv1.hostname}-${local.victim-protections-basenames.linux-srv1}", local.victim-protections-ips.linux-srv1]

    # external (external-benevolent, 7 hosts)
    redirect1          = ["${local.external-benevolent-vms.redirect-srv1.hostname}-${local.external-benevolent-basenames.redirect1}", local.external-benevolent-ips.redirect1-ip1]
    redirect2          = ["${local.external-benevolent-vms.redirect-srv2.hostname}-${local.external-benevolent-basenames.redirect2}", local.external-benevolent-ips.redirect2-ip1]
    redirect3          = ["${local.external-benevolent-vms.redirect-srv3.hostname}-${local.external-benevolent-basenames.redirect3}", local.external-benevolent-ips.redirect3-ip1]
    redirect4          = ["${local.external-benevolent-vms.redirect-srv4.hostname}-${local.external-benevolent-basenames.redirect4}", local.external-benevolent-ips.redirect4-ip1]
    remote-worker-srv1 = ["${local.external-benevolent-vms.remote-worker-srv1.hostname}-${local.external-benevolent-basenames.remote-worker-1}", local.external-benevolent-ips.remote-worker-1-ip1]
    remote-worker-srv2 = ["${local.external-benevolent-vms.remote-worker-srv2.hostname}-${local.external-benevolent-basenames.remote-worker-2}", local.external-benevolent-ips.remote-worker-2-ip1]
    choco-mirror-srv1  = ["${local.external-benevolent-vms.choco-mirror-srv1.hostname}-${local.external-benevolent-basenames.choco-mirror}", local.external-benevolent-ips.choco-mirror-ip1]

    # attacker
    kali1    = ["${local.attacker-vms.kali1.hostname}-${local.attacker-basenames.kali1}", local.attacker-ip.kali1]
    win-jump = ["${local.attacker-vms.win-jumpbox1.hostname}-${local.attacker-basenames.win-jumpbox1}", local.attacker-ip.win-jumpbox1]
  }

  time-to-delete = timeadd(timestamp(), "24h")
}

output "rdp_data" {
  value = tolist([
    local.rdp_out.kali1,
    local.rdp_out.win-jump,

    local.rdp_out.a-addc-srv1,
    local.rdp_out.a-win11-desk1,
    local.rdp_out.a-win11-desk2,
    local.rdp_out.a-file-srv1,
    local.rdp_out.a-macos-desk1,
    local.rdp_out.a-mail-srv1,

    local.rdp_out.b-addc-srv1,
    local.rdp_out.b-win11-desk1,
    local.rdp_out.b-win11-desk2,
    local.rdp_out.b-kvm-srv1,
    local.rdp_out.b-file-srv1,
    local.rdp_out.b-mail-srv1,

    local.rdp_out.p-addc-srv1,
    local.rdp_out.p-win11-desk1,
    local.rdp_out.p-win11-desk2,
    local.rdp_out.p-file-srv1,
    local.rdp_out.p-macos-desk1,
    local.rdp_out.p-mail-srv1,
    local.rdp_out.p-linux-srv1,

    local.rdp_out.redirect1,
    local.rdp_out.redirect2,
    local.rdp_out.redirect3,
    local.rdp_out.redirect4,
    local.rdp_out.remote-worker-srv1,
    local.rdp_out.remote-worker-srv2,
    local.rdp_out.choco-mirror-srv1
  ])
}

# output "windows-credentials" {
#   value = {
#     (module.a-addc-srv1.name) : module.a-addc-srv1.Windows_Admin_Password,
#     (module.win-jumpbox1.name) : module.win-jumpbox1.Windows_Admin_Password,
#     (module.a-win11-desk1.name) : module.a-win11-desk1.Windows_Admin_Password,
#     (module.a-win11-desk2.name) : module.a-win11-desk2.Windows_Admin_Password,
#   }
# }

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

output "mac-info" {
  value = "Dedicated instances may be deleted in 24 hours at: ${local.time-to-delete}"
}
