 # ---------------------------------------------------------------------------
 # insert_data_into_mssql.ps1 - Inserts data into MSSQL in base64 or base64/encrypted form.

 # Copyright 2023 MITRE Engenuity. Approved for public release. Document number CT0005.
 # Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

 # http://www.apache.org/licenses/LICENSE-2.0

 # Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

 # This project makes use of ATT&CK®
 # ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/

 # Usage: .\insert_data_into_mssql.ps1 -csvPath C:\users.csv -Encrypt

 # Revision History:

 # ---------------------------------------------------------------------------

param ([String] $csvPath, [Switch] $Encrypt)

$filePath = $csvPath

#Checking if file exists. Quitting otherwise.
if (-not(Test-path -Path $filePath)) {

    write-host "Error, $filePath does not exist! Quitting.";
    exit 1;
}


<#

    .SYNOPSIS
        base64 encode a string and return the data

    .PARMATER $data
        string value to be encoded
#>
function b64enc($data) {
    $Bytes = [System.Text.Encoding]::Unicode.GetBytes($data)

    $EncodedText =[Convert]::ToBase64String($Bytes)

    return $EncodedText
}


<#
    .SYNOPSIS
       inserts data into MSSQL

    .PARAMETER $filepath
        specify path to CSV file to read data into.

#>
function insert_data($filePath) {
    [void] [Reflection.Assembly]::LoadWithPartialName("System.Security")
    $scope = [System.Security.Cryptography.DataProtectionScope]::CurrentUser
    Import-CSV $filePath -Header name,pass,desc | ForEach-Object {
        $plaintext = [System.Text.UTF8Encoding]::UTF8.GetBytes($_.pass)
        if ($Encrypt) {
            $ciphertext = [System.Security.Cryptography.ProtectedData]::Protect(
            $plaintext, $null, $scope )
            $b64_pass = [System.Convert]::ToBase64String($ciphertext)
        } else {
            $b64_pass = [System.Convert]::ToBase64String($plaintext)
        }

        $insert_query = "INSERT INTO Credentials VALUES('$($_.name)', '$($b64_pass)', '$($_.desc)')"

        $Command.CommandText = $insert_query

        write-host $Command.CommandText;
        $Command.ExecuteNonQuery()
    }

}


$serverName = hostname
$databaseName = "NetbnmBackup"
$tableName = "dbo.Credentials"

#Using local account Windows authentication
$Connection = New-Object System.Data.SQLClient.SQLConnection
$Connection.ConnectionString = "server='$serverName';database='$databaseName';trusted_connection=true;"
$Connection.Open()
$Command = New-Object System.Data.SQLClient.SQLCommand
$Command.Connection = $Connection

insert_data($filePath)

$Command.Connection.Close()
