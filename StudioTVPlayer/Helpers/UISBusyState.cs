using System;
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
        /// Sets the cursor to busy and wait when UI thread is idle.
        /// </summary>
        public static void SetBusyState()
        {
            SetBusyState(true);
        }

        private static void SetBusyState(bool busy)
        {
            if (busy == _isBusy)
                return;
            _isBusy = busy;
            Application.Current.Dispatcher.BeginInvoke((Action)(() => Mouse.OverrideCursor = busy ? Cursors.Wait : null));
            if (_isBusy)
                new DispatcherTimer(TimeSpan.Zero, DispatcherPriority.ContextIdle, dispatcherTimer_Tick, Application.Current.Dispatcher);
        }

        private static void dispatcherTimer_Tick(object sender, EventArgs _)
        {
            if (!(sender is DispatcherTimer dispatcherTimer))
                return;
            SetBusyState(false);
            dispatcherTimer.Stop();
            dispatcherTimer.Tick -= dispatcherTimer_Tick;
        }
    }
}
