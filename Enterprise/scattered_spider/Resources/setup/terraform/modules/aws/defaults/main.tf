resource "aws_resourcegroups_group" "defaults" {
  name = var.name
  tags = {
    Category    = tostring(try(var.category))
    ChargeCode  = tostring(try(var.charge_code))
    Description = tostring(try(var.description))
    Environment = tostring(try(var.environment))
    Name        = tostring(try(var.name))
    Owner       = tostring(try(var.owner))
    Performer   = tostring(try(var.performer))
    Provisioner = tostring(try(var.provisioner))
    Region      = tostring(try(var.region))
    ShutdownOk  = tostring(try(var.shutdown_ok))
    Vendor      = tostring(try(var.vendor))
    Week        = tostring(try(var.week))
    ExecDay     = tostring(try(var.exec_day))
    RangeStatus = tostring(try(var.range_status))
  }

  resource_query {
    type  = "TAG_FILTERS_1_0"
    query = "{\"ResourceTypeFilters\":[\"AWS::AllSupported\"],\"TagFilters\":[{\"Key\":\"Stage\",\"Values\":[\"Test\"]}]}"
  }
}
