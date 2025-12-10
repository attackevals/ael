# IAM policy for vendor access to IAM
resource "aws_iam_policy" "vendor_iam_access" {
  provider = aws.cloud-detections

  name        = "${var.name-prefix}-vendor-iam-access"
  description = "Separate IAM policy for vendor access to IAM services"

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Sid    = "AllowVendorIAMAccess"
        Effect = "Allow"
        Action = [
          "iam:GetUser",
          "iam:ListUsers",
          "iam:ListGroups",
          "iam:ListRoles",
          "iam:ListPolicies",
          "iam:GetPolicy",
          "iam:GetPolicyVersion",
          "iam:ListAttachedUserPolicies",
          "iam:ListAttachedGroupPolicies",
          "iam:ListAttachedRolePolicies",
          "iam:ListUserPolicies",
          "iam:ListGroupPolicies",
          "iam:ListRolePolicies"
        ]
        Resource = "*"
      }
    ]
  })

  tags = merge(
    module.defaults.tags, {
      Name        = "${var.name-prefix}-vendor-iam-access"
      Description = "Policy for vendor access to IAM services"
    }
  )
}

# Output the policy ARN
output "vendor_iam_access_policy_arn" {
  description = "ARN of the vendor IAM access policy"
  value       = aws_iam_policy.vendor_iam_access.arn
}
