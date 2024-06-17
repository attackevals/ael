using Quasar.Common.Cryptography;
using System;
using System.IO;

namespace Quasar.Common.Logger
{
    public class Logger
    {
        static readonly object _logLock = new object();
        public static string LogPath = "";
        private static bool initialized = false;
        private static bool _encryptLogs = false;
        private static byte[] logEncKey = {
            0xcc, 0x4a, 0xcd, 0xe9, 0x28, 0x6d, 0x68, 0xe6,
            0xa5, 0x40, 0xc9, 0x35, 0x76, 0xd2, 0x16, 0x91,
            0x06, 0xd8, 0xaf, 0xfd, 0xa5, 0x12, 0x3f, 0x3d,
            0xfb, 0xdf, 0x9e, 0xb2, 0xfd, 0x32, 0x78, 0xe6
        };

        public static void InitializeLogger(string logPath, bool encryptLogs=false)
        {
            LogPath = logPath;
            initialized = true;
            _encryptLogs = encryptLogs;
        }

        public static void Debug(string logMessage)
        {
            if (!initialized) return;
            try
            {
                LogMessage("DEBUG", logMessage, LogPath);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        public static void Info(string logMessage)
        {
            if (!initialized) return;
            try
            {
                LogMessage("INFO", logMessage, LogPath);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        public static void Warn(string logMessage)
        {
            if (!initialized) return;
            try
            {
                LogMessage("WARN", logMessage, LogPath);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        public static void Error(string logMessage)
        {
            if (!initialized) return;
            try
            {
                LogMessage("ERROR", logMessage, LogPath);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        static void LogMessage(string verbosity, string msg, string path)
        {
            if (!initialized) return;
            string formatted = string.Format(
                "[{0}] {1}: {2}",
                verbosity,
                DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"),
                msg
            );
            string toWrite = _encryptLogs ? Aes256.SimpleEncryptAndEncode(logEncKey, formatted) : formatted;
            lock (_logLock)
            {
                File.AppendAllText(path, toWrite + "\n");
            }
        }
    }
}
