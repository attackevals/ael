#!/usr/bin/env bash

# create ssh key pairs for scenario use and support
aws_keys=(
	"aws1"        # range key
	"aws1-vendor" # preflights key
)

# Note: Since key will be used for Windows host, must be RSA key pair

for KEY in "${aws_keys[@]}"; do
	echo "Processing ${KEY}"

	if ! [ -s "${KEY}" ]; then
		ssh-keygen -b 4096 -t rsa -f ./"${KEY}" -q -N ""
		echo "Created new ssh keypair, filename: ${KEY}"

	else
		echo "Key file ${KEY} already exists, skipping..."

	fi

done
