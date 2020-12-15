using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows.Controls;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ConfigurationChannelsViewModel : ViewModelBase
    {
        public Model.Configuration Configuration { get => Model.Configuration.Instance; }

        private IExchangeService _exchangeService;
        public UiCommand AddRowCommand { get; set; }
        public UiCommand DeleteRowCommand { get; set; }
        public UiCommand UnloadedCommand { get; set; }
        public UiCommand BrowseCommand { get; set; }

        private Channel _selectedChannel;
        public Channel SelectedChannel
        {
            get => _selectedChannel;
            set
            {
                Set(ref _selectedChannel, value);
            }
        }

        private ObservableCollection<Channel> _channels = new ObservableCollection<Channel>();
        public ObservableCollection<Channel> Channels
        {
            get => _channels;
            set
            {
                Set(ref _channels, value);
            }
        }      

        public ConfigurationChannelsViewModel(IExchangeService vMNotifyService)
        {
            _exchangeService = vMNotifyService;
            LoadCommands();
            LoadData();
        }

        private void LoadCommands()
        {
            AddRowCommand = new UiCommand(AddRow);
            DeleteRowCommand = new UiCommand(DeleteRow, CanDeleteRow);
            UnloadedCommand = new UiCommand((param) => { CommitChanges(param); });            
        }

        private void LoadData()
        {           
            for (int i = 0; i < Model.Configuration.Instance.Channels.Count; ++i)
            {
                Channels.Add(Model.Configuration.Instance.Channels[i].Clone());
                Channels[i].PropertyChanged += Channels_PropertyChanged;
            }
            Channels.CollectionChanged += Channels_CollectionChanged;
        }

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
                Channels.Remove((Channel)obj);
            }
        }        

        private void AddRow(object obj)
        {
            Channels.Add(new Channel());
            int index = 0;
            while (true)
            {
                if (Channels.FirstOrDefault(param => param.ID == index) != null)
                {
                    ++index;
                    continue;
                }
                Channels.Last().ID = index;
                break;
            }
        }

        protected override bool Set<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (!EqualityComparer<T>.Default.Equals(field, value))
                _exchangeService.RaiseConfigurationIsModifiedChanged(new ModifiedEventArgs(true));
            return base.Set(ref field, value, propertyName);
        }
    }
}
