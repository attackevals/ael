<#

Recycle Bin File Generation Script

    - Generates random number of files and directories (can set the minimum and maximum) with random names and data,

#>

$dir = 'C:\Users\Public'
$numFiles = Get-Random -Minimum 10 -Maximum 25

    # moveToRecycleBin - moves the file specified at $itemPath to the recycle bin
    $moveToRecycleBin = {
        param($itemPath)
        $sh = new-object -comobject "Shell.Application"
        $ns = $sh.Namespace(0).ParseName($itemPath)
        $ns.InvokeVerb("delete")
    }

    $rng = [System.Security.Cryptography.RNGCryptoServiceProvider]::new();
    0..$numFiles | ForEach-Object {
        # Generate random dir names
        $name = (-join ((48..57) + (97..122) | Get-Random -Count 10 | ForEach-Object {[char]$_}))
        $split =  5, 7, 10 | Get-Random
        $p = $(Join-Path $dir (($name -split '(.{'+ [regex]::escape($split) + '})'|Where-Object{$_}) -join '/')) + ".txt"
        $d = [System.IO.Path]::GetDirectoryName($p)

        # Create the file/dir and generate the random content
        New-Item -ItemType Directory -Path $d -ErrorAction SilentlyContinue | Out-Null
        $size = 1023 * (Get-Random -Minimum 1 -Maximum 512);
        $contents = [Byte[]]::new($size);
        $rng.GetBytes($contents);
        Set-Content -Force -Path $p -Value $contents

        # Move the files / directories to the recycle bin
        if ($dir -ne $d) {
            & $moveToRecycleBin $d
        } else {
            & $moveToRecycleBin $p
        }
    };
