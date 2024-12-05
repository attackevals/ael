### Generate certificate authority certs/keys

/*
A Client VPN endpoint supports 1024-bit and 2048-bit RSA key sizes only.
Also, the client certificate must have the CN attribute in the Subject field.

When the certificate being used by the Client VPN endpoint is updated,
either through ACM auto-rotation or manually importing a new certificate,
the Client VPN service will update the Client VPN endpoint with the newer certificate.
However, this automated process can take up to 24 hours.
*/

resource "tls_private_key" "ca" {
  algorithm = "RSA"
}

resource "tls_self_signed_cert" "ca" {
  private_key_pem = tls_private_key.ca.private_key_pem

  subject {
    common_name  = "${var.name}.vpn.ca"
    organization = var.organization_name
  }

  validity_period_hours = 87600
  is_ca_certificate     = true

  allowed_uses = [
    "cert_signing",
    "crl_signing",
  ]
}

resource "aws_acm_certificate" "ca" {
  private_key      = tls_private_key.ca.private_key_pem
  certificate_body = tls_self_signed_cert.ca.cert_pem
}
