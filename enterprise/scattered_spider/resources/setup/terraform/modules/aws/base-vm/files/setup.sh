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
	line_content=$(printf "%s\t%s\n" "$IP" "$HOSTNAME")
	if [ -n "$(grep -P $HOSTS_LINE $ETC_HOSTS)" ]; then
		echo "$line_content exists : $(grep $HOSTNAME $ETC_HOSTS)"
	else
		echo "Adding $line_content to $ETC_HOSTS"
		sudo -- sh -c -e "echo '$line_content' >> /etc/hosts"

		if [ -n "$(grep -P $HOSTNAME $ETC_HOSTS)" ]; then
			echo "$line_content added successfully"
		else
			echo "Error adding $line_content"
		fi
	fi
}

add ${HostIp} ${RemoteHostname}

sed -i 's@preserve_hostname: false@preserve_hostname: true@g' /etc/cloud/cloud.cfg
sed -i '/^preserve_hostname: true/ahostname: ${RemoteHostname}' /etc/cloud/cloud.cfg
hostnamectl set-hostname ${RemoteHostname}

#DEBIAN_FRONTEND=noninteractive apt install -y kali-desktop-xfce xorg xrdp augeas-{tools,lenses} git etckeeper

# add xrdp user to ssl-cert group
#usermod -a -G ssl-cert xrdp
#systemctl enable xrdp --now

###
# set in /etc/xrdp/xrdp.ini
#security_layer=tls
#crypt_level=high
#ssl_protocols=TLSv1.2, TLSv1.3
#tls_ciphers=FIPS:-eNULL:-aNULL

# disable parameter expansion to be safe
#augtool -A -b <<'EOF'
#set /augeas/load/PHP/lens PHP.lns
#set /augeas/load/PHP/incl /etc/xrdp/xrdp.ini
#set /files/etc/xrdp/xrdp.ini/Globals/security_layer "tls"
#set /files/etc/xrdp/xrdp.ini/Globals/crypt_level "high"
#set /files/etc/xrdp/xrdp.ini/Globals/ssl_protocols "TLSv1.2, TLSv1.3"
#set /files/etc/xrdp/xrdp.ini/Globals/tls_ciphers "FIPS:-eNULL:-aNULL"
#save
#quit
#EOF

mkdir -p /opt
mkdir -p /srv
