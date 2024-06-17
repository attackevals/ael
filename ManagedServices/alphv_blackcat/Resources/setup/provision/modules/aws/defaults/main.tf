resource "aws_resourcegroups_group" "defaults" {
  name = var.name
  tags = {
    Name        = tostring(try(var.name))
    Description = tostring(try(var.description))
    Category    = tostring(try(var.category))
    ChargeCode  = tostring(try(var.charge_code))
    Environment = tostring(try(var.environment))
    Region      = tostring(try(var.region))
    Vendor      = tostring(try(var.vendor))
    Provisioner = tostring(try(var.provisioner))
    ShutdownOk  = tostring(try(var.shutdown_ok))
  }

  resource_query {
    type  = "TAG_FILTERS_1_0"
    query = "{\"ResourceTypeFilters\":[\"AWS::AllSupported\"],\"TagFilters\":[{\"Key\":\"Stage\",\"Values\":[\"Test\"]}]}"
  }
}
