################################
# S3 bucket
################################

resource "aws_s3_bucket" "admin-noise" {
  provider = aws.cloud-detections
  bucket   = "${var.name-prefix}-backup-data"
  tags     = merge(module.cloud-defaults-detections.tags, { Name = "Backup data" })
  lifecycle {
    prevent_destroy = true
  }
}

################################
# Backup bot group and policies
################################
resource "aws_iam_user" "backup-bot" {
  provider = aws.cloud-detections
  name     = "backup-bot"
  tags     = merge(module.cloud-defaults-detections.tags, { Name = "Backup bot" })
  lifecycle {
    ignore_changes = [
      tags_all,
      tags
    ]
  }
}

resource "aws_iam_group" "backup" {
  provider = aws.cloud-detections
  name     = "backup"
}

resource "aws_iam_policy" "backup_bot_policy" {
  provider    = aws.cloud-detections
  name        = "backup_policy"
  path        = "/"
  description = "BackupBot Policy"

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Action = "s3:*"
        Effect = "Allow"
        Resource = [
          "arn:aws:s3:::${aws_s3_bucket.admin-noise.bucket}",
          "arn:aws:s3:::${aws_s3_bucket.admin-noise.bucket}/*",
        ]
      },
    ]
  })
  tags = merge(module.cloud-defaults-detections.tags, { Name = "BackupBot Policy" })
}

resource "aws_iam_group_membership" "backup-grp" {
  provider = aws.cloud-detections
  name     = "backup-grp-membership"
  users = [
    aws_iam_user.backup-bot.name
  ]
  group = aws_iam_group.backup.name
}

resource "aws_iam_group_policy_attachment" "backup_policy_attach" {
  provider   = aws.cloud-detections
  group      = aws_iam_group.backup.name
  policy_arn = aws_iam_policy.backup_bot_policy.arn
}
