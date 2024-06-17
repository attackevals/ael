output "rdp_data" {
  value = tolist([
    local.a_iis,
    local.a_ad,
    local.a_ex,
    local.a_desk1,
    local.a_desk2,
    local.a_desk3,
    local.b_ad,
    local.b_file,
    local.b_ex,
    local.b_sql,
    local.b_kvm,
    local.b_desk1,
    local.b_desk2,
    local.b_desk3,
    local.c_desk1,
    local.red_kali0,
    local.red_kali1,
    local.sup_jumpbox0,
    local.sup_jumpbox1,
    local.sup_dns,
    local.sup_postfix,
    local.sup_rd_a1,
    local.sup_rd_a1_0,
    local.sup_rd_a1_1,
    local.sup_rd_a2,
    local.sup_rd_a2_0,
    local.sup_rd_b,
    local.sup_rd_b_0,
    local.sup_rd_b_1,
  ])
}

output "windows-credentials" {
  value = {
    (module.a-ad-srv1.name) : module.a-ad-srv1.Windows_Admin_Password,
    (module.a-iis-srv1.name) : module.a-iis-srv1.Windows_Admin_Password,
    (module.a-ex-srv1.name) : module.a-ex-srv1.Windows_Admin_Password,
    (module.b-ad-srv1.name) : module.b-ad-srv1.Windows_Admin_Password,
    (module.b-ex-srv1.name) : module.b-ex-srv1.Windows_Admin_Password,
    (module.b-file-srv1.name) : module.b-file-srv1.Windows_Admin_Password,
    (module.b-sql-srv1.name) : module.b-sql-srv1.Windows_Admin_Password,
    (module.support-jumpbox-srv1.name) : module.support-jumpbox-srv1.Windows_Admin_Password,
  }
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
