using Quasar.Server.Enums;
using Quasar.Server.Forms;
using Quasar.Server.Messages;
using Quasar.Server.Models;
using Quasar.Server.Networking;
using Quasar.Server.RestApiConstants;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;

namespace Quasar.Server.RestApi.Tests
{
    public class MockFrmMain : FrmMain
    {
        public Client[] clientList { get; set; }

        public static string TEST_ID = "testid";
        public static string TEST_ID2 = "testid2";

        public MockFrmMain(bool initialize) : base(initialize)
        {
            Client testClient = new Client(null, null, new IPEndPoint(0x0101000a, 12345)); // IP address 10.0.1.1 in big-endian hex format
            testClient.ClientId = TEST_ID;
            testClient.Value.Username = "testuser";
            testClient.Value.PcName = "testhost";
            testClient.Value.OperatingSystem = "testos";
            testClient.Value.AccountType = "Admin";
            testClient.Value.Tag = "testtag";
            testClient.Value.CountryCode = "us";
            testClient.Value.Id = "testhwid";

            Client testClient2 = new Client(null, null, new IPEndPoint(0x0201000a, 12345)); // IP address 10.0.1.2 in big-endian hex format
            testClient2.ClientId = TEST_ID2;
            testClient2.Value.Username = "testuser2";
            testClient2.Value.PcName = "testhost2";
            testClient2.Value.OperatingSystem = "testos";
            testClient2.Value.AccountType = "User";
            testClient2.Value.Tag = "testtag2";
            testClient2.Value.CountryCode = "us";
            testClient2.Value.Id = "testhwid2";

            clientList = new Client[] { testClient, testClient2 };
        }

        public override Client[] GetConnectedClients()
        {
            return this.clientList;
        }

        public override Client LookupClientById(string clientId)
        {
            foreach (Client c in clientList)
            {
                if (clientId == c.ClientId)
                    return c;
            }
            return null;
        }
    }

    [TestClass]
    public class RestApiTests
    {
        private TestContext testContextInstance;
        public TestContext TestContext
        {
            get
            {
                return testContextInstance;
            }
            set
            {
                testContextInstance = value;
            }
        }

        private static string restAddr = "127.0.0.1";
        private static int restPort = 61123;
        private static string restBaseUri = String.Format("http://{0}:{1}/api/", restAddr, restPort);
        private static string beaconsUri = restBaseUri + "beacons";
        private static string transfersUri = restBaseUri + "transfers";
        private static string uploadsUri = restBaseUri + "uploadedfiles";
        private static string validApiKey = "81152cc4c24d327f8fe800afbfb9777c";

        private static readonly object _transfersLock = new object();

        [ClassInitialize]
        public static void RestApiTestClassInit(TestContext context)
        {
            if (Directory.Exists(RestServer.baseUploadsDir))
            {
                Directory.Delete(RestServer.baseUploadsDir, true);
            }
            if (!RestServer.StartRestServer(new MockFrmMain(false), restAddr, restPort))
            {
                throw new Exception("Failed to start rest server.");
            }
        }

        [ClassCleanup]
        public static void RestApiTestClassCleanup()
        {
            RestServer.StopRestServer();
            if (Directory.Exists(RestServer.baseUploadsDir))
            {
                Directory.Delete(RestServer.baseUploadsDir, true);
            }
        }

        private static FileTransfer dummyFileTransferDownloadPending = new FileTransfer
        {
            Id = 123,
            Type = TransferType.Download,
            LocalPath = "C:\\test\\local\\path",
            RemotePath = "C:\\test\\remote\\path",
            Status = "Pending...",
            StatusCode = FileTransferStatus.STATUS_PENDING,
            Size = 100,
            TransferredSize = 0
        };

