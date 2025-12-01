# `defaults`

# AWS Resource Group Defaults Module

This module creates an AWS Resource Group with standardized tags.

## Features

- Creates a resource group with standardized tags
- Provides consistent tag outputs for other modules to use
- Applies a standard set of tags to resources

## Usage

```
module "defaults" {
  source      = "../modules/aws/defaults"
  name        = "my-resource-group"
  description = "My resource group description"
  category    = "infrastructure"
  environment = "dev"
  region      = "us-west-2"
  # Other variables as needed
}
```

## Working with Existing Resource Groups

To work with existing resource groups, you'll need to import them into your Terraform state:

```bash
terraform import module.<module_name>.aws_resourcegroups_group.defaults <resource_group_name>
```

For example:

```bash
terraform import module.defaults.aws_resourcegroups_group.defaults my-resource-group
```

## Variables

| Name | Description | Type | Default |
|------|-------------|------|---------|
| name | Unique prefix to use when naming resources | string | required |
| region | AWS region for resources | string | required |
| description | Description tag of resource group | string | optional |
| environment | Environment tag of resource group | string | optional |
| category | Category of resource group | string | optional |
| charge_code | Charge code for billing | string | optional |
| owner | AWS Cost allocation tag for owner name | string | optional |
| performer | AWS Cost allocation tag for vendor name | string | optional |
| provisioner | Person or tooling that provisions this resource | string | optional |
| vendor | Name of vendor | string | optional |
| shutdown_ok | This resource can be safely disabled or shutdown | string | optional |
| week | Week of evaluations | string | "init" |
| exec_day | Specifies execution week scenario or day | string | "none" |
| range_status | Cleanliness of range | string | "clean" |
| created | Timestamp of when resource was created | string | optional |
| updated | Timestamp of when resource was last updated | string | optional |

## Outputs

| Name | Description |
|------|-------------|
| name | The name of the resource group |
| tags | The tags of the resource group |
| category | The category of the resource group |
| environment | The environment of the resource group |
| region | The region of the resource group |
| provisioner | The provisioner of the resource group |
| charge_code | The charge code of the resource group |
| performer | The performer of the resource group |
| owner | The owner of the resource group |
