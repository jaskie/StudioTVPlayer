using StudioTVPlayer.Helpers;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Shared
{
    public class DirectorySelectorViewModel : ModifyableViewModelBase, IDataErrorInfo
    {

        private string _directoryName;

        public DirectorySelectorViewModel(string initialFolder) : base()
        {
            _directoryName = initialFolder ?? RecentDirectories.LastOrDefault();
            BrowseForFolderCommand = new UiCommand(BrowseForFolder);
        }

        public ICommand BrowseForFolderCommand { get; }

        public string DirectoryName { get => _directoryName; set => Set(ref _directoryName, value); }

        public IEnumerable<string> RecentDirectories => Providers.MostRecentUsed.Current.Folders;

        public string Error => string.Empty;

        public string this[string columnName] => columnName switch
        {
            nameof(DirectoryName) when !Directory.Exists(_directoryName) => "Directory does not exists",
            _ => string.Empty,
        };

        private void BrowseForFolder(object _)
        {
            if (FolderHelper.BrowseForFolder(ref _directoryName, $"Select folder to capture video"))
            {
                NotifyPropertyChanged(nameof(DirectoryName));
                Providers.MostRecentUsed.Current.AddMostRecentlyUsedFolder(_directoryName);
            }
        }

        public override bool IsValid()
        {
            return string.IsNullOrEmpty(this[nameof(DirectoryName)]);
        }
    }
}