        private static FileTransfer dummyFileTransferDownloadComplete = new FileTransfer
        {
            Id = 234,
            Type = TransferType.Download,
            LocalPath = "C:\\test\\local\\path",
            RemotePath = "C:\\test\\remote\\path",
            Status = "Complete",
            StatusCode = FileTransferStatus.STATUS_SUCCESS,
            Size = 100,
            TransferredSize = 100
        };

        private static FileTransfer dummyFileTransferUploadCanceled = new FileTransfer
        {
            Id = 345,
            Type = TransferType.Upload,
            LocalPath = "C:\\test\\local\\path",
            RemotePath = "C:\\test\\remote\\path",
            Status = "Canceled",
            StatusCode = FileTransferStatus.STATUS_CANCELED,
            Size = 100,
            TransferredSize = 50
        };

        private static FileTransfer dummyFileTransferUploadErrored = new FileTransfer
        {
            Id = 456,
            Type = TransferType.Upload,
            LocalPath = "C:\\test\\local\\path",
            RemotePath = "C:\\test\\remote\\path",
            Status = "Error failed",
            StatusCode = FileTransferStatus.STATUS_ERROR,
            Size = 100,
            TransferredSize = 50
        };

        private bool DictEqual(Dictionary<string, string> a, Dictionary<string, string> b)
        {
            if (a.Count != b.Count)
            {
                TestContext.WriteLine(String.Format("Mismatched dict size. {0} != {1}", a.Count, b.Count));
                return false;
            }
            foreach (KeyValuePair<string, string> entry in a)
            {
                if (!b.ContainsKey(entry.Key)) 
                {
                    TestContext.WriteLine("Right hand dictionary missing key " + entry.Key);
                    return false;
                }
                if (b[entry.Key] != entry.Value)
                {
                    TestContext.WriteLine(String.Format("Mismatched value for key {0}: {1} != {2}", entry.Key, entry.Value, b[entry.Key]));
                    return false;
                }
            }
            return true;
        }

        private bool DictEqual(Dictionary<int, int> a, Dictionary<int, int> b)
        {
            if (a.Count != b.Count)
            {
                TestContext.WriteLine(String.Format("Mismatched dict size. {0} != {1}", a.Count, b.Count));
                return false;
            }
            foreach (KeyValuePair<int, int> entry in a)
            {
                if (!b.ContainsKey(entry.Key)) 
                {
                    TestContext.WriteLine("Right hand dictionary missing key " + entry.Key);
                    return false;
                }
                if (b[entry.Key] != entry.Value)
                {
                    TestContext.WriteLine(String.Format("Mismatched value for key {0}: {1} != {2}", entry.Key, entry.Value, b[entry.Key]));
                    return false;
                }
            }
            return true;
        }

        private bool DictEqual(Dictionary<int, string> a, Dictionary<int, string> b)
        {
            if (a.Count != b.Count)
            {
                TestContext.WriteLine(String.Format("Mismatched dict size. {0} != {1}", a.Count, b.Count));
                return false;
            }
            foreach (KeyValuePair<int, string> entry in a)
            {
                if (!b.ContainsKey(entry.Key)) 
                {
                    TestContext.WriteLine("Right hand dictionary missing key " + entry.Key);
                    return false;
                }
                if (b[entry.Key] != entry.Value)
                {
                    TestContext.WriteLine(String.Format("Mismatched value for key {0}: {1} != {2}", entry.Key, entry.Value, b[entry.Key]));
                    return false;
                }
            }
            return true;
        }

        // Reference: https://stackoverflow.com/a/27108442
        private Tuple<int, string> PerformHttpGet(string uri, string apiKey)
        {
            HttpWebRequest req = (HttpWebRequest)WebRequest.Create(uri);
            if (apiKey.Length > 0) 
            {
                req.Headers.Add(RestServer.API_KEY_HEADER, apiKey);
            }
            using (HttpWebResponse resp = (HttpWebResponse)req.GetResponse())
            {
                using (Stream respStream = resp.GetResponseStream())
                {
                    using(StreamReader respReader = new StreamReader(respStream))
                    {
                        return Tuple.Create((int)resp.StatusCode, respReader.ReadToEnd());
                    }
                }
            }
        }

