using StudioTVPlayer.Model.Configuration;
using System;
using System.ComponentModel;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public abstract class OutputViewModelBase : RemovableViewModelBase, IDataErrorInfo, ICheckErrorInfo
    {
        private bool _isFrameClock;
        private TVPlayR.TimecodeOverlaySource _timecodeOverlay;

        public OutputViewModelBase(OutputBase output)
        {
            Output = output;
            _isFrameClock = output.IsFrameClock;
            _timecodeOverlay = output.TimecodeOverlay;
        }

        public string this[string columnName] => ReadErrorInfo(columnName);

        public bool IsFrameClock { get => _isFrameClock; set => Set(ref _isFrameClock, value); }

        public TVPlayR.TimecodeOverlaySource TimecodeOverlay { get => _timecodeOverlay; set => Set(ref _timecodeOverlay, value); }

        public Array TimecodeOverlaySources { get; } = Enum.GetValues(typeof(TVPlayR.TimecodeOverlaySource));

        public string Error => string.Empty;

        internal OutputBase Output { get; }

        public event EventHandler<CheckErrorEventArgs> CheckErrorInfo;

        public override void Apply()
        {
            if (!IsModified)
                return;
            Output.IsFrameClock = _isFrameClock;
            Output.TimecodeOverlay = _timecodeOverlay;
        }

        protected virtual string ReadErrorInfo(string propertyName)
        {
            if (CheckErrorInfo is null)
                return string.Empty;
            var checkErrorInfo = new CheckErrorEventArgs(this, propertyName);
            CheckErrorInfo.Invoke(this, checkErrorInfo);
            return checkErrorInfo.Message;
        }
    }
}
