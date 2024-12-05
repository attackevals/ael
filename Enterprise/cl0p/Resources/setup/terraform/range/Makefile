#
# See `make help` for a list of all available commands.
#

# ------------------------------
ROUND := er6
# ------------------------------
V_NAME := enterpriseround6
RANGE_TYPE := $(ROUND)
PY_VERSION := python3
TIMESTAMP := $(shell date -u +"%Y%m%d_%H%M%S")
VPN_GEN_SCRIPT := ../../utils/cert-gen/vpn-create-user.sh
VPN_ZIP_PASS := att&ckevals
VPN_ZIP_NAME := $(V_NAME)-vpn-profiles.zip
VPN_ZIP_PATH := s3://attackevals-participant-resources/evaluation/$(ROUND)/vpn-profiles/$(V_NAME)/$(VPN_ZIP_NAME)
VPN_URL_EXP_SEC := 518400
VPN_URL_EXP_DAY := $(shell expr $(VPN_URL_EXP_SEC) / 86400 | bc)
TF_CA_CERT := cacert
TF_CA_KEY := cakey
TF_VPN_ENDPOINT := endpointid
VPN_ENDPOINT_ID := $(shell terraform output -raw $(TF_VPN_ENDPOINT))
TF_CA_CERT_FILE := cacert.pem
TF_CA_KEY_FILE := cacert.key
ANSIBLE_MAKEFILE := ../../ansible/Makefile
EMPTY_CRL_FILE := all_allowed_crl.pem
VENDOR_DISABLED_CRL_FILE := vendor_disabled_crl.pem
EMPTY_CRL_DER_FILE := all_allowed_crl.der
VENDOR_DISABLED_CRL_DER_FILE := vendor_disabled_crl.der
LIVE_CRL_TMP_FILE := live_crl.pem
CRL_DIR := publish/crt-files
VENDOR_1 := vendor01-client.crt
VENDOR_2 := vendor02-client.crt
VENDOR_3 := vendor03-client.crt
VENDOR_4 := vendor04-client.crt
VENDOR_5 := vendor05-client.crt
VENDOR_6 := vendor06-client.crt
VENDOR_7 := vendor07-client.crt
VENDOR_8 := vendor08-client.crt
VENDOR_9 := vendor09-client.crt
VENDOR_10 := vendor10-client.crt
AWS_REGION := us-east-1
TRAFFIC_MIRROR_TF_FILE_OFF := traffic-mirroring.vendor-request
TRAFFIC_MIRROR_TF_FILE_ON := traffic-mirroring.tf
S3_BUCKET_CRL := evals-crl-storage
S3_CRL_PARENT_DIR := $(ROUND)
ALL_DISABLED_CRL_FILE := all_disabled_crl.pem
CRL_TMP_FILE := tmp_crl.pem

# scheduled jobs related
# to translate this date gobblygook on the line below, get the date of the next saturday at least 3 days in the future
# done this way so we can run this on a Friday, and get the saturday of the next week (not the next day)

0500PM := $(shell echo "T17:00:00")
0500AM := $(shell echo "T05:00:00")
SETUP_FRIDAY_DATE := $(shell TZ='America/New_York' date -Idate -v+Fri)
SETUP_FRIDAY_TIMESTAMP := $(SETUP_FRIDAY_DATE)$(0500PM)
OVR_MONDAY_DATE := $(shell TZ='America/New_York' date -Idate -v+Mon)
OVR_MONDAY_TIMESTAMP := $(OVR_MONDAY_DATE)$(0500AM)

NEXT_SATURDAY_TIMESTAMP := $(shell TZ='America/New_York' date -Iseconds -v+3d -v+Sat)
NEXT_FRIDAY_DATE := $(shell TZ='America/New_York' date -Idate -v+3d -v+Fri)
NEXT_FRIDAY_TIME := $(shell echo "T01:30:30-04:00")
NEXT_FRIDAY_TIMESTAMP := $(NEXT_FRIDAY_DATE)$(NEXT_FRIDAY_TIME)

SCHEDULE_PATH := automations
SCHEDULE_SETUP_BACKUP := $(SCHEDULE_PATH)/setup-vendor-bk-1.json
SCHEDULE_OVR_PRE_BACKUP := $(SCHEDULE_PATH)/ovr-vendor-bk-1.json
SCHEDULE_OVR_VPN_OFF := $(SCHEDULE_PATH)/ovr-vendor-vpn-off.json
SCHEDULE_EXEC_MORNING_BACKUP := $(SCHEDULE_PATH)/exec-vendor-bk-1.json
SCHEDULE_EXEC_AFTERNOON_BACKUP := $(SCHEDULE_PATH)/exec-vendor-bk-2.json
SCHEDULE_EXEC_VPN_ON := $(SCHEDULE_PATH)/exec-vendor-vpn-on.json
SCHEDULE_EXEC_VPN_OFF := $(SCHEDULE_PATH)/exec-vendor-vpn-off.json

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

