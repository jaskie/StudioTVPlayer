using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;

namespace StudioTVPlayer.ViewModel.Main.Browser
{
    public class BrowserTabItemViewModel : ViewModelBase
    {

        private Media _media;
        public Media Media
        {
            get => _media;
            set
            {
                if (_media == value)
                    return;

                if (_media != null)
                    Media.PropertyChanged -= Media_PropertyChanged;

                _media = value;
                Media.PropertyChanged += Media_PropertyChanged;
            }
        }

        private IExchangeService _exchangeService;

        public string Name { get => Media.Name; }
        public TimeSpan Duration { get => Media.Duration; set { Media.Duration = value; } }
        public DateTime CreationDate { get => Media.CreationDate; }

        private bool _isVerified;
        public bool IsVerified { get => _isVerified; set => Set(ref _isVerified, value); }

        private bool _isQueued;
        public bool IsQueued
        {
            get => _isQueued;
            set
            {
                Set(ref _isQueued, value);
            }
        }

        private ImageSource _thumbnail;
        public ImageSource Thumbnail { get => _thumbnail; set => Set(ref _thumbnail, value); }

        public UiCommand MediaItem_MoveCommand { get; private set; }        
        public UiCommand QueueToPlayerByChannelIDCommand { get; private set; }

        public BrowserTabItemViewModel(IExchangeService exchangeService)
        {
            _exchangeService = exchangeService;
            _thumbnail = null;            
            LoadCommands();
        }

        private void LoadCommands()
        {
            MediaItem_MoveCommand = new UiCommand(param => MediaItem_Move(param));          
            QueueToPlayerByChannelIDCommand = new UiCommand(QueueToPlayerByChannelID);            
        }        

        public void MediaItem_Move(object param)
        {
            if (param != null)
            {
                object[] parameters = param as object[];
                FrameworkElement sender = (FrameworkElement)parameters[0];
                MouseEventArgs e = (MouseEventArgs)parameters[1];

                if (e.LeftButton == MouseButtonState.Pressed)
                    DragDrop.DoDragDrop(sender, this, DragDropEffects.Copy);
            }
        }

        private void QueueToPlayerByChannelID(object obj)
        {
            if (obj == null)
                return;
            var id= Int32.Parse(obj.ToString());

            _exchangeService.AddToPlayerQueueByChannelID(id, this);
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
    }
}
