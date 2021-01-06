using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class MediaPlayer : IDisposable
    {
        private readonly Channel _channel;
        private readonly List<MediaPlayerQueueItem> _mediaQueue = new List<MediaPlayerQueueItem>();
        private MediaPlayerQueueItem _playingQueueItem;
        private TVPlayR.InputFile _inputFile;

        public MediaPlayer(Channel channel)
        {
            _channel = channel;
        }

        public IReadOnlyCollection<MediaPlayerQueueItem> MediaQueue => _mediaQueue;
        
        public MediaPlayerQueueItem PlayingQueueItem
        {
            get => _playingQueueItem; 
            private set
            {
                if (_playingQueueItem == value)
                    return;
                _playingQueueItem = value;
                InternalLoad(value.Media.FullPath);
                Loaded?.Invoke(this, new MediaPlayerQueueItemEventArgs(value));
            }
        }

        public void Play()
        {
            _inputFile?.Play();
        }

        public event EventHandler<MediaPlayerQueueItemEventArgs> Loaded;
        public event EventHandler<MediaPlayerProgressEventArgs> Progress;

        public bool Load(MediaPlayerQueueItem media)
        {
            if (!_mediaQueue.Contains(media))
                return false;
            PlayingQueueItem = media;
            return true;
        }

        public bool AddToQueue(MediaPlayerQueueItem media, int index)
        {
            if (index < _mediaQueue.Count)
            {
                _mediaQueue.Insert(index, media);
                return true;
            }
            if (index == MediaQueue.Count)
            {
                _mediaQueue.Add(media);
                return true;
            }
            return false;
        }

        private void InternalLoad(string fullPath)
        {
            if (_inputFile != null)
            {
                _inputFile.FramePlayed -= InputFile_FramePlayed;
                _inputFile.Stopped -= InputFile_Stopped;
                _inputFile.Dispose();
            }
            _inputFile = new TVPlayR.InputFile(fullPath);
            _inputFile.FramePlayed += InputFile_FramePlayed;
            _inputFile.Stopped += InputFile_Stopped;
            _channel.Load(_inputFile);
        }

        private void InputFile_Stopped(object sender, EventArgs e)
        {
            
        }

        private void InputFile_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            Progress?.Invoke(this, new MediaPlayerProgressEventArgs(e.Time));
        }

        public void Dispose()
        {
            if (_inputFile == null)
                return;
            _inputFile.FramePlayed -= InputFile_FramePlayed;
            _inputFile.Stopped -= InputFile_Stopped;
            _inputFile.Dispose();
            _inputFile = null;
        }
    }

    
}
