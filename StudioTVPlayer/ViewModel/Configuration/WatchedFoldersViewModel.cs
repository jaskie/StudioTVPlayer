using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Controls;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Providers;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class WatchedFoldersViewModel : ModifyableViewModelBase
    {
        public UiCommand AddWatchedFolderCommand { get; }
        public UiCommand DeleteWatchedFolderCommand { get; }
        public UiCommand UnloadedCommand { get; }

        private WatchedFolderViewModel _selectedWatchedFolder;
        public WatchedFolderViewModel SelectedWatchedFolder
        {
            get => _selectedWatchedFolder;
            set => Set(ref _selectedWatchedFolder, value);
        }

        public ObservableCollection<WatchedFolderViewModel> WatchedFolders { get; }

        public WatchedFoldersViewModel()
        {
            WatchedFolders = new ObservableCollection<WatchedFolderViewModel>(GlobalApplicationData.Current.Configuration.WatchedFolders.Select(f =>
            {
                var vm = new WatchedFolderViewModel(f);
                vm.PropertyChanged += WatchedFolder_PropertyChanged;
                return vm;
            }));
            AddWatchedFolderCommand = new UiCommand(AddWatchedFolder);
            DeleteWatchedFolderCommand = new UiCommand(DeleteWatchedFolder, CanEditOrDeleteWatchedFolder);
            UnloadedCommand = new UiCommand((param) => { CommitChanges(param); });
        }

        private void WatchedFolder_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(IsModified))
                IsModified = true;
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

        private bool CanEditOrDeleteWatchedFolder(object obj) => SelectedWatchedFolder != null;

        private void DeleteWatchedFolder(object obj)
        {
            var folder = obj as WatchedFolderViewModel ?? throw new ArgumentException(nameof(obj));
            WatchedFolders.Remove(folder);
        }       

        private void AddWatchedFolder(object obj)
        {
            string path = Environment.GetFolderPath(Environment.SpecialFolder.MyComputer);
            if (FolderHelper.Browse(ref path, "Select path for new watched folder"))
            {
                var name = path.Split(new[] { System.IO.Path.DirectorySeparatorChar }, StringSplitOptions.RemoveEmptyEntries).LastOrDefault() ?? "New watched folder";
                WatchedFolders.Add(new WatchedFolderViewModel(new WatchedFolder() { Path = path, Name = name }));
            }
        }

        public override void Apply()
        {
            GlobalApplicationData.Current.Configuration.WatchedFolders = WatchedFolders.Select(f => {
                f.Apply();
                return f.WatchedFolder;
            }).ToList();
        }
    }
}
