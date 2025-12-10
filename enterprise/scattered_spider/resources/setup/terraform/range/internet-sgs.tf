locals {
  ### INTERNET ON
  attacker-security-groups-to-apply       = tolist([aws_security_group.attacker.id, aws_security_group.allow-outbound-attacker.id])
  neutral-security-groups-to-apply        = tolist([aws_security_group.attacker.id, aws_security_group.allow-outbound-attacker.id])
  victim-on-prem-security-groups-to-apply = tolist([aws_security_group.victim-on-prem.id, aws_security_group.allow-outbound-victim.id])
  victim-dmz-security-groups-to-apply     = tolist([aws_security_group.victim-dmz.id, aws_security_group.allow-outbound-victim.id])
  b3-dmz-security-groups-to-apply         = tolist([aws_security_group.b3-dmz.id, aws_security_group.allow-outbound-b3.id])
  b3-security-groups-to-apply             = tolist([aws_security_group.b3.id, aws_security_group.allow-outbound-b3.id])
  #   cloud-security-groups-to-apply          = tolist([aws_security_group.cloud.id, aws_security_group.allow-outbound-victim.id])

  ### INTERNET OFF
  # attacker-security-groups-to-apply = tolist([aws_security_group.attacker.id, aws_security_group.block-outbound-attacker.id])
  # neutral-security-groups-to-apply = tolist([aws_security_group.attacker.id, aws_security_group.block-outbound-attacker.id])
  # victim-on-prem-security-groups-to-apply = tolist([aws_security_group.victim-on-prem.id, aws_security_group.block-outbound-victim.id])
  # victim-dmz-security-groups-to-apply = tolist([aws_security_group.victim-dmz.id, aws_security_group.block-outbound-victim.id])
  # cloud-security-groups-to-apply = tolist([aws_security_group.cloud.id, aws_security_group.block-outbound-victim.id])
}

resource "aws_security_group" "allow-outbound-victim" {
  name_prefix = "${local.victim-sg-name}-outbound-victim-allow"
  description = local.victim-sg-description
  vpc_id      = aws_vpc.victim.id
}

resource "aws_security_group" "block-outbound-victim" {
  name_prefix = "${local.victim-sg-name}-outbound-victim-block"
  description = local.victim-sg-description
  vpc_id      = aws_vpc.victim.id
}

resource "aws_security_group" "allow-outbound-b3" {
  name_prefix = "${local.b3-sg-name}-outbound-b3-allow"
  description = local.victim-sg-description
  vpc_id      = aws_vpc.b3.id
}

resource "aws_security_group" "block-outbound-b3" {
  name_prefix = "${local.b3-sg-name}-outbound-b3-block"
  description = local.victim-sg-description
  vpc_id      = aws_vpc.b3.id
}

resource "aws_security_group" "allow-outbound-attacker" {
  name_prefix = "${local.attacker-sg-name}-outbound-attacker-allow"
  description = local.attacker-sg-description
  vpc_id      = aws_vpc.attacker.id
}

resource "aws_security_group" "block-outbound-attacker" {
  name_prefix = "${local.attacker-sg-name}-outbound-attacker-block"
  description = local.attacker-sg-description
  vpc_id      = aws_vpc.attacker.id
}

resource "aws_security_group_rule" "outbound-allow-victim" {
  from_port = 0
  to_port   = 0
  protocol  = "-1"

  # block internet except for jfrog, splunk, wazuh
  cidr_blocks       = ["0.0.0.0/0"]
  security_group_id = aws_security_group.allow-outbound-victim.id
  type              = "egress"
}

resource "aws_security_group_rule" "outbound-allow-b3" {
  from_port = 0
  to_port   = 0
  protocol  = "-1"

  # block internet except for jfrog, splunk, wazuh
  cidr_blocks       = ["0.0.0.0/0"]
  security_group_id = aws_security_group.allow-outbound-b3.id
  type              = "egress"
}

resource "aws_security_group_rule" "outbound-allow-attacker" {
  from_port = 0
  to_port   = 0
  protocol  = "-1"

  # block internet except for jfrog, splunk, wazuh
  cidr_blocks       = ["0.0.0.0/0"]
  security_group_id = aws_security_group.allow-outbound-attacker.id
  type              = "egress"
}

resource "aws_security_group_rule" "outbound-lockdown-victim" {
  from_port = 0
  to_port   = 0
  protocol  = "-1"

  # block internet except for jfrog, splunk, wazuh
  prefix_list_ids   = [data.aws_ec2_managed_prefix_list.outbound.id]
  security_group_id = aws_security_group.block-outbound-victim.id
  type              = "egress"
}

resource "aws_security_group_rule" "outbound-lockdown-b3" {
  from_port = 0
  to_port   = 0
  protocol  = "-1"

  # block internet except for jfrog, splunk, wazuh
  prefix_list_ids   = [data.aws_ec2_managed_prefix_list.outbound.id]
  security_group_id = aws_security_group.block-outbound-b3.id
  type              = "egress"
}

resource "aws_security_group_rule" "outbound-lockdown-attacker" {
  from_port = 0
  to_port   = 0
  protocol  = "-1"

  # block internet except for jfrog, splunk, wazuh
  prefix_list_ids   = [data.aws_ec2_managed_prefix_list.outbound.id]
  security_group_id = aws_security_group.block-outbound-attacker.id
  type              = "egress"
}

# resource "aws_security_group_rule" "outbound-lockdown-sgva" {
#   from_port = 0
#   to_port   = 0
#   protocol  = "-1"
#
#   # block internet except for jfrog, splunk, wazuh
#   prefix_list_ids = [data.aws_ec2_managed_prefix_list.outbound.id]
#
#   security_group_id = aws_security_group.victim-a.id
#   type              = "egress"
# }
# resource "aws_security_group_rule" "outbound-lockdown-sgvb" {
#   from_port = 0
#   to_port   = 0
#   protocol  = "-1"
#
#   # block internet except for jfrog, splunk, wazuh
#   prefix_list_ids = [data.aws_ec2_managed_prefix_list.outbound.id]
#
#   # open internet
#   # cidr_blocks = ["0.0.0.0/0"]
#
#   security_group_id = aws_security_group.victim-b.id
#   type              = "egress"
# }
#
# resource "aws_security_group_rule" "outbound-lockdown-sgvp" {
#   from_port = 0
#   to_port   = 0
#   protocol  = "-1"
#
#   # block internet except for jfrog, splunk, wazuh
#   prefix_list_ids = [data.aws_ec2_managed_prefix_list.outbound.id]
#
#   # open internet
#   # cidr_blocks = ["0.0.0.0/0"]
#
#   security_group_id = aws_security_group.victim-protections.id
#   type              = "egress"
# }
#
# resource "aws_security_group_rule" "outbound-lockdown-sgvred" {
#   from_port = 0
#   to_port   = 0
#   protocol  = "-1"
#
#   # block internet except for jfrog, splunk, wazuh
#   prefix_list_ids = [data.aws_ec2_managed_prefix_list.outbound.id]
#   security_group_id = aws_security_group.attacker.id
#   type              = "egress"
# }