        private void PerformGenericHttpRequest(string uri, string httpMethod)
        {
            switch (httpMethod)
            {
                case "GET":
                    PerformHttpGet(uri, validApiKey);
                    break;
                case "PUT":
                case "POST":
                case "DELETE":
                    using (var client = new System.Net.WebClient())
                    {
                        client.Headers.Add(RestServer.API_KEY_HEADER, validApiKey);
                        client.UploadData(uri, httpMethod, new Byte[0]);
                    }
                    break;
                default:
                    throw new NotImplementedException();
            }
        }

        private byte[] PerformHttpFileDownload(string uri)
        {
            using (WebClient client = new WebClient())
            {
                client.Headers.Add(RestServer.API_KEY_HEADER, validApiKey);
                return client.DownloadData(uri);
            }
        }

        private byte[] PerformJsonPost(string uri, Dictionary<string, object> dataDict)
        {
            byte[] data = Encoding.ASCII.GetBytes(JsonSerializer.Serialize(dataDict));
            using (WebClient client = new WebClient())
            {
                client.Headers.Add(RestServer.API_KEY_HEADER, validApiKey);
                client.Headers.Add("Content-Type", "application/json");
                return client.UploadData(uri, "POST", data);
            } 
        }

        private string GetSHA256Hash(byte[] input)
        {
            using (SHA256 sha = SHA256.Create())
            {
                byte[] hash = sha.ComputeHash(input);
                StringBuilder hexBuilder = new StringBuilder(hash.Length * 2);
                foreach (byte b in hash)
                {
                    hexBuilder.AppendFormat("{0:x2}", b);
                }
                return hexBuilder.ToString();
            }
        }

        private FileTransfer ConvertJsonDictToFileTransfer(Dictionary<string, JsonElement> dataDict)
        {
            return new FileTransfer
            {
                Id = dataDict[RestConstants.TRANSFER_ID_FIELD].GetInt32(),
                Type = dataDict[RestConstants.TRANSFER_TYPE_FIELD].GetInt32() == RestConstants.TRANSFER_TYPE_SERVER_TO_CLIENT ? TransferType.Upload : TransferType.Download,
                Size = dataDict[RestConstants.TRANSFER_SIZE_FIELD].GetInt32(),
                TransferredSize = dataDict[RestConstants.TRANSFER_TRANSFERRED_SIZE_FIELD].GetInt32(),
                LocalPath = dataDict[RestConstants.TRANSFER_LOCAL_PATH_FIELD].ToString(),
                RemotePath = dataDict[RestConstants.TRANSFER_REMOTE_PATH_FIELD].ToString(),
                Status = dataDict[RestConstants.TRANSFER_STATUS_MSG_FIELD].ToString(),
                StatusCode = dataDict[RestConstants.TRANSFER_STATUS_CODE_FIELD].GetInt32()
            };
        }

        private string GetWebExceptionMessage(WebException ex)
        {
            using (Stream descStream = ((HttpWebResponse)ex.Response).GetResponseStream())
            {
                using (StreamReader sr = new StreamReader(descStream))
                {
                    return sr.ReadToEnd();
                }
            }
        }

        [TestMethod]
        public void TestGenerateClientID()
        {
            Client testClient = new Client(null, null, new IPEndPoint(0x0101000a, 12345)); // IP address 10.0.1.1 in big-endian hex format
            testClient.Value.Username = "testuser";
            testClient.Value.PcName = "testhost";
            testClient.Value.AccountType = "Admin";
            Assert.AreEqual(QuasarServer.GenerateClientID(testClient).ToLower(), "30c54dff7ebab724136cc5ce37a4368f");
        }

