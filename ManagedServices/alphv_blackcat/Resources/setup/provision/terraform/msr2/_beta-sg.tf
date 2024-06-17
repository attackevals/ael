resource "aws_security_group" "beta" {
  name_prefix = "${var.name-prefix}-beta-sg-"
  description = "Beta Security Group"
  vpc_id      = aws_vpc.beta.id
  ingress {
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  # dns
  ingress {
    from_port   = 53
    to_port     = 53
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  ingress {
    from_port   = 53
    to_port     = 53
    protocol    = "udp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }

  ingress {
    from_port   = 3389
    to_port     = 3389
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  ingress {
    from_port   = 5985
    to_port     = 5986
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  # SMB Redirect
  ingress {
    from_port   = 445
    to_port     = 445
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  # SMB Redirect
  ingress {
    from_port   = 139
    to_port     = 139
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  # Adversary HTTP server
  ingress {
    from_port   = 80
    to_port     = 8081
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  # Quasar
  ingress {
    from_port   = 4782
    to_port     = 4782
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  # SodaMaster
  ingress {
    from_port   = 80
    to_port     = 8082
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  # Rclone WebDAV
  ingress {
    from_port   = 8080
    to_port     = 8080
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  ingress {
    from_port   = 20
    to_port     = 20
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  ingress {
    from_port   = 21
    to_port     = 21
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }
  ingress {
    from_port   = 40000
    to_port     = 40100
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }

  # ghosts traffic
  ingress {
    from_port   = 52380
    to_port     = 52390
    protocol    = "tcp"
    cidr_blocks = tolist([local.cidr.alpha-all])
  }

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
    Name        = "${var.name-prefix}-beta-sg"
    Description = "Beta Security Group"
  })
}
