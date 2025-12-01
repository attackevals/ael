locals {
  # Sets this local to either the string "platform" or false
  setup_required = coalesce(var.platform, false)

  # If var.platform is provided, check whether it is windows
  is_windows = true

  password_data = (var.get_password_data && local.is_windows) && can(file(var.ssh_private_key_path)) ? rsadecrypt(aws_instance.main.password_data, file(var.ssh_private_key_path)) : "not used"

  win_user_data = templatefile("${path.module}/files/winrm.tftpl", { AdminUser = var.admin_username, AdminPass = var.admin_password, RemoteHostname = var.hostname })
}
