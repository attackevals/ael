using ProtoBuf;

namespace Quasar.Common.Messages
{
    [ProtoContract]
    public class DoPortScan : IMessage
    {
        [ProtoMember(1)]
        public string TaskId { get; set; }

        [ProtoMember(2)]
        public string TargetRange { get; set; }

        [ProtoMember(3)]
        public int[] TargetPorts { get; set; }
    }
}
