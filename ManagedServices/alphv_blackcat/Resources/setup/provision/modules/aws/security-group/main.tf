
resource "aws_security_group" "main" {
  name_prefix = "${var.name_prefix}-default-sg-"
  description = var.description
  vpc_id      = var.vpc_id
  ingress {
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = var.ingress_whitelist
  }
  ingress {
    from_port   = 3389
    to_port     = 3389
    protocol    = "tcp"
    cidr_blocks = var.ingress_whitelist
  }
  ingress {
    from_port   = 5985
    to_port     = 5986
    protocol    = "tcp"
    cidr_blocks = var.ingress_whitelist
  }
  # SMB Redirect
  ingress {
    from_port   = 445
    to_port     = 445
    protocol    = "tcp"
    cidr_blocks = var.ingress_whitelist
  }
  # SMB Redirect
  ingress {
    from_port   = 139
    to_port     = 139
    protocol    = "tcp"
    cidr_blocks = var.ingress_whitelist
  }
  # Adversary HTTP server
  ingress {
    from_port   = 80
    to_port     = 8081
    protocol    = "tcp"
    cidr_blocks = var.ingress_whitelist
  }
  # Quasar
  ingress {
    from_port   = 4782
    to_port     = 4782
    protocol    = "tcp"
    cidr_blocks = var.ingress_whitelist
  }
  # SodaMaster
  ingress {
    from_port   = 80
    to_port     = 8082
    protocol    = "tcp"
    cidr_blocks = var.ingress_whitelist
  }
  # Rclone WebDAV
  ingress {
    from_port   = 8080
    to_port     = 8080
    protocol    = "tcp"
    cidr_blocks = var.ingress_whitelist
  }
  ingress {
    from_port   = 20
    to_port     = 20
    protocol    = "tcp"
    cidr_blocks = var.ingress_whitelist
  }
  ingress {
    from_port   = 21
    to_port     = 21
    protocol    = "tcp"
    cidr_blocks = var.ingress_whitelist
  }
  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  lifecycle {
    create_before_destroy = true
    ignore_changes        = [tags, tags_all, ingress, egress]
  }
  tags = merge(var.tags, {
    Name        = "${var.name_prefix}-default-sg"
    Description = var.description
  })
}
