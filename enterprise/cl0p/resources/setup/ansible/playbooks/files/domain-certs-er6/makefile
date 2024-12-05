

CFSSL_JSON := ../cfssl.json
INTERMEDIATE_PREFIX := intermediate-ca
INTERMEDIATE_PROFILE := intermediate-ca
SONICBEATS := sonicbeats37
SONICBEATS_DIR := sonicbeats37.fm
ENCRYPTPOTTER := encryptpotter
ENCRYPTPOTTER_DIR := encryptpotter.net
DECRYPTMALFOY := decryptmalfoy
DECRYPTMALFOY_DIR := decryptmalfoy.net



.DEFAULT_GOAL := help

.PHONY: help
help:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

build-sonicbeats-ca: ## build ca
	(cd $(SONICBEATS_DIR); cfssl gencert -initca $(SONICBEATS)-ca.json| cfssljson -bare $(SONICBEATS)-ca)

build-sonicbeats-intermediate: ## build intermediate cert
	(cd $(SONICBEATS_DIR); cfssl gencert -initca $(INTERMEDIATE_PREFIX)-$(SONICBEATS).json| cfssljson -bare $(INTERMEDIATE_PREFIX)-$(SONICBEATS))

build-sonicbeats-server: ## gen and sign server cert
	(cd $(SONICBEATS_DIR); cfssl gencert -ca $(INTERMEDIATE_PREFIX)-$(SONICBEATS).pem -ca-key $(INTERMEDIATE_PREFIX)-$(SONICBEATS)-key.pem -config $(CFSSL_JSON) -profile=server host-$(SONICBEATS).json| cfssljson -bare host-$(SONICBEATS)-server)


build-encryptpotter-ca: ## build ca
	(cd $(ENCRYPTPOTTER_DIR); cfssl gencert -initca $(ENCRYPTPOTTER)-ca.json| cfssljson -bare $(ENCRYPTPOTTER)-ca)

build-encryptpotter-intermediate: ## build intermediate cert
	(cd $(ENCRYPTPOTTER_DIR); cfssl gencert -initca $(INTERMEDIATE_PREFIX)-$(ENCRYPTPOTTER).json| cfssljson -bare $(INTERMEDIATE_PREFIX)-$(ENCRYPTPOTTER))

build-encryptpotter-server: ## gen and sign server cert
	(cd $(ENCRYPTPOTTER_DIR); cfssl gencert -ca $(INTERMEDIATE_PREFIX)-$(ENCRYPTPOTTER).pem -ca-key $(INTERMEDIATE_PREFIX)-$(ENCRYPTPOTTER)-key.pem -config $(CFSSL_JSON) -profile=server host-$(ENCRYPTPOTTER).json| cfssljson -bare host-$(ENCRYPTPOTTER)-server)


build-decryptmalfoy-ca: ## build ca
	(cd $(DECRYPTMALFOY_DIR); cfssl gencert -initca $(DECRYPTMALFOY)-ca.json| cfssljson -bare $(DECRYPTMALFOY)-ca)

build-decryptmalfoy-intermediate: ## build intermediate cert
	(cd $(DECRYPTMALFOY_DIR); cfssl gencert -initca $(INTERMEDIATE_PREFIX)-$(DECRYPTMALFOY).json| cfssljson -bare $(INTERMEDIATE_PREFIX)-$(DECRYPTMALFOY))

build-decryptmalfoy-server: ## gen and sign server cert
	(cd $(DECRYPTMALFOY_DIR); cfssl gencert -ca $(INTERMEDIATE_PREFIX)-$(DECRYPTMALFOY).pem -ca-key $(INTERMEDIATE_PREFIX)-$(DECRYPTMALFOY)-key.pem -config $(CFSSL_JSON) -profile=server host-$(DECRYPTMALFOY).json| cfssljson -bare host-$(DECRYPTMALFOY)-server)


#sonicbeats-all: build-sonicbeats-ca build-sonicbeats-intermediate build-sonicbeats-server
