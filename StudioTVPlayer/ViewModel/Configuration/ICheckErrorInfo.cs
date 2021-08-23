using System;

namespace StudioTVPlayer.ViewModel.Configuration
{
    interface ICheckErrorInfo
    {
        event EventHandler<CheckErrorEventArgs> CheckErrorInfo;
    }

    public class CheckErrorEventArgs: EventArgs
    {
        public CheckErrorEventArgs(ViewModelBase source, string propertyName)
        {
            Source = source;
            PropertyName = propertyName;
        }

        public ViewModelBase Source { get; }
        
        public string PropertyName { get; }

        public string Message { get; set; } = string.Empty;
    }
}
