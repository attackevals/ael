#!/usr/bin/env bash

# same for all systems
iptables -t nat -F

# print out updated nat to validate change
iptables -L -t nat -v
