using System;
using System.Reflection;

namespace Action
{
    public class NetRunner
    {
        public static int Main(string[] args)
        {
            Assembly TestAssembly = Assembly.LoadFrom("./managed-loader.dll");
            Type type = TestAssembly.GetType("ManagedLoader.ManagedLoader");
            MethodInfo method = type.GetMethod("EntryPoint");

            method.Invoke(null, null);
            return 0;
        }
    }
}