using System;
using System.Windows.Forms;

namespace ForceTopMost
{
    public partial class MainForm : Form
    {
        bool isActive = false;

        public MainForm()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Program.InstallHook();
            isActive = true;
            MainForm.ActiveForm.Focus();
            Program.Pump();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Program.RemoveHook();
            isActive = false;
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (isActive)
                Program.RemoveHook();
        }
    }
}
