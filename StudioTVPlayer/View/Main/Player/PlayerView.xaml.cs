using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace StudioTVPlayer.View.Main.Player
{
    /// <summary>
    /// Interaction logic for PlayerView.xaml
    /// </summary>
    public partial class PlayerView : UserControl
    {
        public PlayerView()
        {
            InitializeComponent();           
        }

        private void Slider_DragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
        {
            if (!(DataContext is ViewModel.Main.Player.PlayerViewModel vm))
                return;
            vm.BeginSliderThumbDrag();
        }

        private void Slider_DragCompleted(object sender, RoutedEventArgs e)
        {
            if (!(DataContext is ViewModel.Main.Player.PlayerViewModel vm))
                return;
            vm.EndSliderThumbDrag();
        }

    }
}
