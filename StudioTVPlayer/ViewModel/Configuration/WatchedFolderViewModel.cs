using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using System.IO;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class WatchedFolderViewModel : RemovableViewModelBase
    {
        private string _name;
        private string _path;
        private bool _isFilteredByDate;
        private string _filter;

        public WatchedFolderViewModel(WatchedFolder watchedFolder)
        {
            WatchedFolder = watchedFolder;
            _path = watchedFolder.Path;
            _name = watchedFolder.Name;
            _isFilteredByDate = watchedFolder.IsFilteredByDate;
            _filter = watchedFolder.Filter;
            BrowseCommand = new UiCommand((_) =>
            {
                var path = Path;
                if (FolderHelper.Browse(ref path, $"Select path for folder {Name}"))
                    Path = path;
            });

        }

        public string Name { get => _name; set => Set(ref _name, value); }

        public string Path { get => _path; set => Set(ref _path, value); }

        public bool IsFilteredByDate { get => _isFilteredByDate; set => Set(ref _isFilteredByDate, value); }

        public string Filter { get => _filter; set => Set(ref _filter, value); }

        public UiCommand BrowseCommand { get; }

        public WatchedFolder WatchedFolder { get; }

        public override void Apply()
        {
            WatchedFolder.Path = Path;
            WatchedFolder.Name = Name;
            WatchedFolder.IsFilteredByDate = IsFilteredByDate;
            WatchedFolder.Filter = Filter;
            IsModified = false;
        }

        public override bool IsValid()
        {
            return !string.IsNullOrEmpty(Name) && Directory.Exists(Path);
        }
    }
}