        [TestMethod]
        public void TestGenerateClientDictionary()
        {
            Client testClient = new Client(null, null, new IPEndPoint(0x0101000a, 12345)); // IP address 10.0.1.1 in big-endian hex format
            testClient.ClientId = "testid";
            testClient.Value.Username = "testuser";
            testClient.Value.PcName = "testhost";
            testClient.Value.OperatingSystem = "testos";
            testClient.Value.AccountType = "Admin";
            testClient.Value.Tag = "testtag";
            testClient.Value.CountryCode = "us";
            testClient.Value.Id = "testhwid";
            
            Dictionary<string, string> result = RestServer.GenerateClientDictionary(testClient);
            Dictionary<string, string> want = new Dictionary<string, string>
            {
                [RestConstants.CLIENT_ID_FIELD] = "testid",
                [RestConstants.CLIENT_USERNAME_FIELD] = "testuser",
                [RestConstants.CLIENT_HOSTNAME_FIELD] = "testhost",
                [RestConstants.CLIENT_IP_ADDR_FIELD] = "10.0.1.1",
                [RestConstants.CLIENT_OS_FIELD] = "testos",
                [RestConstants.CLIENT_ACCOUNT_TYPE_FIELD] = "Admin",
                [RestConstants.CLIENT_TAG_FIELD] = "testtag",
                [RestConstants.CLIENT_COUNTRY_FIELD] = "us",
                [RestConstants.CLIENT_HARDWARE_ID_FIELD] = "testhwid"
            };
            
            Assert.IsTrue(DictEqual(want, result));
        }

        [TestMethod]
        public void TestApiAuthenticationNoApiKey()
        {
            // No API key
            try
            {
                var result = PerformHttpGet(restBaseUri, "");
                Assert.Fail(); // expected exception not thrown
            }
            catch (WebException ex)
            {
                Assert.AreEqual("The remote server returned an error: (401) Unauthorized.", ex.Message);
            }
        }

        [TestMethod]
        public void TestApiAuthenticationIncorrectApiKey()
        {
            // Incorrect API key
            try
            {
                var result = PerformHttpGet(restBaseUri, "incorrect");
                Assert.Fail(); // expected exception not thrown
            }
            catch (WebException ex)
            {
                Assert.AreEqual("The remote server returned an error: (401) Unauthorized.", ex.Message);
            }
        }

        [TestMethod]
        public void TestApiAuthenticationCorrectKey()
        {
            // Correct API key, 404
            try 
            {
                var result = PerformHttpGet(restBaseUri, validApiKey);
                Assert.Fail(); // expected exception not thrown
            }
            catch (WebException ex) 
            {
                Assert.AreEqual("The remote server returned an error: (404) Not Found.", ex.Message);
            }
        }

        [TestMethod]
        public void TestUnsupportedHttpMethods()
        {
            List<string> httpMethods = new List<string>
            {
                "GET",
                "POST",
                "PUT",
                "DELETE",
            };

            Dictionary<string, HashSet<string>> permittedMethods = new Dictionary<string, HashSet<string>>
            {
                [beaconsUri] = new HashSet<string>{"GET"},
                [transfersUri] = new HashSet<string>{"GET", "POST"},
                [uploadsUri] = new HashSet<string>{"GET"},
            };

            foreach (KeyValuePair<string, HashSet<string>> entry in permittedMethods)
            {
                foreach (string method in httpMethods)
                {
                    if (!entry.Value.Contains(method))
                    {
                        try 
                        {
                            PerformGenericHttpRequest(entry.Key, method);
                            TestContext.WriteLine(String.Format("Expected {0} to be unsupported for {1}", method, entry.Key));
                            Assert.Fail(); // expected exception not thrown
                        }
                        catch (WebException ex) 
                        {
                            Assert.AreEqual("The remote server returned an error: (405) Method Not Allowed.", ex.Message);
                        }
                    } 
                }
            }
        }

