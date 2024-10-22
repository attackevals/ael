# ExMatter

## Overview

After BlackCat deploys InfoStealer and successfully recovers credentials from the SQL server, the credentials are then used to remotely deploy ExMatter via a PowerShell script.
Once ExMatter is deployed, it will build out a queue of files for exfiltration using a combination of inclusion/exclusion lists and file time metadata.
After the list is created, ExMatter will package the files into zips and exfiltrate them to a remote SFTP server.
When exfiltration is finished, ExMatter will destroy itself by spawning a PowerShell script to overwrite some bytes of its executable, and then delete the executable.

# Troubleshooting

## Logging

The ExMatter log file will be created in the same folder where the ExMatter executable is.
The name of the log file will be `EMlog.txt` and will be encrypted using AES-256-CBC with the key `0370dd5addcd980e8f4b424c92d8049e99c7c7c5d09eedfcc58f6abca9e72f99`.
To decrypt an encrypted log file, use the `alphv_blackcat/Resources/log_decryptor/aes_base64_log_decryptor.py` Python utility and pass in the correct cipher and AES key:

```
python3 aes_base64_log_decryptor.py -i path/to/log/file -o path/to/output/file --aes-256-cbc -k 0370dd5addcd980e8f4b424c92d8049e99c7c7c5d09eedfcc58f6abca9e72f99
```

Alternatively, you can use the `aes-tool.exe` (built with the cmake pipeline) with the following command:

- `aes-tool.exe -d <PATH TO LOG FILE>`

If you run into permission issues for SFTP, ensure that the SFTP server is set up with the expected user credentials,
and that the SFTP user has write access to its SFTP home directory.

# Functionality

## [Scan logical drives](README.md#scan-logical-drives)[^1][^2][^3][^4][^6]

- Discover logical drives using `DriveInfo.GetDrives()`.
- Log drive information.
- Populate list of drives to investigate.

## [File discovery](README.md#file-discovery)[^1][^2][^3][^4][^6][^7]

Recursively parse drives for files of interest, build a list of files found:

- Minimum file size: 4KB
- Maximum file size: 64MB
- Ignore files that have the `System` or `Temporary` file attribute.
- Ignore directories that are symlinks or junctions.
- Ignore these directories:

| | | |
|-|-|-|
| `AppData\Local\Microsoft\` | `C:\Documents and Settings\` | `C:\ProgramData\` |
| `AppData\Local\Packages\` | `C:\PerfLogs\` | `C:\Users\All Users\Microsoft\` |
| `AppData\Roaming\Microsoft\` | `C:\Program Files\` | `C:\Windows\` |
| `C:\$Recycle.Bin\` | `C:\Program Files (x86)\` | |

- Only include files with these extension:

| | | | | | | | | |
|-|-|-|-|-|-|-|-|-|
| `.bmp` | `.doc` | `.docx` | `.dwg` | `.ipt` | `.jpeg` | `.jpg` | `.msg` | `.pdf` |
| `.png` | `.pst` | `.rdp` | `.rtf` | `.sql` | `.txt` | `.xls` | `.xlsx` | `.zip` |

## [Stage files](README.md#stage-files)[^1][^6][^7][^5]

- Sort the files from most to least recent last write time.
- Compress the files into zips, 100 files per zip, processing the most recently written to files first.

## [Exfiltration](README.md#exfiltration)[^1][^6][^7][^5]

- Upload the zips to the SFTP server in the order that they were created. Each time ExMatter is run, it will create a unique upload directory
on the remote SFTP server to store the exfiltrated archives. The upload directory name format is local hostname + timestamp (e.g. `victimmachine20231129131516`).
- [SSH.NET](https://github.com/sshnet/SSH.NET/) is used to perform SFTP actions.

## [Self-destruction](README.md#self-destruction)[^2][^7][^8]

Spawn a PowerShell process that:

- Stops ExMatter process.
- Overwrites first 65535 bytes of ExMatter executable.
- Deletes ExMatter executable.

```PowerShell
powershell.exe -WindowStyle Hidden -C $path = '[FILEPATH_OF_THE_EXECUTING_SAMPLE]';
Get-Process | Where-Object {$_.Path -like $path} | Stop-Process -Force;
[byte[]]$arr = new-object byte[] 65536;
Set-Content -Path $path -Value $arr;
Remove-Item -Path $path;
```

# Obfuscation

ExMatter uses [ConfuserEx](https://github.com/mkaring/ConfuserEx) for obfuscation[^5].
The obfuscated binary will be output to `bin/Release/ExMatter.exe`, and the settings used to obfuscate are [here](ExMatter/ConfuserSettings.crproj.in).
You can use the [standalone version of ConfuserEx](https://github.com/mkaring/ConfuserEx/releases) to obfuscate ExMatter with this `.crproj` file with slight modifications.

# Build

Built exe files are in `/bin/`:

- `Debug/ExMatter.exe`: ExMatter built in debug mode
- `Release/ExMatter.exe`: ExMatter built in release mode
- `aes-tool.exe`: Encrypted log decryptor

ExMatter uses CMake to build.
CMake will pull down [ConfuserEx](https://github.com/mkaring/ConfuserEx) and [ILRepack](https://github.com/Alexx999/il-repack/) from github to use post-build.
ExMatter's post-build actions are as follows:

- Build ILMerge
- Run ILMerge on ExMatter to merge the SSH.NET dll into ExMatter
- Run Confuser on ExMatter and move the finished binary to the folder in `/bin/` depending on build configuration

# Testing

ExMatter was tested with the included `test.ps1` script, which creates "good" files for ExMatter to find and "bad" files that ExMatter shouldn't find.
The SFTP server IP and credentials are hard coded, so those should be changed to reflect your environment when testing.
`test.ps1` is intended to be run in a high integrity context, though this is not requied.

## Debug

ExMatter has a built-in debug mode when built with the `ExMatter-debug` configuration defined in `CMakePresets.json`.
This mode uses compiler directives to disable encrypted logging, only target files output from `test.ps1`, remove files from the SFTP server after upload, and disable the powershell destructor.

## aes-tool and Logging

ExMatter includes a small executable to decrypt its encrypted logs.
ExMatter creates its log file where its exe is located.
Use the following command to decrypt the logs:

- `aes-tool.exe -d <PATH TO LOG FILE>`

You can also run the aes-tool interactively.

## SFTP Setup

``` bash
# create and configure ftp root dir
sudo mkdir -p /ftproot
sudo chmod 755 /ftproot

