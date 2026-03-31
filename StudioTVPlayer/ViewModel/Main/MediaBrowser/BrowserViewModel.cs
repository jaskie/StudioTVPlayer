using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Windows.Input;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Args;

namespace StudioTVPlayer.ViewModel.Main.MediaBrowser
{
    public class BrowserViewModel : ViewModelBase, IDisposable
    {
        private readonly IList<MediaViewModel> _mediaFiles;
        private readonly WatchedFolder _watchedFolder;
        private bool _isDisposed;
        private Sorting _selectedSorting;
        private MediaViewModel _selectedMedia;
        private DateTime _selectedDate;

        public ICommand QueueToPlayerByIndexCommand { get; }

        public ICommand ChangeDateCommand { get; }

        public ICommand ExploreFolderCommand { get; }

        public BrowserViewModel(WatchedFolder watchedFolder)
        {
            _watchedFolder = watchedFolder;
            _watchedFolder.MediaChanged += WatchedFolder_MediaChanged;
            _watchedFolder.Initialize();
            _mediaFiles = [.. watchedFolder.Medias.Select(m => new MediaViewModel(m))];
            MediaFiles = System.Windows.Data.CollectionViewSource.GetDefaultView(_mediaFiles);
            MediaFiles.Filter = MediaFilter;
            MediaFiles.SortDescriptions.Add(new SortDescription(nameof(MediaViewModel.CreationTime), ListSortDirection.Descending));
            _selectedSorting = Sorting.CreationTime;
            Name = watchedFolder.Name;
            Path = watchedFolder.Path;
            IsFilteredByDate = watchedFolder.IsFilteredByDate;
            _selectedDate = watchedFolder.FilterDate;
            QueueToPlayerByIndexCommand = new UiCommand(QueueToPlayerByIndex);
            ChangeDateCommand = new UiCommand(ChangeDate, _ => IsFilteredByDate);
            ExploreFolderCommand = new UiCommand(ExploreFolder);
        }

        #region Properties

        public string Name { get; }
        public string Path { get; }
        public bool IsFilteredByDate { get; }

        private bool _isFocused;
        private string _filter;

        public bool IsFocused
        {
            get => _isFocused;
            set => Set(ref _isFocused, value);
        }

        public ICollectionView MediaFiles { get; }

        public MediaViewModel SelectedMedia
        {
            get => _selectedMedia;
            set => Set(ref _selectedMedia, value);
        }

        public DateTime SelectedDate
        {
            get => _selectedDate;
            set
            {
                if (!Set(ref _selectedDate, value))
                    return;
                _watchedFolder.FilterDate = value;
                _mediaFiles.Clear();
                foreach (var media in _watchedFolder.Medias)
                    _mediaFiles.Add(new MediaViewModel(media));
                MediaFiles.Refresh();
            }
        }

        public string Filter
        {
            get => _filter; 
            set
            {
                if (!Set(ref _filter, value))
                    return;
                MediaFiles.Refresh();
            }
        }


        public Array Sortings { get; } = Enum.GetValues(typeof(Sorting));

        public Sorting SelectedSorting
        {
            get => _selectedSorting;
            set
            {
                if (!Set(ref _selectedSorting, value))
                    return;
                SortMediaCollection();
            }
        }

        #endregion

        private void SortMediaCollection()
        {
            MediaFiles.SortDescriptions.Clear();
            switch (SelectedSorting)
            {
                case Sorting.Name:
                    MediaFiles.SortDescriptions.Add(new SortDescription(nameof(MediaViewModel.Name), ListSortDirection.Ascending));
                    break;
                case Sorting.Duration:
                    MediaFiles.SortDescriptions.Add(new SortDescription(nameof(MediaViewModel.Duration), ListSortDirection.Ascending));
                    break;
                case Sorting.CreationTime:
                    MediaFiles.SortDescriptions.Add(new SortDescription(nameof(MediaViewModel.CreationTime), ListSortDirection.Descending));
                    break;
            }
        }

        private bool MediaFilter(object mediaItem)
        {
            if (string.IsNullOrEmpty(_filter))
                return true;
            var media = mediaItem as MediaViewModel;
            if (media == null || string.IsNullOrWhiteSpace(media.Name))
                return false;
            return media.Name.IndexOf(_filter, StringComparison.CurrentCultureIgnoreCase) >= 0;
        }

        private void QueueToPlayerByIndex(object obj)
        {
            if (SelectedMedia is null)
                return;
            var player = Providers.GlobalApplicationData.Current.RundownPlayers.ElementAtOrDefault(int.Parse(obj as string));
            if (player is null)
                return;
            player.Submit(SelectedMedia.Media);
        }

        private void ChangeDate(object days)
        {
            if (!(days is string str && int.TryParse(str, out var increment)))
                throw new ArgumentException(nameof(days));
            SelectedDate = SelectedDate.AddDays(increment);
        }

        private void ExploreFolder(object obj)
        {
            Process.Start("explorer.exe", Path);
        }

        private void WatchedFolder_MediaChanged(object sender, MediaEventArgs e)
        {
            OnUiThread(() =>
            {
                switch (e.Kind)
                {
                    case MediaEventKind.Create:
                        _mediaFiles.Add(new MediaViewModel(e.Media));
                        MediaFiles.Refresh();
                        break;
                    case MediaEventKind.Delete:
                        var mediaVm = _mediaFiles.FirstOrDefault(m => m.Media == e.Media);
                        if (mediaVm != null)
                            _mediaFiles.Remove(mediaVm);
                        MediaFiles.Refresh();
                        break;
                }
                Debug.WriteLine("Media {0} {1}", e.Media.Name, e.Kind);
            });
        }

        public void Dispose()
        {
            if (_isDisposed)
                return;
            _isDisposed = true;
            _watchedFolder.MediaChanged -= WatchedFolder_MediaChanged;
        }
    }
}
