# Certificate Management Role

This role manages certificate generation using cfssl.

## Features

- CA certificate generation
- Server certificate generation for multiple domains and sites
- Integration with CRL functionality (via certs_crl role)

## Variables

### Certificate Variables

- `certs_folder`: Base directory for certificate storage
- `ca_common_name`: Common name for the CA certificate
- `ca_country_name`: Country for the CA certificate
- `ca_locality_name`: Locality for the CA certificate
- `ca_organization_name`: Organization for the CA certificate
- `ca_state_or_province_name`: State/Province for the CA certificate
- `ca_cert_name`: Filename for the CA certificate
- `cert_domains`: List of domains and sites for certificate generation
- `ca_crl_url`: URL where the CRL will be published (used by certs_crl role)

## Example Playbook

```yaml
- hosts: certificate_servers
  become: yes
  roles:
    - role: certs
      vars:
        certs_folder: "/etc/pki/certs"
        ca_common_name: "Example CA"
        ca_country_name: "US"
        ca_locality_name: "San Francisco"
        ca_organization_name: "Example Org"
        ca_state_or_province_name: "California"
        ca_cert_name: "ca"
        cert_domains:
          - domain: "example.com"
            sites:
              - name: "www"
                dns: ["api", "admin"]
```

## Related Roles

- `certs_crl`: Provides CRL generation and distribution functionality