        [TestMethod]
        public void TestBeaconsGet()
        {
            var result = PerformHttpGet(beaconsUri, validApiKey);
            Assert.AreEqual(result.Item1, 200);

            List<Dictionary<string, string>> got = JsonSerializer.Deserialize<List<Dictionary<string, string>>>(result.Item2);
            List<Dictionary<string, string>> want = new List<Dictionary<string, string>>
            {
                new Dictionary<string, string>
                {
                    [RestConstants.CLIENT_ID_FIELD] = "testid",
                    [RestConstants.CLIENT_USERNAME_FIELD] = "testuser",
                    [RestConstants.CLIENT_HOSTNAME_FIELD] = "testhost",
                    [RestConstants.CLIENT_IP_ADDR_FIELD] = "10.0.1.1",
                    [RestConstants.CLIENT_OS_FIELD] = "testos",
                    [RestConstants.CLIENT_ACCOUNT_TYPE_FIELD] = "Admin",
                    [RestConstants.CLIENT_TAG_FIELD] = "testtag",
                    [RestConstants.CLIENT_COUNTRY_FIELD] = "us",
                    [RestConstants.CLIENT_HARDWARE_ID_FIELD] = "testhwid"
                },
                new Dictionary<string, string>
                {
                    [RestConstants.CLIENT_ID_FIELD] = "testid2",
                    [RestConstants.CLIENT_USERNAME_FIELD] = "testuser2",
                    [RestConstants.CLIENT_HOSTNAME_FIELD] = "testhost2",
                    [RestConstants.CLIENT_IP_ADDR_FIELD] = "10.0.1.2",
                    [RestConstants.CLIENT_OS_FIELD] = "testos",
                    [RestConstants.CLIENT_ACCOUNT_TYPE_FIELD] = "User",
                    [RestConstants.CLIENT_TAG_FIELD] = "testtag2",
                    [RestConstants.CLIENT_COUNTRY_FIELD] = "us",
                    [RestConstants.CLIENT_HARDWARE_ID_FIELD] = "testhwid2"
                },
            };
            Assert.AreEqual(want.Count, got.Count);
            for (int i = 0; i < want.Count; i++)
            {
                Assert.IsTrue(DictEqual(want[i], got[i]));
            }
        }

        [TestMethod]
        public void TestTransfersGetEmpty()
        {
            lock (_transfersLock)
            {
                FileManagerHandler.ClearRegisteredTransfers();
                var result = PerformHttpGet(transfersUri, validApiKey);
                Assert.AreEqual(result.Item1, 200);
                Assert.AreEqual(result.Item2, "[]");
            }
        }

        [TestMethod]
        public void TestTransferGetNonexistent()
        {
            try 
            {
                var result = PerformHttpGet(transfersUri + "/000", validApiKey);
                TestContext.WriteLine("Expected 404 error code exception");
                Assert.Fail(); // expected exception not thrown
            }
            catch (WebException ex) 
            {
                Assert.AreEqual("The remote server returned an error: (404) Not Found.", ex.Message);
                Assert.AreEqual("Transfer ID 000 does not exist.", GetWebExceptionMessage(ex));
            }
        }

        [TestMethod]
        public void TestTransferGetBadTransferId()
        {
            try
            {
                var result = PerformHttpGet(transfersUri + "/badtransferID", validApiKey);
                TestContext.WriteLine("Expected 400 error code exception");
                Assert.Fail(); // expected exception not thrown
            }
            catch (WebException ex) 
            {
                Assert.AreEqual("The remote server returned an error: (400) Bad Request.", ex.Message);
                Assert.AreEqual("Bad transfer ID badtransferID", GetWebExceptionMessage(ex));
            }
        }

