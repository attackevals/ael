### Generate client VPN endpoint
resource "aws_ec2_client_vpn_endpoint" "openvpn_endpoint" {
  description            = "${var.name}-Client-VPN"
  server_certificate_arn = aws_acm_certificate.server.arn
  client_cidr_block      = var.cidr
  split_tunnel           = var.split_tunnel
  dns_servers            = var.dns_servers
  self_service_portal    = local.self_service_portal
  security_group_ids     = [var.security_group_id == "" ? aws_security_group.default.id : var.security_group_id]
  vpc_id                 = var.vpc_id

  authentication_options {
    type                       = var.authentication_type
    root_certificate_chain_arn = aws_acm_certificate.root.arn
  }

  connection_log_options {
    enabled               = true
    cloudwatch_log_group  = aws_cloudwatch_log_group.vpn.name
    cloudwatch_log_stream = aws_cloudwatch_log_stream.vpn.name
  }

  client_connect_options {
    enabled             = true
    lambda_function_arn = var.lambda_function_arn
  }

  lifecycle {
    ignore_changes = [
      tags,
      tags_all,
    ]
  }
  tags = merge(var.tags, { "Name" = "${var.name}-Client-VPN", })
}

### Enable VPN connectivity for clients
resource "aws_ec2_client_vpn_network_association" "default" {
  count                  = length(var.subnet_ids)
  client_vpn_endpoint_id = aws_ec2_client_vpn_endpoint.openvpn_endpoint.id
  subnet_id              = element(var.subnet_ids, count.index)
}

### Add explicit route for vpn connections for non-RFC 1918 CIDR blocks
resource "aws_ec2_client_vpn_route" "main" {
  count                  = length(var.target_cidr_blocks)
  client_vpn_endpoint_id = aws_ec2_client_vpn_endpoint.openvpn_endpoint.id
  destination_cidr_block = element(var.target_cidr_blocks, count.index)
  target_vpc_subnet_id   = var.target_subnet_id # for MSR, the subnet target will always be the same
  timeouts {
    create = "15m"
    delete = "15m"
  }
  lifecycle {
    ignore_changes = [target_vpc_subnet_id]
  }
}

### Authorize clients to access network [1/2]
resource "aws_ec2_client_vpn_authorization_rule" "all_groups" {
  count                  = length(var.allowed_access_groups) > 0 ? 0 : length(var.allowed_cidr_ranges)
  client_vpn_endpoint_id = aws_ec2_client_vpn_endpoint.openvpn_endpoint.id
  target_network_cidr    = var.allowed_cidr_ranges[count.index]
  authorize_all_groups   = true
  lifecycle {
    ignore_changes = [target_network_cidr]
  }
}

### Authorize clients to access network [2/2]
resource "aws_ec2_client_vpn_authorization_rule" "specific_groups" {
  count                  = length(var.allowed_access_groups) * length(var.allowed_cidr_ranges)
  client_vpn_endpoint_id = aws_ec2_client_vpn_endpoint.openvpn_endpoint.id
  target_network_cidr    = element(var.allowed_cidr_ranges, count.index)
  access_group_id        = var.allowed_access_groups[count.index % length(var.allowed_cidr_ranges)]
}
