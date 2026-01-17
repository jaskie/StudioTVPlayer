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
            if (_shouldClose)
            {
                base.OnClosing(e);
                return;
            }

            e.Cancel = true;

            var canClose = await ViewModel.MainViewModel.Instance.ConfirmCloseAsync();
            if (!canClose)
                return;

            _shouldClose = true;
            Close(); // re-enter OnClosing, will pass through
        }

        protected override void OnClosed(EventArgs e)
        {
            ViewModel.MainViewModel.Instance.Dispose();
            base.OnClosed(e);
        }
    }
}
