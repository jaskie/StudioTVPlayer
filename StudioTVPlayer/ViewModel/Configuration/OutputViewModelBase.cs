using System;
using System.ComponentModel;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public abstract class OutputViewModelBase(Model.Configuration.OutputBase outputConfiguration) : RemovableViewModelBase, IDataErrorInfo, ICheckErrorInfo
    {
        private bool _isFrameClock = outputConfiguration.IsFrameClock;
        private TVPlayR.TimecodeOutputSource _timecodeOverlay = outputConfiguration.TimecodeOverlay;
        internal Model.Configuration.OutputBase OutputConfiguration = outputConfiguration;

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
