using StudioTVPlayer.Model.Args;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class RundownPlayer : Player
    {
        private static readonly TimeSpan PreloadTime = TimeSpan.FromSeconds(2);
        private RundownItemBase _loadedItem;
        private RundownItemBase _loadedNextRundownItem;
        private PlayerState _playerState = PlayerState.Unloaded;
        private readonly Rundown _rundown = new Rundown();
        private bool _disposed;

        public RundownPlayer(Configuration.Player configuration) : base(configuration)
        {
            _rundown.ItemLoaded += Rundown_ItemLoaded;
            _rundown.ItemAdded += Rundown_ItemAdded;
            _rundown.ItemRemoved += Rundown_ItemRemoved;
        }
        public RundownItemBase LoadedItem
        {
            get => _loadedItem;
            private set
            {
                var oldItem = _loadedItem;
                if (_loadedItem == value)
                    return;
                _loadedItem = value;
                AfterPlayed(oldItem);
                BeforePlay(value);
                if (value is null)
                    PlayerState = PlayerState.Unloaded;
                else
                    PlayerState = value.IsPlaying() ? PlayerState.Playing : PlayerState.Cue;
            }
        }

        public bool DisableAfterUnload { get; set; }

        public bool IsEof() => _loadedItem?.IsEof ?? true;

        public TimeSpan OneFrame => VideoFormat.FrameNumberToTime(1);

        public bool IsPlaying() => _loadedItem?.IsPlaying() == true;

        public bool IsLoaded() => _loadedItem != null;

        #region Rundown class wrapper stuff
        public bool IsLoop { get => _rundown.IsLoop; set => _rundown.IsLoop = value; }
        
        public List<RundownItemBase> Items => _rundown.Items;

        public void MoveItem(int srcIndex, int destIndex) => _rundown.MoveItem(srcIndex, destIndex);

        private void Rundown_ItemAdded(object sender, RundownItemIndexedEventArgs e)
        {
            ItemAdded?.Invoke(this, e);
        }

        private void Rundown_ItemRemoved(object sender, RundownItemIndexedEventArgs e)
        {
            ItemRemoved?.Invoke(this, e);
        }

        #endregion Rundown class wrapper stuff
        public void Play()
        {
            var rundownItem = _loadedItem;
            if (rundownItem is null)
                return;
            rundownItem.Play();
            PlayerState = PlayerState.Playing;
        }

        public void Pause()
        {
            var rundownItem = _loadedItem;
            if (!(rundownItem?.CanSeek() ?? false))
                return;
            rundownItem.Pause();
            PlayerState = PlayerState.Paused;
        }

        public void Toggle()
        {
            var rundownItem = _loadedItem;
            if (!(rundownItem?.CanSeek() ?? false))
                return;
            if (rundownItem.IsPlaying())
                rundownItem.Pause();
            else
                rundownItem.Play();
            PlayerState = rundownItem.IsPlaying() ? PlayerState.Playing : PlayerState.Paused;
        }

        public void Cue()
        {
            if (!(_loadedItem is RundownItemBase rundownItem))
                return;
            rundownItem.Pause();
            if (rundownItem.Seek((rundownItem as FileRundownItem)?.Media.StartTime ?? TimeSpan.Zero))
                PlayerState = PlayerState.Cue;
            else 
                PlayerState = PlayerState.Paused;
        }

        public bool CanCue()
        {
            var loadedItem = _loadedItem;
            if (loadedItem is null || !loadedItem.CanSeek())
                return false;
            return true;
        }

        public void LoadNextItem() => Load(_rundown.FindNextItemToLoad(_loadedItem));

        public PlayerState PlayerState
        {
            get =>  _playerState;
            set
            {
                if (value == _playerState)
                    return;
                _playerState = value;
                PlayerStateChanged?.Invoke(this, EventArgs.Empty);
            }
        }

        public event EventHandler<RundownItemEventArgs> ItemLoaded;
        public event EventHandler<RundownItemIndexedEventArgs> ItemAdded;
        public event EventHandler<RundownItemIndexedEventArgs> ItemRemoved;
        public event EventHandler Seeked;
        public event EventHandler<TVPlayR.TimeEventArgs> FramePlayed;
        public event EventHandler PlayerStateChanged;
        public event EventHandler MediaDurationChanged;

        public void Load(RundownItemBase item)
        {
            if (!_rundown.Contains(item) || item.IsDisabled)
                return;
            item.Prepare(AudioChannelCount);
            if (item.IsAutoStart)
                item.Play();
            LoadedItem = item;
            Load(item.TVPlayRInput);
        }

        public bool CanLoadNextItem() => _rundown.CanLoadNextItem(_loadedItem);

        public void AddMediaToQueue(MediaFile media, int index)
        {
            var item = new FileRundownItem(media) { IsAutoStart = AddItemsWithAutoPlay };
            _rundown.Add(item, index);
        }

        public void AddLiveToQueue(InputBase input, int index)
        {
            var item = new LiveInputRundownItem(input) { IsAutoStart = AddItemsWithAutoPlay };
            _rundown.Add(item, index);
        }

        public void LoadRundown(string fileName) => Persistence.RundownPersister.LoadRundown(_rundown, fileName);

        public void SaveRundown(string fileName) => Persistence.RundownPersister.SaveRundown(_rundown, fileName);

        private void AfterPlayed(RundownItemBase rundownItem)
        {
            if (rundownItem == null)
                return;
            Debug.WriteLine(rundownItem.Name, "InternalUnload");
            rundownItem.FramePlayed -= PlaiyngRundownItem_FramePlayed;
            if (rundownItem is FileRundownItem fileRundownItem)
            {
                fileRundownItem.Paused -= PlaiyngRundownItem_Paused;
                fileRundownItem.Media.PropertyChanged -= Media_PropertyChanged;
            }
            rundownItem.Pause();
            rundownItem.Unload();
            if (DisableAfterUnload)
                rundownItem.IsDisabled = true;
        }

        private void BeforePlay(RundownItemBase rundownItem)
        {
            if (rundownItem != null)
            {
                rundownItem.FramePlayed += PlaiyngRundownItem_FramePlayed;
                switch (rundownItem)
                {
                    case FileRundownItem fileRundownItem:
                        fileRundownItem.Paused += PlaiyngRundownItem_Paused;
                        fileRundownItem.Media.PropertyChanged += Media_PropertyChanged;
                        break;
                    case LiveInputRundownItem liveInputRundownItem:
                        break;
                    default:
                        Debug.Fail("Invalid rundownItem type");
                        break;
                }
            }
        }

        private void Media_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (!(sender is MediaFile media))
                return;
            switch (e.PropertyName)
            {
                case nameof(MediaFile.Duration):
                    MediaDurationChanged?.Invoke(this, EventArgs.Empty);
                    break;
            }
        }

        internal void Submit(MediaFile media)
        {
            var item = new FileRundownItem(media) { IsAutoStart = AddItemsWithAutoPlay };
            _rundown.Add(item);
        }

        public bool Seek(TimeSpan timeSpan)
        {
            if (!(LoadedItem is FileRundownItem file) ||
                !file.Seek(timeSpan))
                return false;
            Seeked?.Invoke(this, EventArgs.Empty);
            return true;
        }

        public override void Clear()
        {
            LoadedItem = null;
            base.Clear();
            PlayerState = PlayerState.Unloaded;
        }

        public void DeleteDisabled() => _rundown.DeleteDisabled();

        private void PlaiyngRundownItem_Paused(object sender, EventArgs e)
        {
            Task.Run(() => // do not block incoming thread
            {
                var rundownItem = sender as RundownItemBase ?? throw new ArgumentException(nameof(sender));
                if (rundownItem == _loadedItem) // next didn't loaded
                {
                    if (rundownItem.IsEof)
                        PlayerState = PlayerState.Finished;
                    else 
                        PlayerState = PlayerState.Paused;
                }
            });
        }

        private void Rundown_ItemLoaded(object sender, RundownItemEventArgs e)
        {
            LoadedItem = e.RundownItem;
            ItemLoaded?.Invoke(this, e);
        }

        private void PlaiyngRundownItem_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            FramePlayed?.Invoke(this, e);
            var current = sender as FileRundownItem;
            if (current == null || current != _loadedItem)
                return;
            if (current.Media.Duration - e.TimeFromBegin > PreloadTime)
                return;
            var next = _rundown.NextAutoPlayItem;
            if (next == null)
                return;
            if (LoadNext(next))
            {
                var previouslyLoadedNext = Interlocked.Exchange(ref _loadedNextRundownItem, next);
                if (previouslyLoadedNext != null && previouslyLoadedNext != _loadedItem)
                    previouslyLoadedNext.Unload();
            }
        }

        public override void Initialize()
        {
            base.Initialize();
            LoadedItem = null;
        }

        public override void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            base.Dispose();
            _rundown.Dispose();
            _rundown.ItemLoaded -= Rundown_ItemLoaded;
            _rundown.ItemAdded -= Rundown_ItemAdded;
            _rundown.ItemRemoved -= Rundown_ItemRemoved;
            LoadedItem = null;
        }

    }


}
