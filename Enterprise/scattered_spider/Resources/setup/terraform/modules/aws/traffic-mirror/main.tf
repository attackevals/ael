##################################################################################################################
# Create mirror target
##################################################################################################################

# Create traffic mirror target on subnet
resource "aws_ec2_traffic_mirror_target" "target" {
  description                       = "Mirror target attached"
  network_interface_id              = var.target_nic_id
  gateway_load_balancer_endpoint_id = var.target_gateway_load_balancer_endpoint_id
  network_load_balancer_arn         = var.target_network_load_balancer_arn
  tags = merge(var.tags, {
    Name : var.name
    Description : "Mirror target attached"
    TargetID : var.target_nic_id != null ? var.target_nic_id : (var.target_gateway_load_balancer_endpoint_id != null ? var.target_gateway_load_balancer_endpoint_id : var.target_network_load_balancer_arn)
  })
}

# Create traffic mirror filter
resource "aws_ec2_traffic_mirror_filter" "filter" {
  description      = "traffic mirror filter"
  network_services = ["amazon-dns"]
  tags             = merge(var.tags, { Name : local.filter_name })
}

# Egress filter rule
# let it all through
resource "aws_ec2_traffic_mirror_filter_rule" "egress" {
  description              = "${var.name} - egress filter rule"
  traffic_mirror_filter_id = aws_ec2_traffic_mirror_filter.filter.id
  destination_cidr_block   = "0.0.0.0/0"
  source_cidr_block        = "0.0.0.0/0"
  protocol                 = 6
  rule_number              = 1
  rule_action              = "accept"
  traffic_direction        = "egress"
}

# Ingress filter rule
# let it all through
resource "aws_ec2_traffic_mirror_filter_rule" "ingress" {
  description              = "${var.name} - ingress filter rule"
  traffic_mirror_filter_id = aws_ec2_traffic_mirror_filter.filter.id
  destination_cidr_block   = "0.0.0.0/0"
  source_cidr_block        = "0.0.0.0/0"
  protocol                 = 6
  rule_number              = 1
  rule_action              = "accept"
  traffic_direction        = "ingress"
}

##################################################################################################################
# Create mirror sessions
##################################################################################################################

# Attach traffic mirror filter to interface
# LOOP - Create traffic mirror sessions
resource "aws_ec2_traffic_mirror_session" "session" {
  count                    = length(var.interface_ids)
  description              = "Traffic Mirror Session for ${var.interface_ids[count.index].name}"
  network_interface_id     = var.interface_ids[count.index].network_interface_id
  traffic_mirror_target_id = aws_ec2_traffic_mirror_target.target.id
  traffic_mirror_filter_id = aws_ec2_traffic_mirror_filter.filter.id
  session_number           = var.session_number

  tags = merge(var.tags, {
    Name : "${var.interface_ids[count.index].name}-tmt-eni"
    Hostname : var.interface_ids[count.index].name
    instance-id : var.interface_ids[count.index].network_interface_id
  })
}
