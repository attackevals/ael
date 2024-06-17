using Microsoft.VisualStudio.TestTools.UnitTesting;
using Quasar.Common.Cryptography;
using Quasar.Common.Helpers;
using System;
using System.Text;

namespace Quasar.Common.Tests.Cryptography
{
    [TestClass]
    public class Aes128Tests
    {
        [TestMethod, TestCategory("Cryptography")]
        public void EncryptAndDecryptStringTest()
        {
            var input = StringHelper.GetRandomString(100);
            var password = StringHelper.GetRandomString(50);

            var aes = new Aes256(password);

            var encrypted = aes.Encrypt(input);

            Assert.IsNotNull(encrypted);
            Assert.AreNotEqual(encrypted, input);

            var decrypted = aes.Decrypt(encrypted);

            Assert.AreEqual(input, decrypted);
        }

        [TestMethod, TestCategory("Cryptography")]
        public void EncryptAndDecryptByteArrayTest()
        {
            var input = StringHelper.GetRandomString(100);
            var inputByte = Encoding.UTF8.GetBytes(input);
            var password = StringHelper.GetRandomString(50);

            var aes = new Aes256(password);

            var encryptedByte = aes.Encrypt(inputByte);

            Assert.IsNotNull(encryptedByte);
            CollectionAssert.AllItemsAreNotNull(encryptedByte);
            CollectionAssert.AreNotEqual(encryptedByte, inputByte);

            var decryptedByte = aes.Decrypt(encryptedByte);

            CollectionAssert.AreEqual(inputByte, decryptedByte);
        }

        [TestMethod, TestCategory("Cryptography")]
        public void TestDecrypt()
        {
            byte[] ciphertext = {
                // HMAC
                0xe8, 0x89, 0x49, 0xef, 0x9f, 0x1d, 0x3b, 0x46, 0x1a, 0xe2, 0xc0, 0x6a, 0x1d, 0x2f, 0x5d, 0xa7, 
                0x36, 0x2a, 0x90, 0x98, 0x86, 0x32, 0xc6, 0xd4, 0xbc, 0x45, 0x5c, 0x3c, 0xf3, 0xf5, 0x6b, 0x81,

                // IV
                0xdb, 0x90, 0x75, 0xf7, 0xab, 0xca, 0x9e, 0xe8, 0x0f, 0xbc, 0x96, 0xf3, 0xb1, 0x2b, 0xa0, 0x22,

                // Ciphertext
                0x97, 0x3e, 0xe2, 0xb5, 0x04, 0xcb, 0x27, 0xa4, 0x2b, 0x8c, 0xb5, 0xdc, 0x56, 0x10, 0x45, 0x09
            };

            var aes = new Aes256("b9154cb999eb576f4d1541d5ede5e20d34fe455b");
            var decryptedByte = aes.Decrypt(ciphertext);

            CollectionAssert.AreEqual(Encoding.UTF8.GetBytes("thisistestinput"), decryptedByte);
        }

        [TestMethod, TestCategory("Cryptography")]
        public void SimpleEncryptAndDecryptRandomTest()
        {
            byte[] testKey = { 
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            };
            string input = StringHelper.GetRandomString(100);
            string encryptedEncoded = Aes256.SimpleEncryptAndEncode(testKey, input);

            Assert.IsFalse(String.IsNullOrEmpty(encryptedEncoded));
            Assert.AreNotEqual(encryptedEncoded, input);

            string decryptedDecoded = Aes256.SimpleDecodeAndDecrypt(testKey, encryptedEncoded);
            Assert.AreEqual(input, decryptedDecoded);
        }

        [TestMethod, TestCategory("Cryptography")]
        public void SimpleDecryptAndDecodeTest()
        {
            string encryptedEncoded = "AAAAAAAAAAAAAAAAAAAAAN0xZ9/c/L57vru4RJybayFBNuWsQQTlMLQ883zblWmqDznH9cJz2ejSflkkrhBcjeVWgWbv6SDjuDUP74nLSlHnlghtP6pRcpJrwA4KoDIxwQrpzw/6gm275YkMRBp34aEAjPLi5JXu7LLjxmEHCveEkTHXnlN3FHnvskrWTMICAiuHz7qUDBQGqRi8c8D21M3MOpzicEnnE3UTyeKiKbzWbXQVjBsyxUVS+1xgdlWAq9N50sgg60EZeD+uf/Ey8a3sZR1whyFbsxEQMlFD0BGbK0z1g5qG/Jb7EobLaZNrcY2EmWxXR71GYfHj8gfvW8+KPh9nHs3JiFwsZx8MQJl7aHGwz1DSQoMpt2c35971LZcC8PfP0Y+Jl0x7RO21W1abhonulfCUqtQQ5FQt3GX4YZH2Fs35tM8xT6MCJ2ppo2BzZ/lRguSuc76d2yg7/o6hyncBwZwg0V7A5UrGz2jQK0cb38NWOM4p+q2uzuJDA7Y4eq/YTUSkYuPIIneR77wsnuUzgnUKr4R7a5E/sA1/DF2Haz2QTswhqWYbx4tcoy4cKix1WztKEJMrXJUjK47Sf3oM2aq2/N2ISaWNKdo=";
            byte[] testKey = { 
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            };
            string decryptedDecoded = Aes256.SimpleDecodeAndDecrypt(testKey, encryptedEncoded);
            Assert.IsFalse(String.IsNullOrEmpty(decryptedDecoded));

            string want = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
            Assert.AreEqual(decryptedDecoded, want);
        }
    }
}
