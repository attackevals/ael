# ---------------------------------------------------------------------------
# build_shellcode.ps1 - Build SigLoader shellcode for testing purposes

# Copyright 2024 MITRE Engenuity. Approved for public release. Document number CT0005.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: 
#   Generating just the shellcode string: .\build_shellcode.ps1 -dllPath C:\Path\To\DLL.dll -funcName ExportedFunctionName -outFileName payload
#   Generating a cpp array for testing: .\build_shellcode.ps1 -dllPath C:\Path\To\DLL.dll -funcName ExportedFunctionName -outFileName payload -cpp $True

# Revision History:

# --------------------------------------------------------------------------- 

param(
	[String] $dllPath,
	[String] $funcName,
	[String] $outFileName,
	[Switch] $cpp = $False
)

# Clone sRDI repo and import ConvertTo-Shellcode module
Write-Host "Cloning the sRDI repo and importing CovertTo-Shellcode module..." -NoNewLine

if (Test-Path -Path 'C:\Users\Public\sRDI') {
	Write-Host "sRDI Repo already downloaded."
}
else {
	git clone https://github.com/monoxgas/sRDI.git C:\Users\Public\sRDI *>$null
}
git -C C:\Users\Public\sRDI checkout 5690685aee6751d0dbcf2c50b6fdd4427c1c9a0a *>$null
import-module C:\Users\Public\sRDI\PowerShell\ConvertTo-Shellcode.ps1 -Force

Write-Host " Success!"

# Perform error checking on build commands
function errCheck {

	$err = Get-Content .\err.txt
	Remove-Item .\err.txt
	if ($err -ne $null) {
		Write-Host "Error running command: `n`n$err"
		Write-Host "`nExiting the build script"
		exit
	}

}

# Convert specified dll to shellcode
# Params: dllPath (path to the compiled DLL to be converted), functionName (name of the exported function), outFile (output file for shellcode)
function convertToShellcode($dllPath, $functionName, $outFile) {

	$sc = ConvertTo-Shellcode -File $dllPath -FunctionName $functionName
	
	if ($cpp) {
		$sc2 = $sc | % { write-output ([System.String]::Format('0x{0:X2}', $_)) }
		$sc3 = $sc2 -join ", "

		$length = $sc2.Length
		$array = "unsigned char shellcode[$length] = {$sc3};"
		[io.File]::WriteAllText("$PSScriptRoot\..\payload\$outFile.txt", $array)
	}
	else {
		Write-Host "$PSScriptRoot\..\payload\$outFile"
		Set-Content "$PSScriptRoot\..\payload\$outFile" -Value $sc -Encoding Byte
	}

}


convertToShellcode $dllPath $funcName $outFileName