# print list of connected vpn users
view-active-vpn-users: write-ca-files-to-disk ## list users connected to VPN and their status
	$(info listing users connected to VPN and connection status...)
	# disable pagination in aws cli to get all users in one go
	aws ec2 describe-client-vpn-connections --no-paginate --client-vpn-endpoint-id $(VPN_ENDPOINT_ID) --region $(AWS_REGION) --query "Connections[].[ClientVpnEndpointId,Timestamp,CommonName,Status.Code]"

disable-all-vpn-access: gen-revoke-all-crl ## upload all cert revocation to aws vpn endpoint
	$(info import disable-all crl to aws...)
	$(info NOTE: No users will be able to connect to VPN after deploying...)
	@echo "###### START OF LOCAL FILE"
	openssl crl -in $(ALL_DISABLED_CRL_FILE) -text -noout|grep Serial
	@echo "###### END OF LOCAL FILE"
	@echo "###### ###### ###### ###### ###### ######"
	@echo "###### ABOVE SHOULD MATCH OUTPUT OF view-active-crl make target"
	@echo "###### ###### ###### ###### ###### ######"
	aws ec2 import-client-vpn-client-certificate-revocation-list --certificate-revocation-list file://./$(ALL_DISABLED_CRL_FILE) --client-vpn-endpoint-id $(VPN_ENDPOINT_ID) --region $(AWS_REGION)

.PHONY: gen-revoke-all-crl
gen-revoke-all-crl: write-ca-files-to-disk ## create CRL that will disable all VPN certificates
	$(info creating crl with ALL connections revoked...)
	# create a starting point, then iterate over all files in dir and add to crl to revoke
	ipsec pki --signcrl --cacert $(TF_CA_CERT_FILE) --cakey $(TF_CA_KEY_FILE) --outform pem > $(ALL_DISABLED_CRL_FILE)
	for cert in $(wildcard $(CRL_DIR)/*.crt); do \
  	  ipsec pki --signcrl --cacert $(TF_CA_CERT_FILE) --cakey $(TF_CA_KEY_FILE) --lastcrl $(ALL_DISABLED_CRL_FILE) --cert $$cert --outform pem > $(CRL_TMP_FILE); \
  	  mv $(CRL_TMP_FILE)  $(ALL_DISABLED_CRL_FILE); \
  	done

write-ca-files-to-disk: ## write ca crt and ca key files to disk
	terraform output -raw $(TF_CA_CERT) > $(TF_CA_CERT_FILE)
	terraform output -raw $(TF_CA_KEY) > $(TF_CA_KEY_FILE)

create-vpn-profiles: write-ca-files-to-disk ## create VPN profiles for range
	$(VPN_GEN_SCRIPT) -e $(VPN_ENDPOINT_ID) -m -c $(TF_CA_CERT_FILE) -k $(TF_CA_KEY_FILE) -v $(V_NAME) && git add $(CRL_DIR)

setup-ansible-keys:  ## install keys in the right place to run ansible
	cp -af ./aws1 ./aws1.pub ../../
	chmod 600 ../../aws1

configure-ansible-bluelab-range-name:
	$(info configuring range name...)
	$(SED) -i "s/^BLUE_RANGE :=.*/BLUE_RANGE := $(V_NAME)/" $(ANSIBLE_MAKEFILE)

setup-ansible: setup-ansible-keys configure-ansible-bluelab-range-name ## setup ansible keys and configuration

# generate empty crl, and convert from default der format to pem
gen-empty-crl: write-ca-files-to-disk ## generate updated empty crl
	$(info creating empty crl with no revocations...)
	ipsec pki --signcrl --cacert $(TF_CA_CERT_FILE) --cakey $(TF_CA_KEY_FILE) > $(EMPTY_CRL_DER_FILE)
	openssl crl -inform der -in $(EMPTY_CRL_DER_FILE) -out $(EMPTY_CRL_FILE)

gen-revoke-vendor-crl: write-ca-files-to-disk ## generate updated crl with vendor one through six connections revoked
	$(info creating crl with vendor connections revoked...)
	ipsec pki --signcrl --cacert $(TF_CA_CERT_FILE) --cakey $(TF_CA_KEY_FILE) \
	--cert $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_1) \
	--cert $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_2) \
	--cert $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_3) \
	--cert $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_4) \
	--cert $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_5) \
	--cert $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_6) \
	--cert $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_7) \
	--cert $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_8) \
	--cert $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_9) \
	--cert $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_10) \
	\ > $(VENDOR_DISABLED_CRL_DER_FILE)
	openssl crl -inform der -in $(VENDOR_DISABLED_CRL_DER_FILE) -out $(VENDOR_DISABLED_CRL_FILE)

list-revoked-certs: ## list serial #s of certificates revoked in crl
	$(info listing serial numbers in crl. compare to vendor crt serial numbers...)
	openssl crl -in $(VENDOR_DISABLED_CRL_FILE) -text -noout|grep Serial

