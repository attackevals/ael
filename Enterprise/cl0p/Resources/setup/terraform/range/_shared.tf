locals {
  shared-pcx-red2victim-name        = "${var.name-prefix}-red2victim-pcx"
  shared-pcx-red2benevolent-name    = "${var.name-prefix}-red2benevolent-pcx"
  shared-pcx-victim2benevolent-name = "${var.name-prefix}-victim2benevolent-pcx"
}

resource "aws_vpc_peering_connection" "red2victim-pcx" {
  vpc_id      = aws_vpc.victim.id
  peer_vpc_id = aws_vpc.attacker.id
  auto_accept = true
  accepter { allow_remote_vpc_dns_resolution = true }
  requester { allow_remote_vpc_dns_resolution = true }
  tags = merge(module.defaults.tags, { Name = local.shared-pcx-red2victim-name })
  lifecycle {
    ignore_changes = [tags, tags_all]
  }
}