        [TestMethod]
        public void TestTransfersGet()
        {
            lock (_transfersLock)
            {
                try 
                {
                    FileManagerHandler.ClearRegisteredTransfers();
                    FileManagerHandler.DebugRegisterTransfer(dummyFileTransferDownloadPending);
                    FileManagerHandler.DebugRegisterTransfer(dummyFileTransferDownloadComplete);
                    FileManagerHandler.DebugRegisterTransfer(dummyFileTransferUploadCanceled);
                    FileManagerHandler.DebugRegisterTransfer(dummyFileTransferUploadErrored);

                    var result = PerformHttpGet(transfersUri, validApiKey);
                    Assert.AreEqual(result.Item1, 200);

                    List<Dictionary<string, JsonElement>> got = JsonSerializer.Deserialize<List<Dictionary<string, JsonElement>>>(result.Item2);
                    Dictionary<int, FileTransfer> want = new Dictionary<int, FileTransfer>
                    {
                        [dummyFileTransferDownloadPending.Id] = dummyFileTransferDownloadPending,
                        [dummyFileTransferDownloadComplete.Id] = dummyFileTransferDownloadComplete,
                        [dummyFileTransferUploadCanceled.Id] = dummyFileTransferUploadCanceled,
                        [dummyFileTransferUploadErrored.Id] = dummyFileTransferUploadErrored
                    };
                    foreach (Dictionary<string, JsonElement> gotDict in got)
                    {
                        Assert.IsTrue(gotDict.ContainsKey(RestConstants.TRANSFER_ID_FIELD) && want.ContainsKey(gotDict[RestConstants.TRANSFER_ID_FIELD].GetInt32()));
                        FileTransfer gotTransfer = ConvertJsonDictToFileTransfer(gotDict);
                        var wantTransfer = want[gotTransfer.Id];
                        Assert.IsTrue(gotTransfer.Equals(wantTransfer));
                    }
                }
                catch (WebException ex)
                {
                    TestContext.WriteLine(ex.ToString());
                    TestContext.WriteLine(GetWebExceptionMessage(ex));
                    Assert.Fail();
                }
                finally
                {
                    FileManagerHandler.ClearRegisteredTransfers();
                }
            }
        }

        [TestMethod]
        public void TestTransferPostNonexistentClient()
        {
            try 
            {
                Dictionary<string, object> postDict = new Dictionary<string, object>
                {
                    [FileTransferApiRequest.CLIENT_ID_FIELD_NAME] = "DNE",
                    [FileTransferApiRequest.TRANSFER_TYPE_FIELD_NAME] = 0,
                    [FileTransferApiRequest.SOURCE_PATH_FIELD_NAME] = "C:\\dummy\\path",
                    [FileTransferApiRequest.DEST_PATH_FIELD_NAME] = "C:\\dummy\\path2"
                };
                PerformJsonPost(transfersUri, postDict); // should generate exception here
                TestContext.WriteLine("Did not get expected internal server error exception.");
                Assert.Fail(); 
            }
            catch (WebException ex)
            {
                Assert.AreEqual(ex.Message, "The remote server returned an error: (500) Internal Server Error.");
                Assert.AreEqual("Could not find connected client by ID DNE", GetWebExceptionMessage(ex));
            }
            catch (Exception ex)
            {
                TestContext.WriteLine(ex.ToString());
                Assert.Fail();
            }
        }

        [TestMethod]
        public void TestTransferPostInvalidTransferType()
        {
            try 
            {
                Dictionary<string, object> postDict = new Dictionary<string, object>
                {
                    [FileTransferApiRequest.CLIENT_ID_FIELD_NAME] = MockFrmMain.TEST_ID,
                    [FileTransferApiRequest.TRANSFER_TYPE_FIELD_NAME] = 2,
                    [FileTransferApiRequest.SOURCE_PATH_FIELD_NAME] = "C:\\dummy\\path",
                    [FileTransferApiRequest.DEST_PATH_FIELD_NAME] = "C:\\dummy\\path2"
                };
                PerformJsonPost(transfersUri, postDict); // should generate exception here
                TestContext.WriteLine("Did not get expected internal server error exception.");
                Assert.Fail(); 
            }
            catch (WebException ex)
            {
                Assert.AreEqual(ex.Message, "The remote server returned an error: (500) Internal Server Error.");
                Assert.AreEqual("Invalid transfer type 2", GetWebExceptionMessage(ex));
            }
            catch (Exception ex)
            {
                TestContext.WriteLine(ex.ToString());
                Assert.Fail();
            }
        }

