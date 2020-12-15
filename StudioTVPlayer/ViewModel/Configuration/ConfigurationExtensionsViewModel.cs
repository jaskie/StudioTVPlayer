using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ConfigurationExtensionsViewModel : ViewModelBase
    {
        private IExchangeService _exchangeService = null;
        public UiCommand AddRowCommand { get; set; }
        public UiCommand DeleteRowCommand { get; set; }
        public UiCommand UnloadedCommand { get; set; }
        public UiCommand BrowseCommand { get; set; }

        

        private StringWrapper _selectedExtension;
        public StringWrapper SelectedExtension
        {
            get => _selectedExtension;
            set
            {
                Set(ref _selectedExtension, value);
            }
        }

        private ObservableCollection<StringWrapper> _extensions = new ObservableCollection<StringWrapper>();
        public ObservableCollection<StringWrapper> Extensions
        {
            get => _extensions;
            set
            {
                Set(ref _extensions, value);
            }
        }    

        public ConfigurationExtensionsViewModel(IExchangeService vMNotifyService)
        {
            _exchangeService = vMNotifyService;
            Extensions.CollectionChanged += Extensions_CollectionChanged;           
            LoadCommands();
            LoadData();
        }

        private void LoadData()
        {
            foreach(var extension in Model.Configuration.Instance.Extensions)
            {
                Extensions.Add(new StringWrapper(extension));
            }
        }

        private void Extensions_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            _exchangeService.RaiseConfigurationIsModifiedChanged(new ModifiedEventArgs(true));
        }

        private void LoadCommands()
        {
            AddRowCommand = new UiCommand(AddRow);
            DeleteRowCommand = new UiCommand(DeleteRow, CanDeleteRow);
            UnloadedCommand = new UiCommand((param) => { CommitChanges(param); });
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
            if (_selectedExtension != null) return true;
            return false;
        }

        private void DeleteRow(object obj)
        {
            if (obj != null)
            {
                Extensions.Remove((StringWrapper)obj);
            }
        }

        private void AddRow(object obj)
        {
            Extensions.Add(new StringWrapper(String.Empty));
        }

        protected override bool Set<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (!EqualityComparer<T>.Default.Equals(field, value))
                _exchangeService.RaiseConfigurationIsModifiedChanged(new ModifiedEventArgs(true));
            return base.Set(ref field, value, propertyName);
        }
    }
}
