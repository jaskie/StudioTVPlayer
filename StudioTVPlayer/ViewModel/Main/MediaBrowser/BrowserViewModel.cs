using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
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

        public UiCommand MediaItem_MoveCommand { get;  }
        public UiCommand SortDirectionCommand { get; }
        public UiCommand ChangeDateCommand { get; }
        public UiCommand QueueToPlayerByIndexCommand { get; }



        public BrowserViewModel(WatchedFolder watchedFolder)
        {
            _watchedFolder = watchedFolder;
            _watchedFolder.MediaChanged += WatchedFolder_MediaChanged;
            _watchedFolder.Initialize();
            _mediaFiles = new ObservableCollection<MediaViewModel>(watchedFolder.Medias.Select(m => new MediaViewModel(m)));
            _mediaFilesView = System.Windows.Data.CollectionViewSource.GetDefaultView(_mediaFiles);
            Name = watchedFolder.Name;
            IsFilteredByDate = watchedFolder.IsFilteredByDate;

            SortDirectionCommand = new UiCommand(ChangeSortDirection);
            ChangeDateCommand = new UiCommand(ChangeDate);
            QueueToPlayerByIndexCommand = new UiCommand(QueueToPlayerByIndex);
        }



        #region Properties

        public IList<MediaViewModel> MediaFiles => _mediaFiles;
        public string Name { get; }
        public bool IsFilteredByDate { get; }

        private bool _isFocused;
        public bool IsFocused
        {
            get => _isFocused;
            set => Set(ref _isFocused, value);
        }

        private MediaViewModel _selectedMedia;
        public MediaViewModel SelectedMedia
        {
            get => _selectedMedia;
            set
            {
                if (!Set(ref _selectedMedia, value))
                    return;
            }
        }

        private DateTime _selectedDate = DateTime.Today;
        public DateTime SelectedDate
        {
            get => _selectedDate;
            set
            {
                if (!Set(ref _selectedDate, value))
                    return;
                LoadMediaCollectionByDate();
            }
        }

        private ListSortDirection _sortDirection = ListSortDirection.Ascending;
        public ListSortDirection SortDirection
        {
            get { return _sortDirection; }
            set
            {
                if (!Set(ref _sortDirection, value))
                    return;
                SortMediaCollection();
            }
        }

        private Sortings _selectedSorting;
        public Sortings SelectedSorting
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



       

        private void ChangeDate(object obj)
        {
            if (obj == null)
                return;

            SelectedDate = SelectedDate.AddDays(Int32.Parse(obj.ToString()));
        }

        private void LoadMediaCollectionByDate()
        {
            _mediaFilesView.Filter = o =>
            {
                var browserItem = o as MediaViewModel;
                return browserItem.Media.CreationTime.ToShortDateString() == _selectedDate.ToShortDateString();
            };
            //_mediaDataProvider.LoadMediaFiles(_mediaFilesView, _mediaWatcher);
        }

        private void SortMediaCollection()
        {
            switch (SelectedSorting)
            {
                case Sortings.Name:
                    {
                        _mediaFilesView.SortDescriptions.Clear();
                        _mediaFilesView.SortDescriptions.Add(new SortDescription(nameof(MediaViewModel.Media.Name), _sortDirection));
                        break;
                    }

                case Sortings.Duration:
                    {
                        _mediaFilesView.SortDescriptions.Clear();
                        _mediaFilesView.SortDescriptions.Add(new SortDescription(nameof(MediaViewModel.Media.Duration), _sortDirection));
                        break;
                    }


                case Sortings.CreationDate:
                    {
                        _mediaFilesView.SortDescriptions.Clear();
                        _mediaFilesView.SortDescriptions.Add(new SortDescription(nameof(MediaViewModel.Media.CreationTime), _sortDirection));
                        break;
                    }
            }
        }

        private void ChangeSortDirection(object obj)
        {
            if (_sortDirection == ListSortDirection.Ascending)
                SortDirection = ListSortDirection.Descending;
            else
                SortDirection = ListSortDirection.Ascending;
        }

        private void QueueToPlayerByIndex(object obj)
        {
            if (obj == null || SelectedMedia == null)
                return;
            var index = Int32.Parse(obj.ToString());

            //_exchangeService.AddToPlayerQueueByIndex(index, SelectedMedia);
        }


        private void WatchedFolder_MediaChanged(object sender, MediaEventArgs e)
        {
            OnUiThread(() =>
            {
                switch (e.Kind)
                {
                    case MediaEventKind.Create:
                        _mediaFiles.Add(new MediaViewModel(e.Media));
                        break;
                    case MediaEventKind.Delete:
                        var mediaVm = _mediaFiles.FirstOrDefault(m => m.Media == e.Media);
                        if (mediaVm != null)
                            _mediaFiles.Remove(mediaVm);
                        break;
                    case MediaEventKind.Change:
                        break;
                }
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
