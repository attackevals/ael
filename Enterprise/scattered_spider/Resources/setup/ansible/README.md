---
tags: [infrastructure, enterprise2025, ansible, configuration]
label: Configuration Overview
order: 0
icon: "../assets/ansible.svg"
---

# Ansible Configuration Overview

The following is an overview of configurations (i.e., Ansible playbooks) applied to Enterprise 2025 infrastructure.

In `terraform/range/`, please run `make create-vpn-profiles` to generate OpenVPN profiles. These are stored as `.zip` files in `terraform/range/publish/`. Please use these to connect to your range and run host-based configurations.

<!--
1. Scenario-specific playbooks - Scattered Spider, Mustang Panda, and MITRE ATT&CK infrastructure
2. Common playbooks that are generally applied to multiple instances across the environment, categorized against function (e.g., based on the operating system)
 -->

---

## Table of Contents

- [Roles](#roles)
- [Playbooks](#playbooks)
- [Reusable Tasks](#reusable-tasks)

---

## Roles

### `authentik`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| Create directories | builtin/unknown | shortname, dc, domain_name, tld, post_deploy |
| Create Authentik docker-compose.yml | builtin/unknown | shortname, dc, domain_name, tld, post_deploy |
| Move authentik cert | builtin/unknown | shortname, dc, domain_name, tld, post_deploy |
| Move authentik key | builtin/unknown | shortname, dc, domain_name, tld, post_deploy |
| Start Authentik | [community.docker.docker_compose_v2](https://docs.ansible.com/ansible/latest/collections/community/docker/docker_compose_v2_module.html) | shortname, dc, domain_name, tld, post_deploy |
| Wait for Authentik to be ready | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | shortname, dc, domain_name, tld, post_deploy |
| debug | builtin/unknown | shortname, dc, domain_name, tld, post_deploy |
| Configure LDAP source | [ansible.builtin.include_tasks](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/include_tasks_module.html) | shortname, dc, domain_name, tld, post_deploy |
| query built-in brand | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | shortname, dc, domain_name, tld, post_deploy |
| change default brand | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | shortname, dc, domain_name, tld, post_deploy |
| query certificates | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | shortname, dc, domain_name, tld, post_deploy |
| create brand | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | shortname, dc, domain_name, tld, post_deploy |
| query brands | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | shortname, dc, domain_name, tld, post_deploy |
| update brand to default | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | shortname, dc, domain_name, tld, post_deploy |
| Finalize AWS authentication | [ansible.builtin.include_tasks](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/include_tasks_module.html) | shortname, dc, domain_name, tld, post_deploy |
| Configure AWS authentication | [ansible.builtin.include_tasks](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/include_tasks_module.html) | shortname, dc, domain_name, tld, post_deploy |

### `awscli`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| Install awscli windows | builtin/unknown |  |
| Install awscli linux | builtin/unknown |  |

### `certs`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| Create CA directory | [ansible.builtin.file](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/file_module.html) | cfssl_base_dir, certs_folder, ca_common_name, ca_country_name, ca_locality_name, ca_organization_name, ca_state_or_province_name, ca_cert_name, cert_domains, ca_domain, ca_crl_url, crl_enabled, crl_dir, crl_filename, crl_days, crl_nginx_port, crl_nginx_server_name |
| Create cfssl config directory | [ansible.builtin.file](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/file_module.html) | cfssl_base_dir, certs_folder, ca_common_name, ca_country_name, ca_locality_name, ca_organization_name, ca_state_or_province_name, ca_cert_name, cert_domains, ca_domain, ca_crl_url, crl_enabled, crl_dir, crl_filename, crl_days, crl_nginx_port, crl_nginx_server_name |
| Template CA configuration | [ansible.builtin.template](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/template_module.html) | cfssl_base_dir, certs_folder, ca_common_name, ca_country_name, ca_locality_name, ca_organization_name, ca_state_or_province_name, ca_cert_name, cert_domains, ca_domain, ca_crl_url, crl_enabled, crl_dir, crl_filename, crl_days, crl_nginx_port, crl_nginx_server_name |
| Template CA CSR configuration | [ansible.builtin.template](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/template_module.html) | cfssl_base_dir, certs_folder, ca_common_name, ca_country_name, ca_locality_name, ca_organization_name, ca_state_or_province_name, ca_cert_name, cert_domains, ca_domain, ca_crl_url, crl_enabled, crl_dir, crl_filename, crl_days, crl_nginx_port, crl_nginx_server_name |
| Generate CA certificate | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) | cfssl_base_dir, certs_folder, ca_common_name, ca_country_name, ca_locality_name, ca_organization_name, ca_state_or_province_name, ca_cert_name, cert_domains, ca_domain, ca_crl_url, crl_enabled, crl_dir, crl_filename, crl_days, crl_nginx_port, crl_nginx_server_name |
| Create site directories | builtin/unknown | cfssl_base_dir, certs_folder, ca_common_name, ca_country_name, ca_locality_name, ca_organization_name, ca_state_or_province_name, ca_cert_name, cert_domains, ca_domain, ca_crl_url, crl_enabled, crl_dir, crl_filename, crl_days, crl_nginx_port, crl_nginx_server_name |
| Template server CSR configurations | [ansible.builtin.template](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/template_module.html) | cfssl_base_dir, certs_folder, ca_common_name, ca_country_name, ca_locality_name, ca_organization_name, ca_state_or_province_name, ca_cert_name, cert_domains, ca_domain, ca_crl_url, crl_enabled, crl_dir, crl_filename, crl_days, crl_nginx_port, crl_nginx_server_name |
| Generate server certificates | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) | cfssl_base_dir, certs_folder, ca_common_name, ca_country_name, ca_locality_name, ca_organization_name, ca_state_or_province_name, ca_cert_name, cert_domains, ca_domain, ca_crl_url, crl_enabled, crl_dir, crl_filename, crl_days, crl_nginx_port, crl_nginx_server_name |

### `certs_crl`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| _No tasks_ | | |

### `certs_sync`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| Create certs dir | [ansible.builtin.file](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/file_module.html) |  |
| Pull from AWS | [ansible.builtin.command](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/command_module.html) |  |
| Push to AWS (complete two way sync) | [ansible.builtin.command](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/command_module.html) |  |

### `evals_ad_domain`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| CREATE Windows Domain {{ domain_info.dns_domain_name }} | [microsoft.ad.domain](https://docs.ansible.com/ansible/latest/collections/microsoft/ad/domain_module.html) | domain_info, domain_users, domain_groups, range_dns_server_ip |
| Delay-start WinRM after system has booted and works reliably | [ansible.windows.win_service](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_service_module.html) | domain_info, domain_users, domain_groups, range_dns_server_ip |
| Configure AD Server DNS | [ansible.windows.win_dns_client](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_dns_client_module.html) | domain_info, domain_users, domain_groups, range_dns_server_ip |
| set dns forwarder to use range dns | [ansible.windows.win_shell](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_shell_module.html) | domain_info, domain_users, domain_groups, range_dns_server_ip |
| CREATE groups for scenario | [microsoft.ad.group](https://docs.ansible.com/ansible/latest/collections/microsoft/ad/group_module.html) | domain_info, domain_users, domain_groups, range_dns_server_ip |
| CREATE Domain Users | [microsoft.ad.user](https://docs.ansible.com/ansible/latest/collections/microsoft/ad/user_module.html) | domain_info, domain_users, domain_groups, range_dns_server_ip |
| Debug user creation | [ansible.builtin.debug](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/debug_module.html) | domain_info, domain_users, domain_groups, range_dns_server_ip |
| Create domain dns records | [community.windows.win_dns_record](https://docs.ansible.com/ansible/latest/collections/community/windows/win_dns_record_module.html) | domain_info, domain_users, domain_groups, range_dns_server_ip |
| Enable network sharing | builtin/unknown | domain_info, domain_users, domain_groups, range_dns_server_ip |

### `evals_exchange`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| Check if Exchange is installed | builtin/unknown | ludus_install_directory, exchange_dotnet_install_path, vcredist2013_install_path, rewrite_module_path, ucma_runtime_path, ludus_exchange_iso_directory, ludus_exchange_iso_url, ludus_exchange2016_iso_url, ludus_exchange_domain, ludus_exchange_dc, ludus_exchange_host, ludus_exchange_domain_username, ludus_exchange_domain_password, ludus_os_version, exchange_prereqs_complete_file, send_connector_name, send_connector_smtpserver, send_connector_address_spaces, send_connector_source_transport_servers, s3_bucket, s3_filename |
| Download Exchange ISO for Windows Server 2016 | [ansible.builtin.include_tasks](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/include_tasks_module.html) | ludus_install_directory, exchange_dotnet_install_path, vcredist2013_install_path, rewrite_module_path, ucma_runtime_path, ludus_exchange_iso_directory, ludus_exchange_iso_url, ludus_exchange2016_iso_url, ludus_exchange_domain, ludus_exchange_dc, ludus_exchange_host, ludus_exchange_domain_username, ludus_exchange_domain_password, ludus_os_version, exchange_prereqs_complete_file, send_connector_name, send_connector_smtpserver, send_connector_address_spaces, send_connector_source_transport_servers, s3_bucket, s3_filename |
| Download Exchange ISO for Windows Server 2019 | [ansible.builtin.include_tasks](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/include_tasks_module.html) | ludus_install_directory, exchange_dotnet_install_path, vcredist2013_install_path, rewrite_module_path, ucma_runtime_path, ludus_exchange_iso_directory, ludus_exchange_iso_url, ludus_exchange2016_iso_url, ludus_exchange_domain, ludus_exchange_dc, ludus_exchange_host, ludus_exchange_domain_username, ludus_exchange_domain_password, ludus_os_version, exchange_prereqs_complete_file, send_connector_name, send_connector_smtpserver, send_connector_address_spaces, send_connector_source_transport_servers, s3_bucket, s3_filename |
| Ludus Exchange Server features to be installed | [ansible.builtin.include_tasks](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/include_tasks_module.html) | ludus_install_directory, exchange_dotnet_install_path, vcredist2013_install_path, rewrite_module_path, ucma_runtime_path, ludus_exchange_iso_directory, ludus_exchange_iso_url, ludus_exchange2016_iso_url, ludus_exchange_domain, ludus_exchange_dc, ludus_exchange_host, ludus_exchange_domain_username, ludus_exchange_domain_password, ludus_os_version, exchange_prereqs_complete_file, send_connector_name, send_connector_smtpserver, send_connector_address_spaces, send_connector_source_transport_servers, s3_bucket, s3_filename |
| Install Exchange Server for Windows Server 2016 | [ansible.builtin.include_tasks](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/include_tasks_module.html) | ludus_install_directory, exchange_dotnet_install_path, vcredist2013_install_path, rewrite_module_path, ucma_runtime_path, ludus_exchange_iso_directory, ludus_exchange_iso_url, ludus_exchange2016_iso_url, ludus_exchange_domain, ludus_exchange_dc, ludus_exchange_host, ludus_exchange_domain_username, ludus_exchange_domain_password, ludus_os_version, exchange_prereqs_complete_file, send_connector_name, send_connector_smtpserver, send_connector_address_spaces, send_connector_source_transport_servers, s3_bucket, s3_filename |
| Install Exchange Server for Windows Server 2016 | [ansible.builtin.include_tasks](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/include_tasks_module.html) | ludus_install_directory, exchange_dotnet_install_path, vcredist2013_install_path, rewrite_module_path, ucma_runtime_path, ludus_exchange_iso_directory, ludus_exchange_iso_url, ludus_exchange2016_iso_url, ludus_exchange_domain, ludus_exchange_dc, ludus_exchange_host, ludus_exchange_domain_username, ludus_exchange_domain_password, ludus_os_version, exchange_prereqs_complete_file, send_connector_name, send_connector_smtpserver, send_connector_address_spaces, send_connector_source_transport_servers, s3_bucket, s3_filename |
| Install Exchange Server for Windows Server 2019 | [ansible.builtin.include_tasks](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/include_tasks_module.html) | ludus_install_directory, exchange_dotnet_install_path, vcredist2013_install_path, rewrite_module_path, ucma_runtime_path, ludus_exchange_iso_directory, ludus_exchange_iso_url, ludus_exchange2016_iso_url, ludus_exchange_domain, ludus_exchange_dc, ludus_exchange_host, ludus_exchange_domain_username, ludus_exchange_domain_password, ludus_os_version, exchange_prereqs_complete_file, send_connector_name, send_connector_smtpserver, send_connector_address_spaces, send_connector_source_transport_servers, s3_bucket, s3_filename |
| Run the send connector task | [ansible.builtin.include_tasks](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/include_tasks_module.html) | ludus_install_directory, exchange_dotnet_install_path, vcredist2013_install_path, rewrite_module_path, ucma_runtime_path, ludus_exchange_iso_directory, ludus_exchange_iso_url, ludus_exchange2016_iso_url, ludus_exchange_domain, ludus_exchange_dc, ludus_exchange_host, ludus_exchange_domain_username, ludus_exchange_domain_password, ludus_os_version, exchange_prereqs_complete_file, send_connector_name, send_connector_smtpserver, send_connector_address_spaces, send_connector_source_transport_servers, s3_bucket, s3_filename |

### `ftp_setup`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| Create FTP users | [ansible.builtin.user](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/user_module.html) | ftp_srv_dir, ftp_user, ftp_users, ftp_passive_min_port, ftp_passive_max_port, use_ftp_redirector, ftp_redirector_ip |
| Configure VSFTPD | [ansible.builtin.template](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/template_module.html) | ftp_srv_dir, ftp_user, ftp_users, ftp_passive_min_port, ftp_passive_max_port, use_ftp_redirector, ftp_redirector_ip |
| Create FTP directories | [ansible.builtin.file](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/file_module.html) | ftp_srv_dir, ftp_user, ftp_users, ftp_passive_min_port, ftp_passive_max_port, use_ftp_redirector, ftp_redirector_ip |
| Enable and Restart VSFTP service | [ansible.builtin.systemd](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/systemd_module.html) | ftp_srv_dir, ftp_user, ftp_users, ftp_passive_min_port, ftp_passive_max_port, use_ftp_redirector, ftp_redirector_ip |

### `gitlab-authentik`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| query authentik authorization flow UUID | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| set auth flow uuid | builtin/unknown | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| query authentik authentication flow UUID | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| set authentication flow uuid | builtin/unknown | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| query authentik invalidation flow UUID | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| set invalidation flow uuid | builtin/unknown | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| Create OIDC Provider for GitLab | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| query oidc provider | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| query scope mappings | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| query certificates | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| Update GitLab Provider in Authentik | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| Create GitLab Application in Authentik | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| set secret | builtin/unknown | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| set oidc template content | builtin/unknown | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |
| render the template and append to the gitlab rb config file | [ansible.builtin.blockinfile](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/blockinfile_module.html) | app_name, app_slug, domain_info, domain_name, authentik_admin_token, authentik_port, authentik_domain, gitlab_domain, gitlab_port, oidc_client_id, gitlab_authentik_label_text, gitlab_config_file |

### `guacamole`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| query authentik authorization flow UUID | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) |  |
| set auth flow uuid | builtin/unknown |  |
| query authentik authentication flow UUID | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) |  |
| set authentication flow uuid | builtin/unknown |  |
| query authentik invalidation flow UUID | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) |  |
| set invalidation flow uuid | builtin/unknown |  |
| Create OIDC Provider for Guacamole | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) |  |
| query oidc provider | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) |  |
| query scope mappings | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) |  |
| query certificates | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) |  |
| Update Guacamole Provider in Authentik | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) |  |
| Create Guacamole Application in Authentik | [ansible.builtin.uri](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/uri_module.html) |  |
| set secret | builtin/unknown |  |
| Create Guacamole directories | builtin/unknown |  |
| Create Guacamole docker-compose.yml | builtin/unknown |  |
| Upload prepare script | [ansible.builtin.copy](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/copy_module.html) |  |
| Run prepare script | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |  |
| Create nginx template | builtin/unknown |  |
| Move guac key | [ansible.builtin.copy](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/copy_module.html) |  |
| Move guac certs | builtin/unknown |  |
| Move root cert | builtin/unknown |  |
| Start Guacamole | [community.docker.docker_compose_v2](https://docs.ansible.com/ansible/latest/collections/community/docker/docker_compose_v2_module.html) |  |
| Add trusted ca cert to guac | [community.docker.docker_container_exec](https://docs.ansible.com/ansible/latest/collections/community/docker/docker_container_exec_module.html) |  |

### `s3_user_key_management`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| Build vendor loop list | builtin/unknown |  |
| Include create_user_output for each defined vendor from Terraform | builtin/unknown |  |
| Check if key staging directory exists | [ansible.builtin.stat](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/stat_module.html) |  |
| Remove key staging directory | [ansible.builtin.file](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/file_module.html) |  |

### `s3file_staging`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| Stage S3 directories from source to destination in bulk | builtin/unknown | source_bucket, source_path_prefix, source_path_suffix, dest_cloud_bucket, CrossAccountAccess_arn, Cloud_Protections_arn, cloud_detections_arn, target_account, source_cloud_directories, protections_cloud_files |

### `ssm_hybrid_activation`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| Perform SSM hybrid activation | builtin/unknown | ssm_activation_code, ssm_activation_id, ssm_region, ssm_agent_download_url |

### `windows_remote_desktop_session_host`

| Task Name | Module | Variables |
|-----------|--------|-----------|
| Install RDS Session Host role | [ansible.windows.win_feature](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_feature_module.html) | licensing_server_ad_group |
| Install RDS Licensing Server role | [ansible.windows.win_feature](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_feature_module.html) | licensing_server_ad_group |
| Reboot if required | [ansible.windows.win_reboot](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_reboot_module.html) | licensing_server_ad_group |
| Set RDS licensing mode (Per User) | builtin/unknown | licensing_server_ad_group |

## Playbooks

## Reusable Tasks

### `add-local-admin-groups.yml`

**Required Variables:** `domain_shortname:, admin_group:`

| Task Name | Module |
|-----------|--------|
| Add domain admins to local admin | [ansible.windows.win_group_membership](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_group_membership_module.html) |

### `configure-addc-gpo.yml`

**Required Variables:** `deploy_dir:, gpo_src_dir:, gpo_dst_dir:, evals_range:, domain_shortname:, domain_tld:`

| Task Name | Module |
|-----------|--------|
| Create directory for gpo files: {{ deploy_dir }} | [ansible.windows.win_file](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_file_module.html) |
| Copy gpo folder to DC: {{ gpo_src_dir }} | [ansible.windows.win_copy](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_copy_module.html) |
| install nuget provider | [ansible.windows.win_shell](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_shell_module.html) |
| Install GPOTools module | builtin/unknown |
| Restore GPO policies in: {{ gpo_dst_dir }} | builtin/unknown |
| Link GPO policies | builtin/unknown |
| Cleanup | [ansible.windows.win_file](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_file_module.html) |

### `configure-and-join-domain-linux.yml`

**Required Variables:** `linux_hostname:, domain_admin_user:, domain_admin_pass:, domain_dns_name:, tuned_profile:`

| Task Name | Module |
|-----------|--------|
| Install packages required for AD and some useful tools | [ansible.builtin.apt](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/apt_module.html) |
| restart sssd | builtin/unknown |
| disable ufw | builtin/unknown |
| Forcibly remove resolv.conf | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| disable systemd resolved | builtin/unknown |
| Copy resolv dns configuration | builtin/unknown |
| Set hostname | [ansible.builtin.hostname](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/hostname_module.html) |
| Preserve hostname after reboot | builtin/unknown |
| Disable Kerberos reverse dns during domain join | [ansible.builtin.copy](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/copy_module.html) |
| Join the AD domain | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| See the Results from Joining | builtin/unknown |
| Check the AD join result | builtin/unknown |
| Validate the AD join | [ansible.builtin.command](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/command_module.html) |
| Check the validation result | builtin/unknown |
| install sudo configuration | [ansible.builtin.copy](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/copy_module.html) |
| allow password auth for domain users over ssh | [ansible.builtin.copy](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/copy_module.html) |
| Enable home dir creation on first login | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| See the result from configuring mkhomedir | builtin/unknown |
| Permit domain logins | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| Allow user Login without FQDN | builtin/unknown |
| add gpo disable login configuration to sssd config | builtin/unknown |
| check tuned profile | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| set tuned profile | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| reboot host | [ansible.builtin.reboot](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/reboot_module.html) |

### `configure-and-join-domain-win-all.yml`

**Required Variables:** `ad_ip_addr:, domain_dns_name:, domain_shortname:, domain_admin_user:, domain_admin_pass:`

| Task Name | Module |
|-----------|--------|
| Configure DNS to use AD Server | [ansible.windows.win_dns_client](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_dns_client_module.html) |
| Join AD Domain | [microsoft.ad.membership](https://docs.ansible.com/ansible/latest/collections/microsoft/ad/membership_module.html) |
| Add domain admins to local admin | [ansible.windows.win_group_membership](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_group_membership_module.html) |
| Add admins and users to RDP group | [ansible.windows.win_group_membership](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_group_membership_module.html) |
| Enable network sharing | builtin/unknown |

### `configure-choco-mirror.yml`

**Required Variables:** `support_vars.choco_mirror_url:`

| Task Name | Module |
|-----------|--------|
| Install chocolatey | [chocolatey.chocolatey.win_chocolatey](https://docs.ansible.com/ansible/latest/collections/chocolatey/chocolatey/win_chocolatey_module.html) |
| Remove the default public source | [chocolatey.chocolatey.win_chocolatey_source](https://docs.ansible.com/ansible/latest/collections/chocolatey/chocolatey/win_chocolatey_source_module.html) |
| Add new internal source | [chocolatey.chocolatey.win_chocolatey_source](https://docs.ansible.com/ansible/latest/collections/chocolatey/chocolatey/win_chocolatey_source_module.html) |

### `configure-fleet-activity-exporter.yml`

**Required Variables:** `scripts_dir:, log_dir:`

| Task Name | Module |
|-----------|--------|
| Create scripts directory | [ansible.builtin.file](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/file_module.html) |
| Create log directory | [ansible.builtin.file](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/file_module.html) |
| Copy Fleet activity exporter script | [ansible.builtin.copy](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/copy_module.html) |
| Copy systemd service file | [ansible.builtin.copy](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/copy_module.html) |
| Copy systemd timer file | [ansible.builtin.copy](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/copy_module.html) |
| Reload systemd daemon | [ansible.builtin.systemd](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/systemd_module.html) |
| Enable and start Fleet activity exporter timer | [ansible.builtin.systemd](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/systemd_module.html) |
| Test the exporter (run once manually) | [ansible.builtin.command](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/command_module.html) |
| Display test results | [ansible.builtin.debug](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/debug_module.html) |
| Configure logrotate for Fleet activity logs | [ansible.builtin.copy](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/copy_module.html) |

### `csv-to-s3.yml`

**Required Variables:** `item.username:, item.password:, red_dev.s3.red_bucket:, evals_cycle:`

| Task Name | Module |
|-----------|--------|
| Populate j2 template csv for s3 (github) | builtin/unknown |
| Upload csv to s3 | [amazon.aws.aws_s3](https://docs.ansible.com/ansible/latest/collections/amazon/aws/aws_s3_module.html) |

### `firefox-win-client-setup.yml`

**Required Variables:** `None`

| Task Name | Module |
|-----------|--------|
| install firefox | builtin/unknown |
| Disable automatic firefox updates | [ansible.windows.win_regedit](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_regedit_module.html) |
| Disable firefox default browser check | [ansible.windows.win_regedit](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_regedit_module.html) |
| Disable firefox first run | [ansible.windows.win_regedit](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_regedit_module.html) |

### `fix-domain-user-name-display.yml`

**Required Variables:** `domain_users:, domain_info.dns_domain_name:`

| Task Name | Module |
|-----------|--------|
| Update Domain Users to display first and last name | [microsoft.ad.user](https://docs.ansible.com/ansible/latest/collections/microsoft/ad/user_module.html) |

### `fix-rdp.yml`

**Required Variables:** `ad_domain:, ad_user:, ad_pass:`

| Task Name | Module |
|-----------|--------|
| Repairing Domain Trust to {{ ad_domain }} | [ansible.builtin.win_shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/win_shell_module.html) |
| Delete PSCrendential Automations | [ansible.builtin.win_shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/win_shell_module.html) |

### `fix-win-expired-logins.yml`

**Required Variables:** `None`

| Task Name | Module |
|-----------|--------|
| Ensure devadmin does not expire | [ansible.windows.win_user](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_user_module.html) |

### `kali-firefox-trust-ca.yml`

**Required Variables:** `cert_name:, cert_path:, ca_cert_name:`

| Task Name | Module |
|-----------|--------|
| create firefox policy directory | [ansible.builtin.file](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/file_module.html) |
| copy firefox policy file | [ansible.builtin.template](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/template_module.html) |
| install libnss3-tools for certutil | [ansible.builtin.apt](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/apt_module.html) |
| add certificate to chrome nss database for all users | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |

### `kali-xfce-disable-screenlock.yml`

**Required Variables:** `None`

| Task Name | Module |
|-----------|--------|
| Get list of home directories | [ansible.builtin.find](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/find_module.html) |
| Create XFCE config directories | [ansible.builtin.file](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/file_module.html) |
| Configure power manager settings | [ansible.builtin.copy](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/copy_module.html) |
| Configure screensaver settings | [ansible.builtin.copy](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/copy_module.html) |

### `prepop-profile-directories.yml`

**Required Variables:** `prepop_domain_users:, office_dest_prefix:, office_cfg_dest_dir:, office_cfg_src_file:, office_cfg_filename:`

| Task Name | Module |
|-----------|--------|
| Create profile directories if they do not exist | [community.windows.win_user_profile](https://docs.ansible.com/ansible/latest/collections/community/windows/win_user_profile_module.html) |
| Create directory for office profile dir | [ansible.windows.win_file](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_file_module.html) |
| Upload office configuration to profile | [ansible.windows.win_copy](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_copy_module.html) |

### `redirect-setup.yml`

**Required Variables:** `None`

| Task Name | Module |
|-----------|--------|
| Copy resolv dns configuration | builtin/unknown |
| Install redirector deps and useful tools | [ansible.builtin.apt](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/apt_module.html) |
| Get primary interface MAC | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| Get primary interface IPs | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| Get all interface MACs | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| Initialize local IPs list | [ansible.builtin.set_fact](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/set_fact_module.html) |
| Add primary IPs to local IPs list | [ansible.builtin.set_fact](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/set_fact_module.html) |
| Process secondary interfaces for local IPs | builtin/unknown |
| Configure redirect script for host | builtin/unknown |
| Copy disable script | builtin/unknown |
| Copy print script | builtin/unknown |
| Copy sysctl conf | builtin/unknown |
| Apply sysctl conf | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| Configure netplan for additional IPs | builtin/unknown |
| Apply netplan rules | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| Flush iptables rules | [ansible.builtin.command](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/command_module.html) |
| Process forwarding rules | [ansible.builtin.command](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/command_module.html) |
| Save and persist forwarding rules | [ansible.builtin.shell](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/shell_module.html) |
| reboot hosts | [ansible.builtin.reboot](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/reboot_module.html) |

### `run-windows-reg-file.yml`

**Required Variables:** `winrar_dest_dir:, winrar_reg_file:, winrar_dest_file:`

| Task Name | Module |
|-----------|--------|
| make sure dest dir exists | [ansible.windows.win_file](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_file_module.html) |
| copy reg file to host | [ansible.windows.win_copy](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_copy_module.html) |
| run winrar reg file on windows host | [ansible.windows.win_shell](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_shell_module.html) |
| cleanup and remove reg file from host | [ansible.windows.win_file](https://docs.ansible.com/ansible/latest/collections/ansible/windows/win_file_module.html) |

## Notice

© 2025 MITRE. Approved for public release. Document number 25-2969.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
file except in compliance with the License. You may obtain a copy of the License at

<http://www.apache.org/licenses/LICENSE-2.0>

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied. See the License for the specific language governing
permissions and limitations under the License.
