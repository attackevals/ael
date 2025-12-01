#!/bin/bash

# Download and install the AirByte command-line tool
curl -LsfS https://get.airbyte.com | bash -
abctl version

# Install in low resource mode and over HTTP (insecure-cookies)
sudo abctl local install --insecure-cookies --low-resource-mode

# Replace example email with the login email
sudo abctl local credentials --email "test@test.example"

# Display login credentials
sudo abctl local credentials
