using System;
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
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.Services;

namespace StudioTVPlayer.ViewModel.Main.Browser
{
    public class BrowserViewModel : ViewModelBase
    {
        private readonly ICollectionView _mediaFilesView;
        private readonly MediaWatcherService _mediaWatcher;

        public UiCommand MediaItem_MoveCommand { get; private set; }
        public UiCommand SortDirectionCommand { get; private set; }        
        public UiCommand ChangeDateCommand { get; private set; }
        public UiCommand QueueToPlayerByIndexCommand { get; private set; }


        public ObservableCollection<BrowserMediaViewModel> MediaFiles { get; } = new ObservableCollection<BrowserMediaViewModel>();

        #region Properties

        private WatchedFolder _watcherMeta;
        public WatchedFolder WatcherMeta
        {
            get => _watcherMeta;
            set
            {
                if (!Set(ref _watcherMeta, value))
                    return;

                InitTab();               
            }
        }

        private bool _isFocused;
        public bool IsFocused
        {
            get => _isFocused;
            set
            {
                Set(ref _isFocused, value);
            }
        }

        private BrowserMediaViewModel _selectedMedia;
        public BrowserMediaViewModel SelectedMedia
        {
            get => _selectedMedia;
            set
            {                
                if (!Set(ref _selectedMedia, value))
                    return;
                //_exchangeService.RaiseSelectedMediaChanged(new BrowserItemEventArgs(value));
            }
        }

        private DateTime _selectedDate = DateTime.Now;
        public DateTime SelectedDate
        {
            get => _selectedDate;
            set
            {                
                if (!Set(ref _selectedDate, value))
                    return;
                if (!WatcherMeta.IsFilteredByDate)
                    return;
                //Jeżeli jakiś item był zanaczony na liście i zmienił się widok to kontrolka wysyła Keyboard.Focus(null) co wywala focus do main window             
                IsFocused = false; //by przygotować odpalenie ValueChanged (ewentualnie można (a nawe trzeba) podpiąć pod event i wtedy bez tej linijki oraz wcześniejszego zerowania tej zmiennej przy komendach focusa)
                //_mediaDataProvider.UnloadMediaFiles(_mediaFilesView);
                LoadMediaCollectionByDate();              
                IsFocused = true; //by wymusić ponowny focus elementu
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

        public BrowserViewModel(MediaWatcherService mediaWatcher)
        {            
            SelectedMedia = null;            

            _mediaFilesView = System.Windows.Data.CollectionViewSource.GetDefaultView(MediaFiles);
            LoadCommands();
            _mediaWatcher = mediaWatcher;
        }

        private void InitTab() 
        {
            _mediaWatcher.NotifyOnMediaAdd += MediaAdded;
            _mediaWatcher.NotifyOnMediaDelete += MediaDeleted;
            _mediaWatcher.NotifyOnMediaChanged += MediaChanged;
            _mediaWatcher.NotifyOnMediaRenamed += MediaRenamed;
            _mediaWatcher.NotifyOnMediaVerified += MediaVerified;

            MediaFiles.Clear();
            //foreach (var media in _mediaDataProvider.GetBrowserTabItems(_mediaWatcher.GetPath()))
            //    MediaFiles.Add(media);
            

            //if (WatcherMeta.IsFilteredByDate)
            //    LoadMediaCollectionByDate();
            //else
            //    _mediaDataProvider.LoadMediaFiles(_mediaFilesView, _mediaWatcher);
        }

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
                var browserItem = o as BrowserMediaViewModel;
                return browserItem.Media.CreationDate.ToShortDateString() == _selectedDate.ToShortDateString();
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
                        _mediaFilesView.SortDescriptions.Add(new SortDescription(nameof(BrowserMediaViewModel.Media.Name), _sortDirection));
                        break;
                    }

                case Sortings.Duration:
                    {
                        _mediaFilesView.SortDescriptions.Clear();
                        _mediaFilesView.SortDescriptions.Add(new SortDescription(nameof(BrowserMediaViewModel.Media.Duration), _sortDirection));
                        break;
                    }


                case Sortings.CreationDate:
                    {
                        _mediaFilesView.SortDescriptions.Clear();
                        _mediaFilesView.SortDescriptions.Add(new SortDescription(nameof(BrowserMediaViewModel.Media.CreationDate), _sortDirection));
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



        private void MediaChanged(object sender, MediaEventArgs e)
        {
            BrowserMediaViewModel browserItem = MediaFiles.FirstOrDefault(ItemParam => ItemParam.Media.Path == e.Media.Path);           

            if (browserItem == null)
                return;

            if (!browserItem.IsVerified)
                return;

            Debug.WriteLine("Media Track Point");

            if (e.Media.Duration == default(TimeSpan))
                _mediaWatcher.AddMediaToTrack(e.Media);
            else
                Application.Current.Dispatcher.BeginInvoke((Action)(() =>
                {
                    browserItem.Duration = e.Media.Duration;
                }));
        }

        private void MediaDeleted(object sender, MediaEventArgs e)
        {
            var item = MediaFiles.FirstOrDefault(ItemParam => ItemParam.Media.Path == e.Media.Path);
            Application.Current.Dispatcher.BeginInvoke((Action)(() =>
            {               
                if (item.IsQueued)
                {
                    item.IsVerified = false;
                    item.GetResourceThumbnail(ThumbnailType.Loading);
                    item.Duration = default(TimeSpan);
                }
                else
                {
                    MediaFiles.Remove(item);
                }
            }));
        }

        private void MediaRenamed(object sender, MediaEventArgs e)
        {
            BrowserMediaViewModel browserItem = MediaFiles.FirstOrDefault(ItemParam => ItemParam.Media.Path == e.OldPath);            

            if (browserItem == null)
                return;

            Application.Current.Dispatcher.BeginInvoke((Action)(() =>
            {
                browserItem.Media.Name = Path.GetFileName(e.Media.Path);
                browserItem.Media.Path = e.Media.Path;
            }));
        }

        private void MediaVerified(object sender, MediaEventArgs e)
        {
            var browserItem = MediaFiles.FirstOrDefault(ItemParam => ItemParam.Media.Path == e.Media.Path);
            
            if (browserItem == null)
                return;

            var IsVisible = _mediaFilesView.Contains(browserItem);

            Application.Current.Dispatcher.BeginInvoke((Action)(() =>
            {
                if (e.Media.Duration == null)
                    browserItem.GetFFMeta(FFMeta.Thumbnail);
                else
                {
                    browserItem.Media.Duration = e.Media.Duration;
                    if (IsVisible || browserItem.IsQueued)
                        browserItem.GetFFMeta(FFMeta.Thumbnail);
                }
                browserItem.IsVerified = true;
            }));

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
    }
}
