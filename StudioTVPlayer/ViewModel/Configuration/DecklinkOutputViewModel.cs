﻿using StudioTVPlayer.Model;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class DecklinkOutputViewModel : OutputViewModelBase
    {
        private DecklinkOutput _decklink;
        private TVPlayR.DecklinkOutput _selectedDevice;

        public DecklinkOutputViewModel(DecklinkOutput decklink): base(decklink)
        {
            _decklink = decklink;
            SelectedDevice = Devices.FirstOrDefault(d => d.Index == decklink.DeviceIndex);
        }

        public static TVPlayR.DecklinkOutput[] Devices { get; } = TVPlayR.DecklinkOutput.EnumerateDevices();

        public TVPlayR.DecklinkOutput SelectedDevice { get => _selectedDevice; set => Set(ref _selectedDevice, value); }

        public override void Apply()
        {
            if (!IsModified)
                return;
            base.Apply();
            _decklink.DeviceIndex = SelectedDevice.Index;
        }

        public override bool IsValid()
        {
            return !(SelectedDevice is null);
        }
    }
}