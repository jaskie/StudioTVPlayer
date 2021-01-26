using System;
using System.Windows.Media;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Args;
using StudioTVPlayer.ViewModel.Main.MediaBrowser;

namespace StudioTVPlayer.ViewModel.Main
{
    public class InfoViewModel : ViewModelBase
    {

        private MediaViewModel _browserItem;
        public MediaViewModel BrowserItem
        {
            get => _browserItem;
            set
            {
                if (_browserItem == value)
                    return;

                if (_browserItem != null)
                    _browserItem.Media.PropertyChanged -= Media_PropertyChanged;

                Set(ref _browserItem, value);

                if (value == null)
                    return;

                _browserItem.Media.PropertyChanged += Media_PropertyChanged;
            }
        }

        public string Name { get => _browserItem?.Name; }
        public TimeSpan Duration { get => _browserItem == null ? default(TimeSpan) : _browserItem.Duration; }
        public DateTime CreationDate { get => _browserItem == null ? default(DateTime) : _browserItem.CreationTime; }
        public ImageSource Thumbnail { get => _browserItem?.Thumbnail; }

        private void Media_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            switch (e.PropertyName)
            {
                case nameof(Media.Name):
                    NotifyPropertyChanged(nameof(Name));
                    break;              

                case nameof(Media.Duration):                   
                    NotifyPropertyChanged(nameof(Duration));
                    break;


                case nameof(Media.CreationTime):
                    NotifyPropertyChanged(nameof(CreationDate));
                    break;               
            }
        }

        private void SelectedMediaChanged(object sender, BrowserItemEventArgs e)
        {
            BrowserItem = e.BrowserItem;
            NotifyPropertyChanged(nameof(Name));
            NotifyPropertyChanged(nameof(Duration));
            NotifyPropertyChanged(nameof(CreationDate));
            NotifyPropertyChanged(nameof(Thumbnail));
        }
    }
}
