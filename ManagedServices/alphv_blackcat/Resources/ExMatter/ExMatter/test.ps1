# ---------------------------------------------------------------------------
# test.ps1 - creates files for ExMatter to analyze

# Copyright 2024 MITRE Engenuity. Approved for public release. Document number CT0005.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: ./test.ps1

# Revision History:

# --------------------------------------------------------------------------- 

$MIN_BYTES = 4096;
$MAX_BYTES = 67108864;
$GOOD_EXT_ARR = @(".bmp",".doc",".docx",".dwg",".ipt",".jpeg",".jpg",".msg",".pdf",".png",".pst",".rdp",".rtf",".sql",".txt",".xls",".xlsx",".zip");
$BAD_DIR_NAME = @("$Env:userprofile\AppData\Local\Microsoft\","$Env:userprofile\AppData\Local\Packages\","$Env:userprofile\AppData\Roaming\Microsoft\","C:\`$Recycle.Bin\","C:\PerfLogs\","C:\Program Files\","C:\Program Files (x86)\","C:\ProgramData\","C:\Users\All Users\Microsoft\","C:\Windows\");
$GOOD_DIR_NAME = "GoodDir";
$GOOD_FILE_NAME = "GoodFile";
$BAD_FILE_NAME = "BadFile";
$NUM_GOOD_DIRS = 5;
$NUM_BAD_DIRS = 5;
$NUM_FILES_IN_DIR = 3;

$StartDir = Get-Location;
New-Item -Path $StartDir -Name "TestingDir" -ItemType "directory";

# store the files we created and their sizes
$testFiles = @{};

# make test directories with files that should and should not get picked up
# directories that should be looked in
for ($i = 0; $i -lt $NUM_GOOD_DIRS; $i++) {
	$DirName = ($GOOD_DIR_NAME + $i.ToString());
	New-Item -Path "$StartDir/TestingDir/" -Name $DirName -ItemType "directory";
	$DirPath = "$StartDir/TestingDir/$DirName/"

	# files that should get picked up
	for ($j = 0; $j -lt $NUM_FILES_IN_DIR*21; $j++) {
		$name = $GOOD_FILE_NAME + $j.ToString() + $GOOD_EXT_ARR[(Get-Random -Minimum 0 -Maximum $GOOD_EXT_ARR.Length)];
		$file = New-Object System.IO.FileStream "$DirPath$name", Create, ReadWrite;
		$file.SetLength((Get-Random -Minimum $MIN_BYTES -Maximum $MAX_BYTES));
		$testFiles.Add($File.Name.ToString(), $file.Length);
		$file.Close();
		$fileDate = Get-Item "$DirPath$name"
		$fileDate.LastWriteTime = (Get-Date).AddDays((Get-Random -Minimum -10 -Maximum 0));
	}

	# files that should not get picked up: file size
	for ($j = 0; $j -lt $NUM_FILES_IN_DIR; $j++) {
		$name = $BAD_FILE_NAME + $j.ToString() + $GOOD_EXT_ARR[(Get-Random -Minimum 0 -Maximum $GOOD_EXT_ARR.Length)];
		$file = New-Object System.IO.FileStream "$DirPath$name", Create, ReadWrite;
		$file.SetLength(2048);
		$testFiles.Add($File.Name.ToString(), $file.Length);
		$file.Close();
	}

	# files that should not get picked up: file ext
	for ($j = 0; $j -lt $NUM_FILES_IN_DIR; $j++) {
		$name = $BAD_FILE_NAME + $j.ToString() + ".bin";
		$file = New-Object System.IO.FileStream "$DirPath$name", Create, ReadWrite;
		$file.SetLength($MIN_BYTES*2);
		$testFiles.Add($File.Name.ToString(), $file.Length);
		$file.Close();
	}

}

# make test files in directories that should not get looked in
# directories that should not be looked in
for ($i = 0; $i -lt $NUM_BAD_DIRS; $i++) {
	$DirPath = $BAD_DIR_NAME[(Get-Random -Minimum 0 -Maximum $BAD_DIR_NAME.Length)];

	#files that should get picked up
	for ($j = 0; $j -lt $NUM_FILES_IN_DIR; $j++) {
		$name = $BAD_FILE_NAME + $j.ToString() + $GOOD_EXT_ARR[(Get-Random -Minimum 0 -Maximum $GOOD_EXT_ARR.Length)];
		$file = New-Object System.IO.FileStream "$DirPath$name", Create, ReadWrite;
		$file.SetLength((Get-Random -Minimum $MIN_BYTES -Maximum $MAX_BYTES));
		$testFiles.Add($File.Name.ToString(), $file.Length);
		$file.Close();
	}

}

$testFiles.GetEnumerator() | ForEach-Object {
	$file = Get-Item $_.Key
	Write-Host "$($_.Key): $($_.Value) bytes $($file.LastWriteTime)";
}

Write-Host "";
$nothing = Read-Host "Testing files and directories created. Enter any key to remove.";

Remove-Item -Path "./TestingDir" -Force -Recurse;
# loop through the testFilesArr and remove anything we created
foreach ($file in $testFiles.Keys) {
	Remove-Item -Path $file -Force -ErrorAction SilentlyContinue;
}
