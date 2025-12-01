# Certificate Revocation List (CRL) Role

This role manages Certificate Revocation List (CRL) generation and distribution using cfssl.

## Features

- CRL generation using cfssl
- CRL distribution via nginx
- Automated CRL renewal via cron
- Manual CRL regeneration capability

## Variables

### Base Certificate Variables

- `cfssl_base_dir`: Base directory for cfssl (default: "/opt/ca/cfssl")
- `certs_folder`: Path to certificates folder
- `ca_cert_name`: Filename for the CA certificate
- `ca_domain`: Domain name for the CA

### CRL Variables

- `ca_crl_url`: URL where the CRL will be published
- `crl_enabled`: Enable/disable CRL functionality (default: true)
- `crl_folder`: Directory for CRL storage (default: "{{ cfssl_base_dir}}/{{ certs_folder }}/crl")
- `crl_filename`: Filename for the CRL (default: "ca.crl")
- `crl_days`: CRL validity period in days (default: 15)
- `crl_nginx_port`: Port for nginx to serve the CRL (default: 80)
- `crl_nginx_server_name`: Server name for nginx CRL hosting (default: "{{ ca_domain }}")

## Example Playbook

```yaml
- hosts: certificate_servers
  become: yes
  roles:
    - role: certs_crl
      vars:
        cfssl_base_dir: "/opt/ca/cfssl"
        certs_folder: "/etc/pki/certs"
        ca_cert_name: "ca"
        ca_domain: "crl.example.com"
        ca_crl_url: "http://crl.example.com/ca.crl"
        crl_days: 30
        crl_nginx_port: 80
```

## Manual CRL Regeneration

To manually regenerate the CRL:

```bash
ansible-playbook regenerate-crl.yml -e "target=certificate_servers"
```

## Related Roles

- `certs`: Provides certificate generation functionality
