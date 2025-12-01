########################################
# red  S3 access group and policies A
########################################
resource "aws_s3_bucket" "per-range-s3" {
  provider = aws.default
  bucket   = "${var.name-prefix}-vermillion"

  tags = merge(local.attacker-tags, { Name = "${var.name-prefix} Vermillion s3" })

}

resource "aws_iam_group_membership" "per-range-s3" {
  provider = aws.default
  name     = "${var.name-prefix}-vermillion-group"
  users = [
    aws_iam_user.red-exfil-s3-user.name,
  ]
  group = aws_iam_group.red-exfil-s3.name
}

resource "aws_iam_user" "red-exfil-s3-user" {
  provider = aws.default
  name     = "${var.name-prefix}-vermillion-user"
  tags     = merge(module.defaults.tags, { Name = "${var.name-prefix}-vermillion-user" })
}

###################################
# red  S3 access group and policies
###################################
resource "aws_iam_group" "red-exfil-s3" {
  provider = aws.default
  name     = "${var.name-prefix}-vermillion"
}

resource "aws_iam_policy" "red-exfil-s3-policy" {
  provider    = aws.default
  name        = "${var.name-prefix}-vermillion-policy"
  path        = "/"
  description = "${var.name-prefix} Vermillion Policy"

  policy = jsonencode({
    "Version" : "2012-10-17",
    "Statement" : [
      {
        "Sid" : "ListObjectsInBucket",
        "Effect" : "Allow",
        "Action" : ["s3:ListBucket"],
        "Resource" : ["arn:aws:s3:::${aws_s3_bucket.per-range-s3.bucket}"]
      },
      {
        "Sid" : "AllObjectActions",
        "Effect" : "Allow",
        "Action" : "s3:*Object",
        "Resource" : ["arn:aws:s3:::${aws_s3_bucket.per-range-s3.bucket}/*"]
      }
    ]
  })
  tags = merge(module.defaults.tags, { Name = "${var.name-prefix}-vermillion-policy" })
}

resource "aws_iam_group_policy_attachment" "red-exfil-s3_policy_attch" {
  provider   = aws.default
  group      = aws_iam_group.red-exfil-s3.name
  policy_arn = aws_iam_policy.red-exfil-s3-policy.arn
}


########################################
# red  S3 access group and policies b3
########################################
resource "aws_s3_bucket" "per-range-s3-b3" {
  provider = aws.default
  bucket   = "${local.b3-prefix}-b3"

  tags = merge(module.defaults.tags, { Name = "${local.b3-prefix}-b3" })

}

resource "aws_iam_group_membership" "per-range-s3-b3" {
  provider = aws.default
  name     = "${local.b3-prefix}-b3-group"
  users = [
    aws_iam_user.red-exfil-s3-user-b3.name,
  ]
  group = aws_iam_group.red-exfil-s3-b3.name
}

resource "aws_iam_user" "red-exfil-s3-user-b3" {
  provider = aws.default
  name     = "${local.b3-prefix}-b3-user"
  tags     = merge(module.defaults.tags, { Name = "${local.b3-prefix}-b3-user" })
}

resource "aws_iam_group" "red-exfil-s3-b3" {
  provider = aws.default
  name     = "${local.b3-prefix}-b3"
}

resource "aws_iam_policy" "red-exfil-s3-policy-b3" {
  provider    = aws.default
  name        = "${local.b3-prefix}-b3-policy"
  path        = "/"
  description = "${local.b3-prefix} b3 Policy"

  policy = jsonencode({
    "Version" : "2012-10-17",
    "Statement" : [
      {
        "Sid" : "ListObjectsInBucket",
        "Effect" : "Allow",
        "Action" : ["s3:ListBucket"],
        "Resource" : ["arn:aws:s3:::${aws_s3_bucket.per-range-s3-b3.bucket}"]
      },
      {
        "Sid" : "AllObjectActions",
        "Effect" : "Allow",
        "Action" : "s3:*Object",
        "Resource" : ["arn:aws:s3:::${aws_s3_bucket.per-range-s3-b3.bucket}/*"]
      }
    ]
  })
  tags = merge(module.defaults.tags, { Name = "${local.b3-prefix}-b3-policy" })
}

resource "aws_iam_group_policy_attachment" "red-exfil-s3_policy_b3" {
  provider   = aws.default
  group      = aws_iam_group.red-exfil-s3-b3.name
  policy_arn = aws_iam_policy.red-exfil-s3-policy-b3.arn
}
