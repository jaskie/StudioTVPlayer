using System.Linq;
using System.Windows.Media;

namespace StudioTVPlayer.ViewModel.Main.Input
{
    public class DecklinkInputViewModel : InputViewModelBase
    {
        private TVPlayR.VideoFormat _videoFormat;
        private TVPlayR.DecklinkInfo _selectedDevice;
        private readonly Model.DecklinkInput _input;

        public DecklinkInputViewModel(Model.DecklinkInput input)
        {
            _input = input;
            _selectedDevice = Devices.FirstOrDefault(d => d.Index == input.DeviceIndex);
            _videoFormat = VideoFormats.FirstOrDefault(f => f.Name == input.VideoFormat);
            input.InputFormatChanged += Input_InputFormatChanged;
            if (_input.Input is null)
                Input.Initialize();
        }

        public TVPlayR.DecklinkInfo SelectedDevice
        {
            get => _selectedDevice; 
            set
            {
                if (!Set(ref _selectedDevice, value))
                    return;
                _input.DeviceIndex = value.Index;
                if (_input.Initialize())
                    ApplyChanges();
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
                    ApplyChanges();
            }
        }

        public TVPlayR.VideoFormat[] VideoFormats => TVPlayR.VideoFormat.Formats;

        public ImageSource Thumbnail => Input.Thumbnail;

        public override Model.InputBase Input => _input;

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
            return true;
        }

        protected override string ReadErrorInfo(string propertyName)
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

        private void Input_InputFormatChanged(object sender, TVPlayR.VideoFormatEventArgs e)
        {
            _videoFormat = e.Format;
            NotifyPropertyChanged(nameof(VideoFormat));
        }

        public override void Dispose()
        {
            _input.InputFormatChanged -= Input_InputFormatChanged;
            base.Dispose();
        }

        private void ApplyChanges()
        {
            Providers.InputList.Current.Save();
            NotifyPropertyChanged(nameof(Thumbnail));
        }

    }
}