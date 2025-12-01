#
# See `make help` for a list of all available commands.
#

V ?= "enterpriseround7"

PY_VERSION := python3
TIMESTAMP := $(shell date -u +"%Y%m%d_%H%M%S")
VPN_GEN_SCRIPT := ../utils/cert-gen/vpn-create-user.sh
TF_CA_CERT := cacert
TF_CA_KEY := cakey
TERRAFORM_PATH = ../terraform/range/
SSH_PRIV_KEY := ../terraform/range/aws1
TF_VPN_ENDPOINT := endpointid
VPN_ENDPOINT_ID := $(shell terraform output -raw $(TF_VPN_ENDPOINT))
TF_CA_CERT_FILE := cacert.pem
TF_CA_KEY_FILE := cacert.key
REDIRECTOR_SSH_ALT_PORT := 122
ANSIBLE_ROLES_DIR := $(HOME)/.ansible/roles/
ANSIBLE_VAULT_DIR := $(HOME)/.ansible/vault
ANSIBLE_VAULT_PASS_TOTP := $(ANSIBLE_VAULT_DIR)/.totp_vault_pass.txt
INVENTORY_FILE := inventory/$(V).yml
GOLDROAD_EXTRA_INSTANCE_INVENTORY_FILE := inventory/goldroad.yml

.DEFAULT_GOAL := help


.PHONY: help
help:
	@grep -E '^[a-zA-Z0-9_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

check-path:
	@if [ -z "$(V)" ]; then \
		echo "Error: V is not set. Use 'make deploy V=<value>'."; \
		exit 1; \
	fi
	@if [ ! -d "$(TERRAFORM_PATH)" ]; then \
		echo "Error: Directory $(TERRAFORM_PATH) does not exist."; \
		exit 1; \
	fi

check-vpn:
	@ansible -m ping dns-srv1 -i $(INVENTORY_FILE) -e "ansible_ssh_private_key_file=../terraform/range/aws1" || (echo "Error: Connection test failed. Check VPN"; exit 1)

get-list-of-hosts-with-ip: ## print out list of hosts and their ip addresses
	@ansible-inventory --list --yaml -i $(INVENTORY_FILE) | grep -E '^\s+ansible_host:\s' -B1 | grep -E '^\s+[^[:space:]]|ansible_host' | sed 'N;s/\n[[:space:]]*ansible_host: /: /' | sed 's/^[[:space:]]*//'

fix-key-perms: check-path ## fix aws ssh key permissions
	@echo Running with vendor terraform: $(TERRAFORM_PATH)
	$(info Change key permissions... $(SSH_PRIV_KEY))
	chmod 600 $(SSH_PRIV_KEY)

generate-keys: check-path ## create and push aws keys for s3 users
	@echo "Generating keys for vendor: $(V)"
	ansible-playbook -v -i inventory/$(V).yml -e "range_name=$(V)" playbooks/tasks/s3-keys.yml

install-ansible-modules: fix-key-perms ## install custom ansible roles locally
	$(info installing custom ansible roles locally)
	ansible-galaxy install -r requirements.yml
	mkdir -p $(ANSIBLE_ROLES_DIR)
	cp -a roles/evals_ad_domain $(ANSIBLE_ROLES_DIR)
	cp -a roles/evals_exchange $(ANSIBLE_ROLES_DIR)

configure-rds: install-ansible-modules check-vpn ## configure remote desktop services servers
	$(info configuring rds servers...)
	ansible-playbook -vvv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-rds.yml

create-domain: install-ansible-modules check-vpn ## create ad domains
	ansible-playbook -vv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-addc-kingslanding.yml
	ansible-playbook -vv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-addc-vale.yml
	ansible-playbook -vv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-addc-kingslanding-gpo.yml
	ansible-playbook -vv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-addc-vale-gpo.yml

create-domain-users: install-ansible-modules check-vpn ## ensure all users and groups exist
	ansible-playbook -vv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-addc-kingslanding.yml --tags "users"
	ansible-playbook -vv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-addc-vale.yml --tags "users"

setup-local-admin: check-vpn ## ensure windows boxes have local admin groups configured
	ansible-playbook -vv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/add-system-local-admin.yml

setup-local-mount: check-vpn ## ensure windows boxes have local admin groups configured
	ansible-playbook -vv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/single-host-share-mount.yml

cloudwatch-monitoring: check-vpn ## deploy monitoring config
	ansible-playbook -vv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/cloudwatch-monitoring.yml

configure-dns: check-path ## configure support unbound dns
	$(info configure DNS only)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-dns.yml

configure-file-servers: install-ansible-modules ## configure file servers
	$(info configure file servers)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-file-servers.yml

configure-linux-docker: install-ansible-modules ## configure docker endpoints
	$(info configure docker endpoints)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-linux-docker.yml

configure-airbyte: install-ansible-modules ## configure airbyte
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/airbyte.yml

configure-wekan: install-ansible-modules ## configure wekan
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/wekan.yml

configure-rustdesk: install-ansible-modules check-vpn ## deploy rustdesk and generate installer
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/rustdesk.yml

