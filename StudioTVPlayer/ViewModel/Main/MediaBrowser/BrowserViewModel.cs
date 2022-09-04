using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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
        private readonly ICollectionView _mediaFilesView;
        private readonly WatchedFolder _watchedFolder;
        private readonly ObservableCollection<MediaViewModel> _mediaFiles;
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
            _mediaFiles = new ObservableCollection<MediaViewModel>(watchedFolder.Medias.Select(m => new MediaViewModel(m)));
            _mediaFilesView = System.Windows.Data.CollectionViewSource.GetDefaultView(_mediaFiles);
            _mediaFilesView.SortDescriptions.Add(new SortDescription(nameof(MediaViewModel.CreationTime), ListSortDirection.Descending));
            _selectedSorting = Sorting.CreationTime;
            Name = watchedFolder.Name;
            Path = watchedFolder.Path;
            IsFilteredByDate = watchedFolder.IsFilteredByDate;
            _selectedDate = watchedFolder.FilterDate;
            QueueToPlayerByIndexCommand = new UiCommand(QueueToPlayerByIndex, index => SelectedMedia != null && int.Parse(index as string) < Providers.GlobalApplicationData.Current.MediaPlayers.Count);
            ChangeDateCommand = new UiCommand(ChangeDate, _ => IsFilteredByDate);
            ExploreFolderCommand = new UiCommand(ExploreFolder);
        }


        #region Properties

        public IList<MediaViewModel> MediaFiles => _mediaFiles;
        public string Name { get; }
        public string Path { get; }
        public bool IsFilteredByDate { get; }

        private bool _isFocused;
        public bool IsFocused
        {
            get => _isFocused;
            set => Set(ref _isFocused, value);
        }

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
                _mediaFilesView.Refresh();
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
            _mediaFilesView.SortDescriptions.Clear();
            switch (SelectedSorting)
            {
                case Sorting.Name:
                    _mediaFilesView.SortDescriptions.Add(new SortDescription(nameof(MediaViewModel.Name), ListSortDirection.Ascending));
                    break;
                case Sorting.Duration:
                    _mediaFilesView.SortDescriptions.Add(new SortDescription(nameof(MediaViewModel.Duration), ListSortDirection.Ascending));
                    break;
                case Sorting.CreationTime:
                    _mediaFilesView.SortDescriptions.Add(new SortDescription(nameof(MediaViewModel.CreationTime), ListSortDirection.Descending));
                    break;
            }
        }

     
        private void QueueToPlayerByIndex(object obj)
        {
            var index = int.Parse(obj as string);
            Providers.GlobalApplicationData.Current.MediaPlayers[index].Submit(SelectedMedia.Media);
        }

        private void ChangeDate(object days)
        {
            if (!(days is string str && int.TryParse(str, out var increment)))
                throw new ArgumentException("Invalid parameter", nameof(days));
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
                        _mediaFilesView.Refresh();
                        break;
                    case MediaEventKind.Delete:
                        var mediaVm = _mediaFiles.FirstOrDefault(m => m.Media == e.Media);
                        if (mediaVm != null)
                            _mediaFiles.Remove(mediaVm);
                        _mediaFilesView.Refresh();
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
