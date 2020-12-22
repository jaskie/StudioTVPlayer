using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows.Controls;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ChannelsViewModel : ViewModelBase
    {
        private readonly IExchangeService _exchangeService;
        private ChannelViewModel _selectedChannel;

        public ChannelsViewModel(IGlobalApplicationData globalApplicationData, IExchangeService exchangeService)
        {
            _exchangeService = exchangeService;
            AddChannelCommand = new UiCommand(AddRow);
            DeleteChannelCommand = new UiCommand(DeleteRow, CanDeleteRow);
            UnloadedCommand = new UiCommand((param) => { CommitChanges(param); });
            Channels = new ObservableCollection<ChannelViewModel>(globalApplicationData.Configuration.Channels.Select(c => new ChannelViewModel(c)));
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

        private void Channels_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            _exchangeService.RaiseConfigurationIsModifiedChanged(new ModifiedEventArgs(true));
        }

        private void Channels_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            _exchangeService.RaiseConfigurationIsModifiedChanged(new ModifiedEventArgs(true));
        }

        private void CommitChanges(object param)
        {
            if (param != null)
            {
                object[] parameters = param as object[];
                DataGrid dg = parameters[0] as DataGrid;
                dg.CommitEdit(DataGridEditingUnit.Row, true);
            }
        }
       
        private bool CanDeleteRow(object obj)
        {
            if (_selectedChannel != null) return true;
            return false;
        }

        private void DeleteRow(object obj)
        {
            if (obj != null)
            {
                Channels.Remove((ChannelViewModel)obj);
            }
        }        

        private void AddRow(object obj)
        {
            Channels.Add(new ChannelViewModel(new Channel()));
            int index = 0;
            while (true)
            {
                if (Channels.FirstOrDefault(param => param.Id == index) != null)
                {
                    ++index;
                    continue;
                }
                Channels.Last().Id = index;
                break;
            }
        }


        protected override bool Set<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (!base.Set(ref field, value, propertyName))
                return false;
            _exchangeService.RaiseConfigurationIsModifiedChanged(new ModifiedEventArgs(true));
            return true;
        }
    }
}
