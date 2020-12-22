using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model.Interfaces;
using System.Linq;
using TVPlayR;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ChannelViewModel : ModifyableViewModelBase
    {
        private readonly Model.Channel _channel;
        private readonly IGlobalApplicationData _globalApplicationData = SimpleIoc.Get<IGlobalApplicationData>();
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
            _selectedDevice = Devices.FirstOrDefault(d => d.Index == channel.DeviceIndex);
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

        public DecklinkDevice[] Devices => _globalApplicationData.DecklinkDevices;
        public DecklinkDevice SelectedDevice { get => _selectedDevice; set => Set(ref _selectedDevice, value); }

        public VideoFormat[] VideoFormats => _globalApplicationData.VideoFormats;
        public VideoFormat SelectedVideoFormat { get => _selectedVideoFormat; set => Set(ref _selectedVideoFormat, value); }

        public PixelFormat[] PixelFormats => _globalApplicationData.PixelFormats;
        public PixelFormat SelectedPixelFormat { get => _selectedPixelFormat; set => Set(ref _selectedPixelFormat, value); }

        public override void Apply()
        {
            _channel.Id = Id;
            _channel.Name = Name;
            _channel.DeviceIndex = SelectedDevice.Index;
            IsModified = false;
        }
    }
}
