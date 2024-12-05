 # ---------------------------------------------------------------------------
 # xor_file.ps1 - XOR-encrypts a given file with the given key and writes the
 #                ciphertext to the specified output file.

 # This project makes use of ATT&CK®
 # ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 
 
 # Usage: powershell .\xor_file.ps1 -key [KEY HEX STRING] -infile [INPUT FILE PATH] -outfile [OUTPUT FILE PATH]
 
 # --------------------------------------------------------------------------- 

 <#
.Description

XOR-encrypts a given file with the given key and writes the ciphertext to the specified output file.

.Parameter key

XOR key as a hex string. Ex: "07d063c42aedeacb6c3ff49960f964c5"

.Parameter infile

Path to input file.

.Parameter outfile

Path to output file.

#>

param(
    [Parameter(Mandatory=$true)][String]$key,
    [Parameter(Mandatory=$true)][String]$infile,
    [Parameter(Mandatory=$true)][String]$outfile
)

Write-Host "[INFO] Key: $key";
Write-Host "[INFO] Input File: $infile";
Write-Host "[INFO] Output File: $outfile";

if (!(Test-Path "$infile")) {
    Write-Host "[ERROR] Input file $infile does not exist.";
    exit 1;
}

$key_bytes = [byte[]] -split ($key -replace '..', '0x$& ')
$key_len = $key_bytes.count;
if ($key_len -eq 0) {
    Write-Host "[ERROR] Cannot use empty key.";
    exit 2;
}

Write-Host "[DEBUG] Reading input file $infile.";
$file_bytes = [System.IO.File]::ReadAllBytes("$infile");
$num_bytes = $file_bytes.count;
Write-Host "[DEBUG] Read $num_bytes bytes.";
if ($file_bytes.count -eq 0) {
    Write-Host "[INFO] Empty input file. Exiting.";
    exit 0;
}

Write-Host "[DEBUG] Encrypting file bytes.";
for($i=0; $i -lt $num_bytes; $i++) {
    $file_bytes[$i] = $file_bytes[$i] -bxor ($key_bytes[$i % $key_len]);
}

Write-Host "[DEBUG] Writing encrypted bytes to output file $outfile.";
Set-Content "$outfile" -Value $file_bytes -Encoding Byte;