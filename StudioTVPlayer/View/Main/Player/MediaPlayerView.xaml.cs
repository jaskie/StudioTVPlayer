using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace StudioTVPlayer.View.Main.Player
{
    /// <summary>
    /// Interaction logic for MediaPlayerView.xaml
    /// </summary>
    public partial class MediaPlayerView : UserControl
    {
        public MediaPlayerView()
        {
            InitializeComponent();           
        }

        private void Slider_DragCompleted(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            if (!(DataContext is ViewModel.Main.Player.MediaPlayerViewModel vm))
                return;
            vm.SeekToSliderPosition();
        }
    }
}
