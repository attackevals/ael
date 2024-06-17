#!/usr/bin/env bash

# xrdp: install xrdp, augeas
apt update
DEBIAN_FRONTEND=noninteractive apt install -y git etckeeper

# PATH TO YOUR HOSTS FILE
ETC_HOSTS=/etc/hosts

function add() {
    IP=$1
    HOSTNAME=$2
    HOSTS_LINE="$IP[[:space:]]$HOSTNAME"
    line_content=$( printf "%s\t%s\n" "$IP" "$HOSTNAME" )
    if [ -n "$(grep -P $HOSTS_LINE $ETC_HOSTS)" ]
        then
            echo "$line_content exists : $(grep $HOSTNAME $ETC_HOSTS)"
        else
            echo "Adding $line_content to $ETC_HOSTS";
            sudo -- sh -c -e "echo '$line_content' >> /etc/hosts";

            if [ -n "$(grep -P $HOSTNAME $ETC_HOSTS)" ]
                then
                    echo "$line_content added successfully";
                else
                    echo "Error adding $line_content";
            fi
    fi
}

add ${HostIp} ${RemoteHostname}

sed -i 's@preserve_hostname: false@preserve_hostname: true@g' /etc/cloud/cloud.cfg
sed -i '/^preserve_hostname: true/ahostname: ${RemoteHostname}' /etc/cloud/cloud.cfg
hostnamectl set-hostname ${RemoteHostname}

mkdir -p /opt
mkdir -p /srv
