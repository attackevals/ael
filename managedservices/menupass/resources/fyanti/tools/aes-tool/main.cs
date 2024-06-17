using System;
using System.IO;
using ManagedLoader;

namespace AESTool
{
    public class AESTool
    {
        private static byte[] logEncKey =
{
            0x0a, 0xa9, 0x2b, 0x53, 0xdf, 0x6d, 0x09, 0xe0,
            0xd1, 0x02, 0xb1, 0x9a, 0x9f, 0xa0, 0x27, 0x61,
            0x06, 0x43, 0xca, 0x31, 0x69, 0xd9, 0x79, 0xf3,
            0xee, 0x2c, 0x7a, 0x56, 0x7f, 0x87, 0x17, 0x05
        };
        private static byte[] quasarEncKey =
{
            0x15, 0x0d, 0x72, 0xfd, 0x6f, 0x9e, 0x18, 0x97,
            0x2b, 0x81, 0x04, 0xe9, 0x14, 0x6d, 0x50, 0x49,
            0xdc, 0x02, 0x0f, 0x62, 0x1f, 0x67, 0xa3, 0xa0,
            0x10, 0xa8, 0xc5, 0x9a, 0x22, 0xad, 0xa1, 0x7d
        };

        public static void Main(string[] args)
        {
            foreach (string arg in args) { Console.WriteLine(arg); }

            if (args.Length == 0)
            {
                Interactive();
                return;
            }

            if (!(args[0] == "-d" || args[0] == "-e"))
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

            if (args[0] == "-e" && args[2] == "-o")
            {
                Encrypt(args[1], args[3]);
                return;
            }

            PrintUsage();
            return;
        }

        public static void Interactive()
        {
            Console.WriteLine("Input \"e\" for encryption or \"d\" for decryption:");
            string input = Console.ReadLine();
            if (input == "e")
            {
                Console.WriteLine("Enter file to encrypt:");
                string file = Console.ReadLine();
                if (!(File.Exists(file)))
                {
                    Console.WriteLine("[!] File does not exist");
                    return;
                }

                Console.WriteLine("Enter output file:");
                string outFile = Console.ReadLine();

                Encrypt(file, outFile);
                return;
            }
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
            Console.WriteLine("Quasar AES encryptor and decryptor");
            Console.WriteLine("Arguments:");
            Console.WriteLine("-d <filepath> | Decrypt file at <filepath> and output contents to console");
            Console.WriteLine("-e <filepath1> -o <filepath2> | Encrypt file at <filepath1> and output encrypted version at <filepath2>");
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

        public static void Encrypt(string file, string outFile)
        {
            byte[] bytes = Encryption.SimpleEncrypt(quasarEncKey, File.ReadAllBytes(file));
            using (FileStream fs = File.Create(outFile))
            {
                fs.Write(bytes, 0, bytes.Length);
            }
        }
    }
}