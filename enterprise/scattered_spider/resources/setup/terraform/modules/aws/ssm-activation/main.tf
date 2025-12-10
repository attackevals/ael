resource "aws_iam_role" "ssm_role" {
  name = "${var.name_prefix}-ssm-role"

  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Action = "sts:AssumeRole"
        Effect = "Allow"
        Principal = {
          Service = "ssm.amazonaws.com"
        }
      }
    ]
  })
}

resource "aws_iam_role_policy_attachment" "ssm_managed_instance" {
  role       = aws_iam_role.ssm_role.name
  policy_arn = "arn:aws:iam::aws:policy/AmazonSSMManagedInstanceCore"
}

resource "aws_iam_role_policy_attachment" "cloudwatch_agent" {
  role       = aws_iam_role.ssm_role.name
  policy_arn = "arn:aws:iam::aws:policy/CloudWatchAgentServerPolicy"
}

resource "aws_iam_role_policy" "cross_account_cloudwatch" {
  name = "${var.name_prefix}-cross-account-cloudwatch"
  role = aws_iam_role.ssm_role.id

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Effect = "Allow"
        Action = [
          "cloudwatch:PutMetricData"
        ]
        Resource = "*"
        Condition = {
          StringEquals = {
            "cloudwatch:namespace" = ["CWAgent", "AWS/EC2"]
          }
        }
      },
      {
        Effect = "Allow"
        Action = [
          "sts:AssumeRole"
        ]
        Resource = "arn:aws:iam::756935708346:role/AllowCloudWatchWriteVictimOrgRole"
      }
    ]
  })
}

# Use a local value to calculate the expiration date only once
locals {
  # Format the expiration date as ISO8601 string
  expiration_date = var.fixed_expiration_date != "" ? var.fixed_expiration_date : timeadd(timestamp(), "${var.expiration_days * 24}h")
}

resource "aws_ssm_activation" "ssm_activation" {
  name               = "${var.name_prefix}-activation"
  description        = "SSM activation for ${var.name_prefix}"
  iam_role           = aws_iam_role.ssm_role.id
  registration_limit = var.registration_limit
  expiration_date    = local.expiration_date
  depends_on         = [aws_iam_role_policy_attachment.ssm_managed_instance]

  # Only recreate if explicitly requested or if the activation has expired
  lifecycle {
    ignore_changes        = [expiration_date]
    create_before_destroy = true
  }
}
