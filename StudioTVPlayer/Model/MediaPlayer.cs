using StudioTVPlayer.Model.Args;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public class MediaPlayer : IDisposable
    {
        private readonly List<RundownItem> _rundown = new List<RundownItem>();
        private readonly TimeSpan PreloadTime = TimeSpan.FromSeconds(2);
        private RundownItem _playingRundownItem;
        private RundownItem _nextRundownItem;

        public MediaPlayer(Channel channel)
        {
            Channel = channel;
            Channel.AudioVolume += Channel_AudioVolume;
        }

        public IReadOnlyCollection<RundownItem> Rundown => _rundown;

        public Channel Channel { get; }

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
        public event EventHandler<TimeEventArgs> FramePlayed;
        public event EventHandler Stopped;
        public event EventHandler<RundownItemEventArgs> MediaSubmitted;
        public event EventHandler<AudioVolumeEventArgs> AudioVolume;
        public event EventHandler<RundownItemEventArgs> Removed;

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
                item.AutoStartChanged += RundownItem_AutoStartChanged;
                item.RemoveRequested += RundownItem_RemoveRequested;
                return item;
            }
            if (index == Rundown.Count)
            {
                var item = new RundownItem(media);
                _rundown.Add(item);
                item.AutoStartChanged += RundownItem_AutoStartChanged;
                item.RemoveRequested += RundownItem_RemoveRequested;
                return item;
            }
            return null;
        }

        public bool IsAplha => Channel.PixelFormat == TVPlayR.PixelFormat.bgra;

        public bool IsPlaying => _playingRundownItem?.IsPlaying == true;

        public ImageSource GetPreview(int width, int height)
        {
            return Channel.GetPreview(width, height);
        }

        public void MoveItem(int srcIndex, int destIndex)
        {
            var item = _rundown[srcIndex];
            _rundown.RemoveAt(srcIndex);
            _rundown.Insert(destIndex, item);
        }

        private void InternalUnload(RundownItem rundownItem)
        {
            Debug.WriteLine(rundownItem, "InternalUnload:");
            if (rundownItem == null)
                return;
            rundownItem.FramePlayed -= PlaiyngRundownItem_FramePlayed;
            rundownItem.Stopped -= PlaiyngRundownItem_Stopped;
            rundownItem.Pause();
            rundownItem.Unload();
        }

        private void InternalLoad(RundownItem rundownItem)
        {
            Debug.WriteLine(rundownItem, "InternalLoad:");
            if (rundownItem != null)
            {
                rundownItem.FramePlayed += PlaiyngRundownItem_FramePlayed;
                rundownItem.Stopped += PlaiyngRundownItem_Stopped;
                rundownItem.Preload(Channel.AudioChannelCount);
                Channel.Load(rundownItem);
                if (rundownItem.IsAutoStart)
                    rundownItem.Play();
            }
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

        public void DeleteDisabled()
        {
            RundownItem item = null;
            while (true)
            {
                item = Rundown.FirstOrDefault(i => !i.Enabled);
                if (item is null)
                    break;
                RemoveItem(item);
            }
        }

        private bool RemoveItem(RundownItem rundownItem)
        {
            if (!_rundown.Remove(rundownItem))
                return false;
            rundownItem.AutoStartChanged -= RundownItem_AutoStartChanged;
            rundownItem.RemoveRequested -= RundownItem_RemoveRequested;
            if (rundownItem != PlayingRundownItem)
                rundownItem.Dispose();
            Removed?.Invoke(this, new RundownItemEventArgs(rundownItem));
            return true;
        }

        private void PlaiyngRundownItem_Stopped(object sender, EventArgs e)
        {
            Task.Run(() => // do not block incoming thread
            {
                var nextItem = FindNextAutoPlayItem();
                if (nextItem is null)
                {
                    Stopped?.Invoke(this, EventArgs.Empty);
                    return;
                }
                PlayingRundownItem = nextItem;
                nextItem.Play();
            });
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
            else 
                return null;
        }

        private void PlaiyngRundownItem_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            FramePlayed?.Invoke(this, new TimeEventArgs(e.Time));
            var current = sender as RundownItem ?? throw new ArgumentException(nameof(sender));
            var nextItem = _nextRundownItem;
            if (nextItem is null)
                return;
            if (current.Media.Duration - e.Time < PreloadTime)
                return;
            if (nextItem.Preload(Channel.AudioChannelCount))
                Channel.Preload(nextItem);
        }

        private void RundownItem_AutoStartChanged(object sender, EventArgs e)
        {
            _nextRundownItem = FindNextAutoPlayItem();
        }

        private void Channel_AudioVolume(object sender, AudioVolumeEventArgs e)
        {
            AudioVolume?.Invoke(this, e);
        }

        private void RundownItem_RemoveRequested(object sender, EventArgs e)
        {
            var item = sender as RundownItem ?? throw new ArgumentException(nameof(sender));
            RemoveItem(item);
        }


        public void Dispose()
        {
            Channel.AudioVolume -= Channel_AudioVolume;
            PlayingRundownItem = null;
            foreach (var item in _rundown)
            {
                item.AutoStartChanged -= RundownItem_AutoStartChanged;
                item.RemoveRequested -= RundownItem_RemoveRequested;
                item.Dispose();
            }
            _rundown.Clear();
        }

    }


}
