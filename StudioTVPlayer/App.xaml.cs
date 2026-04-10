#undef DEBUG
using StudioTVPlayer.Providers;
using System;
using System.Windows;
using System.Windows.Threading;
using System.Threading;
using System.Security.AccessControl;
using System.Security.Principal;

namespace StudioTVPlayer
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private static SplashScreen _splashScreen;

        // Single-instance primitives (session-scoped)
        private Mutex _singleInstanceMutex;
        private bool _singleInstanceMutexCreatedNew;
        private RegisteredWaitHandle _showInstanceRegisteredWaitHandle;
        private static EventWaitHandle _showExistingInstanceEvent;

        // Use the "Local\" namespace so other user sessions can start their own instance.
        private const string MutexName = "Local\\StudioTVPlayer_SingleInstance_Mutex";
        private const string EventName = "Local\\StudioTVPlayer_ShowInstance_Event";
        private const int ANOTHER_INSTANCE_RUNNING_EXIT = -1;

        public App()
        {
            FrameworkElement.LanguageProperty.OverrideMetadata(
                    typeof(FrameworkElement),
                    new FrameworkPropertyMetadata(
                    System.Windows.Markup.XmlLanguage.GetLanguage(System.Globalization.CultureInfo.CurrentCulture.IetfLanguageTag)));
            DispatcherUnhandledException += (sender, e) => HandleException(e.Exception, false);
            AppDomain.CurrentDomain.UnhandledException += (sender, e) => HandleException(e.ExceptionObject as Exception, e.IsTerminating);
            Dispatcher.UnhandledException += (sender, e) => e.Handled = true;

            var commandLineArgs = Environment.GetCommandLineArgs();
        }

        protected override void OnStartup(StartupEventArgs e)
        {
            // Ensure only a single instance runs in the same Windows session.
            try
            {
                // Create/open a mutex with permissive security rule so processes running under other accounts
                // in the same session can open/check it.
                var everyoneSid = new SecurityIdentifier(WellKnownSidType.WorldSid, null);
                var mutexSecurity = new MutexSecurity();
                mutexSecurity.AddAccessRule(new MutexAccessRule(everyoneSid, MutexRights.FullControl, AccessControlType.Allow));

                _singleInstanceMutex = new Mutex(true, MutexName, out bool createdNew, mutexSecurity);
                _singleInstanceMutexCreatedNew = createdNew;

                // Create/open an EventWaitHandle with permissive ACL so other processes can signal us.
                var eventSecurity = new EventWaitHandleSecurity();
                eventSecurity.AddAccessRule(new EventWaitHandleAccessRule(everyoneSid, EventWaitHandleRights.FullControl, AccessControlType.Allow));

                _showExistingInstanceEvent = new EventWaitHandle(initialState: false, mode: EventResetMode.AutoReset, name: EventName, createdNew: out bool eventCreatedNew, eventSecurity: eventSecurity);

                if (!createdNew)
                {
                    // Another instance exists in this session - signal it to show/restore and exit.
                    try
                    {
                        // Try to open the named event (if we couldn't create the mutex we still may be able to open the event)
                        _showExistingInstanceEvent.Set();
                    }
                    catch
                    {
                        // If we can't open/set the event for any reason, ignore and just exit.
                    }

                    Shutdown(ANOTHER_INSTANCE_RUNNING_EXIT);
                    return;
                }
                else
                {
                    // Register a wait on the event; the callback runs on a ThreadPool thread (no dedicated thread)
                    _showInstanceRegisteredWaitHandle = ThreadPool.RegisterWaitForSingleObject(
                        _showExistingInstanceEvent,
                        (state, timedOut) =>
                        {
                            // marshal to UI thread
                            Current?.Dispatcher?.BeginInvoke(new Action(() =>
                            {
                                try
                                {
                                    if (Current?.MainWindow is View.MainWindow mw)
                                        mw.Restore();
                                    else
                                        Current?.MainWindow?.Activate();
                                }
                                catch { /* swallow UI exceptions from restore attempt */ }
                            }));
                        },
                        state: null,
                        millisecondsTimeOutInterval: -1, // wait indefinitely
                        executeOnlyOnce: false); // keep listening
                }
            }
            catch (UnauthorizedAccessException)
            {
                // If we can't create due to ACLs, try to open existing mutex. If it exists, signal existing instance's event and exit
                try
                {
                    _singleInstanceMutex = Mutex.OpenExisting(MutexName);
                    if (_singleInstanceMutex is not null)
                    {
                        // we try to notify existing event
                        try
                        {
                            using var existingEvent = EventWaitHandle.OpenExisting(EventName);
                            existingEvent?.Set();
                        }
                        catch { }
                        Shutdown(ANOTHER_INSTANCE_RUNNING_EXIT);
                        return;
                    }
                }
                catch
                {
                    // Couldn't open existing mutex — fall through and continue without single-instance guarantee
                }
            }
            catch
            {
                // Any other error obtaining mutex -> continue startup
            }

            _splashScreen = new SplashScreen("/StudioTVPlayer.ico");
            _splashScreen.Show(autoClose: false, topMost: true);
            base.OnStartup(e);
            GlobalUiHandlers.RegisterTextBoxSelectAllOnFocus();
            try
            {
                GlobalApplicationData.Current.Initialize();
            }
            catch (Exception ex)
            {
                _splashScreen.Close(TimeSpan.Zero);
                HandleException(ex, true);
                Shutdown(1);
            }
       }

        public static void CloseSplashScreen()
        {
            _splashScreen?.Close(TimeSpan.FromSeconds(0.5));
            _splashScreen = null;
        }

        protected override void OnExit(ExitEventArgs e)
        {
            base.OnExit(e);
            try
            {
                // if the application was not initialized, we don't need to shutdown it.
                if (e.ApplicationExitCode > ANOTHER_INSTANCE_RUNNING_EXIT)
                {
                    ViewModel.ShellViewModel.Instance.Dispose();
                    GlobalApplicationData.Current.Shutdown();
                }
            }
            catch (OperationCanceledException)
            { }

            // Release and dispose the mutex and event if we own them.
            try
            {
                try { _showExistingInstanceEvent?.Close(); } catch { }

                if (_singleInstanceMutex != null)
                {
                    if (_singleInstanceMutexCreatedNew)
                    {
                        try { _singleInstanceMutex.ReleaseMutex(); } catch { }
                    }
                    _singleInstanceMutex.Dispose();
                    _singleInstanceMutex = null;
                }
            }
            catch { }
        }

        private void HandleException(Exception e, bool isTerminating)
        {
            var exception = (e?.InnerException ?? e);
            var message = $"{exception?.GetType().Name ?? "Error"} occurred with message: {exception?.Message ?? "empty"}";
#if DEBUG
            CrashLogger.SaveDump(e?.ToString());
#else
            if (isTerminating)
                MessageBox.Show(message, "Error - terminating application", MessageBoxButton.OK, MessageBoxImage.Error);
            else
                MessageBox.Show(message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
#endif
        }

    }
}
