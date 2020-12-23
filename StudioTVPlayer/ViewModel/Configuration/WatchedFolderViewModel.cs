using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class WatchedFolderViewModel : ModifyableViewModelBase
    {
        private string _name;
        private string _path;
        private bool _isFiltered;

        public WatchedFolderViewModel(WatchedFolder watchedFolder)
        {
            WatchedFolder = watchedFolder;
            _path = watchedFolder.Path;
            _name = watchedFolder.Name;
            _isFiltered = watchedFolder.IsFiltered;
            BrowseCommand = new UiCommand((_) => {
                var path = Path;
                if (FolderHelper.Browse(ref path, $"Select path for folder {Name}"))
                    Path = path;
            });

        }

        public string Name { get => _name; set => Set(ref _name, value); }

        public string Path { get => _path; set => Set(ref _path, value); }

        public bool IsFiltered { get => _isFiltered; set => Set(ref _isFiltered, value); }

        public UiCommand BrowseCommand { get; }

        public WatchedFolder WatchedFolder { get; }

        public override void Apply()
        {
            WatchedFolder.Path = Path;
            WatchedFolder.Name = Name;
            WatchedFolder.IsFiltered = IsFiltered;
        }
    }
}
