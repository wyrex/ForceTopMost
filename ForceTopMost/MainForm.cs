using System;
using System.Windows.Forms;

namespace ForceTopMost
{
    public partial class MainForm : Form
    {
        bool isActive = false;

        private NotifyIcon notifyIcon;
        private ContextMenu contextMenu;
        private MenuItem[] menuItems;

        public MainForm()
        {
            InitializeComponent();
            isActive = Program.InstallHook();
            CreateNotifyIcon();
            ShowInTaskbar = false;
            Opacity = 0;
        }

        private void CreateNotifyIcon()
        {
            if (components == null)
                components = new System.ComponentModel.Container();
            contextMenu = new ContextMenu();
            menuItems = new MenuItem[2];

            menuItems[0] = new MenuItem();
            menuItems[0].Index = 0;
            menuItems[0].Text = isActive ? "D&isable" : "E&nable";
            menuItems[0].Click += new EventHandler(menuItem_Click);

            menuItems[1] = new MenuItem();
            menuItems[1].Index = 1;
            menuItems[1].Text = "E&xit";
            menuItems[1].Click += new EventHandler(menuItem_Click);

            contextMenu.MenuItems.AddRange(menuItems);

            notifyIcon = new NotifyIcon(components);
            notifyIcon.Icon = Properties.Resources.icon;
            notifyIcon.ContextMenu = contextMenu;
            notifyIcon.Text = "Always on Top menu";
            notifyIcon.Visible = true;
            notifyIcon.DoubleClick += new EventHandler(notifyIcon_Click);
        }

        private void menuItem_Click(object Sender, EventArgs e)
        {
            int index = ((MenuItem)Sender).Index;

            switch(index)
            {
                case 0:
                    if (isActive)
                        isActive = !Program.RemoveHook();
                    else
                        isActive = Program.InstallHook();

                    menuItems[0].Text = isActive ? "D&isable" : "E&nable";
                    break;
                case 1:
                    Application.Exit();
                    break;
            }
        }

        private void notifyIcon_Click(object Sender, EventArgs e)
        {
            if (this.Visible)
                this.Hide();
            else
                this.Show();

            Opacity = Visible ? 100 : 0;
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

        private void MainForm_Load(object sender, EventArgs e)
        {
            BeginInvoke(new MethodInvoker(delegate
            {
                Application.ApplicationExit += new EventHandler(OnApplicationExit);
                Hide();
            }));
        }

        private void OnApplicationExit(object sender, EventArgs e)
        {
            if (isActive)
                Program.RemoveHook();

            if (notifyIcon != null)
            {
                notifyIcon.Icon = null;
                notifyIcon.Dispose();
                notifyIcon = null;
            }

            Program.UnloadModule("TopMostDLL.dll");
        }
    }
}
