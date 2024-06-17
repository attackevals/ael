/*
================================ Compile as a .Net DLL ==============================
	C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe /target:library /out:TestAssembly.dll TestAssembly.cs
*/

using System;

namespace Quasar.Client
{
    internal static class Program
    {
        [STAThread]
        private static void Main(string[] args)
        {
            Console.WriteLine(
                "Invoked Quasar.Client.Program.Main from test-assembly"
            );
        }
    }
}