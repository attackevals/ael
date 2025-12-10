###########################
# Shared IAM Policy Document
###########################
locals {
  admin_policy_document = {
    Version = "2012-10-17"
    Statement = [
      {
        Sid = "VisualEditor0",
        NotAction = [
          "organizations:*",
          "account:*"
        ]
        Effect   = "Allow"
        Resource = "*"
      },
      {
        Sid    = "VisualEditor2",
        Effect = "Allow"
        Action = [
          "account:GetAccountInformation",
          "account:GetPrimaryEmail",
          "account:ListRegions",
          "ec2-instance-connect:SendSerialConsoleSSHPublicKey"
        ],
        Resource = "*"
      },
      {
        Sid    = "VisualEditor1",
        Effect = "Allow"
        Action = [
          "account:GetAccountInformation",
          "account:GetPrimaryEmail",
          "account:ListRegions",
          "ec2-instance-connect:SendSerialConsoleSSHPublicKey",
          "iam:ListMFADevices",
          "iam:GetUser",
          "iam:ListSigningCertificates",
          "iam:GetLoginProfile",
          "iam:ChangePassword",
          "iam:ListAccessKeys"
        ],
        Resource = "arn:aws:iam::*:user/$${aws:username}"
      },
    ]
  }
}

###########################
# Cloudwatch Role
###########################
resource "aws_iam_role" "cloudwatch-detections" {
  provider = aws.cloud-detections

  name = "CloudWatchAgentServerRole"

  assume_role_policy = <<EOF
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Principal": {
        "Service": "ec2.amazonaws.com"
      },
      "Action": "sts:AssumeRole"
    }
  ]
}
EOF
}

resource "aws_iam_role_policy_attachment" "ssmic-detections" {
  provider   = aws.cloud-detections
  role       = aws_iam_role.cloudwatch-detections.name
  policy_arn = "arn:aws:iam::aws:policy/AmazonSSMManagedInstanceCore"
}
resource "aws_iam_role_policy_attachment" "ssmpatch-detections" {
  provider   = aws.cloud-detections
  role       = aws_iam_role.cloudwatch-detections.name
  policy_arn = "arn:aws:iam::aws:policy/AmazonSSMPatchAssociation"
}
resource "aws_iam_role_policy_attachment" "ssmserver-detections" {
  provider   = aws.cloud-detections
  role       = aws_iam_role.cloudwatch-detections.name
  policy_arn = "arn:aws:iam::aws:policy/CloudWatchAgentServerPolicy"
}

###########################
# Admin group and policies
###########################

resource "aws_iam_policy" "admin_policy-detections" {
  provider    = aws.cloud-detections
  name        = "admin_policy"
  path        = "/"
  description = "Admin Access Policy"

  policy = jsonencode(local.admin_policy_document)
}
