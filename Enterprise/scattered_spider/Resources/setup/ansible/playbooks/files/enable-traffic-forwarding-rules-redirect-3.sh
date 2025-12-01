#!/usr/bin/env bash

# blackcat traffic forwarding

# first flush the rules
iptables -t nat -F

############ rules ##############
iptables -t nat -A PREROUTING -p tcp -d 116.83.4.99 --dport 8080 -j DNAT --to-destination 176.59.1.18:8080 # kali
iptables -t nat -A PREROUTING -p tcp -d 116.83.2.91 --dport 80 -j DNAT --to-destination 176.59.1.18:8081   # kali
iptables -t nat -A PREROUTING -p tcp -d 116.83.44.32 --dport 22 -j DNAT --to-destination 176.59.1.18:22    # kali
iptables -t nat -A PREROUTING -p tcp -d 116.83.44.32 --dport 20 -j DNAT --to-destination 176.59.1.18:20    # kali
iptables -t nat -A PREROUTING -p tcp -d 116.83.44.32 --dport 21 -j DNAT --to-destination 176.59.1.18:21    # kali
iptables -t nat -A POSTROUTING -j MASQUERADE

# print out updated nat
iptables -L -t nat -v
