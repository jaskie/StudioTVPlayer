using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace StudioTVPlayer.Providers
{
    internal static class GlobalUiHandlers
    {
        /// <summary>
        /// Registers event handlers for the TextBox class to automatically select all text when the TextBox gains
        /// keyboard focus.
        /// </summary>
        public static void RegisterTextBoxSelectAllOnFocus()
        {
            EventManager.RegisterClassHandler(typeof(TextBox),
                TextBox.GotKeyboardFocusEvent,
                new RoutedEventHandler(SelectAllText));

            EventManager.RegisterClassHandler(typeof(TextBox),
                TextBox.PreviewMouseDownEvent,
                new MouseButtonEventHandler(SelectivelyIgnoreMouseButton));

            void SelectAllText(object sender, RoutedEventArgs e)
            {
                ((TextBox)sender).SelectAll();
            }

            void SelectivelyIgnoreMouseButton(object sender, MouseButtonEventArgs e)
            {
                if (!((TextBox)sender).IsKeyboardFocusWithin)
                {
                    ((TextBox)sender).Focus();
                    e.Handled = true;
                }
            }
        }
    }
}
