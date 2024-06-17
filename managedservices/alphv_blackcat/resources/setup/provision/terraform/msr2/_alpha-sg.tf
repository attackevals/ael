resource "aws_security_group" "alpha" {
  name_prefix = "${var.name-prefix}-alpha-sg-"
  description = "Alpha Security Group"
  vpc_id      = aws_vpc.alpha.id
  ingress {
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = local.alpha_ingress_whitelist
  }
  ingress {
    from_port   = 3389
    to_port     = 3389
    protocol    = "tcp"
    cidr_blocks = local.alpha_ingress_whitelist
  }
  ingress {
    from_port   = 5985
    to_port     = 5986
    protocol    = "tcp"
    cidr_blocks = local.alpha_ingress_whitelist
  }
  ingress {
    from_port   = 8
    to_port     = 0
    protocol    = "icmp"
    cidr_blocks = local.alpha_ingress_whitelist
  }
  # allow traffic from beta VPC SG
  ingress {
    from_port       = 0
    to_port         = 0
    security_groups = [aws_security_group.beta.id]
    protocol        = "-1"
  }

  # allow traffic from within VPC
  ingress {
    from_port = 0
    to_port   = 0
    protocol  = "-1"
    self      = true
  }

  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  lifecycle {
    create_before_destroy = true
    ignore_changes        = [tags, tags_all, egress]
  }
  tags = merge(module.defaults.tags, {
    Name        = "${var.name-prefix}-alpha-sg"
    Description = "Alpha Security Group"
  })
}
