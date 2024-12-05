# Traffic Redirectors

- [Traffic Redirectors](#traffic-redirectors)
  - [Overview](#overview)
  - [Table of URLs and IPs used by Redirectors](#table-of-urls-and-ips-used-by-redirectors)

## Overview

Multiple redirectors are used to distribute the traffic across subnets and IP address ranges.

Traffic redirection is performed using [iptables](https://www.netfilter.org/) NAT masquerading, with traffic sent to specific destination IP address and port combinations being redirected to a specific IP address and port. Scripts in the `ansible/playbooks/files/redirectors` folder provide the following functionality:

1. `enable-traffic-forwarding-rules-redirect.sh.j2`
   - A template script that is populated and run to configure multiple network forwarding rules on a server.
2. `disable-traffic-forwarding-rules.sh`
   - Running the script on a server will disable and clear any network forwarding rules in place.
3. `print-traffic-forwarding-rules.sh`
   - Running the script will print to standard output the network forwarding rules currently configured.

## Table of URLs and IPs used by Redirectors

| Redirector Host | Scenario    | **Inbound Redirection Domain**   | **Inbound Redirection Source IP** | **Inbound Redirection Source Port** | **Destination Host/IP** | **Destination Port** | **TCP/UDP** |
| --------------- | :---------- | :------------------------------- | :-------------------------------- | :---------------------------------- | :---------------------- | :------------------- | :---------- |
| paladin         | A           | `fortius-quo-fidelius[.]co[.]uk` | `14.121.222.11`                   | 443                                 | `corsair` (Kali)        | 443                  | TCP         |
| paladin         | A           | `curse-breaker[.]org`            | `14.121.203.33`                   | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| hindsightlad    | A           | `forev3r1[.]kr`                  | `199.88.44.44`                    | 80                                  | `corsair` (Kali)        | 8080                 | TCP         |
| hindsightlad    | A           | `t1g3r-heart[.]kr`               | `199.88.44.96`                    | 443                                 | `corsair` (Kali)        | 8443                 | TCP         |
| hindsightlad    | A           | `b0ring-t3min4l[.]kr`            | `199.88.44.199`                   | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| abestosboy      | B           | `knuts-to-galleons[.]net`        | `27.21.12.21`                     | 80                                  | `corsair` (Kali)        | 8080                 | TCP         |
| abestosboy      | B           | `muggle-m0ney[.]com`             | `27.21.12.54`                     | 80                                  | `corsair` (Kali)        | 8081                 | TCP         |
| abestosboy      | B           | `1nvisibility-cl0ak[.]net`       | `27.21.12.59`                     | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| abestosboy      | B           | `evals-preflights[.]org`         | `27.21.12.94`                     | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| doorman         | Protections | `psycho-v3lv3t[.]app`            | `36.43.97.65`                     | 80                                  | `corsair` (Kali)        | 8080                 | TCP         |
| doorman         | Protections | `dejavu122[.]radio`              | `36.43.97.72`                     | 22                                  | `corsair` (Kali)        | 22                   | TCP         |
| paladin         | Protections | `1nf1n1ty-b3y0nd[.]app`          | `14.121.11.99`                    | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| doorman         | Protections | `m1sfir3d[.]blog`                | `36.43.97.151`                    | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| hindsightlad    | Protections | `d3fend-ev1l[.]radio`            | `199.88.44.169`                   | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| paladin         | Protections | `c4sh-m0n33y[.]fr`               | `14.121.35.185`                   | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| doorman         | Protections | `rub3ks-c4be[.]net`              | `36.43.97.203`                    | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| hindsightlad    | Protections | `off1ce-p4p4r[.]co[.]uk`         | `199.88.44.224`                   | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| doorman         | Protections | `p1nk-p3ncil[.]net`              | `36.43.97.230`                    | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| paladin         | Protections | `bw-er3ader[.]org`               | `141.121.97.244`                  | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
| doorman         | Protections | `ta11-acfan[.]net`               | `36.43.97.251`                    | 80                                  | `corsair` (Kali)        | 80                   | TCP         |
