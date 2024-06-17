#
# See `make help` for a list of all available commands.
#

PY_VERSION := python3
TIMESTAMP := $(shell date -u +"%Y%m%d_%H%M%S")
VPN_GEN_SCRIPT := ../../utils/cert-gen/vpn-create-user.sh
TF_CA_CERT := cacert
TF_CA_KEY := cakey
TF_VPN_ENDPOINT := endpointid
VPN_ENDPOINT_ID := $(shell terraform output -raw $(TF_VPN_ENDPOINT))
TF_CA_CERT_FILE := cacert.pem
TF_CA_KEY_FILE := cacert.key
REDIRECTOR_SSH_ALT_PORT := 122

.DEFAULT_GOAL := help

.PHONY: help
help:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

install-ansible-modules: ## install custom ansible modules locally
	ansible-galaxy install -r requirements.yml

configure-range-from-scratch: install-ansible-modules  ## run full range provision
	$(info starting full provision of range)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/msr2-run-all.yml

dns-deploy:  ## update dns configuration for range
	$(info push out dns updates...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/range-dns.yml

shadow-copy-deploy: ## deploy shadow copy configs
	$(info sending shadow copies stuff...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/configure-shadow-copy.yml

gp-update-clients:  ## force all clients to update group policy
	$(info forcing group policy update)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/force-gp-update.yml

gp-updated-policy-deploy:  ## Update group policy files to DC, Does not force push GP to clients, use gp-update-clients target for that
	$(info sending group policy update)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/msr-a-dc-gpo.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/msr-b-dc-gpo.yml

ad-dns-deploy: ## deploy dns configuration to ad servers
	$(info configure ad dns servers...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/msr-a-dc-dns.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/msr-b-dc-dns.yml

ad-set-descriptions: ## set host descriptions on windows AD domains
	$(info update AD descriptions...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/msr-a-set-descriptions.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/msr-b-set-descriptions.yml

kali-deploy: ## deploy kali stuff
	$(info update kali server...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/red-cfg-kali.yml

redirectors-deploy: ## configure redirectors and deploy configuration
	$(info redirectors...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/redirector-1a.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/redirector-1b.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/redirector-2.yml

redirectors-alt-port-deploy: ## configure redirectors, but use alternative ssh port
	$(info redirectors...)
	ansible-playbook -v -i inventory/inventory.yml -e "ansible_port=$(REDIRECTOR_SSH_ALT_PORT)" playbooks/tasks/redirector-1a.yml
	ansible-playbook -v -i inventory/inventory.yml -e "ansible_port=$(REDIRECTOR_SSH_ALT_PORT)" playbooks/tasks/redirector-1b.yml
	ansible-playbook -v -i inventory/inventory.yml -e "ansible_port=$(REDIRECTOR_SSH_ALT_PORT)" playbooks/tasks/redirector-2.yml

kvm-deploy: ## configure kvm host on sub b
	$(info configuring kvm server...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/linux-join-domain.yml

post-cfg-exchange-deploy: ## exchange configuration and deploy
	$(info configuring exchange servers...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/msr-a-dc-post-exchange.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/msr-b-dc-post-exchange.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/msr-a-ex-post-exchange.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/msr-b-ex-post-exchange.yml

post-cleanup: ## cleanup after provisioning
	$(info cleaning up leftover cruft)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/cleanup-sql-files.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/cleanup-logs.yml

contractor-cfg-deploy: ## contractor system cfg
	$(info configure contractor workstation accounts)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/contractor-cfg.yml

disable-win-update: ## disable windows update
	$(info disabling windows update)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/disable-win-update.yml

install-office: ## install office and noise deps across the world
	$(info installing office and firefox for noise)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/install-office.yml

acl-change-deploy: ## deploy acl changes to hosts to allow domain admin access to user profile dirs
	$(info adding domain access to user profile dirs as access to simplify life)
	ansible-playbook -vv -i inventory/inventory.yml playbooks/tasks/victim-hosts-add-admin-access-perms-user.yml
