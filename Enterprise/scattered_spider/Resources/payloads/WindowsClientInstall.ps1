Write-Output "Starting install" >> C:\Windows\Temp\install.log

$ErrorActionPreference= 'silentlycontinue'

# Static RuskDesk ID and password
$byte_id = [System.Text.Encoding]::UTF8.GetBytes(((Get-CimInstance -ClassName Win32_ComputerSystem).Name))
$rustdesk_id = "$byte_id".Replace(' ', '').Substring(0,6)
$rustdesk_pw="QJ1ABOIu6ekkZJop"

# Get your config string from your Web portal and Fill Below
$rustdesk_cfg="0nI9kVSmV1YYB1N1kjdkdVYjJHSH1kSlxEbkRVNq9ERxpUOoRVOxNEb5V3U5EkI6ISeltmIsISbvNmLvN3clZXas9yL6MHc0RHaiojIpBXYiwiIt92Yu82czVmdpxmI6ISehxWZyJCLi02bj5ybzNXZ2lGbiojI0N3boJye"

$RDVERSION = '1.3.5'

# Run as administrator and stays in the current directory
if (-Not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    if ([int](Get-CimInstance -Class Win32_OperatingSystem | Select-Object -ExpandProperty BuildNumber) -ge 6000) {
        Start-Process PowerShell -Verb RunAs -ArgumentList "-NoProfile -ExecutionPolicy Bypass -Command `"cd '$pwd'; & '$PSCommandPath';`"";
        Exit;
    }
}

# Checks the version of RustDesk installed.
$rdver = ((Get-ItemProperty  "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\RustDesk\").Version)

# Skips to inputting the configuration if the latest version of RustDesk is already installed.
if($rdver -eq "$RDVERSION") {
    cd $env:ProgramFiles\RustDesk
    .\rustdesk.exe --config $rustdesk_cfg
    .\rustdesk.exe --password $rustdesk_pw
    $rustdesk_id = .\rustdesk.exe --get-id | Write-Output -OutVariable rustdesk_id
    .\rustdesk.exe
    exit
}

if (!(Test-Path C:\Temp)) {
    New-Item -ItemType Directory -Force -Path C:\Temp > null
}

cd C:\Temp
powershell Invoke-WebRequest "https://github.com/rustdesk/rustdesk/releases/download/$RDVERSION/rustdesk-$RDVERSION-x86_64.exe" -Outfile "rustdesk.exe"
Start-Process .\rustdesk.exe --silent-install
Start-Sleep -Seconds 10

Write-Output "Completed silent install" >> C:\Windows\Temp\install.log

$ServiceName = 'rustdesk'
$arrService = Get-Service -Name $ServiceName -ErrorAction SilentlyContinue

if ($arrService -eq $null)
{
	Write-Output "Service not found, installing as service" >> C:\Windows\Temp\install.log
    cd $env:ProgramFiles\RustDesk
    Start-Process .\rustdesk.exe --install-service -wait -Verbose
    Start-Sleep -Seconds 20
}

Write-Output "Waiting for service to be running" >> C:\Windows\Temp\install.log

do {
    $arrService = Get-Service -Name $ServiceName -ErrorAction SilentlyContinue
    if ($arrService -and $arrService.Status -ne 'Running') {
        Start-Service $ServiceName -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 5
    }
} until ($arrService -and $arrService.Status -eq 'Running')

# Waits for installation to complete before proceeding.
Start-Sleep -Seconds 10

cd $env:ProgramFiles\RustDesk
echo "Inputting configuration now"
Write-Output "Inputting configuration now" >> C:\Windows\Temp\install.log
.\rustdesk.exe --config $rustdesk_cfg
.\rustdesk.exe --password $rustdesk_pw

# Force change the ID to the static ID
net stop RustDesk
taskkill /im RustDesk /f

$id = Get-Content C:\Windows\ServiceProfiles\LocalService\AppData\Roaming\RustDesk\config\RustDesk.toml | Select -First 1
$newId = "id = '$rustdesk_id'"

$filecontent = Get-Content -Path C:\Windows\ServiceProfiles\LocalService\AppData\Roaming\RustDesk\config\RustDesk.toml -Raw

$filecontent.Replace("$id","$newId") | Set-Content -Path C:\Windows\ServiceProfiles\LocalService\AppData\Roaming\RustDesk\config\RustDesk.toml

net start RustDesk
.\rustdesk.exe

$id = $(& 'C:\Program Files\RustDesk\rustdesk.exe' --get-id | Out-String)

# Print ID for verification
echo "RustDesk ID: $id"
Write-Output "RustDesk ID: $id" >> C:\Windows\Temp\install.log
