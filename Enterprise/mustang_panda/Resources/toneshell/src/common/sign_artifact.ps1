 # ---------------------------------------------------------------------------
 # sign_artifact.ps1 - Creates a certificate using the provided parameters
 # and signs the target binary.
 # Created certificate is protected by the user that runs the script.

 # This project makes use of ATT&CK®
 # ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/

 # Usage: powershell -file sign_artifact.ps1 -Target ARTIFACT_TO_SIGN -CertSubject CERT_SUBJECT -CertExportPath EXPORT_PATH -CertDnsName DNS_NAME

 # ---------------------------------------------------------------------------

param(
    [Parameter(Mandatory=$true)][String]$Target,
    [Parameter(Mandatory=$true)][String]$CertSubject,
    [Parameter(Mandatory=$true)][String]$CertExportPath,
    [Parameter(Mandatory=$true)][String]$CertDnsName
)

Write-Output "[DEBUG] Signing artifact $Target";

$params = @{
    Subject = "$CertSubject"
    Type = 'CodeSigning'
    CertStoreLocation = 'Cert:\CurrentUser\My'
    HashAlgorithm = 'sha256'
    DnsName = "$CertDnsName"
    NotBefore = (Get-Date).AddMonths(-6)
    NotAfter = (Get-Date).AddMonths(18)
};
$cert = New-SelfSignedCertificate @params;
if ($cert) {
    # Sign artifact
    Write-Output $cert;
    Set-AuthenticodeSignature -FilePath "$Target" -Certificate $cert;

    if ((Get-AuthenticodeSignature "$Target").SignerCertificate) {
        Write-Output "[DEBUG] Signed $Target";
    } else {
        Write-Output "[ERROR] Failed to sign $Target";
        exit 7;
    }

    # Export certificate
    Write-Output "[DEBUG] Exporting certificate";
    Export-PfxCertificate -Cert $cert -FilePath "$CertExportPath" -ProtectTo "$(whoami)";
    if (Test-Path "$CertExportPath") {
        Write-Output "[DEBUG] Exported certificate to $CertExportPath";
    } else {
        Write-Output "[ERROR] Failed to export certificate.";
        exit 8;
    }
} else {
    Write-Output "[ERROR] Failed to create certificate.";
    exit 6;
}
