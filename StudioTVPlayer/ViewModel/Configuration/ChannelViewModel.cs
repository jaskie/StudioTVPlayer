using StudioTVPlayer.Providers;
using System.Linq;
using TVPlayR;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ChannelViewModel : RemovableViewModelBase
    {
        private readonly Model.Channel _channel;
        private int _id;
        private string _name;
        private DecklinkDevice _selectedDevice;
        private VideoFormat _selectedVideoFormat;
        private PixelFormat _selectedPixelFormat;

        public ChannelViewModel(Model.Channel channel)
        {
            _channel = channel;
            _name = channel.Name;
            _id = channel.Id;
            _selectedDevice = GlobalApplicationData.Current.DecklinkDevices.FirstOrDefault(d => d.Index == channel.DeviceIndex);
            _selectedVideoFormat = GlobalApplicationData.Current.VideoFormats.FirstOrDefault(d => d.Id == channel.VideoFormatId);
            _selectedPixelFormat = channel.PixelFormat;
        }

        public int Id
        {
            get => _id;
            set => Set(ref _id, value);
        }

        public string Name
        {
            get => _name;
            set => Set(ref _name, value);
        }

        public DecklinkDevice SelectedDevice { get => _selectedDevice; set => Set(ref _selectedDevice, value); }

        public VideoFormat SelectedVideoFormat { get => _selectedVideoFormat; set => Set(ref _selectedVideoFormat, value); }

        public PixelFormat SelectedPixelFormat { get => _selectedPixelFormat; set => Set(ref _selectedPixelFormat, value); }

        public override void Apply()
        {
            if (!IsModified)
                return;
            if (_channel.DeviceIndex != SelectedDevice.Index)
                _channel.Uninitialize();
            _channel.Id = Id;
            _channel.Name = Name;
            _channel.DeviceIndex = SelectedDevice.Index;
            _channel.PixelFormat = SelectedPixelFormat;
            _channel.VideoFormat = SelectedVideoFormat;
            IsModified = false;
        }

        public override bool IsValid()
        {
            return SelectedDevice != null && SelectedVideoFormat != null;
        }
    }
}
