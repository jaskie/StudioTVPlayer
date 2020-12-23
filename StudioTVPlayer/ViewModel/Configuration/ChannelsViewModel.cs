using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Controls;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Providers;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ChannelsViewModel : ModifyableViewModelBase
    {
        private ChannelViewModel _selectedChannel;

        public ChannelsViewModel()
        {
            AddChannelCommand = new UiCommand(AddChannel);
            DeleteChannelCommand = new UiCommand(DeleteChannel, CanDeleteChannel);
            UnloadedCommand = new UiCommand((param) => { CommitChanges(param); });
            Channels = new ObservableCollection<ChannelViewModel>(GlobalApplicationData.Current.Configuration.Channels.Select(c => new ChannelViewModel(c)));
            foreach (var channel in Channels)
                channel.PropertyChanged += Channel_PropertyChanged;
        }

        private void Channel_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(IsModified))
                IsModified = true;
        }

        public UiCommand AddChannelCommand { get; }
        public UiCommand DeleteChannelCommand { get; }
        public UiCommand UnloadedCommand { get; }
        public ChannelViewModel SelectedChannel
        {
            get => _selectedChannel;
            set => Set(ref _selectedChannel, value);
        }

        public ObservableCollection<ChannelViewModel> Channels { get; }

        private void CommitChanges(object param)
        {
            if (param != null)
            {
                object[] parameters = param as object[];
                DataGrid dg = parameters[0] as DataGrid;
                dg.CommitEdit(DataGridEditingUnit.Row, true);
            }
        }
       
        private bool CanDeleteChannel(object obj)
        {
            if (_selectedChannel != null) return true;
            return false;
        }

        private void DeleteChannel(object obj)
        {
            var channel = obj as ChannelViewModel ?? throw new ArgumentException(nameof(obj));
            Channels.Remove(channel);            
        }        

        private void AddChannel(object obj)
        {
            var channel = new Channel();
            int index = 0;
            while (true)
            {
                if (Channels.FirstOrDefault(param => param.Id == index++) != null)
                    continue;
                channel.Id = index;
                break;
            }
            Channels.Add(new ChannelViewModel(channel));
        }

        public override void Apply()
        {
            foreach (var channel in Channels)
                channel.Apply();
        }
    }
}
