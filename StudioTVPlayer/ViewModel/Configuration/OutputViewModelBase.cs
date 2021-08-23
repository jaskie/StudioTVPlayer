using StudioTVPlayer.Model;
using System;
using System.ComponentModel;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public abstract class OutputViewModelBase : RemovableViewModelBase, IDataErrorInfo, ICheckErrorInfo
    {
        private readonly OutputBase _output;
        private bool _isFrameClock;

        public OutputViewModelBase(OutputBase output)
        {
            _output = output;
            _isFrameClock = output.IsFrameClock;
        }

        public string this[string columnName] => ReadErrorInfo(columnName);

        public bool IsFrameClock { get => _isFrameClock; set => Set(ref _isFrameClock, value); }

        public string Error => string.Empty;

        public event EventHandler<CheckErrorEventArgs> CheckErrorInfo;

        public override void Apply()
        {
            if (!IsModified)
                return;
            _output.IsFrameClock = _isFrameClock;
        }

        protected string ReadErrorInfo(string propertyName)
        {
            var checkErrorInfo = new CheckErrorEventArgs(this, propertyName);
            if (CheckErrorInfo is null)
                return string.Empty;
            CheckErrorInfo.Invoke(this, checkErrorInfo);
            return checkErrorInfo.Message;
        }
    }
}
