# WinRAR

WinRAR is used in Step 9 of the Managed Services Round 2 menuPass scenario.
The steps below describe how to access and stage the payload for scenario
execution.

## Payload Download

1. Download the latest version of the WinRAR installer:
<https://www.win-rar.com/download.html?&L=0>
1. On a separate machine, run the WinRAR installer to install WinRAR
1. In the install location (C:\Program Files\WinRAR) locate the WinRAR binary
`Rar.exe` for use in the menuPass emulation plan
1. Stage the `Rar.exe` binary as `giag1.crl` in the payloads folder for the
evalsC2server to access

## CTI Reporting

[PwC - Operation Cloud Hopper: Technical Annex](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=27)