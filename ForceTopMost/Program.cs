using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ForceTopMost
{
    static class Program
    {
        #region DLL imports
        [DllImport("TopMostDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "InstallHook")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool InstallHook();

        [DllImport("TopMostDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "RemoveHook")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool RemoveHook();

        [DllImport("TopMostDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "Pump")]
        public static extern void Pump();

        [DllImport("kernel32", SetLastError = true)]
        private static extern bool FreeLibrary(IntPtr hModule);
        #endregion

        public static void UnloadModule(string moduleName)
        {
            foreach (ProcessModule mod in Process.GetCurrentProcess().Modules)
            {
                if (mod.ModuleName == moduleName)
                {
                    FreeLibrary(mod.BaseAddress);
                }
            }
        }

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }
    }
}
