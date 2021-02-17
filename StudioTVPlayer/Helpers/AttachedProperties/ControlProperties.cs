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



        public static readonly DependencyProperty TakesInputBindingPrecedenceProperty =
            DependencyProperty.RegisterAttached("TakesInputBindingPrecedence",
            typeof(bool), typeof(ControlProperties),
            new FrameworkPropertyMetadata(false, OnTakesInputBindingPrecedenceChanged));

        public static bool GetTakesInputBindingPrecedence(UIElement obj) => (bool)obj.GetValue(TakesInputBindingPrecedenceProperty);

        public static void SetTakesInputBindingPrecedence(UIElement obj, bool value) => obj.SetValue(TakesInputBindingPrecedenceProperty, value);

        private static void OnTakesInputBindingPrecedenceChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (!(e.NewValue is bool value))
                return;
            if (value)
                ((UIElement)d).PreviewKeyDown += new KeyEventHandler(InputBindingsBehavior_PreviewKeyDown);
            else
                ((UIElement)d).PreviewKeyDown -= new KeyEventHandler(InputBindingsBehavior_PreviewKeyDown);
        }

        private static void InputBindingsBehavior_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            var uielement = (UIElement)sender;

            var foundBinding = uielement.InputBindings
                .OfType<KeyBinding>()
                .FirstOrDefault(kb => kb.Key == e.Key && kb.Modifiers == e.KeyboardDevice.Modifiers);

            if (foundBinding != null)
            {
                e.Handled = true;
                if (foundBinding.Command.CanExecute(foundBinding.CommandParameter))
                {
                    foundBinding.Command.Execute(foundBinding.CommandParameter);
                }
            }
        }
    }
}
