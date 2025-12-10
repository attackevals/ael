 # ---------------------------------------------------------------------------
 # embed_payload.ps1 - Reads in payload file, XOR-encrypts the data using
 # a randomly generated key, XOR-encrypts the key, and embeds the payload and
 # encrypted key into header template

 # This project makes use of ATT&CK®
 # ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/

 # Usage: powershell -file embed_payload.ps1 -Template HEADER_TEMPLATE -InputFile PAYLOAD_FILE -OutputFile OUTPUT_FILE

 # ---------------------------------------------------------------------------


<#
.Description

Encypts and embeds provided input file as a byte array in header template.
Encryption key is randomly generated, encrypted, and also embedded.

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
    [Parameter(Mandatory=$true)][String]$OutputFile,
    [Parameter(Mandatory=$true)][String]$EmbedNamespace
)

function XOR-Encrypt {
    Param(
        [Parameter(Mandatory=$true)][Byte[]] $Plaintext,
        [Parameter(Mandatory=$true)][Byte[]] $Key,
        [Parameter(Mandatory=$true)][int] $Offset
    )

    $keyLen = $Key.Count;
    $plaintextLen = $Plaintext.Count;
    for ($i = 0; $i -lt $plaintextLen; $i++) {
        $Plaintext[$i] = $Plaintext[$i] -bxor $Key[($i + $Offset) % $keyLen];
    }
}

# Converts byte array to comma-separated hex string to include in
# the header template, with 16 bytes per line.
function BytesToHeaderString {
    Param(
        [Parameter(Mandatory=$true)][Byte[]] $InputBytes
    )

    $embeddedBytesStr = ($InputBytes | %{
        # Line breaks every 16 bytes
        if (($count % 16) -eq 0) {
            "`n    0x{0:X2}" -f $_
        } else {
            "0x{0:X2}" -f $_
        }
        $count = $count + 1;
    }) -join ",";
    $embeddedBytesStr = $embeddedBytesStr + "`n";
    $embeddedBytesStr;
}

echo "[INFO] Embedding $InputFile into template file $Template to create $OutputFile. Using namespace: $EmbedNamespace";

# Read input and template files
$inputBytes = [System.IO.File]::ReadAllBytes("$InputFile");
$templateText = Get-Content -Raw "$Template";

# Generate key
$payloadKey = [Byte[]]::new(32);
$rng = [System.Security.Cryptography.RandomNumberGenerator]::Create();
$rng.GetBytes($payloadKey);

$payloadKeyStr = ($payloadKey | %{"{0:X2}" -f $_}) -join "";
echo "Generated key: $payloadKeyStr";

# Triple-XOR encrypt payload
XOR-Encrypt -Plaintext $inputBytes -Key $payloadKey -Offset 0;
XOR-Encrypt -Plaintext $inputBytes -Key $payloadKey -Offset 1;
XOR-Encrypt -Plaintext $inputBytes -Key $payloadKey -Offset 7;

# Encrypt XOR key
[byte[]]$encKey = @([byte]0x3F);
XOR-Encrypt -Plaintext $payloadKey -Key $encKey -Offset 0;

# Generate array text
$count = 0;
$embeddedBytesStr = BytesToHeaderString -InputBytes $inputBytes;

# Generate key text
$keyStr = BytesToHeaderString -InputBytes $payloadKey;

# Embed encrypted payload bytes
$outputText = $templateText.Replace("PAYLOAD_SIZE", $inputBytes.Count.ToString()).Replace("PAYLOAD_BYTES", "$embeddedBytesStr");

# Embed key
$outputText = $outputText.Replace("KEY_BYTES", "$keyStr");

# Set namespace
$outputText = $outputText.Replace("EMBEDDED_NAMESPACE", "$EmbedNamespace");

$outputText | Out-File -FilePath "$OutputFile";
