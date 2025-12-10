 # ---------------------------------------------------------------------------
 # create_test4_zip.ps1 - Creates zip file for protections test 4 using the
 # provided executable.

 # This project makes use of ATT&CK®
 # ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/

 # Usage: powershell -file create_test4_zip.ps1 -TestExec TEST4_EXECUTABLE_FILE -PdfFile PDF_FILE -OutputFile OUTPUT_ZIP_FILE

 # ---------------------------------------------------------------------------


<#
.Description

Creates zip file for protections test 4 using the provided executable.

.Parameter TestExec

Path to test 4 executable file to include in zip

.Parameter PdfFile

Path to test 4 PDF file to include in zip

.Parameter OutputFile

Path to output zip file

#>

Param(
    [Parameter(Mandatory=$true)][String]$TestExec,
    [Parameter(Mandatory=$true)][String]$OutputFile,
    [Parameter(Mandatory=$true)][String]$PdfFile
)

[Reflection.Assembly]::LoadWithPartialName("System.IO.Compression");
[Reflection.Assembly]::LoadWithPartialName("System.IO.Compression.FileSystem");

$prot4Zip = "$OutputFile";
$prot4StagingMapping = @(
    @{Path = "$TestExec"; Folder = "Appendix I"},
    @{Path = "$PdfFile"; Folder = "Appendix II"}
)

$fileStream = [System.IO.File]::Open($prot4Zip, 'Create')
$zipArchive = [System.IO.Compression.ZipArchive]::new($fileStream, 'Create')

foreach ($item in $prot4StagingMapping) {
    $entry = $zipArchive.CreateEntry("$($item.Folder)\$([System.IO.Path]::GetFileName($item.Path))")
    $entryStream = $entry.Open()
    [System.IO.File]::OpenRead($item.Path).CopyTo($entryStream)
    $entryStream.Close()
}
$zipArchive.Dispose(); $fileStream.Dispose()
