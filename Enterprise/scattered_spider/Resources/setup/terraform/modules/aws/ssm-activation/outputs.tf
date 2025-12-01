output "activation_id" {
  description = "The ID of the SSM activation"
  value       = aws_ssm_activation.ssm_activation.id
}

output "activation_code" {
  description = "The activation code for the SSM activation"
  value       = aws_ssm_activation.ssm_activation.activation_code
  sensitive   = true
}
