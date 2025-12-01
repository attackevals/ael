@echo off
set FILES="%TEMP%\Meeting Invitation.pdf" "%USERPROFILE%\Downloads\2025p2.msi"

@echo Stopping WinGup
taskkill /f /im gup.exe

@echo uninstalling MSI
msiexec /uninstall "%USERPROFILE%\Downloads\2025p2.msi" /quiet

@echo deleting EvRDRunMP folder
cd %localappdata%
rmdir /s /q "%localappdata%\EvRDRunMP"

@echo deleting loose files
for %%F in (%FILES%) do (
    echo Deleting %%F
    del /f /q %%F
)

set RUN_KEY="HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run"
set KEY_VAL="WinGupSvc"
@echo Deleting registry key %RUN_KEY% value %KEY_VAL%
reg delete %RUN_KEY% /v %KEY_VAL% /f >nul 2>&1

@echo Deleting self
start "" /b cmd /c "echo @echo off > %~f0 && echo ping 127.0.0.1 -n 5 ^>nul >> %~f0 && echo del %%~f0 >> %~f0 && %~f0"

exit
