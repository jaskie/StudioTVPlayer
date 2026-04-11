using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Documents;

namespace StudioTVPlayer.Helpers.AttachedProperties
{
    public class HyperlinkProperties
    {
        public static readonly DependencyProperty IsExternalProperty =
            DependencyProperty.RegisterAttached("IsExternal", typeof(bool), typeof(HyperlinkProperties), new UIPropertyMetadata(false, OnIsExternalChanged));

        public static bool GetIsExternal(DependencyObject obj)
        {
            return (bool)obj.GetValue(IsExternalProperty);
        }

        public static void SetIsExternal(DependencyObject obj, bool value)
        {
            obj.SetValue(IsExternalProperty, value);
        }

        private static void OnIsExternalChanged(object sender, DependencyPropertyChangedEventArgs args)
        {
            var hyperlink = sender as Hyperlink ?? throw new ApplicationException($"IsExternalProperty wrongly attached to {sender}");

            if ((bool)args.NewValue)
                hyperlink.RequestNavigate += Hyperlink_RequestNavigate;
            else
                hyperlink.RequestNavigate -= Hyperlink_RequestNavigate;
        }

        private static void Hyperlink_RequestNavigate(object sender, System.Windows.Navigation.RequestNavigateEventArgs e)
        {
            try
            {
                Process.Start(e.Uri.AbsoluteUri);
            }
            catch
            {
                // ignored
            }
            e.Handled = true;
        }
    }
}

