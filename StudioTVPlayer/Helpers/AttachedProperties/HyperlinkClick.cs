using System;
using System.Windows;
using System.Windows.Documents;
using System.Windows.Input;

namespace StudioTVPlayer.Helpers.AttachedProperties
{
    public class HyperlinkClick
    {
        public static readonly DependencyProperty ClickCommandProperty =
            DependencyProperty.RegisterAttached(
                "ClickCommand",
                typeof(ICommand),
                typeof(HyperlinkClick),
                new FrameworkPropertyMetadata(HyperlinkClickCommandChanged));

        private static void HyperlinkClickCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (e.NewValue != null && e.OldValue == null)
                ((Hyperlink)d).Click += OnHyperlinkClick;
            if (e.NewValue == null && e.OldValue != null)
                ((Hyperlink)d).Click -= OnHyperlinkClick;
        }

        private static void OnHyperlinkClick(object sender, RoutedEventArgs e)
        {
            Hyperlink hyperlink = sender as Hyperlink ?? throw new ArithmeticException(nameof(sender));
            ICommand command = GetClickCommand(hyperlink);
            object commandParameter = GetClickCommandParameter(hyperlink);
            command.Execute(commandParameter);
        }


        public static void SetClickCommand(DependencyObject element, ICommand value) => element.SetValue(ClickCommandProperty, value);
        public static ICommand GetClickCommand(DependencyObject element) => (ICommand)element.GetValue(ClickCommandProperty);

        public static readonly DependencyProperty ClickCommandParameterProperty =
        DependencyProperty.RegisterAttached("ClickCommandParameter", typeof(object), typeof(HyperlinkClick));

        public static object GetClickCommandParameter(DependencyObject obj)
        {
            return obj.GetValue(ClickCommandParameterProperty);
        }
        
        public static void SetClickCommandParameter(DependencyObject obj, object value)
        {
            obj.SetValue(ClickCommandParameterProperty, value);
        }

    }

}
