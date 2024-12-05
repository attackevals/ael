#
# See `make help` for a list of all available commands.
#

BLUE_RANGE := enterpriseround6
PY_VERSION := python3
TIMESTAMP := $(shell date -u +"%Y%m%d_%H%M%S")
VPN_GEN_SCRIPT := ../utils/cert-gen/vpn-create-user.sh
TF_CA_CERT := cacert
TF_CA_KEY := cakey
SSH_PRIV_KEY := ../aws1
TF_VPN_ENDPOINT := endpointid
VPN_ENDPOINT_ID := $(shell terraform output -raw $(TF_VPN_ENDPOINT))
TF_CA_CERT_FILE := cacert.pem
TF_CA_KEY_FILE := cacert.key
ANSIBLE_ROLES_DIR := ~/.ansible/roles/

.DEFAULT_GOAL := help

.PHONY: help
help:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

fix-key-perms: ## fix aws ssh key permissions
	$(info key permissions...)
	chmod 600 $(SSH_PRIV_KEY)

install-ansible-modules: fix-key-perms ## install custom ansible roles locally
	$(info installing custom ansible roles locally)
	ansible-galaxy install -r requirements.yml
	mkdir -p $(ANSIBLE_ROLES_DIR)
	cp -a roles/evals_ad_domain $(ANSIBLE_ROLES_DIR)
	cp -a roles/evals_macos $(ANSIBLE_ROLES_DIR)
	cp -a roles/evals_exchange $(ANSIBLE_ROLES_DIR)

configure-range-from-scratch: install-ansible-modules  ## run full range provision
	$(info starting full provision of range)
	ansible-playbook -vv -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/.deploy.yml

ping-win-hosts: ## check that win hosts are responding to ansible
	$(info checking ansible connectivity to win hosts...)
	ansible win_hosts -i inventory/inventory.yml -m win_ping

