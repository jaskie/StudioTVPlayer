using System;

namespace StudioTVPlayer.ViewModel.Configuration
{
    interface ICheckErrorInfo
    {
        event EventHandler<CheckErrorEventArgs> CheckErrorInfo;
    }

    public class CheckErrorEventArgs(ViewModelBase source, string propertyName) : EventArgs
    {
        public ViewModelBase Source { get; } = source;

        public string PropertyName { get; } = propertyName;

        public string Message { get; set; } = string.Empty;
    }
}
