#
# See `make help` for a list of all available commands.
#

V_NAME := msr2
PY_VERSION := python3
TIMESTAMP := $(shell date -u +"%Y%m%d_%H%M%S")
VPN_GEN_SCRIPT := ../../utils/cert-gen/vpn-create-user.sh
TF_CA_CERT := cacert
TF_CA_KEY := cakey
TF_VPN_ENDPOINT := endpointid
VPN_ENDPOINT_ID := $(shell terraform output -raw $(TF_VPN_ENDPOINT))
TF_CA_CERT_FILE := cacert.pem
TF_CA_KEY_FILE := cacert.key
ANSIBLE_MAKEFILE := ../../ansible/Makefile
EMPTY_CRL_FILE := all_allowed_crl.pem
EMPTY_CRL_DER_FILE := all_allowed_crl.der
RANGE_TYPE := msr2
CRL_DIR := publish/crt-files
AWS_REGION := us-east-1

# if mac use gsed
PLATFORM := $(shell uname)
SED := sed
ifeq ($(PLATFORM),Darwin)
SED = gsed
endif

.DEFAULT_GOAL := help

.PHONY: help
help:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

write-ca-files-to-disk: ## write ca crt and ca key files to disk
	terraform output -raw $(TF_CA_CERT) > $(TF_CA_CERT_FILE)
	terraform output -raw $(TF_CA_KEY) > $(TF_CA_KEY_FILE)

create-vpn-profiles: write-ca-files-to-disk ## create VPN profiles for range
	$(VPN_GEN_SCRIPT) -e $(VPN_ENDPOINT_ID) -m -c $(TF_CA_CERT_FILE) -k $(TF_CA_KEY_FILE) -v $(V_NAME)
	git add $(CRL_DIR)

setup-ansible-keys:  ## install keys in the right place to run ansible
	cp -af ./aws1 ./aws1.pub ../../
	chmod 600 ../../aws1

setup-ansible: setup-ansible-keys  ## setup ansible keys and configuration

# generate empty crl, and convert from default der format to pem
gen-empty-crl: write-ca-files-to-disk ## generate updated empty crl
	$(info creating empty crl with no revocations...)
	ipsec pki --signcrl --cacert $(TF_CA_CERT_FILE) --cakey $(TF_CA_KEY_FILE) > $(EMPTY_CRL_DER_FILE)
	openssl crl -inform der -in $(EMPTY_CRL_DER_FILE) -out $(EMPTY_CRL_FILE)
