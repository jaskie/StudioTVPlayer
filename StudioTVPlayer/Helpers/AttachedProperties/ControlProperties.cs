using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Input;

namespace StudioTVPlayer.Helpers.AttachedProperties
{
    public class ControlProperties
    {

        public static readonly DependencyProperty EnableIsKeyboardFocusWithinProperty =
            DependencyProperty.RegisterAttached(
                "EnableIsKeyboardFocusWithin", typeof(bool), typeof(ControlProperties),
                new FrameworkPropertyMetadata(false, EnableIsKeyboardFocusWithinPropertyChanged));
                
        private static void EnableIsKeyboardFocusWithinPropertyChanged(
            DependencyObject d,
            DependencyPropertyChangedEventArgs e)
        {
            var attachEvents = (bool)e.NewValue;
            var targetUiElement = (UIElement)d;
            if (attachEvents)
                targetUiElement.IsKeyboardFocusWithinChanged += TargetUiElement_IsKeyboardFocusWithinChanged;
            else
                targetUiElement.IsKeyboardFocusWithinChanged -= TargetUiElement_IsKeyboardFocusWithinChanged;
        }

        private static void TargetUiElement_IsKeyboardFocusWithinChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            var isSelected = (bool)e.NewValue;
            var targetUiElement = (UIElement)sender;
            System.Windows.Controls.Primitives.Selector.SetIsSelected(targetUiElement, isSelected);
        }

        public static bool GetEnableIsKeyboardFocusWithin(DependencyObject obj) => (bool)obj.GetValue(EnableIsKeyboardFocusWithinProperty);
        public static void SetEnableIsKeyboardFocusWithin(DependencyObject obj, bool value) => obj.SetValue(EnableIsKeyboardFocusWithinProperty, value);

    }
}
