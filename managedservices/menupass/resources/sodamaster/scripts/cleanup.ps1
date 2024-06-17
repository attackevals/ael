# ---------------------------------------------------------------------------
# cleanup.ps1 - Removes files and service created for the SodaMaster implant.

# Copyright 2023 MITRE Engenuity. Approved for public release. Document number CT0005.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: .\cleanup.ps1

# Revision History:

 # Resources: 
 # https://powershellisfun.com/2022/08/10/create-tcp-udp-port-listener-using-powershell/
 # https://riptutorial.com/powershell/example/18117/tcp-listener

# --------------------------------------------------------------------------- 

function Delete-File {
    param (
        [parameter(Mandatory = $true)]
        [string]$filePath
    )

    try {
        # check file exists
        if (Test-Path -Path $filePath) {
            # delete the file
            Remove-Item -Path $filePath
            Write-Host "Deleted $filePath successfully."
        } else {
            Write-Host "File does not exist at the path: $filePath"
        }
    } catch {
        Write-Host "An error occured: $_"
    }
}

function Delete-Service {
    param (
        [parameter(Mandatory = $true)]
        [string]$serviceName
    )

    try {
        # check service exists
        $service = Get-Service -Name $serviceName -ErrorAction SilentlyContinue
        if ($service) {
            # stop the service
            if ($service.Status -eq "Running") {
                Stop-Service -Name $serviceName -Force
            }
            # delete the service
            $deleteResult = sc.exe delete $serviceName
            Write-Host $deleteResult
            Write-Host "Service $serviceName deleted successfully."
        } else {
            Write-Host "Service $serviceName does not exist"
        }
    } catch {
        Write-Host "An error occured: $_"
    }
}

Delete-File -filePath "C:\Program Files\Notepadd++\04DFAA.log"
Delete-File -filePath "C:\Windows\System32\hkp.dll"
Delete-File -filePath "C:\Windows\System32\win64_tools.dll"
Delete-File -filePath "C:\Program Files\Notepadd++\VERSION.dll"

Delete-Service -serviceName "Notepad"