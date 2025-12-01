---
tags: [infrastructure, enterprise2025, networking, redirectors, traffic]
---

# Traffic Redirection

- [Traffic Redirection](#traffic-redirection)
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

<br>

## Table of URLs and IPs used by Redirectors

| **Scenario Purpose**                                    | **Redirection Domain**                 | **Redirection Source IP** | **Redirector** | **Redirection Source Port(s)**               | **Destination Host** | **Destination Port(s)**                      |
|---------------------------------------------------------|:---------------------------------------|:--------------------------|:---------------|:---------------------------------------------|:---------------------|:---------------------------------------------|
| <nobr> Scattered Spider: Simple File Server              </nobr> | <nobr> `kingslanding-rmm.com` </nobr>  | 86.234.15.38              | `stormlands`   | 80                                           | `kali`               | 80                                           |
| <nobr> Scattered Spider: Simple File Server (Preflights) </nobr> | <nobr> `evals-preflights.org` </nobr>  | 86.234.54.133             | `stormlands`   | 80                                           | `kali`               | 80                                           |
| <nobr> Scattered Spider: Spearphish to Adversary AITM    </nobr> | <nobr> `sso-kingslanding.net` </nobr>  | 86.234.158.146            | `stormlands`   | 80                                           | `kali`               | 8888                                         |
| <nobr> Scattered Spider: SFTP                            </nobr> | <nobr> `ramsgate.com`         </nobr>  | 86.234.99.108             | `stormlands`   | 22                                           | `kali`               | 22                                           |
| <nobr> Mustang Panda: Orpheus FTP Exfil                </nobr> | <nobr> `karhold.com`          </nobr>  | 49.67.12.21               | `stark`        | 20<br>21<br>40000<br>40001<br>40002<br>40003 | `kali`               | 20<br>21<br>40000<br>40001<br>40002<br>40003 |
| <nobr> Mustang Panda: Orpheus SFS                      </nobr> | <nobr> `pentos-analysis.org`  </nobr>  | 191.44.44.44              | `bolton`       | 80                                           | `kali`               | 80                                           |
| <nobr> Mustang Panda: Orpheus C2 Handler               </nobr> | <nobr> `karstarkland.net`     </nobr>  | 191.44.44.199             | `bolton`       | 443                                          | `kali`               | 8443                                         |
| <nobr> Mustang Panda: Perseus C2 Handler               </nobr> | <nobr> `ValarMorghulis.org`   </nobr>  | 78.31.97.203              | `greyjoy`      | 443                                          | `kali`               | 9443                                         |
| <nobr> Mustang Panda: Perseus HTML file                </nobr> | <nobr> `mailstreamnet.com`    </nobr>  | 49.67.12.59               | `stark`        | 443                                          | `kali`               | 8443                                         |
| <nobr> Mustang Panda: Toneshell VSCode Tunnel          </nobr> | <nobr> `edupronewscd.com`     </nobr>  | 49.67.12.94               | `stark`        | 80                                           | `kali`               | 8888                                         |
| <nobr> Protections 1: Simple File Server        </nobr> | <nobr> `pasteme.io`           </nobr>  | 86.234.232.55             | `stormlands`   | 80                                           | `kali`               | 80                                           |
| <nobr> Protections 1: Attacker Proxy 1          </nobr> | <nobr> `ironborn.io`          </nobr>  | 78.31.97.65               | `greyjoy`      | 443                                          | `authentik`          | 443                                          |
| <nobr> Protections 3: C2 Redirector             </nobr> | <nobr> `livesso.com`          </nobr>  | 49.67.12.54               | `stark`        | 80                                           | `rustdesk`           | 80                                           |
| <nobr> Protections 3: Attacker Proxy 2          </nobr> | <nobr> `shadowbinders.net`    </nobr>  | 78.31.97.72               | `greyjoy`      | 443                                          | `authentik`          | 443                                          |
| <nobr> Protections 4: Orpheus C2 Handler        </nobr> | <nobr> `casterlystone.com`    </nobr>  | 191.44.44.224             | `bolton`       | 443                                          | `kali`               | 8443                                         |
| <nobr> Protections 5: Perseus C2 Handler        </nobr> | <nobr> `MaesterJam2025.com`   </nobr>  | 78.31.97.251              | `greyjoy`      | 443                                          | `kali`               | 9443                                         |
| <nobr> Protections 5: SFS for MSC file          </nobr> | <nobr> `faithoftheseven.com`  </nobr>  | 191.44.44.96              | `bolton`       | 80                                           | `kali`               | 80                                           |
| <nobr> Protections 5: SFS for MSI file          </nobr> | <nobr> `essos-news.com`       </nobr>  | 191.44.44.169             | `bolton`       | 80                                           | `kali`               | 80                                           |
| <nobr> Protections 7: Attacker Proxy 3          </nobr> | <nobr> `goldcloaks.org`       </nobr>  | 78.31.97.151              | `greyjoy`      | 443                                          | `authentik`          | 443                                          |
