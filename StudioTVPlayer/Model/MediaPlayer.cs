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
                InternalLoad(value);
            }
        }

        public bool IsEof => _inputFile?.IsEof ?? true;

        public bool Play()
        {
            if (_inputFile == null)
                return false;
            _inputFile?.Play();
            return true;
        }

        public void Pause()
        {
            _inputFile?.Pause();
        }


        public event EventHandler<RundownItemEventArgs> Loaded;
        public event EventHandler<TimeEventArgs> Progress;
        public event EventHandler Stopped;
        public event EventHandler<RundownItemEventArgs> MediaSubmitted;

        public bool Load(RundownItem item)
        {
            if (!_rundown.Contains(item))
                return false;
            PlayingQueueItem = item;
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

        private void InternalLoad(RundownItem rundownItem)
        {
            if (_inputFile != null)
            {
                _inputFile.FramePlayed -= InputFile_FramePlayed;
                _inputFile.Stopped -= InputFile_Stopped;
                _inputFile.Dispose();
                Stopped?.Invoke(this, EventArgs.Empty);
            }
            if (rundownItem == null)
                return;
            _inputFile = new TVPlayR.InputFile(rundownItem.Media.FullPath, 2);
            _inputFile.FramePlayed += InputFile_FramePlayed;
            _inputFile.Stopped += InputFile_Stopped;
            Channel.Load(_inputFile);
            Loaded?.Invoke(this, new RundownItemEventArgs(rundownItem));
        }

        internal void Submit(Media media)
        {
            var item = new RundownItem(media);
            _rundown.Add(item);
            MediaSubmitted?.Invoke(this, new RundownItemEventArgs(item));
        }

        public bool Seek(TimeSpan timeSpan)
        {
            if (_inputFile == null)
                return false;
            return _inputFile.Seek(timeSpan);
        }

        public void Clear()
        {
            PlayingQueueItem = null;
            Channel.Clear();
        }


        private void InputFile_Stopped(object sender, EventArgs e)
        {
            Stopped?.Invoke(this, EventArgs.Empty);
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
            _rundown.Clear();
            _inputFile.Dispose();
            _inputFile = null;
        }

    }

    
}
