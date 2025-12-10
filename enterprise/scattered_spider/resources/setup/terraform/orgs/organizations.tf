# ----------------------------------------------------------------------------------------------------------------------

# CLOUD ORGANIZATION

# This organizations tree must exist before these objects are located and imported into Terraform.
# Our infrastructure required an additional OU to manage multiple accounts, which we called "Vendors."

# ----------------------------------------------------------------------------------------------------------------------
# Get organization root:  [OU] Root
data "aws_organizations_organization" "cloud-ou-root" {
  provider = aws.cloud
}

# Get Management OU:  [OU] Management
data "aws_organizations_organizational_unit" "cloud-ou-management" {
  provider  = aws.cloud
  parent_id = data.aws_organizations_organization.cloud-ou-root.roots[0].id
  name      = "Management"
}

# Get Vendors OU:  [OU] Vendors
data "aws_organizations_organizational_unit" "cloud-ou-vendors" {
  provider  = aws.cloud
  parent_id = data.aws_organizations_organizational_unit.cloud-ou-management.id
  name      = "Vendors"
}
