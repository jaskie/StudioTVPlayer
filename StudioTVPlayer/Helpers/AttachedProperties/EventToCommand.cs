using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace StudioTVPlayer.Helpers.AttachedProperties
{
    public class EventToCommand
    {
        public static readonly DependencyProperty MouseDoubleClickCommandProperty =
            DependencyProperty.RegisterAttached(
                "MouseDoubleClickCommand",
                typeof(ICommand),
                typeof(EventToCommand),
                new FrameworkPropertyMetadata(MouseDoubleClickCommandChanged));

        private static void MouseDoubleClickCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (e.NewValue != null && e.OldValue == null)
                ((Control)d).MouseDoubleClick += element_MouseDoubleClick;
            if (e.NewValue == null && e.OldValue != null)
                ((Control)d).MouseDoubleClick -= element_MouseDoubleClick;
        }

        private static void element_MouseDoubleClick(object sender, MouseButtonEventArgs e)
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
            if (e.NewValue != null && e.OldValue == null)
                ((FrameworkElement)d).PreviewMouseLeftButtonDown += element_MouseLeftButtonDown;
            if (e.NewValue == null && e.OldValue != null)
                ((FrameworkElement)d).PreviewMouseLeftButtonDown -= element_MouseLeftButtonDown;
        }

        private static void element_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
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
            if (e.NewValue != null && e.OldValue == null)
                ((FrameworkElement)d).PreviewMouseLeftButtonUp += element_MouseLeftButtonUp;
            if (e.NewValue == null && e.OldValue != null)
                ((FrameworkElement)d).PreviewMouseLeftButtonUp -= element_MouseLeftButtonUp;
        }

        private static void element_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)sender;

            ICommand command = GetMouseLeftButtonUpCommand(element);
            object[] param = { sender, e };

            command.Execute(param);
        }

        public static void SetMouseLeftButtonUpCommand(UIElement element, ICommand value) => element.SetValue(MouseLeftButtonUpCommandProperty, value);
        public static ICommand GetMouseLeftButtonUpCommand(UIElement element) => (ICommand)element.GetValue(MouseLeftButtonUpCommandProperty);

        public static readonly DependencyProperty MouseRightButtonDownCommandProperty =
        DependencyProperty.RegisterAttached("MouseRightButtonDownCommand", 
            typeof(ICommand), 
            typeof(EventToCommand), 
            new FrameworkPropertyMetadata(MouseRightButtonDownCommandChanged));

        private static void MouseRightButtonDownCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (e.NewValue != null && e.OldValue == null)
                ((FrameworkElement)d).PreviewMouseRightButtonDown += element_MouseRightButtonDown;
            if (e.NewValue == null && e.OldValue != null)
                ((FrameworkElement)d).PreviewMouseRightButtonDown -= element_MouseRightButtonDown;
        }

        private static void element_MouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)sender;

            ICommand command = GetMouseRightButtonDownCommand(element);
            object[] param = { sender, e };

            command.Execute(param);
        }

        public static void SetMouseRightButtonDownCommand(UIElement element, ICommand value) => element.SetValue(MouseRightButtonDownCommandProperty, value);
        public static ICommand GetMouseRightButtonDownCommand(UIElement element) => (ICommand)element.GetValue(MouseRightButtonDownCommandProperty);


        public static readonly DependencyProperty UnloadedCommandProperty =
        DependencyProperty.RegisterAttached("UnloadedCommand", 
            typeof(ICommand), 
            typeof(EventToCommand), 
            new FrameworkPropertyMetadata(UnloadedCommandChanged));

        private static void UnloadedCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (e.NewValue != null && e.OldValue == null)
                ((FrameworkElement)d).Unloaded += element_Unloaded;
            if (e.NewValue == null && e.OldValue != null)
                ((FrameworkElement)d).Unloaded -= element_Unloaded;
        }

        private static void element_Unloaded(object sender, RoutedEventArgs e)
        {
            FrameworkElement element = (FrameworkElement)sender;

            ICommand command = GetUnloadedCommand(element);
            object[] param = { sender, e };

            command?.Execute(param);
        }

        public static void SetUnloadedCommand(UIElement element, ICommand value) => element.SetValue(UnloadedCommandProperty, value);
        public static ICommand GetUnloadedCommand(UIElement element) => (ICommand)element.GetValue(UnloadedCommandProperty);


    }
}
