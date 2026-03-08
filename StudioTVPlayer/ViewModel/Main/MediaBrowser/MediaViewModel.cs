using System;
using System.Collections.Generic;
using System.Windows.Media;
using StudioTVPlayer.Helpers;

namespace StudioTVPlayer.ViewModel.Main.MediaBrowser
{
    public class MediaViewModel : ViewModelBase, IDisposable
    {
        private Action _verifyOnceAction;
        public MediaViewModel(Model.MediaFile media)
        {
            Media = media;
            if (!media.IsVerified)
            {
                _verifyOnceAction = () =>
                {
                    _verifyOnceAction = null;
                    Model.MediaVerifier.Current.Queue(media);
                };
            }
            media.PropertyChanged += Media_PropertyChanged;
            QueueToPlayerCommand = new UiCommand(QueueToPlayer);
        }

        public Model.MediaFile Media { get; }

        public string Name => Media.Name;
        public TimeSpan Duration => Media.Duration;
        public TimeSpan StartTime => Media.StartTime;
        public DateTime CreationTime => Media.CreationTime;
        public int Width => Media.Width;
        public int Height => Media.Height;
        public Model.ScanType ScanType => Media.ScanType;
        public string FrameRate => Media.FrameRate;
        public int AudioChannelCount => Media.AudioChannelCount;
        public bool HaveAlphaChannel => Media.HaveAlphaChannel;

        public ImageSource Thumbnail
        {
            get
            {
                _verifyOnceAction?.Invoke();
                return Media.Thumbnail;
            }
        }

        public bool IsVerified => Media.IsVerified;

        public bool IsValid => Media.IsValid;

        public IEnumerable<Model.Player> Players => Providers.GlobalApplicationData.Current.RundownPlayers;

        public UiCommand QueueToPlayerCommand { get; }


        private void QueueToPlayer(object obj)
        {
            var player = obj as Model.RundownPlayer ?? throw new ArgumentException(nameof(obj));
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
