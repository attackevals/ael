resource "aws_security_group" "default" {
  name_prefix = "${var.name}-Client-VPN"
  description = "security group allowing egress for client-vpn users"
  vpc_id      = var.vpc_id

  tags = {
    Name               = "${var.name}-Client-VPN"
    EnvName            = var.name
    Service            = "client-vpn"
    TerraformWorkspace = terraform.workspace
  }
}

resource "aws_security_group_rule" "default_egress_world" {
  type              = "egress"
  from_port         = -1
  to_port           = -1
  protocol          = "all"
  cidr_blocks       = ["0.0.0.0/0"]
  security_group_id = aws_security_group.default.id
}