# create and configure ftp user and group (shell needed for FTP access in case of alternative steps)
sudo groupadd sftp_users
sudo useradd -g sftp_users -s /bin/bash sftp_user
sudo passwd sftp_user

# create and configure ftp user dir
sudo mkdir -p /ftproot/sftp_user
sudo chown -R root:sftp_users /ftproot
sudo chown -R sftp_user:sftp_users /ftproot/sftp_user

# add the following to /etc/ssh/sshd_config:
Match Group sftp_users
 ChrootDirectory /ftproot
 ForceCommand internal-sftp -d /%u

# apply configurations
sudo systemctl restart sshd

# test
sftp sftp_user@<SERVER_IP>
```

# Alternative PowerShell Script

A [PowerShell version](../../payloads/ExMatterBackup.ps1) of ExMatter was created in case ExMatter fails to run. Major deviations between the two versions are as follows:

- Use the PowerShell command `Compress-Archive` to create the zip files instead of the C# library
- Use the PowerShell command `ftp -s` to upload zips to the ftp site instead of the SSH.NET library
- Use a ftp server instead of an sftp server so we can make use of the -s flag and for easier authentication
- No debug mode since we cannot use compiler directives

## FTP Setup

``` bash
# vsftpd was used when testing the backup script
sudo apt-get install vsftpd

# modify the vsftpd config file
sudo nano /etc/vsftpd.conf
# append the following:
local_enable=YES
write_enable=YES

# apply configurations
sudo systemctl restart vsftpd

# test
ftp
ftp> open <SERVER_IP>

```

# CTI References

[^1]: <https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool>
[^2]: <https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration>
[^3]: <https://www.cyderes.com/blog/threat-advisory-exmatter-data-extortion/>
[^4]: <https://www.netskope.com/blog/blackcat-ransomware-tactics-and-techniques-from-a-targeted-attack>
[^5]: <https://www.bleepingcomputer.com/news/security/blackcat-ransomware-s-data-exfiltration-tool-gets-an-upgrade/>
[^6]: <https://stairwell.com/news/threat-research-report-exmatter-future-of-data-extortion/>
[^7]: <https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps>
[^8]: <https://securityintelligence.com/posts/blackcat-ransomware-levels-up-stealth-speed-exfiltration/>
