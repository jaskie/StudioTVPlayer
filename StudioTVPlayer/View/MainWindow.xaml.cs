using MahApps.Metro.Controls;
using MahApps.Metro.Controls.Dialogs;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace StudioTVPlayer.View
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : MetroWindow
    {
        public MainWindow()
        {          
            InitializeComponent();
            DataContext = ViewModel.MainViewModel.Instance;
        }

        protected override void OnClosing(CancelEventArgs e)
        {

        }

        private void MetroWindow_Loaded(object sender, RoutedEventArgs e)
        {
            ViewModel.MainViewModel.Instance.ShowPlayoutView();
        }

        private void MetroWindow_Closing(object sender, CancelEventArgs e)
        {
            base.OnClosing(e);
            e.Cancel = e.Cancel || ViewModel.MainViewModel.Instance.CanClose();
            if (!e.Cancel)
                ViewModel.MainViewModel.Instance.Dispose();
        }
    }
}
