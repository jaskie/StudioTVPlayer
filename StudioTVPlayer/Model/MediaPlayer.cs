using StudioTVPlayer.Model.Args;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class MediaPlayer : IDisposable
    {
        private readonly List<RundownItem> _rundown = new List<RundownItem>();
        private RundownItem _playingQueueItem;
        private TVPlayR.InputFile _inputFile;

        public MediaPlayer(Channel channel)
        {
            Channel = channel;
        }

        public IReadOnlyCollection<RundownItem> Rundown => _rundown;

        public Channel Channel { get; }

        public RundownItem PlayingQueueItem
        {
            get => _playingQueueItem; 
            private set
            {
                if (_playingQueueItem == value)
                    return;
                _playingQueueItem = value;
                InternalLoad(value.Media.FullPath);
                Loaded?.Invoke(this, new RundownItemEventArgs(value));
            }
        }


        public void Play()
        {
            _inputFile?.Play();
        }

        public event EventHandler<RundownItemEventArgs> Loaded;
        public event EventHandler<TimeEventArgs> Progress;

        public bool Load(RundownItem media)
        {
            if (!_rundown.Contains(media))
                return false;
            PlayingQueueItem = media;
            return true;
        }

        public RundownItem AddToQueue(Media media, int index)
        {
            if (index < _rundown.Count)
            {
                var item = new RundownItem(media);
                _rundown.Insert(index, item);
                return item;
            }
            if (index == Rundown.Count)
            {
                var item = new RundownItem(media);
                _rundown.Add(item);
                return item;
            }
            return null;
        }

        private void InternalLoad(string fullPath)
        {
            if (_inputFile != null)
            {
                _inputFile.FramePlayed -= InputFile_FramePlayed;
                _inputFile.Stopped -= InputFile_Stopped;
                _inputFile.Dispose();
            }
            _inputFile = new TVPlayR.InputFile(fullPath, 2);
            _inputFile.FramePlayed += InputFile_FramePlayed;
            _inputFile.Stopped += InputFile_Stopped;
            Channel.Load(_inputFile);
        }

        private void InputFile_Stopped(object sender, EventArgs e)
        {
            
        }

        private void InputFile_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            Progress?.Invoke(this, new TimeEventArgs(e.Time));
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
