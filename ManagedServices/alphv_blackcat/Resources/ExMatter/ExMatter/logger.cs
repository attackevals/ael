using System;
using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace ExMatter
{
    public class Logger
    {
        static readonly object _logLock = new object();
        public static string LogPath = "";
        private static bool initialized = false;
        private static bool _encryptLogs = false;
        private static byte[] logEncKey =
        {
            0x03, 0x70, 0xdd, 0x5a, 0xdd, 0xcd, 0x98, 0x0e,
            0x8f, 0x4b, 0x42, 0x4c, 0x92, 0xd8, 0x04, 0x9e,
            0x99, 0xc7, 0xc7, 0xc5, 0xd0, 0x9e, 0xed, 0xfc,
            0xc5, 0x8f, 0x6a, 0xbc, 0xa9, 0xe7, 0x2f, 0x99
        };

        public static void InitializeLogger(string logPath, bool encryptLogs = false)
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
            string toWrite = _encryptLogs ? Encryption.SimpleEncryptAndEncode(logEncKey, formatted) : formatted;
            lock (_logLock)
            {
                File.AppendAllText(path, toWrite + "\n");
            }
        }
    }
}