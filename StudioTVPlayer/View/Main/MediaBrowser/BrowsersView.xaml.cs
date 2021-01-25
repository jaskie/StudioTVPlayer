using StudioTVPlayer.ViewModel.Main.MediaBrowser;
using System.Windows.Controls;


namespace StudioTVPlayer.View.Main.MediaBrowser
{
    /// <summary>
    /// Interaction logic for BrowsersView.xaml
    /// </summary>
    public partial class BrowsersView : UserControl
    {
        public BrowsersView()
        {
            InitializeComponent();
            DataContext = new BrowsersViewModel();
            Unloaded += (s, e) => ((BrowsersViewModel)DataContext).Dispose();
        }

    }
}
