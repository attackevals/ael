using System.Collections.Generic;
using ProtoBuf;

namespace Quasar.Common.Messages
{
    [ProtoContract]
    public class DoPortScanResponse : IMessage
    {
        [ProtoMember(1)]
        public string TaskId { get; set; }

        [ProtoMember(2)]
        public bool Success { get; set; }
        
        [ProtoMember(3)]
        public string ResultMessage { get; set; }
        
        [ProtoMember(4)]
        public Dictionary<string, int[]> Result { get; set; } // IP addresses mapped to open ports
    }
}
