# Certificates for ER6

- [Certificates for ER6](#certificates-for-er6)
  - [Summary](#summary)
  - [Example Creation](#example-creation)
    - [Certificate Authority](#certificate-authority)
    - [Host Certificate](#host-certificate)
  - [File Structure Breakdown](#file-structure-breakdown)
    - [Configuration and CSR Files](#configuration-and-csr-files)
    - [Domain Certificate](#domain-certificate)
    - [Certificate Authority/Intermediate Certificate Authority](#certificate-authorityintermediate-certificate-authority)

## Summary

[CloudFlare's CFSSL project](https://github.com/cloudflare/cfssl) was used to generate a certificate authority, intermediate certificate, and host certificate for scenarios A, B and the protections range to emulate a valid issued TLS certificate.

## Example Creation

### Certificate Authority

The following provides an example of how to generate the CA, intermediate CA, and host certificate using CFSSL. A full explanation of CFSSL can be found on the project [GitHub](https://github.com/cloudflare/cfssl) page.

1. Update `ca.json`, `intermediate-ca.json`, and `host1.json` as appropriate to reflect the new certificate authority, certificate authority intermediate certificate, and host certificate for web host.
2. The `cfssl.json` provided has the needed roles predefined, but may need to be modified based on your particular needs.
3. Run the following commands to generate the CA certificate, and intermediate issuing certificate:

```shell
cfssl gencert -initca ca.json|cfssljson -bare ca
cfssl gencert -initca intermediate-ca.json| cfssljson -bare intermediate_ca
cfssl sign -ca ca.pem -ca-key ca-key.pem -config cfssl.json -profile intermediate_ca intermediate_ca.csr | cfssljson -bare intermediate_ca
```

### Host Certificate

1. Run the following command:

   ```shell
     cfssl gencert -ca intermediate_ca.pem -ca-key intermediate_ca-key.pem -config cfssl.json -profile=server host1.json|cfssljson -bare host-1-server
     ```

2. Jobs done.

## File Structure Breakdown

The csfll.json file used to configure all the certs with the predefined roles can be found at the top level of
`ansible/files/domain-certs-er6'.

### Configuration and CSR Files

All `.json` and `.csr` files are configuration and certificate signing request files, and are only used during the certificate generation process.

Each domain has its own subdirectory, and within each subdirectory are the following files:

### Domain Certificate

| File                                                         | Purpose            | Note                  |
| ------------------------------------------------------------ | ------------------ | --------------------- |
| host-*[decryptmalfoy\|encryptpotter\|sonicbeats37]*-server.pem | Domain Certificate | Use this to sign code |
| host-*[decryptmalfoy\|encryptpotter\|sonicbeats37]*-server-key.pem                            | Associated Key     |                       |

### Certificate Authority/Intermediate Certificate Authority

| File                                  | Purpose                           | Note                                                         |
| ------------------------------------- | --------------------------------- | :---------------------------------------- |
| ca-*[decryptmalfoy\|encryptpotter\|sonicbeats37]*.pem                  | Certificate Authority Certificate | Use this to generate more intermediate CAs                   |
| ca-*[decryptmalfoy\|encryptpotter\|sonicbeats37]*-key.pem              | Associated Key                    |                                                              |
| intermediate-ca-*[decryptmalfoy\|encryptpotter\|sonicbeats37]*.pem     | Intermediate CA Certificate       | Use intermediate to sign more host certificates. Intermediate certificates are traditionally used to limit collateral damage if a certificate is compromised. |
| intermediate-ca-*[decryptmalfoy\|encryptpotter\|sonicbeats37]*-key.pem | Associated Key                    |                                                              |
