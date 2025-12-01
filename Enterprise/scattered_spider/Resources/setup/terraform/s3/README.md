---
tags: [infrastructure, enterprise2025, aws, terraform, s3, stage2]
label: Attacker Staging
order: 2
---

# `s3`

Terraform deployment 2 of 3. Please see the main [Terraform Deployment](../README.md) page for more information.

The `s3` deployment stages scenario-relevant data in customer's AWS environments (i.e., in the `aws.cloud-detections` and `aws.cloud-protections` accounts). The `s3` deployment also stages the attacker's S3 access under the `aws.default` account. This part of the emulation allows the attacker to redirect customer data flows to an adversary-controlled S3 bucket for exfiltration.

Before proceeding with this stage, please review `deploy.auto.tfvars` and set these variables to your desired values.
