using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace StudioTVPlayer.View.Main.Piloting.Player
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

        private void UserControl_GotFocus(object sender, RoutedEventArgs e)
        {
            this.Focus();
            Keyboard.Focus(this);
            e.Handled = true;
        }

        private void UserControl_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            this.Focus();
            Keyboard.Focus(this);
            e.Handled = true;
        }
    }
}