list-serials-of-vendor-certs: ## list serial numbers of vendor certificates to compare against crl
	$(info iterate over list of vendor vpn client certificates and print out the serial numbers for comparison...)
	@echo "===================================="
	@echo "$(VENDOR_1) serial..."
	@openssl x509  -noout -serial -in $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_1)
	@echo "===================================="
	@echo "$(VENDOR_2) serial..."
	@openssl x509  -noout -serial -in $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_2)
	@echo "===================================="
	@echo "$(VENDOR_3) serial..."
	@openssl x509  -noout -serial -in $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_3)
	@echo "===================================="
	@echo "$(VENDOR_4) serial..."
	@openssl x509  -noout -serial -in $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_4)
	@echo "===================================="
	@echo "$(VENDOR_5) serial..."
	@openssl x509  -noout -serial -in $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_5)
	@echo "===================================="
	@echo "$(VENDOR_6) serial..."
	@openssl x509  -noout -serial -in $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_6)
	@echo "===================================="
	@echo "$(VENDOR_7) serial..."
	@openssl x509  -noout -serial -in $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_7)
	@echo "===================================="
	@echo "$(VENDOR_8) serial..."
	@openssl x509  -noout -serial -in $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_8)
	@echo "===================================="
	@echo "$(VENDOR_9) serial..."
	@openssl x509  -noout -serial -in $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_9)
	@echo "===================================="
	@echo "$(VENDOR_10) serial..."
	@openssl x509  -noout -serial -in $(CRL_DIR)/$(RANGE_TYPE)-$(V_NAME)-$(VENDOR_10)
	@echo "===================================="

disable-vendor-vpn-access: gen-revoke-vendor-crl ## upload vendor cert revocation list to aws vpn endpoint
	$(info import disable-vendor crl to aws...)
	aws ec2 import-client-vpn-client-certificate-revocation-list --certificate-revocation-list file://./$(VENDOR_DISABLED_CRL_FILE) --client-vpn-endpoint-id $(VPN_ENDPOINT_ID) --region $(AWS_REGION)
	@echo "*********************************************************************************************************************"
	@echo "DO NOT FORGET TO LOGIN TO AWS CONSOLE AND MANUALLY TERMINATE ANY EXISTING VENDOR VPN CONNECTIONS FOR VENDOR: $(V_NAME)"
	@echo "*********************************************************************************************************************"

enable-vendor-vpn-access: gen-empty-crl ## upload crl with no certs revoked and enabling all access
	$(info import empty crl enabling vendor access to vpn...)
	aws ec2 import-client-vpn-client-certificate-revocation-list --certificate-revocation-list file://./$(EMPTY_CRL_FILE) --client-vpn-endpoint-id $(VPN_ENDPOINT_ID) --region $(AWS_REGION)

enable-traffic-mirror: ## make traffic-mirroring terraform available
	$(info enabling traffic mirror (terraform)...)
	@#ls $(TRAFFIC_MIRROR_TF_FILE_OFF) || echo "[!] traffic mirror is enabled. exiting..."; exit 1
	@mv $(TRAFFIC_MIRROR_TF_FILE_OFF) $(TRAFFIC_MIRROR_TF_FILE_ON)

disable-traffic-mirror: ## exclude traffic-mirroring from terraform plan/apply
	$(info disabling traffic mirror (terraform)...)
	@#ls $(TRAFFIC_MIRROR_TF_FILE_ON) || echo "[!] traffic mirror is disabled. exiting..."; exit 1
	@mv $(TRAFFIC_MIRROR_TF_FILE_ON) $(TRAFFIC_MIRROR_TF_FILE_OFF)

view-active-crl: ## get live crl from aws currently applied
	$(info getting live crl from aws vpn...)
	$(info if no certificates are currently revoked no output...)
	aws ec2 export-client-vpn-client-certificate-revocation-list --client-vpn-endpoint-id $(VPN_ENDPOINT_ID) --output text --region $(AWS_REGION) | grep -v "STATUS" > $(LIVE_CRL_TMP_FILE)
	openssl crl -in $(LIVE_CRL_TMP_FILE) -text -noout|grep Serial

upload-crls-aws-storage: gen-empty-crl gen-revoke-vendor-crl  ## regenerate and upload both crl files to s3, does not modify vpn access
	$(info regenerating and uploading crl files for enabling and disabling vendor access for storage Upload only no changes will be made to vpn access)
	aws s3 cp $(VENDOR_DISABLED_CRL_FILE) s3://$(S3_BUCKET_CRL)/$(S3_CRL_PARENT_DIR)/$(V_NAME)/$(VENDOR_DISABLED_CRL_FILE)
	aws s3 cp $(EMPTY_CRL_FILE) s3://$(S3_BUCKET_CRL)/$(S3_CRL_PARENT_DIR)/$(V_NAME)/$(EMPTY_CRL_FILE)
	@echo "Jobs done"
