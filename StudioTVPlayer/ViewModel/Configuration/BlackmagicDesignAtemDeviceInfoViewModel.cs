using StudioTVPlayer.Model;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public sealed class BlackmagicDesignAtemDeviceInfoViewModel : ViewModelBase
    {
        private readonly string _deviceId;
        private string _deviceName;
        private string _address;
        private string _modelName;
        private int _port;
        public BlackmagicDesignAtemDeviceInfoViewModel(Model.BlackmagicDesignAtemDeviceInfo deviceInfo)
        {
            _deviceId = deviceInfo.DeviceId;
            Update(deviceInfo);
        }
        public string DeviceId => _deviceId;
        public string DeviceName { get => _deviceName; set => Set(ref _deviceName, value); }
        public string Address { get => _address; set => Set(ref _address, value); }
        public string ModelName { get => _modelName; set => Set(ref _modelName, value); }
        public int Port { get => _port; set => Set(ref _port, value); }

        public void Update(BlackmagicDesignAtemDeviceInfo device)
        {
            DeviceName = device.DeviceName;
            Address = device.Address.ToString();
            ModelName = device.ModelName;
            Port = device.Port;
        }
    }
}
