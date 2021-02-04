﻿using System;
using System.Collections.Generic;
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

        public IList<Model.MediaPlayer> Players => Providers.GlobalApplicationData.Current.Players;

        public string Name => Media.Name;
        public TimeSpan Duration => Media.Duration;
        public DateTime CreationTime => Media.CreationTime;

        public ImageSource Thumbnail => Media.Thumbnail;

        public bool IsVerified => Media.IsVerified;

        public UiCommand QueueToPlayerCommand { get; }

        public MediaViewModel(Media media)
        {
            Media = media;
            media.PropertyChanged += Media_PropertyChanged;
            QueueToPlayerCommand = new UiCommand(QueueToPlayer);
        }

        private void QueueToPlayer(object obj)
        {
            var player = obj as Model.MediaPlayer ?? throw new ArgumentException(nameof(obj));
            player.Submit(Media);
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
