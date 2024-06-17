#!/usr/bin/env bash

# xrdp: install xrdp, augeas
apt update
DEBIAN_FRONTEND=noninteractive apt install -y git etckeeper
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
