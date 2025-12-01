#!/usr/bin/env bash

# menupass traffic forwarding

# first flush the rules
iptables -t nat -F

############ rules ##############
iptables -t nat -A PREROUTING -p tcp -d 121.93.99.100 --dport 4782 -j DNAT --to-destination 116.83.1.29:4782 # homelander
iptables -t nat -A PREROUTING -p tcp -d 121.93.44.121 --dport 80 -j DNAT --to-destination 176.59.1.18:8082   # kali
iptables -t nat -A POSTROUTING -j MASQUERADE

# print out updated nat
iptables -L -t nat -v