        [TestMethod]
        public void TestTransferPostMissingClientId()
        {
            try 
            {
                Dictionary<string, object> postDict = new Dictionary<string, object>
                {
                    [FileTransferApiRequest.TRANSFER_TYPE_FIELD_NAME] = 0,
                    [FileTransferApiRequest.SOURCE_PATH_FIELD_NAME] = "C:\\dummy\\path",
                    [FileTransferApiRequest.DEST_PATH_FIELD_NAME] = "C:\\dummy\\path2"
                };
                PerformJsonPost(transfersUri, postDict); // should generate exception here
                TestContext.WriteLine("Did not get expected internal server error exception.");
                Assert.Fail(); 
            }
            catch (WebException ex)
            {
                Assert.AreEqual(ex.Message, "The remote server returned an error: (500) Internal Server Error.");
                Assert.AreEqual("File transfer POST request did not contain required key \"client_id\"", GetWebExceptionMessage(ex));
            }
            catch (Exception ex)
            {
                TestContext.WriteLine(ex.ToString());
                Assert.Fail();
            }
        }

        [TestMethod]
        public void TestTransferPostServerToClient()
        {
            lock (_transfersLock)
            {
                try 
                {
                    FileManagerHandler.ClearRegisteredTransfers();
                    string dummyfilepath = Directory.GetParent(Directory.GetCurrentDirectory()).Parent.FullName + "\\testingresources\\testfile.txt";
                    Dictionary<string, object> postDict = new Dictionary<string, object>
                    {
                        [FileTransferApiRequest.CLIENT_ID_FIELD_NAME] = MockFrmMain.TEST_ID,
                        [FileTransferApiRequest.TRANSFER_TYPE_FIELD_NAME] = RestConstants.TRANSFER_TYPE_SERVER_TO_CLIENT,
                        [FileTransferApiRequest.SOURCE_PATH_FIELD_NAME] = dummyfilepath,
                        [FileTransferApiRequest.DEST_PATH_FIELD_NAME] = "C:\\dummy\\path2"
                    };
                    var response = PerformJsonPost(transfersUri, postDict);
                    Dictionary<string, JsonElement> gotDict = JsonSerializer.Deserialize<Dictionary<string, JsonElement>>(response);
                    FileTransfer gotTransfer = ConvertJsonDictToFileTransfer(gotDict);
                    Assert.IsTrue((gotTransfer.Id > 0));
                    Assert.AreEqual(gotTransfer.Type, TransferType.Upload);
                    Assert.AreEqual(gotTransfer.LocalPath, dummyfilepath);
                    Assert.AreEqual(gotTransfer.RemotePath, "C:\\dummy\\path2");
                    Assert.AreEqual(gotTransfer.StatusCode, FileTransferStatus.STATUS_PENDING);
                }
                catch (WebException ex)
                {
                    TestContext.WriteLine(ex.ToString());
                    TestContext.WriteLine(GetWebExceptionMessage(ex));
                    Assert.Fail();
                }
                catch (Exception ex)
                {
                    TestContext.WriteLine(ex.ToString());
                    Assert.Fail();
                }
                finally
                {
                    FileManagerHandler.ClearRegisteredTransfers();
                }
            }
        }