configure-file-server: install-ansible-modules ## file servers
	$(info deploying file servers...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/file-server.yml

deploy-exchange: install-ansible-modules ## deploy exchange servers a
	$(info deploying exchange servers...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/exchange2019.yml

deploy-kali: install-ansible-modules ## deploy kali servers
	$(info deploying kali servers...)
	ansible-playbook -v -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/configure-kali-all.yml

deploy-kali-updated-build-only: install-ansible-modules ## deploy latest vendor build only, does not run full playbook
	$(info deploying kali servers...)
	ansible-playbook -v -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/configure-kali-all.yml --tags "download,fix"

choco-mirror: install-ansible-modules ## configure choco mirror
	$(info configuring choco mirror sonatype nexus...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/choco-mirror-setup.yml

install-win-all: install-ansible-modules ## install all windows hosts software
	$(info installing software on all windows hosts...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/install-packages-win-all.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/install-packages-win-dev.yml

join-domain: install-ansible-modules ## join all hosts to domain
	$(info deploying exchange servers...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/join-win-victim-a.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/join-win-victim-b.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/join-win-victim-protections.yml

deploy-exchange-b: install-ansible-modules ## deploy exchange servers b
	$(info deploying exchange servers...)
	ansible-playbook -vvv -i inventory/inventory.yml playbooks/tasks/exchange2019-b.yml

deploy-exchange-p: install-ansible-modules ## deploy exchange servers protections
	$(info deploying exchange servers...)
	ansible-playbook -vvv -i inventory/inventory.yml playbooks/tasks/exchange2019-protections.yml

cwagent-rollout: install-ansible-modules ## Install and Configure CWAgent to all Hosts
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/cwagent-linux-rollout.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/cwagent-win-rollout.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/cwagent-mac-rollout.yml

configure-linux-hosts: install-ansible-modules ## configure linux and join domain
	$(info join linux...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/join-linux-victim-protections.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/join-linux-victim-b.yml

configure-mac-hogshead: install-ansible-modules ## hogshead
	$(info hogshead...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/configure-mac-hogshead.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/join-mac-victim-a.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/fix-mac-hogshead.yml

configure-mac-itzy: install-ansible-modules ## itzy
	$(info itzy...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/configure-mac-itzy.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/join-mac-victim-protections.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/fix-mac-itzy.yml

configure-spitfire: install-ansible-modules ## spitfire
	$(info spitfire magic...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/red-cfg-win.yml

fix-mac-issues: install-ansible-modules ## fix mac dns and sudo group membership, assumes ad already joined
	$(info fix mac issues...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/fix-mac-hogshead.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/fix-mac-itzy.yml

fix-win-rdp: install-ansible-modules ## fix RDP connections
	$(info fix win RDP issues...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/fix-rdp-encryptpotter.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/fix-rdp-decryptmalfoy.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/fix-rdp-sonicbeats.yml

populate-profile-dirs: install-ansible-modules ## populate profile dirs
	$(info populate profile directories)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/victim-a-populate.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/victim-b-populate.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/victim-protections-populate.yml

configure-b-file-srv: install-ansible-modules ## configure b file server only
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/.deploy.yml --limit "addc_srv1_b"

sw-install: ## install software packages only
	$(info installing software packages only)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/install-packages-mac.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/install-packages-win-all.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/install-packages-mac.yml
	ansible-playbook -v -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/install-packages-win-all.yml
	ansible-playbook -v -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/install-packages-win-dev.yml
	ansible-playbook -v -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/install-packages-win-b.yml

gpo: ## gpo deploy
	$(info deploying gpo...)
	ansible-playbook -v -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/configure-addc-encryptpotter-gpo.yml
	ansible-playbook -v -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/configure-addc-decryptmalfoy-gpo.yml
	ansible-playbook -v -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/configure-addc-sonicbeats-gpo.yml

redirects: ## redirector deploy
	$(info deploying redirectors...)
	ansible-playbook -vv -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/redirect-setup-red1.yml
	ansible-playbook -vv -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/redirect-setup-red2.yml
	ansible-playbook -vv -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/redirect-setup-red3.yml
	ansible-playbook -vv -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/redirect-setup-red4.yml

linux-updates: ## disable linux updates
	$(info disable updates on linux...)
	ansible-playbook -v -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/linux-disable-updates.yml

jumpbox: ## provision jumpbox
	$(info configure jumpbox...)
	ansible-playbook -v -i inventory/inventory.yml  -e "range_name=$(BLUE_RANGE)" playbooks/tasks/red-cfg-win.yml

enable-ping: install-ansible-modules ## enable ping on windows hosts
	ansible-playbook -v -i inventory/inventory.yml  playbooks/tasks/allow-ping.yml

mac-sw-install: install-ansible-modules ## mac sw install
	$(info mac software install...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/.mac-configure.yml

force-group-policy-update: install-ansible-modules ## force gpo update
	$(info force group policy update...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/force-gp-update.yml

configure-dns: ## configure support unbound dns
	$(info configure DNS only)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/support-unbound-dns.yml

test-os: ## gather windows host info
	$(info gather windows host info)
	ansible-playbook -v -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/test-os.yml
	ansible-playbook -v -i inventory/inventory.yml -e "range_name=$(BLUE_RANGE)" playbooks/tasks/test-os-linux-mac.yml

configure-win-netbios: install-ansible-modules ## configure NetBios Firewall and Registry Key
	$(info Configure NetBios for Protections)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/configure-win-netbios.yml

configure-quirrell: install-ansible-modules ## Configure Qurriell
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/configure-addc-decryptmalfoy.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/add-admin-quirrell.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/victim-b-quirrell.yml

configure-diagonalley: install-ansible-modules ## install sql services on diagonalley
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/victim-a-diagonalley.yml

cleanup: install-ansible-modules ## cleanupe hosts after provision
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/cleanup-win.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/cleanup-linux.yml

fresh-range-setup: install-ansible-modules deploy-kali test-os  ## run after setting up new vendor range
	$(info post setup target to document ansible release)

fresh-range-setup-dev: install-ansible-modules deploy-kali-dev test-os  ## DEV RANGES ONLY: run after setting up new vendor range
	$(info post setup target to document ansible release)

fix-expired-logins: install-ansible-modules ## fix expired devadmin and windesk accounts
	$(info fixing expired logins...)
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/fix-expired-login-win-servers.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/fix-expired-login-win-desktops.yml
	ansible-playbook -v -i inventory/inventory.yml playbooks/tasks/fix-expired-login-win-remote-workers.yml
