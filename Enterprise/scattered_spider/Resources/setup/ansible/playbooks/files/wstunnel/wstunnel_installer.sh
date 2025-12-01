#!/bin/bash

# Get wstunnel installer:
curl -L https://github.com/erebe/wstunnel/releases/download/v10.1.8/wstunnel_10.1.8_linux_amd64.tar.gz -o wstunnel_10.1.8_linux_amd64.tar.gz

# Unzip the file:
tar -xzf wstunnel_10.1.8_linux_amd64.tar.gz

# Make installer file executable:
sudo chmod +x wstunnel

# Run wstunnel as a server, listening on port 443 [Headless]:
sudo nohup ./wstunnel server wss://[::]:443 &
