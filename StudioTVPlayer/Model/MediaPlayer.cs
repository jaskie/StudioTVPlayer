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
        private RundownItem _playingRundownItem;
        private RundownItem _nextRundownItem;

        public MediaPlayer(Channel channel)
        {
            Channel = channel;
        }

        public IReadOnlyCollection<RundownItem> Rundown => _rundown;

        public Channel Channel { get; }

        public RundownItem NextRundownItem
        {
            get => _nextRundownItem; 
            set
            {
                var oldItem = _nextRundownItem;
                if (_nextRundownItem == value)
                    return;
                _nextRundownItem = value;
                if (oldItem != null && oldItem != _playingRundownItem)
                    oldItem.Unload();
                value.Preload(Channel.AudioChannelCount);                    
            }
        }



        public RundownItem PlayingRundownItem
        {
            get => _playingRundownItem;
            private set
            {
                var oldItem = _playingRundownItem;
                if (_playingRundownItem == value)
                    return;
                _playingRundownItem = value;
                InternalUnload(oldItem);
                InternalLoad(value);
            }
        }

        public bool IsEof => PlayingRundownItem?.InputFile?.IsEof ?? true;

        public TimeSpan OneFrame => Channel.VideoFormat.FrameNumberToTime(1);

        public bool Play()
        {
            if (PlayingRundownItem is null)
                return false;
            PlayingRundownItem.Play();
            return true;
        }

        public void Pause()
        {
            PlayingRundownItem?.Pause();
        }


        public event EventHandler<RundownItemEventArgs> Loaded;
        public event EventHandler<TimeEventArgs> Progress;
        public event EventHandler Stopped;
        public event EventHandler<RundownItemEventArgs> MediaSubmitted;

        public void Load(RundownItem item)
        {
            if (!_rundown.Contains(item))
                return;
            PlayingRundownItem = item;
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

        public bool RemoveItem(RundownItem rundownItem)
        {
            if (!_rundown.Remove(rundownItem))
                return false;
            rundownItem.Dispose();
            return true;
        }

        public void MoveItem(int srcIndex, int destIndex)
        {
            var item = _rundown[srcIndex];
            _rundown.RemoveAt(srcIndex);
            _rundown.Insert(destIndex, item);
        }

        private void InternalUnload(RundownItem rundownItem)
        {
            if (rundownItem == null)
                return;
            rundownItem.Pause();
            rundownItem.FramePlayed -= PlaiyngRundownItem_FramePlayed;
            rundownItem.Stopped -= PlaiyngRundownItem_Stopped;
            rundownItem.Unload();
            Stopped?.Invoke(this, EventArgs.Empty);
        }

        private void InternalLoad(RundownItem rundownItem)
        {
            if (rundownItem == null)
                return;
            rundownItem.FramePlayed += PlaiyngRundownItem_FramePlayed;
            rundownItem.Stopped += PlaiyngRundownItem_Stopped;
            rundownItem.Preload(Channel.AudioChannelCount);
            Channel.Load(rundownItem);
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
            if (PlayingRundownItem == null)
                return false;
            return PlayingRundownItem.Seek(timeSpan);
        }

        public void SetVolume(double value)
        {
            Channel.SetVolume(value);
        }

        public void Clear()
        {
            PlayingRundownItem = null;
            Channel.Clear();
        }

        private void PlaiyngRundownItem_Stopped(object sender, EventArgs e)
        {
            var nextItem = FindNextAutoPlayItem();
            if (nextItem is null)
            {
                Stopped?.Invoke(this, EventArgs.Empty);
                return;
            }
            PlayingRundownItem = nextItem;
            nextItem.Play();
        }

        private RundownItem FindNextAutoPlayItem()
        {
            var currentItem = PlayingRundownItem;
            RundownItem next = null;
            using (var iterator = _rundown.GetEnumerator())
            {
                bool found = false;
                while (iterator.MoveNext())
                {
                    if (found)
                    {
                        if (iterator.Current != null && iterator.Current.IsAutoStart && iterator.Current.Enabled)
                            return iterator.Current;
                        else
                            return null;
                    }
                    if (iterator.Current == currentItem)
                        found = true;
                }
            }
            if (next != null && next.IsAutoStart && next.Enabled)
                return next;
            else NextRundownItem = null;
            return null;
        }

        private void PlaiyngRundownItem_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            Progress?.Invoke(this, new TimeEventArgs(e.Time));
        }

        public void Dispose()
        {
            PlayingRundownItem = null;
            foreach (var item in _rundown)
                item.Dispose();
            _rundown.Clear();
        }

    }


}
