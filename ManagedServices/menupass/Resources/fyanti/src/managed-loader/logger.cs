using System.IO;
using System.Security.Cryptography;
using System.Text;
using System;

namespace ManagedLoader
{
    public class Logger
    {
        static readonly object _logLock = new object();
        public static string LogPath = "";
        private static bool initialized = false;
        private static bool _encryptLogs = false;
        private static byte[] logEncKey =
        {
            0x0a, 0xa9, 0x2b, 0x53, 0xdf, 0x6d, 0x09, 0xe0,
            0xd1, 0x02, 0xb1, 0x9a, 0x9f, 0xa0, 0x27, 0x61,
            0x06, 0x43, 0xca, 0x31, 0x69, 0xd9, 0x79, 0xf3,
            0xee, 0x2c, 0x7a, 0x56, 0x7f, 0x87, 0x17, 0x05
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