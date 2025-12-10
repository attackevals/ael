### Generate root certs/keys

/*
A Client VPN endpoint supports 1024-bit and 2048-bit RSA key sizes only.
Also, the client certificate must have the CN attribute in the Subject field.

When the certificate being used by the Client VPN endpoint is updated,
either through ACM auto-rotation or manually importing a new certificate,
the Client VPN service will update the Client VPN endpoint with the newer certificate.
However, this automated process can take up to 24 hours.
*/

resource "tls_private_key" "root" {
  algorithm = "RSA"
}

resource "tls_cert_request" "root" {
  private_key_pem = tls_private_key.root.private_key_pem

  subject {
    common_name  = "${var.name}.vpn.client"
    organization = var.organization_name
  }
}

resource "tls_locally_signed_cert" "root" {
  cert_request_pem   = tls_cert_request.root.cert_request_pem
  ca_private_key_pem = tls_private_key.ca.private_key_pem
  ca_cert_pem        = tls_self_signed_cert.ca.cert_pem

  validity_period_hours = 87600

  allowed_uses = [
    "key_encipherment",
    "digital_signature",
    "client_auth",
  ]
}

resource "aws_acm_certificate" "root" {
  private_key       = tls_private_key.root.private_key_pem
  certificate_body  = tls_locally_signed_cert.root.cert_pem
  certificate_chain = tls_self_signed_cert.ca.cert_pem
}
