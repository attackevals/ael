# ---------------------------------------------------------------------------
# gen_dpapi_blob.ps1 - DPAPI encrypts the provided string

# Copyright 2024 MITRE Engenuity. Approved for public release. Document number CT0005.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: ./gen_dpapi_blob.ps1 -String Sup3rS3cr3tP@ssword1!

# Revision History:

# --------------------------------------------------------------------------- 

param ([String] $String)

$String = $String

#Checking if file exists. Quitting otherwise.
if (-not($String)) {

    write-host "Error, no string provided! Quitting.";
    exit 1;
}


<#
    .SYNOPSIS
       dpapi encrypts a string

    .PARAMETER $$String
        specify a string to be encrypted.

#>
function create_dpapi_blob($String) {
    [void] [Reflection.Assembly]::LoadWithPartialName("System.Security")
    $scope = [System.Security.Cryptography.DataProtectionScope]::CurrentUser
    $plaintext = [System.Text.UTF8Encoding]::UTF8.GetBytes($String)
    $ciphertext = [System.Security.Cryptography.ProtectedData]::Protect($plaintext, $null, $scope )  
    $b64 = [System.Convert]::ToBase64String($ciphertext)
    write-host $b64
}


create_dpapi_blob($String)