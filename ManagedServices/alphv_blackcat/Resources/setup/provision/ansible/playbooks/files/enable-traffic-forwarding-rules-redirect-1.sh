#!/usr/bin/env bash

# menupass traffic forwarding

# first flush the rules
iptables -t nat -F

############ rules ##############
iptables -t nat -A PREROUTING -p tcp -d 121.93.24.2 --dport 445 -j DNAT --to-destination 176.59.1.18:445    # kali
iptables -t nat -A PREROUTING -p tcp -d 121.93.24.2 --dport 139 -j DNAT --to-destination 176.59.1.18:139    # kali
iptables -t nat -A PREROUTING -p tcp -d 121.93.66.49 --dport 80 -j DNAT --to-destination 176.59.1.18:8081    # kali
iptables -t nat -A PREROUTING -p tcp -d 121.93.4.32 --dport 4782 -j DNAT --to-destination 116.83.1.29:4782  # homelander
iptables -t nat -A POSTROUTING -j MASQUERADE

# print out updated nat
iptables -L -t nat -v
