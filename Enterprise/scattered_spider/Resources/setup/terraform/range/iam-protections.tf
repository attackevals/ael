# Locals block for all configurations
locals {
  # Full role name with environment prefix
  protections_full_role_name = local.cross_account_snapshot_role_name
}

# IAM Role
resource "aws_iam_role" "protections_cross_account_role" {
  provider = aws.cloud-protections

  name        = local.protections_full_role_name
  description = "Role for cross-account snapshot management"

  # Assume Role Policy
  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Sid    = "AllowTrustedAccountRoot"
        Effect = "Allow"
        Principal = {
          AWS = "arn:aws:iam::${var.aws_trusted_account_id}:root"
        }
        Action = "sts:AssumeRole"
      },
      {
        Sid    = "AllowLambdaSnapshotRole"
        Effect = "Allow"
        Principal = {
          AWS = [
            "arn:aws:iam::${var.aws_trusted_account_id}:role/AWSLambdaSnapshotCreationJobRole",
            "arn:aws:iam::${var.aws_trusted_account_id}:role/service-role/ManageRangeState-role-2r6n9l3v",
            "arn:aws:iam::${var.aws_trusted_account_id}:role/service-role/ManageRangeShutdownSchedule-role-denphdj5"
          ]
        }
        Action = "sts:AssumeRole"
      }
    ]
  })

  # Merge common tags with role-specific tags
  tags = merge(
    module.defaults.tags, {
      Name        = local.protections_full_role_name
      Service     = "Snapshot Management"
      Description = "Cross-account access role for snapshot management"
    }
  )
}

# Policy document for snapshot management
data "aws_iam_policy_document" "protections_snapshot_management" {
  provider = aws.cloud-protections

  statement {
    sid    = "AllowSnapshotCreation"
    effect = "Allow"
    actions = [
      "ec2:CreateSnapshot",
      "ec2:CreateSnapshots",
      "ec2:DeleteSnapshot",
      "ec2:DescribeSnapshots",
      "ec2:ModifySnapshotAttribute",
      "ec2:ResetSnapshotAttribute"
    ]
    resources = ["*"]
  }

  statement {
    sid    = "AllowVolumeAndInstanceAccess"
    effect = "Allow"
    actions = [
      "ec2:DescribeVolumes",
      "ec2:DescribeInstances",
      "ec2:RebootInstances",
      "ec2:StartInstances",
      "ec2:StopInstances"
    ]
    resources = ["*"]
  }

  statement {
    sid    = "AllowTagging"
    effect = "Allow"
    actions = [
      "ec2:CreateTags",
      "ec2:DeleteTags"
    ]
    resources = [
      "arn:aws:ec2:*::snapshot/*",
      "arn:aws:ec2:*:${data.aws_caller_identity.cloud-protections.account_id}:instance/*"
    ]
  }
}

# Inline policy attachment to the role
resource "aws_iam_role_policy" "protections_snapshot_management" {
  provider = aws.cloud-protections
  name     = "snapshot-management-policy"
  role     = aws_iam_role.protections_cross_account_role.id
  policy   = data.aws_iam_policy_document.protections_snapshot_management.json
}

# Outputs
output "protections_role_arn" {
  description = "ARN of the created IAM role"
  value       = aws_iam_role.protections_cross_account_role.arn
}

output "protections_role_name" {
  description = "Name of the created IAM role"
  value       = aws_iam_role.protections_cross_account_role.name
}

##########################################
# Assumable Roles for Authentik (scenario)
##########################################

resource "aws_iam_role" "protections_authentik_admin_role" {
  provider = aws.cloud-protections

  name        = local.roles.protections-sso-admin-role
  description = "Role for SSO admin"

  # Assume Role Policy
  assume_role_policy = jsonencode({
    Version : "2012-10-17"
    Statement : [
      {
        Effect : "Allow",
        Principal : {
          Federated : "arn:aws:iam::${local.account-protections-id}:saml-provider/authentik"
        }
        Action : "sts:AssumeRoleWithSAML"
        Condition : {
          StringEquals : {
            "SAML:aud" : "https://signin.aws.amazon.com/saml"
          }
        }
      }
    ]
  })

  # Merge common tags with role-specific tags
  tags = merge(
    module.defaults.tags, {
      Service = "Authentik SSO"
    }
  )
}

resource "aws_iam_policy_attachment" "protections_admin_policy_sso" {
  provider   = aws.cloud-protections
  name       = "admin-sso-attachment"
  roles      = [aws_iam_role.protections_authentik_admin_role.name]
  policy_arn = aws_iam_policy.admin_policy-protections.arn
}

##########################################
# Assumable Roles for Authentik (vendor)
##########################################

resource "aws_iam_role" "protections_authentik_vendor_role" {
  provider = aws.cloud-protections

  name        = local.roles.protections-sso-vendor-role
  description = "Role for SSO vendor"

  # Assume Role Policy
  assume_role_policy = jsonencode({
    Version : "2012-10-17"
    Statement : [
      {
        Effect : "Allow",
        Principal : {
          Federated : "arn:aws:iam::${local.account-protections-id}:saml-provider/authentik"
        }
        Action : "sts:AssumeRoleWithSAML"
        Condition : {
          StringEquals : {
            "SAML:aud" : "https://signin.aws.amazon.com/saml"
          }
        }
      }
    ]
  })

  # Merge common tags with role-specific tags
  tags = merge(
    module.defaults.tags, {
      Service = "Authentik SSO"
    }
  )
}

resource "aws_iam_policy_attachment" "protections_vendor_policy_sso" {
  provider   = aws.cloud-protections
  name       = "vendor-sso-attachment"
  roles      = [aws_iam_role.protections_authentik_vendor_role.name]
  policy_arn = "arn:aws:iam::aws:policy/ReadOnlyAccess"
}
