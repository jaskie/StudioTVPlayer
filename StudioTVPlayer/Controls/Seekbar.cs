using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace StudioTVPlayer.Controls
{
    class Seekbar : Slider
    {        
        public static readonly DependencyProperty SliderDragStartCommandProperty =
            DependencyProperty.Register(
                "SliderDragStartCommand", 
                typeof(ICommand), typeof(Seekbar), 
                new FrameworkPropertyMetadata(new PropertyChangedCallback(SliderDragStartCommandChanged)));

        private static void SliderDragStartCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var element = (FrameworkElement)d;
            element.GotMouseCapture += SliderDragStart;
        }       
      
        private static void SliderDragStart(object sender, MouseEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)sender;

            ICommand command = GetSliderDragStartCommand(element);
            object[] param = { sender, e };

            command.Execute(param);
        }

        protected override void OnPreviewMouseDown(MouseButtonEventArgs e)
        {
            SliderDragStart(this, e);
            base.OnPreviewMouseDown(e);
        }

        protected override void OnPreviewKeyDown(KeyEventArgs e)
        {
            e.Handled = true;
            //base.OnKeyDown(e);
        }

        public static void SetSliderDragStartCommand(UIElement element, ICommand value)
        {
            element.SetValue(SliderDragStartCommandProperty, value);
        }

        public static ICommand GetSliderDragStartCommand(UIElement element)
        {
            return (ICommand)element.GetValue(SliderDragStartCommandProperty);
        }
    }
}
