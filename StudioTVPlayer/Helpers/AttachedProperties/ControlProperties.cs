using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace StudioTVPlayer.Helpers.AttachedProperties
{
    public class ControlProperties
    {
        public static readonly DependencyProperty MouseStartDragCommandProperty =
            DependencyProperty.RegisterAttached(
                "MouseStartDragCommand", 
                typeof(ICommand), typeof(ControlProperties), 
                new FrameworkPropertyMetadata(MouseStartDragCommandChanged));

        public static readonly DependencyProperty IsFocusedProperty =
            DependencyProperty.RegisterAttached(
                "IsFocused", typeof(bool), typeof(ControlProperties),
                new FrameworkPropertyMetadata(false, OnIsFocusedPropertyChanged));


        private static void MouseStartDragCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)d;
            element.PreviewMouseMove += element_MouseStartDrag;
        }

        static void element_MouseStartDrag(object sender, MouseEventArgs e)
        {
            if (e.LeftButton != MouseButtonState.Pressed)
                return;

            FrameworkElement element = (FrameworkElement)sender;

            ICommand command = GetMouseStartDragCommand(element);
            object[] param = { sender, e };

            command.Execute(param);
        }

        public static void SetMouseStartDragCommand(UIElement element, ICommand value) => element.SetValue(MouseStartDragCommandProperty, value);
        public static ICommand GetMouseStartDragCommand(UIElement element) => (ICommand)element.GetValue(MouseStartDragCommandProperty);


        private static void OnIsFocusedPropertyChanged(
            DependencyObject d,
            DependencyPropertyChangedEventArgs e)
        {
            var uie = (UIElement)d;
            if ((bool)e.NewValue)
            {
                uie.Focus(); // Don't care about false values.
                Keyboard.Focus(uie);
            }
        }

        public static bool GetIsFocused(DependencyObject obj) => (bool)obj.GetValue(IsFocusedProperty);
        public static void SetIsFocused(DependencyObject obj, bool value) => obj.SetValue(IsFocusedProperty, value);

    }
}
