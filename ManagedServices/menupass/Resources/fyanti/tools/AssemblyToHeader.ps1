<#
    .SYNOPSIS
        FYAnti helper script which embeds a .NET assembly into a C++ header.
    
    .DESCRIPTION
        The FYAnti native loader bootstraps the .NET runtime into an unmanaged
        process and executes a .NET assembly. This assembly is embedded within
        the native code as a byte array. This is a convenience script which
        consumes a .NET assembly and generates a C++ header containing the
        assembly as a byte array.

    .PARAMETER FirstParameter
        Assembly accepts the path to a .NET assembly

    .PARAMETER SecondParameter
        Outfile specifies the path of the generated header file.

    .EXAMPLE
        AssemblyToHeader.ps1 -Assembly path/to/assembly.dll -Outfile assembly.hpp

    .NOTES
        This is a modification of the script contained within the following
        Proof of Concept:
        
        https://gist.github.com/Arno0x/386ebfebd78ee4f0cbbbb2a7c4405f74
#>

param(
[string]$Assembly,
[string]$Outfile
)

$template = @'
// This is a header template that was instantiated at build time by the script
// AssemblyToHeader.ps1. The original assembly path was:
//
// @ASSEMBLY_PATH@

namespace embedded {

inline const unsigned int assembly_length{ @ASSEMBLY_LENGTH@ };
inline const unsigned char assembly[]{
    @ASSEMBLY_BYTES@
};

} // namespace embedded
'@

$bytes = Get-Content $Assembly -Encoding Byte
$hex = [System.Text.StringBuilder]::new($bytes.Length * 4)
ForEach($byte in $bytes) { $hex.AppendFormat("0x{0:x2}, ", $byte) | Out-Null }

# Remove trailing comma
$hex.Length--
$hex.Length--

$template = $template -replace '@ASSEMBLY_PATH@', $Assembly
$template = $template -replace '@ASSEMBLY_LENGTH@', $bytes.Length
$template = $template -replace '@ASSEMBLY_BYTES@', $hex.ToString()

$template | Out-File -FilePath $Outfile
