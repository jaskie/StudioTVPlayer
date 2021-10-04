using StudioTVPlayer.Model.Args;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public class MediaPlayer : IDisposable
    {
        private readonly List<RundownItemBase> _rundown = new List<RundownItemBase>();
        private readonly TimeSpan PreloadTime = TimeSpan.FromSeconds(2);
        private RundownItemBase _playingRundownItem;
        private RundownItemBase _nextRundownItem;

        public MediaPlayer(Channel channel)
        {
            Channel = channel;
            Channel.AudioVolume += Channel_AudioVolume;
        }

        public IReadOnlyCollection<RundownItemBase> Rundown => _rundown;

        public Channel Channel { get; }

        public RundownItemBase PlayingRundownItem
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

        public bool IsLoop { get; set; }

        public bool DisableAfterUnload { get; set; }

        public bool IsEof => (PlayingRundownItem?.Input as TVPlayR.FileInput)?.IsEof ?? true;

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

        public void Load(RundownItemBase item)
        {
            if (!_rundown.Contains(item))
                return;
            PlayingRundownItem = item;
        }

        public FileRundownItem AddMediaToQueue(MediaFile media, int index)
        {
            var item = new FileRundownItem(media);
            AddToQueue(item, index);
            return item;
        }

        public LiveInputRundownItem AddLiveToQueue(InputBase input, int index)
        {
            var item = new LiveInputRundownItem(input);
            AddToQueue(item, index);
            return item;
        }

        private void AddToQueue(RundownItemBase rundownItem, int index)
        {
            if (index < _rundown.Count)
            {
                _rundown.Insert(index, rundownItem);
                rundownItem.PropertyChanged += RundownItem_PropertyChanged;
                rundownItem.RemoveRequested += RundownItem_RemoveRequested;
            }
            else if (index == Rundown.Count)
            {
                _rundown.Add(rundownItem);
                rundownItem.PropertyChanged += RundownItem_PropertyChanged;
                rundownItem.RemoveRequested += RundownItem_RemoveRequested;
            }
            else
                throw new ArgumentException(nameof(index));
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

        private void InternalUnload(RundownItemBase rundownItem)
        {
            if (rundownItem == null)
                return;
            Debug.WriteLine(rundownItem.Name, "InternalUnload");
            rundownItem.FramePlayed -= PlaiyngRundownItem_FramePlayed;
            if (rundownItem is FileRundownItem fileRundownItem)
            {
                fileRundownItem.Stopped -= PlaiyngRundownItem_Stopped;
                if (DisableAfterUnload)
                    fileRundownItem.IsDisabled = true;
            }
            rundownItem.Pause();
            rundownItem.Unload();
        }

        private void InternalLoad(RundownItemBase rundownItem)
        {
            Debug.WriteLine(rundownItem?.Name, "InternalLoad");
            if (rundownItem != null)
            {
                rundownItem.FramePlayed += PlaiyngRundownItem_FramePlayed;
                switch (rundownItem)
                {
                    case FileRundownItem fileRundownItem:
                        fileRundownItem.Stopped += PlaiyngRundownItem_Stopped;
                        break;
                    case LiveInputRundownItem liveInputRundownItem:
                        break;
                    default:
                        Debug.Fail("Invalid rundownItem type");
                        break;
                }
                rundownItem.Prepare(Channel.AudioChannelCount);
                Channel.Load(rundownItem.Input);
                if (rundownItem.IsAutoStart)
                    rundownItem.Play();
            }
            Loaded?.Invoke(this, new RundownItemEventArgs(rundownItem));
        }

        internal void Submit(MediaFile media)
        {
            var item = new FileRundownItem(media);
            _rundown.Add(item);
            MediaSubmitted?.Invoke(this, new RundownItemEventArgs(item));
        }

        public bool Seek(TimeSpan timeSpan)
        {
            if (!(PlayingRundownItem is FileRundownItem file))
                return false;
            return file.Seek(timeSpan);
        }

        public void SetVolume(double value)
        {
            Channel.SetVolume(value);
        }

        public void Clear()
        {
            PlayingRundownItem = null;
            SetNext(null);
            Channel.Clear();
        }

        public void DeleteDisabled()
        {
            RundownItemBase item = null;
            while (true)
            {
                item = Rundown.FirstOrDefault(i => i.IsDisabled);
                if (item is null)
                    break;
                RemoveItem(item);
            }
        }

        private bool RemoveItem(RundownItemBase rundownItem)
        {
            if (!_rundown.Remove(rundownItem))
                return false;
            rundownItem.PropertyChanged -= RundownItem_PropertyChanged;
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
                var nextItem = _nextRundownItem;
                if (nextItem is null)
                {
                    Stopped?.Invoke(this, EventArgs.Empty);
                    return;
                }
                PlayingRundownItem = nextItem;
                nextItem.Play();
            });
        }

        private RundownItemBase FindNextAutoPlayItem()
        {
            var currentItem = PlayingRundownItem;
            using (var iterator = _rundown.GetEnumerator())
            {
                bool found = false;
                while (iterator.MoveNext())
                {
                    if (found)
                    {
                        if (iterator.Current != null && iterator.Current.IsAutoStart && !iterator.Current.IsDisabled)
                            return iterator.Current;
                    }
                    if (iterator.Current == currentItem)
                        found = true;
                }
            }
            if (IsLoop)
                return _rundown.FirstOrDefault(i => i != currentItem && i.IsAutoStart && !i.IsDisabled);
            else 
                return null;
        }

        private void PlaiyngRundownItem_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            FramePlayed?.Invoke(this, new TimeEventArgs(e.Time));
            var current = sender as FileRundownItem;
            if (current == null)
                return;
            var nextItem = _nextRundownItem;
            if (nextItem is null)
                return;
            if (current.Media.Duration - e.Time < PreloadTime)
                return;
            if (nextItem.Prepare(Channel.AudioChannelCount))
                Channel.Preload(nextItem.Input);
        }

        private void SetNext(RundownItemBase item)
        {
            var oldNext = _nextRundownItem;
            if (oldNext == item)
                return;
            _nextRundownItem = item;
            oldNext?.Unload();
        }

        private void RundownItem_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(FileRundownItem.IsAutoStart) ||
                e.PropertyName == nameof(FileRundownItem.IsDisabled))
                SetNext(FindNextAutoPlayItem());
        }

        private void Channel_AudioVolume(object sender, AudioVolumeEventArgs e)
        {
            AudioVolume?.Invoke(this, e);
        }

        private void RundownItem_RemoveRequested(object sender, EventArgs e)
        {
            var item = sender as RundownItemBase ?? throw new ArgumentException(nameof(sender));
            RemoveItem(item);
        }


        public void Dispose()
        {
            Channel.AudioVolume -= Channel_AudioVolume;
            PlayingRundownItem = null;
            foreach (var item in _rundown)
            {
                item.PropertyChanged -= RundownItem_PropertyChanged;
                item.RemoveRequested -= RundownItem_RemoveRequested;
                item.Dispose();
            }
            _rundown.Clear();
        }

    }


}
