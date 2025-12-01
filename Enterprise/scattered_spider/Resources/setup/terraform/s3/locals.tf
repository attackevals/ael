locals {

  b3-prefix = md5("${var.name-prefix}")

  scope = {
    victim-tag   = { Scope : "victim" } # Vendor has visibility
    attacker-tag = { Scope : "red" }
  }

  context = {
    cloud-tag    = { Context : "cloud" }
    b3-tag       = { Context : "b3" }
    attacker-tag = { Context : "red" }
  }

  tags = {
    Category    = tostring(try(var.category))
    ChargeCode  = tostring(try(var.charge-code))
    Environment = tostring(try(var.environment))
    Owner       = tostring(try(var.round-name))
    Performer   = tostring(try(var.name-prefix))
    Provisioner = tostring(try(var.provisioner))
    ShutdownOk  = tostring(try(var.shutdown_ok))
    Vendor      = tostring(try(var.vendor))
  }
  # Tags
  victim-tags       = merge(local.tags, local.scope.victim-tag)
  attacker-tags     = merge(local.tags, local.scope.attacker-tag, local.context.attacker-tag)
  victim-cloud-tags = merge(local.victim-tags, local.context.cloud-tag)
  b3-tags           = merge(local.victim-tags, local.context.b3-tag)
}
