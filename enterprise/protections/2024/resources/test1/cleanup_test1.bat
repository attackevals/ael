@ECHO off
@setlocal enabledelayedexpansion
set originalpwd=%cd%
set targetdir="%SYSTEMDRIVE%\Users"
set logfile="%cd%\Test1Log.txt"
set rclonezip="%cd%\rclone-v1.66.0-windows-amd64.zip"
set rclone="%cd%\rclone-v1.66.0-windows-amd64"

:: Get list of created zips for each user
cd %targetdir%
set numzips=0
FOR /F "tokens=*" %%d IN ('dir /b /a:d-l') DO (
    set numarchives=0
    FOR %%b IN ("%cd%\%%d\Documents" "%cd%\%%d\Downloads" "%cd%\%%d\Desktop") DO (
        set zips[!numzips!]="%originalpwd%\%%d_archive!numarchives!.zip"
        set /A numzips=numzips+1
        set /A numarchives=numarchives+1
    )
)
cd %originalpwd%

:: Try to delete the potentially created zipped files
set /A maxarchiveindex=numzips-1
FOR /L %%x IN (0,1,!maxarchiveindex!) DO (
    set archivepath=!zips[%%x]!
    del /q !archivepath! 2>NUL
)

:: Delete the logfile & rclone
del /q %logfile%
rmdir /q /s %rclone%
del /q %rclonezip%