# ---------------------------------------------------------------------------
# sdbbot_cleanup.ps1 - cleans up SDBBot artifacts from target host.
#                      Run as elevated user.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: powershell .\sdbbot_cleanup.ps1

# --------------------------------------------------------------------------- 

Write-Host "[INFO] Performing SDBBot cleanup on $(hostname)";

# Remove shellcode from registry
if (Get-ItemProperty "HKLM:\SOFTWARE\Microsoft\skw" -ErrorAction SilentlyContinue) {
    Write-Host "[DEBUG] Deleting shellcode registry key HKLM\SOFTWARE\Microsoft\skw";
    reg delete HKLM\SOFTWARE\Microsoft\skw /f;
} else {
    Write-Host "[DEBUG] Shellcode registry key HKLM\SOFTWARE\Microsoft\skw not found. Skipping";
}

# Remove IFEO registry options
if (reg.exe query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\winlogon.exe" 2>$null) {
    Write-Host "[DEBUG] Deleting IFEO registry key for winlogon";
    reg.exe delete "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\winlogon.exe" /f;
} else {
    Write-Host "[DEBUG] IFEO registry key for winlogon not found. Skipping";
}

# Remove file artifacts (except log files)
Write-Host "[INFO] Deleting dropped files";
$artifacts = @(
    "C:\Windows\temp\tmp8AB2.tmp" # loader DLL
    "C:\Windows\System32\msverload.dll" # loader symlink
    "C:\Users\user1\AppData\Roaming\IFInstaller.dll" # installer DLL
);

$artifacts | %{
    if (Test-Path "$_") {
        Write-Host "[DEBUG] Deleting file $_";
        Remove-Item -Force "$_";
    } else {
        Write-Host "[DEBUG] File $_ not found. Skipping";
    }
};
