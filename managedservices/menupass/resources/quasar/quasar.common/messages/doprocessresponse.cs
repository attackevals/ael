using ProtoBuf;
using Quasar.Common.Enums;

namespace Quasar.Common.Messages
{
    [ProtoContract]
    public class DoProcessResponse : IMessage
    {
        [ProtoMember(1)]
        public string TaskId { get; set; }

        [ProtoMember(2)]
        public ProcessAction Action { get; set; }

        [ProtoMember(3)]
        public bool Result { get; set; }
        
        [ProtoMember(4)]
        public string ResultMsg { get; set; } // optional string that provides more details on the Result, such as an error message

        [ProtoMember(5)]
        public int PID { get; set; } // created process ID

        [ProtoMember(6)]
        public int ExitCode { get; set; } // process exit code

        [ProtoMember(7)]
        public string Stdout { get; set; }

        [ProtoMember(8)]
        public string Stderr { get; set; }

        [ProtoMember(9)]
        public bool WaitedForExit { get; set; }
    }
}
