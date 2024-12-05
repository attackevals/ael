# Lockbit Component Cleanup

## Stealbit Cleanup

To remove Stealbit you need to remove the binary and the config file.
You can do this using the `Remove-Item` PowerShell cmdlet.

```PowerShell
Remove-Item -Path C:\path\to\connhost.exe
Remove-Item -Path C:\path\to\SB.conf
```

## LockBit Cleanup

To undo LockBit encryption, run the following PowerShell command on a
domain-connected host as the same domain admin user, which will clean up each
host. Make sure to replace the target hosts and C2 address with the proper names
according to the target environment.

```PowerShell
$attackerC2 = "attacker c2 IP";
$cleanupHosts = @(
    "host1",
    "host2",
    "hostN"
);
$cleanupHosts | %{
    Invoke-Command -ComputerName $_ -ScriptBlock {
        Write-Host "[INFO] Performing cleanup on $(hostname)"

        # Re-enable and restart VSS service
        Write-Host "[INFO] Renabling and restarting VSS"
        sc.exe config vss start= auto;
        sc.exe start vss;

        # Determine which user key has the RSA information
        $userKeys = $(reg query "HKU");
        $rsaKey = "";
        if ([string]::IsNullOrWhitespace($userKeys)) {
            Write-Host "[ERROR] Failed to find user registry keys. Skipping decryption.";
        } else {
            $userKeys | ?{ -not [string]::IsNullOrWhitespace($_) } | %{
                $tempKey = "$_\Software\BlockBlit";
                if (reg.exe query "$tempKey" 2>$null) {
                    Write-Host "[INFO] Found LockBit RSA key at $tempKey";
                    $rsaKey = $tempKey;
                }
            }
        }
        if ($rsaKey) {
            # Run decryption binary
            Write-Host "[INFO] Downloading and executing decryption binary"
            curl "http://$attackerC2/lockbit_decryptor.exe" -o "C:\Users\Public\lockbit_decryptor.exe";
            C:\Users\Public\lockbit_decryptor.exe -reg "$rsaKey";
            if ($LASTEXITCODE -eq 0) {
                Write-Host "[INFO] Successfully ran decryption binary. Removing RSA keys from registry";
                if (reg.exe query "$rsaKey" 2>$null) {
                    Write-Host "[DEBUG] Deleting reg key $rsaKey";
                    reg delete "$rsaKey" /f;
                } else {
                    Write-Host "[ERROR] Failed to find and delete RSA registry keys.";
                }
            } else {
                Write-Host "[ERROR] Decryption binary failed with exit code $LASTEXITCODE";
            }
        } else {
            Write-Host "[ERROR] Failed to find LockBit RSA registry key. Skipping decryption.";
        }

        # Remove remaining registry entries
        if (reg.exe query "HKCR\.sk12uyqzk" 2>$null) {
            Write-Host "[DEBUG] Deleting reg key HKCR\.sk12uyqzk";
            reg delete HKCR\.sk12uyqzk /f;
        } else {
            Write-Host "[DEBUG] Reg key HKCR\.sk12uyqzk not found. Skipping";
        }
        if (reg.exe query "HKCR\sk12uyqzk\DefaultIcon" 2>$null) {
            Write-Host "[DEBUG] Deleting reg key HKCR\sk12uyqzk\DefaultIcon";
            reg delete HKCR\sk12uyqzk\DefaultIcon /f;
        } else {
            Write-Host "[DEBUG] Reg key HKCR\sk12uyqzk\DefaultIcon not found. Skipping";
        }
        
        # Delete files
        Write-Host "[INFO] Deleting dropped files";
        $artifacts = @(
            "C:\ProgramData\83426913-9751-4a75-b159-65f7bf232961.exe" # main lockbit binary
            "C:\Windows\83426913-9751-4a75-b159-65f7bf232961.exe" # main binary (via psexec)
            "C:\windows\83426913-9751-4a75-b159-65f7bf232961.bat" # alternative step batch script (via psexec)
            "C:\ProgramData\83426913-9751-4a75-b159-65f7bf232961.bat" # alternative step batch script
            "C:\ProgramData\83426913-9751-4a75-b159-65f7bf232961.min.exe" # alternative step binary
            "C:\Windows\Temp\min.exe" # alternative step binary (via propagation)
            "C:\Users\Public\lockbit_decryptor.exe" # cleanup binary
            "C:\Windows\Temp\381jwk.tmp" # self-deletion helper
            "$env:ProgramData\sk12uyqzk.bmp" # wallpaper resource
            "$env:ProgramData\sk12uyqzk.ico" # icon resource
        );
        
        $artifacts | %{
            if (Test-Path "$_") {
                Write-Host "[DEBUG] Deleting file $_";
                Remove-Item -Force "$_";
            } else {
                Write-Host "[DEBUG] File $_ not found. Skipping";
            }
        };
    }
}
```
