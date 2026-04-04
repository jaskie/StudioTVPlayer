using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;

namespace StudioTVPlayer.Helpers
{
    /// <summary>
    ///   Contains helper method for showing waitcursor
    /// </summary>
    public static class UISBusyState
    {

        private static bool _isBusy;

        /// <summary>
        /// Sets the cursor to busy and wait when application is idle.
        /// </summary>
        public static void Set()
        {
            SetBusyState(true);
        }

        private static void SetBusyState(bool busy)
        {
            if (busy == _isBusy)
                return;
            _isBusy = busy;
            Application.Current?.Dispatcher.BeginInvoke(() => Mouse.OverrideCursor = busy ? Cursors.Wait : null, DispatcherPriority.Send);
            if (_isBusy)
                Application.Current?.Dispatcher.BeginInvoke(() => SetBusyState(false), DispatcherPriority.ApplicationIdle);
        }

    }
}
