﻿using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class PlayerViewModel : RemovableViewModelBase, IDataErrorInfo, ICheckErrorInfo
    {
        private readonly ObservableCollection<OutputViewModelBase> _outputs;
        private readonly bool _canAddDecklinkOutput;
        private readonly bool _canAddNdiOutput;
        private string _name;
        private TVPlayR.VideoFormat _selectedVideoFormat;
        private TVPlayR.PixelFormat _selectedPixelFormat;
        private bool _livePreview;
        private OutputViewModelBase _frameClockSource;
        private bool _disablePlayedItems;
        private bool _addItemsWithAutoPlay;

        public PlayerViewModel(Model.Configuration.Player player)
        {
            Player = player;
            _name = player.Name;
            OutputViewModelBase viewModelSelector(Model.Configuration.OutputBase o)
            {
                switch (o)
                {
                    case Model.Configuration.DecklinkOutput decklink:
                        return new DecklinkOutputViewModel(decklink);
                    case Model.Configuration.NdiOutput ndi:
                        return new NdiOutputViewModel(ndi);
                    case Model.Configuration.FFOutput stream:
                        return new FFOutputViewModel(stream);
                    default:
                        throw new ApplicationException("Invalid type provided");
                }
            }
            _outputs = new ObservableCollection<OutputViewModelBase>(player.Outputs?.Select(viewModelSelector));
            foreach (var output in _outputs)
            {
                output.Modified += (o, e) => IsModified = true;
                output.RemoveRequested += Output_RemoveRequested;
                output.CheckErrorInfo += Output_CheckErrorInfo;
                output.Modified += Output_Modified;
                if (output.IsFrameClock)
                    _frameClockSource = output;
            }
            _selectedVideoFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(vf => vf.Name == player.VideoFormat);
            _selectedPixelFormat = player.PixelFormat;
            _livePreview = player.LivePreview;
            _disablePlayedItems = player.DisablePlayedItems;
            _addItemsWithAutoPlay = player.AddItemsWithAutoPlay;
            _canAddDecklinkOutput =  TVPlayR.DecklinkIterator.Devices.Any(o => o.HaveOutput);
            _canAddNdiOutput = TVPlayR.VersionInfo.Ndi != null;
            AddStreamOutputCommand = new UiCommand(AddStreamOutput);
            AddDecklinkOutputCommand = new UiCommand(AddDecklinkOutput, _ => _canAddDecklinkOutput);
            AddNdiOutputCommand = new UiCommand(AddNdiOutput, _ => _canAddNdiOutput);
        }

        internal Model.Configuration.Player Player { get; }

        public string Name
        {
            get => _name;
            set => Set(ref _name, value);
        }

        public IList<OutputViewModelBase> Outputs => _outputs;

        public OutputViewModelBase FrameClockSource
        {
            get => _frameClockSource;
            set
            {
                var oldClockSource = _frameClockSource;
                if (!Set(ref _frameClockSource, value))
                    return;
                if (!(oldClockSource is null))
                    oldClockSource.IsFrameClock = false;
                if (!(value is null))
                    value.IsFrameClock = true;
            }
        }

        public TVPlayR.VideoFormat[] VideoFormats => TVPlayR.VideoFormat.Formats;

        public TVPlayR.PixelFormat[] PixelFormats { get; } = new[] { TVPlayR.PixelFormat.bgra, TVPlayR.PixelFormat.yuv422, TVPlayR.PixelFormat.rgb10 };

        public TVPlayR.VideoFormat SelectedVideoFormat { get => _selectedVideoFormat; set => Set(ref _selectedVideoFormat, value); }

        public TVPlayR.PixelFormat SelectedPixelFormat { get => _selectedPixelFormat; set => Set(ref _selectedPixelFormat, value); }

        public bool LivePreview { get => _livePreview; set => Set(ref _livePreview, value); }

        public bool DisablePlayedItems { get => _disablePlayedItems; set => Set(ref _disablePlayedItems, value); }

        public bool AddItemsWithAutoPlay { get => _addItemsWithAutoPlay; set => Set(ref _addItemsWithAutoPlay, value); }

        public ICommand AddStreamOutputCommand { get; }
        public ICommand AddDecklinkOutputCommand { get; }
        public ICommand AddNdiOutputCommand { get; }

        public string Error => string.Empty;

        public string this[string columnName]
        {
            get
            {
                switch (columnName)
                {
                    case nameof(FrameClockSource) when FrameClockSource is null:
                        return "You have to add Decklink or NDI output to provide clock source for the channel";
                    case nameof(SelectedVideoFormat) when SelectedVideoFormat is null:
                        return "A video format is required";
                    case nameof(Name) when string.IsNullOrWhiteSpace(Name):
                        return "Channel name can not be empty";
                    case nameof(Name):
                        if (CheckErrorInfo is null)
                            return string.Empty;
                        var ea = new CheckErrorEventArgs(this, nameof(Name));
                        CheckErrorInfo.Invoke(this, ea);
                        return ea.Message;
                    default:
                        return string.Empty;
                }
            }
        }

        public event EventHandler<CheckErrorEventArgs> CheckErrorInfo;

        public override void Apply()
        {
            if (!IsModified)
                return;
            foreach (var output in Outputs)
                output.Apply();
            Player.Name = Name;
            Player.PixelFormat = SelectedPixelFormat;
            Player.VideoFormat = SelectedVideoFormat.Name;
            Player.LivePreview = LivePreview;
            Player.DisablePlayedItems = DisablePlayedItems;
            Player.AddItemsWithAutoPlay = AddItemsWithAutoPlay;
            Player.Outputs = Outputs.Select(o => o.OutputConfiguration).ToArray();
            base.Apply();
        }

        public override bool IsValid()
        {
            return Outputs.All(o => o.IsValid()) && SelectedVideoFormat != null;
        }

        private void AddNdiOutput(object _)
        {
            var vm = new NdiOutputViewModel(new Model.Configuration.NdiOutput() { IsFrameClock = !Outputs.Any(a => a.IsFrameClock), SourceName = Name });
            AddOutput(vm);
        }

        private void AddDecklinkOutput(object _)
        {
            var vm = new DecklinkOutputViewModel(new Model.Configuration.DecklinkOutput() { IsFrameClock = !Outputs.Any(a => a.IsFrameClock) });
            AddOutput(vm);
        }

        private void AddStreamOutput(object _)
        {
            var vm = new FFOutputViewModel(new Model.Configuration.FFOutput
            {
                IsFrameClock = !Outputs.Any(a => a.IsFrameClock),
                EncoderSettings = new Model.EncoderSettings
                {
                    VideoBitrate = 4000,
                    VideoCodec = TVPlayR.FFOutput.VideoCodecs.FirstOrDefault(),
                    AudioBitrate = 128,
                    AudioCodec = TVPlayR.FFOutput.AudioCodecs.FirstOrDefault()
                }
            });
            AddOutput(vm);
        }

        private void AddOutput(OutputViewModelBase vm)
        {
            Outputs.Add(vm);
            if (vm.IsFrameClock)
                FrameClockSource = vm;
            vm.RemoveRequested += Output_RemoveRequested;
            vm.CheckErrorInfo += Output_CheckErrorInfo;
            vm.Modified += Output_Modified;
            IsModified = true;
        }

        private void Output_Modified(object sender, EventArgs e)
        {
            IsModified = true;
        }

        private void Output_RemoveRequested(object sender, EventArgs e)
        {
            var output = sender as OutputViewModelBase ?? throw new ArgumentException(nameof(sender));
            output.RemoveRequested -= Output_RemoveRequested;
            output.CheckErrorInfo -= Output_CheckErrorInfo;
            output.Modified -= Output_Modified;
            Outputs.Remove(output);
            IsModified = true;
        }

        private void Output_CheckErrorInfo(object sender, CheckErrorEventArgs e)
        {
            CheckErrorInfo?.Invoke(sender, e); // forward the query
        }
    }
}
