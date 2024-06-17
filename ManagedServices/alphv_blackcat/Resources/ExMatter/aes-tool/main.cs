using System;
using System.IO;
using ExMatter;

namespace AESTool
{
    public class AESTool
    {
        private static byte[] logEncKey =
        {
            0x03, 0x70, 0xdd, 0x5a, 0xdd, 0xcd, 0x98, 0x0e,
            0x8f, 0x4b, 0x42, 0x4c, 0x92, 0xd8, 0x04, 0x9e,
            0x99, 0xc7, 0xc7, 0xc5, 0xd0, 0x9e, 0xed, 0xfc,
            0xc5, 0x8f, 0x6a, 0xbc, 0xa9, 0xe7, 0x2f, 0x99
        };

        public static void Main(string[] args)
        {
            foreach (string arg in args) { Console.WriteLine(arg); }

            if (args.Length == 0)
            {
                Interactive();
                return;
            }

            if (!(args[0] == "-d"))
            {
                PrintUsage();
                return;
            }

            if (!(File.Exists(args[1])))
            {
                Console.WriteLine("[!] File does not exist");
                PrintUsage();
                return;
            }

            if (args[0] == "-d")
            {
                Decrypt(args[1]);
                return;
            }

            PrintUsage();
            return;
        }

        public static void Interactive()
        {
            Console.WriteLine("Input \"d\" for decryption:");
            string input = Console.ReadLine();
            if (input == "d")
            {
                Console.WriteLine("Enter file to decrypt:");
                string file = Console.ReadLine();
                if (!(File.Exists(file)))
                {
                    Console.WriteLine("[!] File does not exist");
                    return;
                }

                Decrypt(file);
                return;
            }
        }

        public static void PrintUsage()
        {
            Console.WriteLine("AES decryptor");
            Console.WriteLine("Arguments:");
            Console.WriteLine("-d <filepath> | Decrypt file at <filepath> and output contents to console");
        }

        public static void Decrypt(string file)
        {
            try
            {
                StreamReader sr = new StreamReader(file);
                String line = sr.ReadLine();

                while (line != null)
                {
                    Console.WriteLine(Encryption.SimpleDecodeAndDecrypt(logEncKey, line));
                    line = sr.ReadLine();
                }
                sr.Close();
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
            }
        }
    }
}