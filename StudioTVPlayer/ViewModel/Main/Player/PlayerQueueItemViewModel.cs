using System;
using System.Diagnostics;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.ViewModel.Main.MediaBrowser;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class PlayerQueueItemViewModel : ViewModelBase
    {
        public MediaViewModel BrowserItem { get; }        

        private bool _dragOver = false;
        public bool DragOver
        {
            get => _dragOver;
            set
            {
                Set(ref _dragOver, value);
                if (value)
                    Debug.WriteLine("Drag entered");
            }
        }

        private bool _isLoaded;
        public bool IsLoaded
        {
            get
            {
                if (BrowserItem == null)
                    return false;
                return _isLoaded;
            }
            set
            {
                Set(ref _isLoaded, value);
            }
        }

        private bool _isDisabled;
        public bool IsDisabled
        {
            get { return _isDisabled; }
            set
            {
                Set(ref _isDisabled, value);
            }
        }

        public string Name
        {
            get
            {
                if (BrowserItem == null)
                    return String.Empty;
                return BrowserItem.Name;
            }
        }
        public TimeSpan Duration
        {
            get
            {
                if (BrowserItem == null)
                    return TimeSpan.Zero;
                return BrowserItem.Duration;
            }
        }
        public DateTime CreationDate
        {
            get
            {
                if (BrowserItem == null)
                    return new DateTime(0);
                return BrowserItem.CreationDate;
            }
        }

        public bool? IsVerified => BrowserItem?.IsVerified;


        public PlayerQueueItemViewModel(MediaViewModel browserMedia)
        {
            BrowserItem = browserMedia;
            BrowserItem.PropertyChanged += BrowserItem_PropertyChanged;
        }       

        private void BrowserItem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            switch (e.PropertyName)
            {
                case nameof(BrowserItem.Name):
                    NotifyPropertyChanged(nameof(Name));
                    break;


                case nameof(BrowserItem.Duration):
                    NotifyPropertyChanged(nameof(Duration));
                    break;


                case nameof(BrowserItem.CreationDate):
                    NotifyPropertyChanged(nameof(CreationDate));
                    break;

                case nameof(BrowserItem.IsVerified):
                    NotifyPropertyChanged(nameof(IsVerified));
                    break;

                case nameof(BrowserItem.IsQueued):
                    NotifyPropertyChanged(nameof(IsLoaded));
                    break;
            }
        }       
    }
}
