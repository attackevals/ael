#!/usr/bin/env bash

# fix issue where window manager crashes and causes black screen on rdp session
# a few possible root causes, but until fix is identified, commands below will allow you to reconnect via rdp

# connect over ssh to kali box

# stop the rdp session manager
echo "stopping xrdp session manager service"
sudo systemctl stop xrdp-sesman.service

# force kill any session manager and X server processes still hanging around,
echo "killing any xrdp or x server processes..."
sudo killall -9 xrdp-sesman
sudo killall -9 Xorg

# restart the master xrdp service, which will trigger restarting the session manager
echo "restarting xrdp, which will trigger restart of session manager"
sudo systemctl restart xrdp.service

# ensure xrdp service is active (command below should return "active")
echo "checking rdp service status, should return 'active'"
sudo systemctl is-active xrdp-sesman.service
