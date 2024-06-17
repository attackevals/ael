using System;
using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace ExMatter
{
    public class Encryption
    {
        public static string SimpleEncryptAndEncode(byte[] key, string input)
        {
            return Convert.ToBase64String(SimpleEncrypt(key, Encoding.UTF8.GetBytes(input)));
        }

        public static byte[] SimpleEncrypt(byte[] key, byte[] input)
        {
            if (input == null)
                throw new ArgumentNullException($"{nameof(input)} can not be null.");
            if (key == null)
                throw new ArgumentNullException($"{nameof(key)} can not be null.");
            if (key.Length != 32)
                throw new Exception("AES key must be 32 bytes.");

            using (var ms = new MemoryStream())
            {
                using (var aesProvider = Aes.Create())
                {
                    aesProvider.KeySize = 256;
                    aesProvider.BlockSize = 128;
                    aesProvider.Mode = CipherMode.CBC;
                    aesProvider.Padding = PaddingMode.PKCS7;
                    aesProvider.Key = key;
                    aesProvider.GenerateIV();

                    using (var cs = new CryptoStream(ms, aesProvider.CreateEncryptor(), CryptoStreamMode.Write))
                    {
                        ms.Write(aesProvider.IV, 0, aesProvider.IV.Length); // write next 16 bytes the IV, followed by ciphertext
                        cs.Write(input, 0, input.Length);
                        cs.FlushFinalBlock();
                    }
                }
                return ms.ToArray();
            }
        }

        public static string SimpleDecodeAndDecrypt(byte[] key, string input)
        {
            return Encoding.UTF8.GetString(SimpleDecrypt(key, Convert.FromBase64String(input)));
        }

        public static byte[] SimpleDecrypt(byte[] key, byte[] input)
        {
            if (input == null)
                throw new ArgumentNullException($"{nameof(input)} can not be null.");
            if (key == null)
                throw new ArgumentNullException($"{nameof(key)} can not be null.");
            if (key.Length != 32)
                throw new Exception("AES key must be 32 bytes.");

            using (var ms = new MemoryStream(input))
            {
                using (var aesProvider = Aes.Create())
                {
                    aesProvider.KeySize = 256;
                    aesProvider.BlockSize = 128;
                    aesProvider.Mode = CipherMode.CBC;
                    aesProvider.Padding = PaddingMode.PKCS7;
                    aesProvider.Key = key;

                    byte[] iv = new byte[16];
                    ms.Read(iv, 0, 16); // read next 16 bytes for IV, followed by ciphertext
                    aesProvider.IV = iv;

                    using (var cs = new CryptoStream(ms, aesProvider.CreateDecryptor(), CryptoStreamMode.Read))
                    {
                        byte[] temp = new byte[ms.Length - 16 + 1];
                        byte[] data = new byte[cs.Read(temp, 0, temp.Length)];
                        Buffer.BlockCopy(temp, 0, data, 0, data.Length);
                        return data;
                    }
                }
            }
        }
    }
}