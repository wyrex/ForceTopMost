using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ForceTopMost
{
    static class Program
    {
        [DllImport("TopMostDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "InstallHook")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool InstallHook();

        [DllImport("TopMostDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "RemoveHook")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool RemoveHook();

        [DllImport("TopMostDLL.dll", CharSet = CharSet.Ansi, EntryPoint = "Pump")]
        public static extern void Pump();

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
