using System.Reflection;
using System.Runtime.InteropServices;

namespace EppoScriptCore
{
    public static class ScriptGlue
    {
        private static readonly Dictionary<string, MethodInfo> s_Registry = new();
        private static readonly List<string> S_OrderedNames = new();

        [UnmanagedCallersOnly(EntryPoint = "Bootstrap")]
        public static void Bootstrap()
        {
            s_Registry.Clear();
            S_OrderedNames.Clear();

            var assembly = Assembly.GetExecutingAssembly();
            foreach (var type in SafeGetTypes(assembly))
            {
                if (!type.IsPublic || type == typeof(ScriptGlue))
                    continue;

                foreach (var method in type.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static))
                {
                    if (method.IsSpecialName)
                        continue;

                    string key = $"{type.FullName}.{method.Name}";
                    s_Registry[key] = method;
                    S_OrderedNames.Add(key);
                }
            }
        }

        [UnmanagedCallersOnly(EntryPoint = "GetRegisteredCount")]
        public static int GetRegisteredCount() => S_OrderedNames.Count;

        [UnmanagedCallersOnly(EntryPoint = "GetRegisteredName")]
        public static IntPtr GetRegisteredName(int index) => Marshal.StringToCoTaskMemUTF8(S_OrderedNames[index]);

        [UnmanagedCallersOnly(EntryPoint = "FreeString")]
        public static void FreeString(IntPtr ptr) => Marshal.FreeCoTaskMem(ptr);

        [UnmanagedCallersOnly(EntryPoint = "Invoke")]
        public static void Invoke(IntPtr ptr)
        {
            string? name = Marshal.PtrToStringUTF8(ptr);
            if (name == null || !s_Registry.TryGetValue(name, out var method))
            {
                Console.WriteLine($"[ScriptEngine] No method registered for '{name}'");
                return;
            }

            method.Invoke(null, null);
        }

        private static IEnumerable<Type> SafeGetTypes(Assembly assembly)
        {
            try
            {
                return assembly.GetTypes();
            } catch (ReflectionTypeLoadException exception)
            {
                return exception.Types.Where(t => t != null)!;
            }
        }
    }
}
