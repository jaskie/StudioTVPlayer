using OpenMacroBoard.SDK;
using StreamDeckSharp;
using StreamDeckSharp.Exceptions;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model.Configuration;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public sealed class ElgatoStreamDeckPlayerControllerViewModel : PlayerControllerViewModelBase
    {
        private readonly ObservableCollection<ElgatoStreamDeckPlayerBindingViewModel> _bindings = new ObservableCollection<ElgatoStreamDeckPlayerBindingViewModel>();
        private StreamDeckDevice[] _devices;
        private StreamDeckDevice _selectedDevice;
        private bool _connect;
        private IMacroBoard _macroBoard;
        private bool _isConnected;

        public ElgatoStreamDeckPlayerControllerViewModel(Model.Configuration.ElgatoStreamDeckPlayerController playerControllerConfiguration = null) : base(playerControllerConfiguration ?? new Model.Configuration.ElgatoStreamDeckPlayerController())
        {
            RefreshDevices();
            _selectedDevice = playerControllerConfiguration is null
                ? _devices.FirstOrDefault()
                : _devices.FirstOrDefault(d => d.Device.DevicePath == playerControllerConfiguration.Path);
            RefreshDevicesCommand = new UiCommand(RefreshDevices);
        }

        public ICommand RefreshDevicesCommand { get; }

        public override string DisplayName => _selectedDevice?.Device.DeviceName ?? string.Empty;

        public IEnumerable<StreamDeckDevice> Devices => _devices;

        public IEnumerable<ElgatoStreamDeckPlayerBindingViewModel> Bindings => _bindings;

        public StreamDeckDevice SelectedDevice
        {
            get => _selectedDevice;
            set
            {
                if (!Set(ref _selectedDevice, value))
                    return;
                NotifyPropertyChanged(nameof(DisplayName));
            }
        }

        public string DevicePath => SelectedDevice?.Device.DevicePath;

        public bool CanPressConnect => SelectedDevice != null;

        public bool IsConnected { get => _isConnected; }

        public bool Connect
        {
            get => _connect;
            set
            {
                if (_connect == value)
                    return;
                _connect = value;
                var path = DevicePath;
                if (!string.IsNullOrEmpty(path))
                {
                    if (value)
                    {
                        try
                        {
                            _macroBoard = StreamDeck.OpenDevice(path);
                            _macroBoard.ConnectionStateChanged += OnConnectionStateChanged;
                            _macroBoard.KeyStateChanged += OnKeyStateChanged;
                            _isConnected = _macroBoard.IsConnected;
                            NotifyPropertyChanged(nameof(IsConnected));
                        }
                        catch (StreamDeckNotFoundException)
                        { }
                    }
                    else
                    {
                        if (_macroBoard != null)
                        {
                            _macroBoard.Dispose();
                        }
                    }
                }
                else
                {
                    _isConnected = false;
                    NotifyPropertyChanged(nameof(IsConnected));
                }
                NotifyPropertyChanged();
            }
        }

        private void OnKeyStateChanged(object sender, OpenMacroBoard.SDK.KeyEventArgs e)
        {
            if (e.IsDown)
            {
                foreach (var binding in _bindings)
                    binding.KeyReceived(e.Key);
            }
        }

        private void OnConnectionStateChanged(object sender, ConnectionEventArgs e)
        {
            if (e.NewConnectionState)
                return;
            Debug.Assert(_macroBoard != null);
            _macroBoard.ConnectionStateChanged -= OnConnectionStateChanged;
            _macroBoard.KeyStateChanged -= OnKeyStateChanged;
            _macroBoard = null;
            _isConnected = false;
            _connect = false;
            NotifyPropertyChanged(nameof(IsConnected));
            NotifyPropertyChanged(nameof(Connect));
        }

        public override bool IsValid()
        {
            return !IsModified || (_selectedDevice != null && base.IsValid());
        }

        protected override PlayerControllerBindingViewModelBase CreatePlayerControlerBindingViewModel(PlayerBindingBase bindingConfiguration = null)
        {
            var elgatoStreamDeckPlayerBindingConfiguration = bindingConfiguration as Model.Configuration.ElgatoStreamDeckPlayerBinding ?? throw new ArgumentException(nameof(bindingConfiguration));
            return new ElgatoStreamDeckPlayerBindingViewModel(elgatoStreamDeckPlayerBindingConfiguration);
        }

        protected override string ReadErrorInfo(string columnName)
        {
            switch (columnName)
            {
                case nameof(SelectedDevice) when SelectedDevice is null:
                    return "Please select some panel";
            }
            return base.ReadErrorInfo(columnName);
        }

        private void RefreshDevices(object _ = null)
        {
            int deviceIndex = 1;
            _devices = StreamDeck.EnumerateDevices().Select(d => new StreamDeckDevice(d, deviceIndex++)).ToArray();
            NotifyPropertyChanged(nameof(Devices));
        }

        public class StreamDeckDevice
        {
            private int _index;
            public StreamDeckDevice(StreamDeckDeviceReference Device, int index)
            {
                this.Device = Device;
                _index = index;
            }
            public StreamDeckDeviceReference Device { get; }

            public string DisplayName => $"[{_index}] {Device.DeviceName}";
        }

    }
}
