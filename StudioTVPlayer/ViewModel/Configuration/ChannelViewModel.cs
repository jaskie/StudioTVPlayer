using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ChannelViewModel : RemovableViewModelBase, IDataErrorInfo, ICheckErrorInfo
    {
        private readonly ObservableCollection<OutputViewModelBase> _outputs;
        private string _name;
        private TVPlayR.VideoFormat _selectedVideoFormat;
        private TVPlayR.PixelFormat _selectedPixelFormat;
        private bool _livePreview;
        private OutputViewModelBase _frameClockSource;

        public ChannelViewModel(Model.Channel channel)
        {
            Channel = channel;
            _name = channel.Name;
            OutputViewModelBase viewModelSelector(Model.OutputBase o)
            {
                switch (o)
                {
                    case Model.DecklinkOutput decklink:
                        return new DecklinkOutputViewModel(decklink);
                    case Model.NdiOutput ndi:
                        return new NdiOutputViewModel(ndi);
                    case Model.FFOutput stream:
                        return new FFOutputViewModel(stream);
                    default:
                        throw new ApplicationException("Invalid type provided");
                }
            }
            _outputs = new ObservableCollection<OutputViewModelBase>(channel.Outputs.Select(viewModelSelector));
            foreach (var output in _outputs)
            {
                output.Modified += (o, e) => IsModified = true;
                output.RemoveRequested += Output_RemoveRequested;
                output.CheckErrorInfo += Output_CheckErrorInfo;
                output.Modified += Output_Modified;
                if (output.IsFrameClock)
                    _frameClockSource = output;
            }
            _selectedVideoFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(vf => vf.Name == channel.VideoFormatName);
            _selectedPixelFormat = channel.PixelFormat;
            _livePreview = channel.LivePreview;
            AddStreamOutputCommand = new UiCommand(AddStreamOutput);
            AddDecklinkOutputCommand = new UiCommand(AddDecklinkOutput);
            AddNdiOutputCommand = new UiCommand(AddNdiOutput);
        }

        internal Model.Channel Channel { get; }

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

        public TVPlayR.PixelFormat[] PixelFormats { get; } = new[] { TVPlayR.PixelFormat.bgra, TVPlayR.PixelFormat.yuv422 };

        public TVPlayR.VideoFormat SelectedVideoFormat { get => _selectedVideoFormat; set => Set(ref _selectedVideoFormat, value); }

        public TVPlayR.PixelFormat SelectedPixelFormat { get => _selectedPixelFormat; set => Set(ref _selectedPixelFormat, value); }

        public bool LivePreview { get => _livePreview; set => Set(ref _livePreview, value); }

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
            if (Outputs.Any(o => o.IsModified) || Channel.PixelFormat != SelectedPixelFormat || Channel.VideoFormat != SelectedVideoFormat || Channel.LivePreview != LivePreview)
                Channel.Uninitialize();
            foreach (var output in Outputs)
                output.Apply();
            Channel.Name = Name;
            Channel.PixelFormat = SelectedPixelFormat;
            Channel.VideoFormat = SelectedVideoFormat;
            Channel.LivePreview = LivePreview;
            Channel.Outputs = Outputs.Select(o => o.Output).ToArray();
            IsModified = false;
        }

        public override bool IsValid()
        {
            return Outputs.All(o => o.IsValid()) && SelectedVideoFormat != null;
        }

        private void AddNdiOutput(object _)
        {
            var vm = new NdiOutputViewModel(new Model.NdiOutput() { IsFrameClock = !Outputs.Any(a => a.IsFrameClock), SourceName = Name });
            AddOutput(vm);
        }

        private void AddDecklinkOutput(object _)
        {
            var vm = new DecklinkOutputViewModel(new Model.DecklinkOutput() { IsFrameClock = !Outputs.Any(a => a.IsFrameClock) });
            AddOutput(vm);
        }

        private void AddStreamOutput(object _)
        {
            var vm = new FFOutputViewModel(new Model.FFOutput { 
                IsFrameClock = !Outputs.Any(a => a.IsFrameClock),
                VideoBitrate = 4000,
                VideoCodec = FFOutputViewModel.VideoCodecs.FirstOrDefault(),
                AudioBitrate = 128,
                AudioCodec = FFOutputViewModel.AudioCodecs.FirstOrDefault(),
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
