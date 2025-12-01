locals {
  gitlab-pat-secret-name = "${var.name-prefix}-gitlab-pat-atargaryen2"
}
# AWS Secrets Manager for GitLab PAT
resource "aws_secretsmanager_secret" "gitlab_pat" {
  provider = aws.cloud-detections

  name = local.gitlab-pat-secret-name
  # description = "Personal Access Token for atargaryen user in GitLab"

  tags = merge(
    module.defaults.tags, {
      Name        = local.gitlab-pat-secret-name
      Description = "GitLab PAT"
      Service     = "GitLab"
    }
  )
}

# Secret version with placeholder value - to be updated manually with actual PAT
resource "aws_secretsmanager_secret_version" "gitlab_pat" {
  provider = aws.cloud-detections

  secret_id = aws_secretsmanager_secret.gitlab_pat.id
  secret_string = jsonencode({
    username = "atargaryen"
    token    = "PLACEHOLDER_TOKEN" # Replace with actual token manually after deployment
  })
}

# Output the secret ARNs
output "gitlab_pat_secret_arn" {
  description = "ARN of the GitLab PAT secret"
  value       = aws_secretsmanager_secret.gitlab_pat.arn
}
