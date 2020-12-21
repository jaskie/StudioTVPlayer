using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Runtime.CompilerServices;
using System.Windows.Controls;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ConfigurationWatchersViewModel : ViewModelBase
    {
        private readonly IGlobalApplicationData _globalApplicationData = SimpleIoc.GetInstance<IGlobalApplicationData>();

        private IExchangeService _exchangeService;
        public UiCommand AddRowCommand { get; set; }
        public UiCommand DeleteRowCommand { get; set; }
        public UiCommand UnloadedCommand { get; set; }
        public UiCommand BrowseCommand { get; set; }     

        private Watcher _selectedPath;
        public Watcher SelectedPath
        {
            get => _selectedPath;
            set
            {
                Set(ref _selectedPath, value);
            }
        }

        private ObservableCollection<Watcher> _watcherMetas = new ObservableCollection<Watcher>();
        public ObservableCollection<Watcher> WatcherMetas
        {
            get => _watcherMetas;
            set
            {
                Set(ref _watcherMetas, value);
            }
        }

        public ConfigurationWatchersViewModel(IExchangeService vMNotifyService)
        {
            _exchangeService = vMNotifyService;
            WatcherMetas.CollectionChanged += WatcherMetas_CollectionChanged;
            LoadCommands();
            LoadData();
        }

        private void LoadData()
        {
            foreach (var ingestPath in _globalApplicationData.Configuration.Watchers)
            {
                WatcherMetas.Add(ingestPath.Clone());
            }
        }

        private void WatcherMetas_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            _exchangeService.RaiseConfigurationIsModifiedChanged(new ModifiedEventArgs(true));
        }

        private void LoadCommands()
        {
            AddRowCommand = new UiCommand(AddRow);
            DeleteRowCommand = new UiCommand(DeleteRow, CanDeleteRow);
            UnloadedCommand = new UiCommand((param) => { CommitChanges(param); });
            BrowseCommand = new UiCommand(Browse);
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

        private void Browse(object obj)
        {
            using (var dialog = new System.Windows.Forms.FolderBrowserDialog())
            {
                System.Windows.Forms.DialogResult result = dialog.ShowDialog();
                if (result == System.Windows.Forms.DialogResult.OK)
                {
                    SelectedPath.Path = dialog.SelectedPath;
                }
            }
        }
        
        private bool CanDeleteRow(object obj)
        {
            if (_selectedPath != null) return true;
            return false;
        }

        private void DeleteRow(object obj)
        {
            if (obj != null)
            {
                WatcherMetas.Remove((Watcher)obj);
            }
        }       

        private void AddRow(object obj)
        {
            WatcherMetas.Add(new Watcher());            
        }

        protected override bool Set<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (!EqualityComparer<T>.Default.Equals(field, value))
                _exchangeService.RaiseConfigurationIsModifiedChanged(new ModifiedEventArgs(true));
            return base.Set(ref field, value, propertyName);
        }
    }
}