deploy-rustdesk-kali: install-ansible-modules check-vpn ## deploy rustdesk installer to kali and install client
	$(info requires rustdesk to be installed and installer generated...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/rustdesk-deploy.yml

enable-ping: install-ansible-modules check-vpn ## enable ping on windows hosts
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/enable--ping.yml

join-domain: install-ansible-modules check-vpn ## join all hosts to domain
	$(info joining hosts to domain...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/join-win-victim-a.yml
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/join-win-victim-b3.yml
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/join-linux-victim-a.yml
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/join-linux-victim-b3.yml

generate-certificates: install-ansible-modules check-vpn ## generate pki certs
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/generate-certs.yml

distribute-certificates: install-ansible-modules check-vpn ## deploy pki certs to endpoints
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/distribute-certs.yml

configure-kali: install-ansible-modules ## configure kali servers
	$(info configure kali servers...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V)" -e "ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-kali-all.yml

test-path: install-ansible-modules check-vpn ## test dns path
	ansible-playbook -vv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-addc-kingslanding.yml -t dns --check

preflights: install-ansible-modules ## configure preflights box (vendor validation)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-preflights.yml

fix-expired-logins: install-ansible-modules ## fix expired devadmin accounts
	$(info fixing expired logins...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/util-fix-expired-login-windows.yml

fix-expired-rds: install-ansible-modules ## fix expired devadmin accounts
	$(info fixing expired rds on windows servers...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/util-reset-rds-windows-servers.yml

enroll-ssm-systems: install-ansible-modules check-vpn ## enroll range systems into aws ssm mdm
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/enroll-ssm-systems.yml

stage-s3-files: check-vpn ## stage s3 files for detections and protections s3
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/stage_s3_detections_files.yml
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/stage_s3_protections_files.yml

decoy-files: check-vpn ## decoy files on file servers
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/decoy-files.yml

prepop-user-dirs: check-vpn ## prepopulate user directories on windows hosts
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/victim-populate.yml
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/victim-populate-tharlaw.yml

fix-winrar: check-vpn ## deploy winrar configuration so it works in explorer
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/winrar-config.yml

ping-win-hosts: check-vpn ## check that win hosts are responding to ansible
	$(info checking ansible connectivity to win hosts...)
	ansible windows -i $(INVENTORY_FILE) -m win_ping

fix-kali-timeout-and-browsers: check-vpn ## fix kali xfce timeout and browser and certificate issues
	$(info telling firefox install on kali to use trust store... )
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/kali-configure-browsers.yml
	$(info configure kali xfce timeout...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-kali-all.yml --tags "qol"

fix-rmm-version-check: check-vpn ## make sure tacticalrmm server does not complain about client version
	$(info tell rmm clients they are running the latest at v2.8.0...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/fix-tactical-rmm-client-version-check.yml

fix-exchange-timezone: install-ansible-modules ## deploy exchange servers a
	$(info fix exchange timezone setting...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/exchange2019.yml --tags "exchange_mailbox_enable_users"

deploy-kali: install-ansible-modules ## deploy kali servers
	$(info deploying kali servers...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-kali-all.yml

fix-kali-hostname: check-vpn ## Fix kali hostname
	$(info deploying kali servers...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-kali-all.yml --tags "fix, hostname"

fix-etckeeper-linux-victims: check-vpn ## fix etckeeper install on linux endpoints
	$(info fixing etckeeper...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/fix-etckeeper.yml

fix-rdp: check-vpn ## fix domain join state
	$(info fix domain join state...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/fix-rdp.yml

gpo: ## gpo deploy
	$(info deploying gpo...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-addc-kingslanding-gpo.yml
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/configure-addc-vale-gpo.yml
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/force-gp-update.yml

gpupdate: check-vpn ## update gpo only, no new gpo deployed
	$(info forcing gp update on windows victim hosts...)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/force-gp-update.yml

redirects: ## redirector deploy
	$(info deploying redirectors...)
	ansible-playbook -vv -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/redirect-setup.yml

jumpbox: ## provision jumpbox
	$(info configure jumpbox...)
	ansible-playbook -v -i $(INVENTORY_FILE)  -e "range_name=$(V)" playbooks/tasks/red-cfg-win.yml

gather-host-info: ## gather windows host info
	$(info gather windows host info)
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/gather-info.yml

cleanup: install-ansible-modules ## cleanupe hosts after provision
	ansible-playbook -v -i $(INVENTORY_FILE) -e "ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/cleanup-win.yml
	ansible-playbook -v -i $(INVENTORY_FILE) -e "ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/cleanup-linux.yml

fix-rmm-totp-key-import: install-ansible-modules ## totp token for rmm import to kali
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/rmm-import-totp-token.yml

fix-kali-chrome-browser: install-ansible-modules ## install kali browser
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/kali-configure-browsers.yml

fix-display-name-addc: install-ansible-modules ## update display name
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/fix-update-display-name-addc.yml

fix-exchange-admin-groups: check-vpn ## make sure domain admins are exchange admins
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/fix-exchange-admin-perms.yml

send-staged-emails: check-vpn ## send staged emails
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/send-staged-emails.yml

deploy1: install-ansible-modules check-vpn ## range configuration tasks set 1/3
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/_deploy.yml

deploy2: check-vpn ## range configuration tasks set 2/3
	@make fix-etckeeper-linux-victims
	@echo
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/postdeploy.yml

deploy3: check-vpn ## range configuration tasks set 3/3
	@make stage-s3-files
	@echo
	@make fix-kali-hostname
	@echo
	@make fix-kali-chrome-browser
	@echo
	@make fix-display-name-addc
	@echo
	@make fix-exchange-admin-groups
	@echo
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/write-gitlab-token-to-aws-secret.yml
	ansible-playbook -v -i $(INVENTORY_FILE) -e "range_name=$(V) ansible_ssh_private_key_file=../terraform/range/aws1" playbooks/tasks/cloudwatch-monitoring.yml

deploy: deploy1 deploy2 deploy3 ## ☀️ run all range configurations ☀️
