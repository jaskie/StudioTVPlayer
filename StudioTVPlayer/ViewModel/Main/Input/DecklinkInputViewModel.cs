using System.ComponentModel;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Main.Input
{
    public class DecklinkInputViewModel : RemovableViewModelBase, IDataErrorInfo
    {
        private TVPlayR.VideoFormat _videoFormat;
        private TVPlayR.DecklinkInfo _selectedDevice;
        private readonly Model.DecklinkInput _input;

        public DecklinkInputViewModel(Model.DecklinkInput input)
        {
            _input = input;
            _selectedDevice = Devices.FirstOrDefault(d => d.Index == input.DeviceIndex);
            _videoFormat = VideoFormats.FirstOrDefault(f => f.Name == input.VideoFormat);
        }

        public TVPlayR.DecklinkInfo SelectedDevice { get => _selectedDevice; set => Set(ref _selectedDevice , value); }

        public TVPlayR.DecklinkInfo[] Devices => TVPlayR.DecklinkIterator.Devices;

        public TVPlayR.VideoFormat VideoFormat { get => _videoFormat; set => Set(ref _videoFormat, value); }

        public TVPlayR.VideoFormat[] VideoFormats => TVPlayR.VideoFormat.Formats;

        public bool IsEnabled { get; set; }

        public string Error => string.Empty;

        public string this[string columnName] => ReadErrorInfo(columnName);

        public override bool IsValid()
        {
            return string.IsNullOrEmpty(ReadErrorInfo(nameof(VideoFormat)))
                && string.IsNullOrEmpty(ReadErrorInfo(nameof(SelectedDevice)));
        }

        public override void Apply()
        {
            _input.DeviceIndex = SelectedDevice.Index;
            _input.VideoFormat = VideoFormat.Name;
        }

        protected override bool CanRequestRemove(object obj)
        {
            return !IsEnabled;
        }

        private string ReadErrorInfo(string propertyName)
        {
            switch(propertyName)
            {
                case nameof(SelectedDevice) when SelectedDevice is null:
                    return "Input device have to be set";
                case nameof(VideoFormat) when VideoFormat is null:
                    return "Video format can't be empty";
                default:
                    return string.Empty;                    
            }
        }
    }
}