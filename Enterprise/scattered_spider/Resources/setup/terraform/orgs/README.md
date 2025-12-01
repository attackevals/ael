---
tags: [infrastructure, enterprise2025, aws, terraform, organizations, stage1]
label: AWS Account Setup
order: 3
---

# `orgs`

Terraform deployment 1 of 3. Please see the main [Terraform Deployment](../README.md) page for more information.

Enterprise Evaluation 2025 infrastructure emulates an organization with on-premises resources and an AWS cloud environment,
with network isolation into two environments for detections and protections. The `orgs` deployment uses AWS Organizations to create the two accounts for detections and protections.

## Overview

Before proceeding with this stage:

- [ ] Ensure you have an existing AWS account for cloud-based resource management (i.e., `aws.cloud`).
- [ ] The `aws.cloud` provider depends on an IAM role called `CrossAccountAccess` under its account, which must have:
  - [ ] `AdministratorAccess` permissions attached
  - [ ] a trust policy allowing `aws.default` (root) to assume this role
- [ ] Please review `orgs/*.tf` for [data sources](https://developer.hashicorp.com/terraform/language/block/data), which may define resources that do not yet exist under your cloud management account.
- [ ] Please review `orgs/deploy.auto.tfvars` to ensure the following variables are set:

| Variable Name                                             | Variable Description                                                               |
|-----------------------------------------------------------|------------------------------------------------------------------------------------|
| ⭐️ `account-cloud-management-id`                          | 12-digit ID of the parent AWS account where cloud-based scenarios will be emulated |
| ⭐️ `cloud-identity-store-id`                              | "d-" prefixed identity store ID of the IAM Identity Center instance                |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`account-detections-email`  | valid email address used in account creation                                       |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`account-protections-email` | valid email address used in account creation                                       |

⭐️ The `aws.cloud` provider points to an existing AWS account in which you have `AdministratorAccess` permissions and API access.

The `aws.cloud` provider creates, manages, and provisions resources to the two accounts,
then [outputs](https://developer.hashicorp.com/terraform/language/block/output) account information for the `s3` and `range` deployments.
The other deployments depend on and reference these outputs locally from the `orgs/terraform.tfstate` state file,
using the [`terraform_remote_state`](https://developer.hashicorp.com/terraform/language/state/remote-state-data) data source:

```terraform
## s3/provider.tf
## range/provider.tf

data "terraform_remote_state" "orgs" {
  backend = "local"
  config = {
    path = "../orgs/terraform.tfstate"
  }
}

locals {
  account-detections-id       = data.terraform_remote_state.orgs.outputs.account-detections.id
  account-protections-id      = data.terraform_remote_state.orgs.outputs.account-protections.id
  account-cloud-management-id = data.terraform_remote_state.orgs.outputs.account-cloud-management.account_id
}
```

### Import Existing Accounts

If you have two existing AWS accounts to emulate the Detections and Protections scenarios, please collect their account IDs and follow the import instructions for the [`aws_organizations_account`](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/organizations_account#import) resource:

```bash
terraform import aws_organizations_account.detections  000000000010
terraform import aws_organizations_account.protections 000000000011
```

It is recommended to place both accounts in the same Organizational Unit (OU). Please ensure the fields marked in `deploy.auto.tfvars` are correctly set to the admin account of this OU. Create a [`CrossAccountAccess` IAM role](#quick-start) in each account to preserve role-chaining between stages, or change the `role_name` field in `accounts-*.tf` to the name of your cross-account admin role. See documentation on [`role_name`](https://registry.terraform.io/providers/hashicorp/aws/latest/docs/resources/organizations_account.html#role_name-1).
