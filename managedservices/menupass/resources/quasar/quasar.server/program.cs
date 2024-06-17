using Quasar.Common.Logger;
using Quasar.Server.Forms;
using Quasar.Server.Models;
using Quasar.Server.RestApi;
using Mono.Options;
using System;
using System.Net;
using System.Windows.Forms;

namespace Quasar.Server
{
    internal static class Program
    {
        [STAThread]
        private static void Main(string[] args)
        {
            string restAddr = "*";
            int restPort = 8888;
            string certFilePath = "";
            var options = new OptionSet()
                .Add("ip=", v => restAddr = v)
                .Add("p|port=", (int v) => restPort = v)
                .Add("c|cert=", (string v) => certFilePath = v);
            options.Parse(args);

            Logger.InitializeLogger(".\\QuasarServerLogs.txt");
            Logger.Info("Starting up Quasar Server.");
            Logger.Info("REST server bind address set to " + (restAddr == "*" ? "all interfaces" : restAddr));
            Logger.Info("REST server listen port set to " + restPort);
            if (certFilePath.Length > 0) 
            {
                Logger.Info("Using cert file " + certFilePath);
                Settings.CertificatePath = System.IO.Path.GetFullPath(certFilePath);
            }
            
            // enable TLS 1.2
            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;
            Logger.Debug("Enabled TLS 1.2");

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            FrmMain serverMainForm = new FrmMain(true);

            // Start REST API server
            if (!RestServer.StartRestServer(serverMainForm, restAddr, restPort))
            {
                Logger.Error("Failed to start REST API server.");
                return;
            }
            Logger.Info("Started REST API server.");

            Logger.Debug("Running main form.");
            Application.Run(serverMainForm);

            RestServer.StopRestServer();
        }
    }
}
