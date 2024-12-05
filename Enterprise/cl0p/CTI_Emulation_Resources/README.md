# CL0P

## Adversary Overview

Active since at least 2019, [CL0P](https://attack.mitre.org/software/S0611/) is a sophisticated ransomware family that is associated with the [TA505](https://attack.mitre.org/groups/G0092/) cybercriminal group and possibly FIN11. CL0P employs phishing campaigns to gain initial access, often using macro-enabled documents to deploy malicious loaders. Like most other ransomware families, CL0P leverages “steal, encrypt, and leak” strategy - they identify and encrypt files, append various extensions (e.g., .clop) and leverage threats of data leaks on their Tor site, CL0P^_-LEAKS, to pressure victims into paying ransoms. Recent attacks have exploited vulnerabilities in software such as [MOVEit Transfer](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-158a) and [GoAnywhere](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-158a) MFT, leading to significant breaches and prompting U.S. authorities to offer [rewards](https://x.com/RFJ_USA/status/1669740545403437056?prefetchTimestamp=1731528766880) for information on the group's members. CL0P is utilized for financial gain and therefore, has a more randomized approach in its targeting and does not appear to target any particular region or sector. CL0P's adaptive tactics and extensive targeting capabilities make it one of the most formidable ransomware threats today.

## Key Adversary Report References

| Source ID | Report Links |
| --- | --- |
1 | [CISA #StopRansomware: CL0P Ransomware Gang Exploits CVE-2023-34362 MOVEit Vulnerability](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-158a#:~:text=According%20to%20open%20source%20information,%20beginning%20on%20May%2027,%202023,)
2 | [Proofpoint US Get2 Downloader & SDBbot RAT Analysis](https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader)|
3 | [Ransomware Spotlight: Clop ](https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-clop)
4 | [CL0P Ransomware: The Latest Updates](https://cyberint.com/blog/dark-web/cl0p-ransomware/)
5 | [Investigating CL0P ransomware & MOVEit 2023 breach](https://sectrio.com/blog/deconstructing-cl0p-ransomware-moveit-2023-breach/)
6 | [Cybereason vs. Cl0p Ransomware](https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware)
7 | [McAfee Blog Clop Ransomware](https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/)
8 | [Palo Alto Threat Assessment: Clop Ransomware](https://unit42.paloaltonetworks.com/clop-ransomware/)