# Mustang Panda Cleanup Guide

## End of Scenario

### Voice Track

The following procedures will a conduct a cleanup of the range for the Mustang Panda
scenario.

### Procedures

- If not already open, create an RDP session to `jumpbox (12.78.110.37)`.

  | Destination | Username | Password
  |  - | - | -
  | 12.78.110.37 | evals_domain_admin | Neurotic-Octopus

## Harrenhal Reset

### Voice Track

This section undoes the changes made to the Harrenhal machine during the
scenario.

### Procedures

- From your RDP session to the Windows jumpbox `jumpbox (12.78.110.37)`, RDP into
`harrenhal (10.55.4.103)`. Open a Powershell terminal and execute the script
block.

  | Destination | Username | Password
  |  - | - | -
  | harrenhal.kingslanding.net | kingslanding.net\evals_domain_admin | Neurotic-Octopus

  ```powershell
  if ($env:COMPUTERNAME.ToLower() -eq "harrenhal") {
  Write-Host "Starting cleanup process for Harrenhal..."
  Write-Host "Removing registry key for AccessoryInputServices..."
  $regPath = "HKLM:\Software\Microsoft\Windows\CurrentVersion\Run"
  $regName = "AccessoryInputServices"
  if (Get-ItemProperty -Path $regPath -Name $regName -ErrorAction SilentlyContinue) {
  Remove-ItemProperty -Path $regPath -Name $regName -Force
  }
  Write-Host "Removing scheduled task for AccessoryInputServices..."
  if (Get-ScheduledTask -TaskName "AccessoryInputServices" -ErrorAction SilentlyContinue) {
  Unregister-ScheduledTask -TaskName "AccessoryInputServices" -Confirm:$false
  }
  Write-Host "Deleting dropped files..."
  $harrenhalFiles = @(
  "C:\Users\htargaryen\Downloads\Strategic Competition with Pentos.docx",
  "C:\Users\htargaryen\Downloads\250325_Pentos_Board_Minutes.rar",
  "C:\Users\htargaryen\Downloads\250325_Pentos_Board_Minutes",
  "C:\Users\htargaryen\Downloads\files.txt",
  "C:\Windows\temp\files.txt",
  "C:\Program Files\Microsoft VS Code\prpbg.dat.bak.1",
  "C:\Windows\temp\system.hive",
  "C:\Windows\temp\ntds.dit",
  "C:\Users\htargaryen\appdata\local\CodeHelper.bat")
  $harrenhalfiles +=$(Get-ChildItem -Path "C:\Windows\temp" -Filter "*.rar" -File | ForEach-Object { $_.FullName });
  foreach ($file in $harrenhalFiles) {
  if (Test-Path $file) {
  if ((Get-Item $file).PSIsContainer) {
  Remove-Item $file -Recurse -Force
  } else {
  Remove-Item $file -Force
  }
  } else {
  Write-Host "File not found: $file"
  }
  }
  }

  ```

## Redkeep Reset

### Voice Track

This section undoes the changes made to the Redkeep machine during the scenario.

### Procedures

- From your RDP session to the Windows jumpbox `jumpbox (12.78.110.37)`, RDP into
`redkeep (10.55.3.100)`. Open a Powershell terminal and execute the script
block.

  | Destination | Username | Password
  |  - | - | -
  | redkeep.kingslanding.net | kingslanding\evals_domain_admin | Neurotic-Octopus

  ```powershell
  if ($env:COMPUTERNAME.ToLower() -eq "redkeep") {
      Write-Host "Starting cleanup process for Redkeep..."

      # Remove registry key
      Write-Host "Removing registry key for CodeHelper..."
      $regPath = "HKLM:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"
      $regName = "CodeHelper"
      if (Get-ItemProperty -Path $regPath -Name $regName -ErrorAction SilentlyContinue) {
          Remove-ItemProperty -Path $regPath -Name $regName -Force
      }
      $codeHelperRedkeep = "C:\Windows\codehelper.bat"
      Write-Host "Removing CodeHelper.bat if it exists..."
      foreach ($file in @($codeHelperRedkeep)) {
          if (Test-Path $file) {
              Remove-Item $file -Force
          } else {
              Write-Host "File not found: $file"
          }
      }
  }
  ```

## Stepstones Reset

### Voice Track

This section undoes the changes made to the Stepstones machine during the
scenario.

### Procedures

- From your RDP session to the Windows jumpbox `jumpbox (12.78.110.37)`, RDP into
`stepstones (10.55.4.104)`. Open a Powershell terminal and execute the script
block.

  | Destination | Username | Password
  |  - | - | -
  | stepstones.kingslanding.net | kingslanding\evals_domain_admin | Neurotic-Octopus

  ```powershell
  if ($env:COMPUTERNAME.ToLower() -eq "stepstones" ) {
      Write-Host "Starting Stepstones reset process..."
      Write-Host ""
      Write-Host "Stopping WinGup"
      $gupProc = Get-Process gup -ErrorAction SilentlyContinue
      if ($gupProc) {
          $gupProc | Stop-Process -Force
          Write-Host "WinGup process stopped."
      } else {
          Write-Host "WinGup process not running. Continuing..."
      }

      Write-Host "Uninstalling MSI"
      $msiPath = "C:\\Users\\ccole\\Downloads\\2025p2.msi"
      if (Test-Path $msiPath) {
          Start-Process msiexec.exe -ArgumentList "/x `"$msiPath`" /quiet" -Wait
          Write-Host "2025p2.msi uninstalled successfully."
      } else {
          Write-Host "MSI file not found: $msiPath. Continuing..."
      }

      $stepstonesFiles = @(
          "C:\\AdExplorer",
          "C:\\AdExplorer.zip",
          "C:\\Users\\Public\\Documents\\b44d0xUT5BLOi.rar",
          "C:\\Users\\ccole\\AppData\\Local\\Temp\\Meeting Invitation.pdf",
          "C:\\Users\\ccole\\Downloads\\2025p2.msi"
      )

      foreach ($file in $stepstonesFiles) {
          Write-Host "Deleting $file"
          if (Test-Path $file) {
              Remove-Item $file -Recurse -Force
              Write-Host "$file deleted successfully."
          } else {
              Write-Host "File not found: $file. Continuing..."
          }
      }

      Write-Host "Deleting EvRDRunMP folder for user ccole"
      $evrdPath = "C:\\Users\\ccole\\AppData\\Local\\EvRDRunMP"
      if (Test-Path $evrdPath) {
          Remove-Item $evrdPath -Recurse -Force
          Write-Host "EvRDRunMP folder deleted successfully."
      } else {
          Write-Host "EvRDRunMP folder not found: $evrdPath. Continuing..."
      }

      $runKey = "HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
      $keyVal = "WinGupSvc"
      if (Get-ItemProperty -Path $runKey -Name $keyVal -ErrorAction SilentlyContinue) {
          Write-Host "Registry key value '$keyVal' exists. Deleting..."
          Remove-ItemProperty -Path $runKey -Name $keyVal -ErrorAction SilentlyContinue
      } else {
          Write-Host "Registry key value '$keyVal' does not exist. Continuing..."
      }
  }
  ```
