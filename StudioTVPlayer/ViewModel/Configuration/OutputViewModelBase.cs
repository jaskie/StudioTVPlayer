using System;
using System.ComponentModel;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public abstract class OutputViewModelBase : RemovableViewModelBase, IDataErrorInfo, ICheckErrorInfo
    {
        private bool _isFrameClock;
        private TVPlayR.TimecodeOutputSource _timecodeOverlay;
        internal Model.Configuration.OutputBase OutputConfiguration;

        public OutputViewModelBase(Model.Configuration.OutputBase outputConfiguration)
        {
            OutputConfiguration = outputConfiguration;
            _isFrameClock = outputConfiguration.IsFrameClock;
            _timecodeOverlay = outputConfiguration.TimecodeOverlay;
        }

        public string this[string columnName] => ReadErrorInfo(columnName);

        public bool IsFrameClock { get => _isFrameClock; set => Set(ref _isFrameClock, value); }

        public TVPlayR.TimecodeOutputSource TimecodeOverlay { get => _timecodeOverlay; set => Set(ref _timecodeOverlay, value); }

        public Array TimecodeOutputSources { get; } = Enum.GetValues(typeof(TVPlayR.TimecodeOutputSource));

        public string Error => string.Empty;

        public event EventHandler<CheckErrorEventArgs> CheckErrorInfo;

        public override void Apply()
        {
            if (!IsModified)
                return;
            OutputConfiguration.IsFrameClock = _isFrameClock;
            OutputConfiguration.TimecodeOverlay = _timecodeOverlay;
            base.Apply();
        }

        protected virtual string ReadErrorInfo(string propertyName)
        {
            if (CheckErrorInfo is null)
                return string.Empty;
            var checkErrorInfo = new CheckErrorEventArgs(this, propertyName);
            CheckErrorInfo(this, checkErrorInfo);
            return checkErrorInfo.Message;
        }
    }
}
