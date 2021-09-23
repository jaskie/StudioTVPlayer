using System.ComponentModel;
using System.Linq;
using System.Windows.Media;

namespace StudioTVPlayer.ViewModel.Main.Input
{
    public class DecklinkInputViewModel : RemovableViewModelBase, IDataErrorInfo
    {
        private TVPlayR.VideoFormat _videoFormat;
        private TVPlayR.DecklinkInfo _selectedDevice;

        public DecklinkInputViewModel(Model.DecklinkInput input)
        {
            Input = input;
            _selectedDevice = Devices.FirstOrDefault(d => d.Index == input.DeviceIndex);
            _videoFormat = VideoFormats.FirstOrDefault(f => f.Name == input.VideoFormat);
            if (Input.GetInput() is null)
                Input.Initialize();
        }

        public TVPlayR.DecklinkInfo SelectedDevice
        {
            get => _selectedDevice; 
            set
            {
                if (!Set(ref _selectedDevice, value))
                    return;
                Input.DeviceIndex = value.Index;
                if (Input.Initialize())
                    Providers.InputList.Current.Save();
            }
        }

        public TVPlayR.DecklinkInfo[] Devices => TVPlayR.DecklinkIterator.Devices;

        public TVPlayR.VideoFormat VideoFormat
        {
            get => _videoFormat; 
            set
            {
                if (!Set(ref _videoFormat, value))
                    return;
                Input.VideoFormat = value.Name;
                if (Input.Initialize())
                    Providers.InputList.Current.Save();
            }
        }

        public TVPlayR.VideoFormat[] VideoFormats => TVPlayR.VideoFormat.Formats;

        public ImageSource Thumbnail => Input.Thumbnail;

        public string Error => string.Empty;

        public Model.DecklinkInput Input { get; }

        public string this[string columnName] => ReadErrorInfo(columnName);

        public override bool IsValid()
        {
            return string.IsNullOrEmpty(ReadErrorInfo(nameof(VideoFormat)))
                && string.IsNullOrEmpty(ReadErrorInfo(nameof(SelectedDevice)));
        }

        public override void Apply()
        {
            Input.DeviceIndex = SelectedDevice.Index;
            Input.VideoFormat = VideoFormat.Name;
        }

        protected override bool CanRequestRemove(object obj)
        {
            return true;
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