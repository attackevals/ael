using ProtoBuf;
using Quasar.Common.Models;

namespace Quasar.Common.Messages
{
    [ProtoContract]
    public class GetDirectoryResponse : IMessage
    {
        [ProtoMember(1)]
        public string TaskId { get; set; }

        [ProtoMember(2)]
        public string RemotePath { get; set; }

        [ProtoMember(3)]
        public FileSystemEntry[] Items { get; set; }

        [ProtoMember(4)]
        public bool Keylogging { get; set; }
    }
}
