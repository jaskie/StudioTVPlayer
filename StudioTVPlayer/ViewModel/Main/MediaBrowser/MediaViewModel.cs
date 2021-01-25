using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;

namespace StudioTVPlayer.ViewModel.Main.MediaBrowser
{
    public class MediaViewModel : ViewModelBase, IDisposable
    {

        public Media Media { get; }

        public string Name => Media.Name;
        public TimeSpan Duration => Media.Duration;
        public DateTime CreationDate => Media.CreationTime;

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

        public ImageSource Thumbnail => Media.Thumbnail;

        public UiCommand MediaItem_MoveCommand { get; private set; }        
        public UiCommand QueueToPlayerByChannelIDCommand { get; private set; }

        public MediaViewModel(Media media)
        {
            Media = media;
            media.PropertyChanged += Media_PropertyChanged;
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

            //_exchangeService.AddToPlayerQueueByChannelID(id, this);
        }

        private void Media_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            NotifyPropertyChanged(e.PropertyName);
        }

        public void Dispose()
        {
            Media.PropertyChanged -= Media_PropertyChanged;
        }
    }
}
