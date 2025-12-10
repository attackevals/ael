locals {
  self_service_portal = var.enable_self_service_portal == true ? "enabled" : "disabled"
}
