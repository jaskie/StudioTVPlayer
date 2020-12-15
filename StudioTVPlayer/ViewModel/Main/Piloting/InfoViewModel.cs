using System;
using System.Diagnostics;
using System.Windows.Media;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Args;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.ViewModel.Main.Piloting.Browser;

namespace StudioTVPlayer.ViewModel.Main.Piloting
{
    public class InfoViewModel : ViewModelBase
    {
        private IExchangeService _exchangeService;

        private BrowserTabItemViewModel _browserItem;
        public BrowserTabItemViewModel BrowserItem
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
        public TimeSpan Duration { get => _browserItem == null ? default(TimeSpan) : _browserItem.Duration; set { _browserItem.Duration = value; } }
        public DateTime CreationDate { get => _browserItem == null ? default(DateTime) : _browserItem.CreationDate; }
        public ImageSource Thumbnail { get => _browserItem?.Thumbnail; }

        public InfoViewModel(IExchangeService vmNotifyService)
        {                       
            _exchangeService = vmNotifyService;
            _exchangeService.NotifyOnSelectedMediaChanged += SelectedMediaChanged;                        
        }

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


                case nameof(Media.CreationDate):
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
