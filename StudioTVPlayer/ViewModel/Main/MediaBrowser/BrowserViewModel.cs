using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows;
using StudioTVPlayer.Extensions;
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

        public UiCommand MediaItem_MoveCommand { get; private set; }
        public UiCommand SortDirectionCommand { get; private set; }
        public UiCommand ChangeDateCommand { get; private set; }
        public UiCommand QueueToPlayerByIndexCommand { get; private set; }



        public BrowserViewModel(WatchedFolder watchedFolder)
        {
            _watchedFolder = watchedFolder;
            _watchedFolder.MediaChanged += WatchedFolder_MediaChanged;
            _watchedFolder.Initialize();
            _mediaFilesView = System.Windows.Data.CollectionViewSource.GetDefaultView(MediaFiles);
            LoadCommands();
            _mediaFiles = new ObservableCollection<MediaViewModel>(watchedFolder.Medias.Select(m => new MediaViewModel(m)));
            Name = watchedFolder.Name;
        }



        #region Properties

        public IList<MediaViewModel> MediaFiles => _mediaFiles;
        public string Name { get; }


        private bool _isFocused;
        public bool IsFocused
        {
            get => _isFocused;
            set
            {
                Set(ref _isFocused, value);
            }
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



        private void LoadCommands()
        {
            SortDirectionCommand = new UiCommand(ChangeSortDirection);
            ChangeDateCommand = new UiCommand(ChangeDate);
            QueueToPlayerByIndexCommand = new UiCommand(QueueToPlayerByIndex);
        }

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


        private void MediaChanged(object sender, MediaEventArgs e)
        {
            MediaViewModel browserItem = MediaFiles.FirstOrDefault(ItemParam => ItemParam.Media.DirectoryName == e.Media.DirectoryName);

            if (browserItem == null)
                return;

            if (!browserItem.IsVerified)
                return;

            Debug.WriteLine("Media Track Point");

            //if (e.Media.Duration == default(TimeSpan))
            //    _mediaWatcher.AddMediaToTrack(e.Media);
            //else
            //    Application.Current.Dispatcher.BeginInvoke((Action)(() =>
            //    {
            //        browserItem.Duration = e.Media.Duration;
            //    }));
        }

        private void MediaVerified(object sender, MediaEventArgs e)
        {
            //var browserItem = MediaFiles.FirstOrDefault(ItemParam => ItemParam.Media.DirectoryName == e.Media.DirectoryName);

            //if (browserItem == null)
            //    return;

            //var IsVisible = _mediaFilesView.Contains(browserItem);

            //Application.Current.Dispatcher.BeginInvoke((Action)(() =>
            //{
            //    if (e.Media.Duration == null)
            //        browserItem.GetFFMeta(FFMeta.Thumbnail);
            //    else
            //    {
            //        browserItem.Media.Duration = e.Media.Duration;
            //        if (IsVisible || browserItem.IsQueued)
            //            browserItem.GetFFMeta(FFMeta.Thumbnail);
            //    }
            //    browserItem.IsVerified = true;
            //}));

        }

        private void MediaAdded(object sender, MediaEventArgs e)
        {
            //var browserItem = MediaFiles.FirstOrDefault(param => param.Media.Path == e.Media.Path) ?? _mediaDataProvider.GetNewBrowserTabItem(e.Media);

            //Application.Current.Dispatcher.BeginInvoke((Action)(() =>
            //{
            //    MediaFiles.Add(browserItem);
            //    var IsVisible = _mediaFilesView.Contains(browserItem);

            //    if (IsVisible || browserItem.IsQueued)
            //    {
            //        browserItem.GetResourceThumbnail(ThumbnailType.Loading);
            //        _mediaWatcher.AddMediaToVerify(browserItem.Media);
            //    }
            //}));
        }

        public void Dispose()
        {
            throw new NotImplementedException();
        }
    }
}
