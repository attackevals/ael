# SharpNBTScan

SharpNBTScan is a NetBIOS scanning tool written in C#.

It has been checked out at [commit e2fe08f4788bc2e0b5fc854107f527d8e5221c56](https://github.com/BronzeTicket/SharpNBTScan/commit/e2fe08f4788bc2e0b5fc854107f527d8e5221c56).

* [Original Repository](https://github.com/BronzeTicket/SharpNBTScan)
* [Original README](https://github.com/BronzeTicket/SharpNBTScan/blob/main/README.md)

## Features

* Scans for open NETBIOS nameservers (IP address and hostname enumeration)

## Usage

* To Retrieve host info via NetBIOS:

```powershell
.\SharpNBTScan.exe 192.168.1.216
[*] Tasked beacon to run .NET program: SharpNBTScan.exe 192.168.1.216
[+] host called home. sent: 118853 bytes
[+] received output:
[*]Start udo client ...
[+]Udp client will stop in 10s ...
192.168.1.216 DOMAINNAME\WIN-CAQPP4PU77E  DC     Vmware

[+] received output:
[*]Stop udp client ...
```

## Build Instructions

### Visual Studio

* Open the `...\Resources\sharpnbtscan` directory in Visual Studio

* Load the `SharpNBTScan.sln` solution file

* Build -> Build all

### Command Line

* From the `...\Resources\sharpnbtscan` directory

  ```batch
  MSBuild.exe SharpNBTScan.sln /p:Configuration="Release"
  ```

## CTI Reporting

1. <https://www.security.com/threat-intelligence/china-southeast-asia-espionage>
1. <https://unit42.paloaltonetworks.com/stately-taurus-abuses-vscode-southeast-asian-espionage/>
