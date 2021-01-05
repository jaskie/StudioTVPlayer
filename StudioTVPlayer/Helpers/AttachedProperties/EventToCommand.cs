using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace StudioTVPlayer.Helpers.AttachedProperties
{
    public class EventToCommand
    {
        public static readonly DependencyProperty DropCommandProperty =
        DependencyProperty.RegisterAttached(
            "DropCommand",
            typeof(ICommand),
            typeof(EventToCommand),
            new FrameworkPropertyMetadata(DropCommandChanged));

        private static void DropCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)d;
            element.PreviewDrop += new DragEventHandler(element_Drop);
        }

        static void element_Drop(object sender, DragEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)sender;

            ICommand command = GetDropCommand(element);
            object[] param = { sender, e };

            command.Execute(param);
        }

        public static void SetDropCommand(UIElement element, ICommand value) => element.SetValue(DropCommandProperty, value);
        public static ICommand GetDropCommand(UIElement element) => (ICommand)element.GetValue(DropCommandProperty);

        public static readonly DependencyProperty MouseDoubleClickCommandProperty =
            DependencyProperty.RegisterAttached(
                "MouseDoubleClickCommand",
                typeof(ICommand),
                typeof(EventToCommand),
                new FrameworkPropertyMetadata(MouseDoubleClickCommandChanged));

        private static void MouseDoubleClickCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Control element = (Control)d;
            element.MouseDoubleClick += element_MouseDoubleClick;
        }

        static void element_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton != MouseButton.Left)
                return;

            FrameworkElement element = (FrameworkElement)sender;

            ICommand command = GetMouseDoubleClickCommand(element);
            object[] param = { sender, e };

            command.Execute(param);
        }

        public static void SetMouseDoubleClickCommand(UIElement element, ICommand value) => element.SetValue(MouseDoubleClickCommandProperty, value);
        public static ICommand GetMouseDoubleClickCommand(UIElement element) => (ICommand)element.GetValue(MouseDoubleClickCommandProperty);

        public static readonly DependencyProperty MouseLeftButtonDownCommandProperty =
            DependencyProperty.RegisterAttached(
                "MouseLeftButtonDownCommand",
                typeof(ICommand),
                typeof(EventToCommand),
                new FrameworkPropertyMetadata(MouseLeftButtonDownCommandChanged));

        private static void MouseLeftButtonDownCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)d;
            element.PreviewMouseLeftButtonDown += element_MouseLeftButtonDown;
        }

        static void element_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)sender;

            ICommand command = GetMouseLeftButtonDownCommand(element);
            object[] param = { sender, e };

            command.Execute(param);
        }

        public static void SetMouseLeftButtonDownCommand(UIElement element, ICommand value) => element.SetValue(MouseLeftButtonDownCommandProperty, value);
        public static ICommand GetMouseLeftButtonDownCommand(UIElement element) => (ICommand)element.GetValue(MouseLeftButtonDownCommandProperty);

        public static readonly DependencyProperty MouseLeftButtonUpCommandProperty =
        DependencyProperty.RegisterAttached("MouseLeftButtonUpCommand", 
            typeof(ICommand), 
            typeof(EventToCommand), 
            new FrameworkPropertyMetadata(MouseLeftButtonUpCommandChanged));

        private static void MouseLeftButtonUpCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)d;
            element.PreviewMouseLeftButtonUp += element_MouseLeftButtonUp;
        }

        static void element_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)sender;

            ICommand command = GetMouseLeftButtonUpCommand(element);
            object[] param = { sender, e };

            command.Execute(param);
        }

        public static void SetMouseLeftButtonUpCommand(UIElement element, ICommand value) => element.SetValue(MouseLeftButtonUpCommandProperty, value);
        public static ICommand GetMouseLeftButtonUpCommand(UIElement element) => (ICommand)element.GetValue(MouseLeftButtonUpCommandProperty);

        public static readonly DependencyProperty MouseRightButtonUpCommandProperty =
        DependencyProperty.RegisterAttached("MouseRightButtonUpCommand", 
            typeof(ICommand), 
            typeof(EventToCommand), 
            new FrameworkPropertyMetadata(MouseRightButtonUpCommandChanged));

        private static void MouseRightButtonUpCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)d;
            element.PreviewMouseRightButtonUp += element_MouseRightButtonUp;
        }

        static void element_MouseRightButtonUp(object sender, MouseButtonEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)sender;

            ICommand command = GetMouseRightButtonUpCommand(element);
            object[] param = { sender, e };

            command.Execute(param);
        }

        public static void SetMouseRightButtonUpCommand(UIElement element, ICommand value) => element.SetValue(MouseRightButtonUpCommandProperty, value);
        public static ICommand GetMouseRightButtonUpCommand(UIElement element) => (ICommand)element.GetValue(MouseRightButtonUpCommandProperty);


        public static readonly DependencyProperty UnloadedCommandProperty =
        DependencyProperty.RegisterAttached("UnloadedCommand", 
            typeof(ICommand), 
            typeof(EventToCommand), 
            new FrameworkPropertyMetadata(UnloadedCommandChanged));

        private static void UnloadedCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)d;
            element.Unloaded += element_Unloaded;
        }

        static void element_Unloaded(object sender, RoutedEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)sender;

            ICommand command = GetUnloadedCommand(element);
            object[] param = { sender, e };

            command?.Execute(param);
        }

        public static void SetUnloadedCommand(UIElement element, ICommand value) => element.SetValue(UnloadedCommandProperty, value);
        public static ICommand GetUnloadedCommand(UIElement element) => (ICommand)element.GetValue(UnloadedCommandProperty);

        public static readonly DependencyProperty TakesInputBindingPrecedenceProperty =
        DependencyProperty.RegisterAttached("TakesInputBindingPrecedence", 
            typeof(bool), typeof(EventToCommand), 
            new FrameworkPropertyMetadata(false, OnTakesInputBindingPrecedenceChanged));

        public static bool GetTakesInputBindingPrecedence(UIElement obj) => (bool)obj.GetValue(TakesInputBindingPrecedenceProperty);

        public static void SetTakesInputBindingPrecedence(UIElement obj, bool value) => obj.SetValue(TakesInputBindingPrecedenceProperty, value);

        private static void OnTakesInputBindingPrecedenceChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ((UIElement)d).PreviewKeyDown += new KeyEventHandler(InputBindingsBehavior_PreviewKeyDown);
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
