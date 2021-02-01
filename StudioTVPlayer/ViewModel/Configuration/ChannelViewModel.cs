using StudioTVPlayer.Providers;
using System.Linq;
using TVPlayR;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ChannelViewModel : RemovableViewModelBase
    {
        private string _name;
        private DecklinkDevice _selectedDevice;
        private VideoFormat _selectedVideoFormat;
        private PixelFormat _selectedPixelFormat;

        public ChannelViewModel(Model.Channel channel)
        {
            Channel = channel;
            _name = channel.Name;
            _selectedDevice = DecklinkDevice.EnumerateDevices().FirstOrDefault(dd => dd.Index == channel.DeviceIndex);
            _selectedVideoFormat = VideoFormat.EnumVideoFormats().FirstOrDefault(vf => vf.Name == channel.VideoFormatName);
            _selectedPixelFormat = channel.PixelFormat;
        }

        public Model.Channel Channel { get; }

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
            if (Channel.DeviceIndex != SelectedDevice.Index || Channel.PixelFormat != SelectedPixelFormat || Channel.VideoFormat != SelectedVideoFormat)
                Channel.Uninitialize();
            Channel.Name = Name;
            Channel.DeviceIndex = SelectedDevice.Index;
            Channel.PixelFormat = SelectedPixelFormat;
            Channel.VideoFormat = SelectedVideoFormat;
            IsModified = false;
        }

        public override bool IsValid()
        {
            return SelectedDevice != null && SelectedVideoFormat != null;
        }
    }
}
