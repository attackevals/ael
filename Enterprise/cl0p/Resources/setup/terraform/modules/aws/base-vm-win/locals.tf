locals {
  # Sets this local to either the string "platform" or false
  setup_required = coalesce(var.platform, false)

  # If var.platform is provided, check whether it is windows
  is_windows = true

  # If windows, outputs password data, otherwise "null" (skipped)
  #  password_data = local.is_windows && can(file(var.ssh_private_key_path)) ? rsadecrypt(aws_instance.main.password_data, file(var.ssh_private_key_path)) : null
  # If windows, outputs password data, otherwise "null" (skipped)
  password_data = (var.get_password_data && local.is_windows) && can(file(var.ssh_private_key_path)) ? rsadecrypt(aws_instance.main.password_data, file(var.ssh_private_key_path)) : "not used"

  #  user_data_params = templatefile(file("${path.module}/files/winrm.ps1"), {AdminUser = var.admin_username, AdminPass = var.admin_password} )
  #  local.is_windows ? "<powershell>\nParam($RemoteHostName = \"${var.hostname}\", $AdminUser = \"${var.admin_username}\", $AdminPass = \"${var.admin_password}\", $ComputerName = \"${var.hostname}\")" : null
  win_user_data = templatefile("${path.module}/files/winrm.tftpl", { AdminUser = var.admin_username, AdminPass = var.admin_password, RemoteHostname = var.hostname })

  # Locate & run the right setup scripts for the VM. Otherwise, user_data is set to "null" (skipped)
  #  setup_file = can(file("${path.module}/setup/${var.platform}/setup.sh")) ? file("${path.module}/setup/${var.platform}/setup.sh") : null
}