        [TestMethod]
        public void TestTransferPostClientToServer()
        {
            lock (_transfersLock)
            {
                try 
                {
                    FileManagerHandler.ClearRegisteredTransfers();
                    Dictionary<string, object> postDict = new Dictionary<string, object>
                    {
                        [FileTransferApiRequest.CLIENT_ID_FIELD_NAME] = MockFrmMain.TEST_ID,
                        [FileTransferApiRequest.TRANSFER_TYPE_FIELD_NAME] = RestConstants.TRANSFER_TYPE_CLIENT_TO_SERVER,
                        [FileTransferApiRequest.SOURCE_PATH_FIELD_NAME] = "C:\\dummy\\path\\testfile"
                    };
                    var response = PerformJsonPost(transfersUri, postDict);
                    Dictionary<string, JsonElement> gotDict = JsonSerializer.Deserialize<Dictionary<string, JsonElement>>(response);
                    FileTransfer gotTransfer = ConvertJsonDictToFileTransfer(gotDict);
                    Assert.AreEqual(gotTransfer.Type, TransferType.Download);
                    Assert.IsTrue(gotTransfer.LocalPath.EndsWith("\\uploads\\testid\\testfile"));
                    Assert.AreEqual(gotTransfer.RemotePath, "C:\\dummy\\path\\testfile");
                    Assert.AreEqual(gotTransfer.StatusCode, FileTransferStatus.STATUS_PENDING);
                }
                catch (WebException ex)
                {
                    TestContext.WriteLine(ex.ToString());
                    TestContext.WriteLine(GetWebExceptionMessage(ex));
                    Assert.Fail();
                }
                finally
                {
                    FileManagerHandler.ClearRegisteredTransfers();
                }
            }
        }

        [TestMethod]
        public void TestUploadsGet()
        {
            try 
            {
                FileManagerHandler.ClearCompletedUploadList();
                FileManagerHandler.DebugRegisterCompletedUpload(123, "C:\\my\\dummy\\file.txt");
                FileManagerHandler.DebugRegisterCompletedUpload(234, "C:\\my\\dummy\\file.log");

                var result = PerformHttpGet(uploadsUri, validApiKey);
                Assert.AreEqual(result.Item1, 200);

                Dictionary<int, string> got = JsonSerializer.Deserialize<Dictionary<int, string>>(result.Item2);
                Dictionary<int, string> want = new Dictionary<int, string>
                {
                    [123] = "C:\\my\\dummy\\file.txt",
                    [234] = "C:\\my\\dummy\\file.log"
                };
                Assert.IsTrue(DictEqual(want, got));
            }
            finally
            {
                FileManagerHandler.ClearCompletedUploadList();
            }
        }

        [TestMethod]
        public void TestUploadsGetEmpty()
        {
            try 
            {
                FileManagerHandler.ClearCompletedUploadList();
                var result = PerformHttpGet(uploadsUri, validApiKey);
                Assert.AreEqual(result.Item1, 200);

                Dictionary<int, string> got = JsonSerializer.Deserialize<Dictionary<int, string>>(result.Item2);
                Assert.AreEqual(got.Count, 0);
            }
            finally
            {
                FileManagerHandler.ClearCompletedUploadList();
            }
        }

        [TestMethod]
        public void TestUploadsGetFile()
        {
            string dummyfilepath = Directory.GetParent(Directory.GetCurrentDirectory()).Parent.FullName + "\\testingresources\\dummyfile";
            try 
            {
                FileManagerHandler.ClearCompletedUploadList();
                FileManagerHandler.DebugRegisterCompletedUpload(123, dummyfilepath);
                Assert.IsTrue(FileManagerHandler.GetUploadedFilePath(123).Length > 0);
                TestContext.WriteLine(FileManagerHandler.GetUploadedFilePath(123));
                var result = PerformHttpFileDownload(uploadsUri + "/123");
                Assert.AreEqual(GetSHA256Hash(result).ToUpper(), "5647F05EC18958947D32874EEB788FA396A05D0BAB7C1B71F112CEB7E9B31EEE");
            }
            finally
            {
                FileManagerHandler.ClearCompletedUploadList();
            }
        }
    }
}
