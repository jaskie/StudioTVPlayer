using MahApps.Metro.Controls;
using System;
using System.ComponentModel;
using System.Windows;

namespace StudioTVPlayer.View
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : MetroWindow
    {
        private bool _shouldClose;

        public MainWindow()
        {
            InitializeComponent();
            DataContext = ViewModel.MainViewModel.Instance;
        }

        private void MetroWindow_Loaded(object sender, RoutedEventArgs e)
        {
            ViewModel.MainViewModel.Instance.InitializeAndShowPlayoutView();
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
            var canClose = await ViewModel.MainViewModel.Instance.ConfirmCloseAsync();
            base.OnClosing(e);
            if (canClose)
            {
                _shouldClose = true;
                await Dispatcher.BeginInvoke(Close); // call 2nd pass
            }
        }

        protected override void OnClosed(EventArgs e)
        {
            ViewModel.MainViewModel.Instance.Dispose();
            base.OnClosed(e);
        }
    }
}
