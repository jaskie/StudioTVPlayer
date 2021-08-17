using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ChannelViewModel : RemovableViewModelBase
    {
        private readonly ObservableCollection<OutputViewModelBase> _outputs;
        private string _name;
        private TVPlayR.VideoFormat _selectedVideoFormat;
        private TVPlayR.PixelFormat _selectedPixelFormat;
        private bool _livePreview;

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
                    default:
                        throw new ApplicationException("Invalid type provided");
                }
            }
            _outputs = new ObservableCollection<OutputViewModelBase>(channel.Outputs.Select(viewModelSelector));
            foreach (var output in _outputs)
            {
                output.Modified += (o, e) => IsModified = true;
                output.RemoveRequested += Output_RemoveRequested;

            }
            _selectedVideoFormat = TVPlayR.VideoFormat.EnumVideoFormats().FirstOrDefault(vf => vf.Name == channel.VideoFormatName);
            _selectedPixelFormat = channel.PixelFormat;
            _livePreview = channel.LivePreview;
            AddDecklinkOutputCommand = new UiCommand(AddDecklinkOutput);
            AddNdiOutputCommand = new UiCommand(AddNdiOutput);
        }

        public Model.Channel Channel { get; }

        public string Name
        {
            get => _name;
            set => Set(ref _name, value);
        }

        public IList<OutputViewModelBase> Outputs => _outputs;

        public static TVPlayR.VideoFormat[] VideoFormats { get; } = TVPlayR.VideoFormat.EnumVideoFormats();

        public static TVPlayR.PixelFormat[] PixelFormats { get; } = new[] { TVPlayR.PixelFormat.bgra, TVPlayR.PixelFormat.yuv422 };

        public TVPlayR.VideoFormat SelectedVideoFormat { get => _selectedVideoFormat; set => Set(ref _selectedVideoFormat, value); }

        public TVPlayR.PixelFormat SelectedPixelFormat { get => _selectedPixelFormat; set => Set(ref _selectedPixelFormat, value); }

        public bool LivePreview { get => _livePreview; set => Set(ref _livePreview, value); }

        public ICommand AddDecklinkOutputCommand { get; }
        
        public ICommand AddNdiOutputCommand { get; }


        public override void Apply()
        {
            if (IsModified)
                return;
            if (Outputs.Any(o => o.IsModified) || Channel.PixelFormat != SelectedPixelFormat || Channel.VideoFormat != SelectedVideoFormat || Channel.LivePreview != LivePreview)
                Channel.Uninitialize();
            foreach (var output in Outputs)
                output.Apply();
            Channel.Name = Name;
            Channel.PixelFormat = SelectedPixelFormat;
            Channel.VideoFormat = SelectedVideoFormat;
            Channel.LivePreview = LivePreview;
            IsModified = false;
        }

        public override bool IsValid()
        {
            return Outputs.All(o => o.IsValid()) && SelectedVideoFormat != null;
        }

        private void AddNdiOutput(object obj)
        {
            var vm = new NdiOutputViewModel(new Model.NdiOutput());
            Outputs.Add(vm);
            vm.RemoveRequested += Output_RemoveRequested;
        }

        private void AddDecklinkOutput(object obj)
        {
            var vm = new DecklinkOutputViewModel(new Model.DecklinkOutput());
            Outputs.Add(vm);
            vm.RemoveRequested += Output_RemoveRequested;
        }

        private void Output_RemoveRequested(object sender, EventArgs e)
        {
            var output = sender as OutputViewModelBase ?? throw new ArgumentException(nameof(sender));
            output.RemoveRequested -= Output_RemoveRequested;
            Outputs.Remove(output);
        }

    }
}
