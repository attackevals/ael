###########################
# Cloudwatch Role
###########################
resource "aws_iam_role" "cloudwatch-protections" {
  provider = aws.cloud-protections

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

resource "aws_iam_role_policy_attachment" "ssmic-protections" {
  provider   = aws.cloud-protections
  role       = aws_iam_role.cloudwatch-protections.name
  policy_arn = "arn:aws:iam::aws:policy/AmazonSSMManagedInstanceCore"
}
resource "aws_iam_role_policy_attachment" "ssmpatch-protections" {
  provider   = aws.cloud-protections
  role       = aws_iam_role.cloudwatch-protections.name
  policy_arn = "arn:aws:iam::aws:policy/AmazonSSMPatchAssociation"
}
resource "aws_iam_role_policy_attachment" "ssmserver-protections" {
  provider   = aws.cloud-protections
  role       = aws_iam_role.cloudwatch-protections.name
  policy_arn = "arn:aws:iam::aws:policy/CloudWatchAgentServerPolicy"
}

###########################
# Admin group and policies
###########################

resource "aws_iam_policy" "admin_policy-protections" {
  provider    = aws.cloud-protections
  name        = "admin_policy"
  path        = "/"
  description = "Admin Access Policy"

  policy = jsonencode(local.admin_policy_document)
}
