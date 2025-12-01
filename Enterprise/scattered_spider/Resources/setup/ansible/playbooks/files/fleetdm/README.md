# Fleet Docker Deployment

## Services

- MySQL
- Redis
- Fleet
- Caddy

## Configuration Files

All services are configured using the `default.env` file included in the folder for that service.

## Fleet configuration details

Bare-bones version of Fleet with filesystem logging options.

## TLS

Two basic examples of how to enable TLS based on `caddy`.

To be able to expose the installation, please make sure that:

- Ports `80` and `443` are open/reachable
- A `DNS` entry exists that points to your host (A-record pointing to `fleet.example.com`)

## Standalone setup

This approach includes a `caddy` container with the required configuration in the docker-compose file `docker-compose-caddy-standalone.yml`.

### Install Docker

```bash
# Create required directories
# Add Docker's official GPG key:
sudo apt-get update
sudo apt-get install ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

# Add the repository to Apt sources:
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] \
  https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update

# Installing latest version
sudo apt-get install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

# [Optional] - Verify Docker Installation
sudo docker run hello-world
```

### Set-up Portainer UI

```bash
# 1. Docker User Group Permissions
sudo usermod -aG docker ${USER}

export my_group=$(id -gn)
newgrp docker
newgrp $my_group

# 2. OR REBOOT - then run rest of commands

# Portainer specific (Docker UI)
docker volume create portainer_data
docker run -d -p 8000:8000 -p 9443:9443 --name portainer --restart=always \
    -v /var/run/docker.sock:/var/run/docker.sock \
    -v portainer_data:/data portainer/portainer-ce:latest

# see what is running
docker ps --all
```

### Configure FleetDm

```bash
# Create required directories
mkdir fleet/{logs,vulndb} mysql/data

# Fix permissions
sudo chmod -R o+w fleet/{logs,vulndb} mysql/data
chmod 600 config/ACME/acme.json

# Create the docker network for the edge router
docker network create caddy_proxy

# Docker Compose force recreate Caddy Standalone to run in background
docker compose -f docker-compose-caddy-standalone.yml up -d
```
