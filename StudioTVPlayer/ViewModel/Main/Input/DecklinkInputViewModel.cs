﻿using System;
using System.Linq;
using System.Windows.Media;

namespace StudioTVPlayer.ViewModel.Main.Input
{
    public class DecklinkInputViewModel : InputViewModelBase
    {
        private TVPlayR.VideoFormat _videoFormat;
        private TVPlayR.DecklinkInfo _selectedDevice;
        private bool _formatAutodetection;
        private bool _disposed;
        private readonly Model.DecklinkInput _input;

        public DecklinkInputViewModel(Model.DecklinkInput input) : base(input)
        {
            _input = input;
            _formatAutodetection = _input.FormatAutodetection;
            _selectedDevice = Devices.FirstOrDefault(d => d.Index == input.DeviceIndex);
            _videoFormat = VideoFormats.FirstOrDefault(f => f.Name == input.VideoFormat);
            input.FormatChanged += Input_FormatChanged;
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
                NotifyPropertyChanged(nameof(Thumbnail));
                NotifyPropertyChanged(nameof(CanFormatAutodetection));
                NotifyPropertyChanged(nameof(CanSelectVideoFormat));
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
                NotifyPropertyChanged(nameof(Thumbnail));
            }
        }

        public bool FormatAutodetection
        {
            get => _formatAutodetection;
            set
            {
                if (!Set(ref _formatAutodetection, value))
                    return;
                _input.FormatAutodetection = value;
                if (Input.Initialize())
                    ApplyChanges();
                NotifyPropertyChanged(nameof(CanSelectVideoFormat));
            }
        }

        public bool CanFormatAutodetection => _selectedDevice?.SupportsInputModeDetection ?? false;

        public bool CanSelectVideoFormat => !Input.IsRunning || !FormatAutodetection;

        public TVPlayR.VideoFormat[] VideoFormats => TVPlayR.VideoFormat.Formats;

        public ImageSource Thumbnail => Input.Thumbnail;

        public override bool IsValid()
        {
            return string.IsNullOrEmpty(ReadErrorInfo(nameof(VideoFormat)))
                && string.IsNullOrEmpty(ReadErrorInfo(nameof(SelectedDevice)));
        }

        public override void Apply()
        {
            _input.DeviceIndex = SelectedDevice.Index;
            _input.VideoFormat = VideoFormat.Name;
            base.Apply();
        }

        protected override bool CanRequestRemove(object obj)
        {
            return true;
        }

        protected override string ReadErrorInfo(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(SelectedDevice) when SelectedDevice is null:
                    return "Input device have to be set";
                case nameof(VideoFormat) when VideoFormat is null:
                    return "Video format can't be empty";
                default:
                    return string.Empty;
            }
        }

        private void Input_FormatChanged(object sender, EventArgs e)
        {
            var decklink = sender as Model.DecklinkInput ?? throw new ArgumentException(nameof(sender));
            _videoFormat = decklink.CurrentFormat();
            NotifyPropertyChanged(nameof(VideoFormat));
        }

        public override void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            _input.FormatChanged -= Input_FormatChanged;
            base.Dispose();
        }

        private void ApplyChanges()
        {
            Providers.InputList.Current.Save();
            NotifyPropertyChanged(nameof(Thumbnail));
        }

    }
}