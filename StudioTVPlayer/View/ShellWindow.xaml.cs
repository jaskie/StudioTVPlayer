using MahApps.Metro.Controls;
using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace StudioTVPlayer.View
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : MetroWindow
    {
        private readonly WindowInteropHelper _windowInteropHelper;
        private bool _shouldClose;
        private const int SW_HIDE = 0;
        private const int SW_RESTORE = 9;
        public MainWindow()
        {
            InitializeComponent();
            DataContext = ViewModel.ShellViewModel.Instance;
            _windowInteropHelper = new WindowInteropHelper(this);
        }

        private void MetroWindow_Loaded(object sender, RoutedEventArgs e)
        {
            ViewModel.ShellViewModel.Instance.InitializeAndShowPlayoutView();
            App.CloseSplashScreen();
        }

        protected override async void OnClosing(CancelEventArgs e)
        {
            // 2nd pass: actually close
            if (_shouldClose)
            {
                base.OnClosing(e);
                return;
            }

            // 1st pass: ask user to confirm closing
            e.Cancel = true;
            var canClose = await ViewModel.ShellViewModel.Instance.ConfirmCloseAsync();
            base.OnClosing(e);
            if (canClose)
            {
                _shouldClose = true;
                await Dispatcher.BeginInvoke(Close); // call 2nd pass
            }
        }

        protected override void OnClosed(EventArgs e)
        {
            ViewModel.ShellViewModel.Instance.Dispose();
            base.OnClosed(e);
        }

        private void MinimizeToTray_Click(object sender, RoutedEventArgs e)
        {
            ShowInTaskbar = false;
            var handle = _windowInteropHelper.EnsureHandle();
            ShowWindow(handle, SW_HIDE);
        }

        private void TaskbarIcon_TrayMouseDoubleClick(object sender, RoutedEventArgs e)
        {
            Restore();
        }

        public void Restore()
        {
            if (!ShowInTaskbar)
            {
                ShowInTaskbar = true;
            }
            // using native methods to bring the window to foreground
            var handle = _windowInteropHelper.EnsureHandle();
            ShowWindow(handle, SW_RESTORE);
            SetForegroundWindow(handle);
        }


        [DllImport("user32.dll")]
        private static extern bool SetForegroundWindow(IntPtr hWnd);

        [DllImport("user32.dll")]
        private static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

    }
}
