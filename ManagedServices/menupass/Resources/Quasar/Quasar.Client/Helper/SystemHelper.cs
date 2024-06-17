using Quasar.Common.Helpers;
using Quasar.Common.Logger;
using System;
using System.Management;

namespace Quasar.Client.Helper
{
    public static class SystemHelper
    {
        /// <summary>
        /// Gets system uptime via WMI query.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1082: System Information Discovery
        /// </remarks>
        public static string GetUptime()
        {
            try
            {
                string uptime = string.Empty;

                using (ManagementObjectSearcher searcher = new ManagementObjectSearcher("SELECT * FROM Win32_OperatingSystem WHERE Primary='true'"))
                {
                    foreach (ManagementObject mObject in searcher.Get())
                    {
                        DateTime lastBootUpTime = ManagementDateTimeConverter.ToDateTime(mObject["LastBootUpTime"].ToString());
                        TimeSpan uptimeSpan = TimeSpan.FromTicks((DateTime.Now - lastBootUpTime).Ticks);

                        uptime = string.Format("{0}d : {1}h : {2}m : {3}s", uptimeSpan.Days, uptimeSpan.Hours, uptimeSpan.Minutes, uptimeSpan.Seconds);
                        break;
                    }
                }

                if (string.IsNullOrEmpty(uptime))
                    throw new Exception("Getting uptime failed");

                return uptime;
            }
            catch (Exception ex)
            {
                Logger.Error(String.Format("GetUptime exception: {0}", ex.ToString()));
                return string.Format("{0}d : {1}h : {2}m : {3}s", 0, 0, 0, 0);
            }
        }

        /// <summary>
        /// Gets computer name.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1082: System Information Discovery
        /// </remarks>
        public static string GetPcName()
        {
            return Environment.MachineName;
        }

        /// <summary>
        /// Looks for installed antivirus software via WMI query.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1518.001: Software Discovery: Security Software Discovery
        /// </remarks>
        public static string GetAntivirus()
        {
            try
            {
                string antivirusName = string.Empty;
                // starting with Windows Vista we must use the root\SecurityCenter2 namespace
                string scope = (PlatformHelper.VistaOrHigher) ? "root\\SecurityCenter2" : "root\\SecurityCenter";
                string query = "SELECT * FROM AntivirusProduct";

                using (ManagementObjectSearcher searcher = new ManagementObjectSearcher(scope, query))
                {
                    foreach (ManagementObject mObject in searcher.Get())
                    {
                        antivirusName += mObject["displayName"].ToString() + "; ";
                    }
                }
                antivirusName = StringHelper.RemoveLastChars(antivirusName);

                return (!string.IsNullOrEmpty(antivirusName)) ? antivirusName : "N/A";
            }
            catch (Exception ex)
            {
                Logger.Error(String.Format("GetAntivirus exception: {0}", ex.ToString()));
                return "Unknown";
            }
        }

        /// <summary>
        /// Looks for installed firewalls via WMI query.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1518.001: Software Discovery: Security Software Discovery
        /// </remarks>
        public static string GetFirewall()
        {
            try
            {
                string firewallName = string.Empty;
                // starting with Windows Vista we must use the root\SecurityCenter2 namespace
                string scope = (PlatformHelper.VistaOrHigher) ? "root\\SecurityCenter2" : "root\\SecurityCenter";
                string query = "SELECT * FROM FirewallProduct";

                using (ManagementObjectSearcher searcher = new ManagementObjectSearcher(scope, query))
                {
                    foreach (ManagementObject mObject in searcher.Get())
                    {
                        firewallName += mObject["displayName"].ToString() + "; ";
                    }
                }
                firewallName = StringHelper.RemoveLastChars(firewallName);

                return (!string.IsNullOrEmpty(firewallName)) ? firewallName : "N/A";
            }
            catch (Exception ex)
            {
                Logger.Error(String.Format("GetFirewall exception: {0}", ex.ToString()));
                return "Unknown";
            }
        }
    }
}
