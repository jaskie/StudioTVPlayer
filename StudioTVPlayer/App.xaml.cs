#undef DEBUG
using StudioTVPlayer.Providers;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;

namespace StudioTVPlayer
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public App()
        {
            DispatcherUnhandledException += (object sender, DispatcherUnhandledExceptionEventArgs e) => HandleException(e.Exception, false);
            FrameworkElement.LanguageProperty.OverrideMetadata(
                    typeof(FrameworkElement),
                    new FrameworkPropertyMetadata(
                    System.Windows.Markup.XmlLanguage.GetLanguage(System.Globalization.CultureInfo.CurrentCulture.IetfLanguageTag)));
            AppDomain.CurrentDomain.UnhandledException += (object sender, UnhandledExceptionEventArgs e) => HandleException(e.ExceptionObject as Exception, e.IsTerminating);
            Dispatcher.UnhandledException += (object sender, DispatcherUnhandledExceptionEventArgs e) => e.Handled = true;
        }

        protected override void OnExit(ExitEventArgs e)
        {
            base.OnExit(e);
            try
            {
                ViewModel.MainViewModel.Instance.Dispose();
                GlobalApplicationData.Current.Shutdown();
            }
            catch (OperationCanceledException)
            { }
        }


        private void HandleException(Exception e, bool isTerminating)
        {
            var exception = (e?.InnerException ?? e);
            var message = $"{e?.GetType().Name ?? "Error without exception"} occured with message {e?.Message ?? "empty"}.";
#if DEBUG
            CrashLogger.SaveDump(e.ToString());
#else
            if (isTerminating)
                MessageBox.Show(message, "Error - terminating application", MessageBoxButton.OK, MessageBoxImage.Error);
            else
                MessageBox.Show(message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
#endif
        }

    }
}
