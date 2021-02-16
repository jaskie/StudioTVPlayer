using System;
using System.Collections.Generic;
using System.Diagnostics;
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
            Debug.WriteLine("Focus on {0} attached:{1}", targetUiElement, attachEvents);
            if (attachEvents)
            {
                targetUiElement.IsKeyboardFocusWithinChanged += TargetUiElement_IsKeyboardFocusWithinChanged;
                targetUiElement.MouseDown += TargetUiElement_MouseDown;
            }
            else
            {
                targetUiElement.IsKeyboardFocusWithinChanged -= TargetUiElement_IsKeyboardFocusWithinChanged;
                targetUiElement.MouseDown -= TargetUiElement_MouseDown;
            }
        }

        private static void TargetUiElement_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ButtonState != MouseButtonState.Pressed || e.ChangedButton != MouseButton.Left || e.ClickCount != 1)
                return;
            (sender as UIElement).Focus();
        }

        private static void TargetUiElement_IsKeyboardFocusWithinChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            var isSelected = (bool)e.NewValue;
            var targetUiElement = (UIElement)sender;
            Debug.WriteLine("Focus on {0}:{1}", targetUiElement, isSelected);
            if (isSelected)
                System.Windows.Controls.Primitives.Selector.SetIsSelected(targetUiElement, isSelected);
        }

        public static bool GetEnableIsKeyboardFocusWithin(DependencyObject obj) => (bool)obj.GetValue(EnableIsKeyboardFocusWithinProperty);
        public static void SetEnableIsKeyboardFocusWithin(DependencyObject obj, bool value) => obj.SetValue(EnableIsKeyboardFocusWithinProperty, value);

    }
}
