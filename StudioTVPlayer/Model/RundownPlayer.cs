using StudioTVPlayer.Model.Args;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class RundownPlayer : Player
    {
        private readonly TimeSpan PreloadTime = TimeSpan.FromSeconds(1);
        private RundownItemBase _playingRundownItem;
        private Rundown _rundown = new Rundown();

        public RundownPlayer(Configuration.Player configuration): base(configuration)
        {
            _rundown.Loaded += Rundown_Loaded;
        }

        public RundownItemBase PlayingRundownItem
        {
            get => _playingRundownItem;
            private set
            {
                var oldItem = _playingRundownItem;
                if (_playingRundownItem == value)
                    return;
                _playingRundownItem = value;
                AfterPlayed(oldItem);
                BeforePlay(value);
            }
        }

        public bool DisableAfterUnload { get; set; }

        public bool IsEof => (PlayingRundownItem?.Input as TVPlayR.FileInput)?.IsEof ?? true;

        public TimeSpan OneFrame => VideoFormat.FrameNumberToTime(1);

        public bool IsAplha => PixelFormat == TVPlayR.PixelFormat.bgra;

        public bool IsPlaying => _playingRundownItem?.IsPlaying == true;

        public Rundown Rundown { get => _rundown; }

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
        public event EventHandler<TVPlayR.TimeEventArgs> FramePlayed;
        public event EventHandler Finished;
        public event EventHandler<RundownItemEventArgs> MediaSubmitted;
        public event EventHandler MediaDurationChanged;

        public void Load(RundownItemBase item)
        {
            if (!_rundown.Contains(item))
                return;
            item.Prepare(AudioChannelCount);
            if (item.IsAutoStart)
                item.Play();
            PlayingRundownItem = item;
            base.Load(item.Input);
        }

        public FileRundownItem AddMediaToQueue(MediaFile media, int index)
        {
            var item = new FileRundownItem(media) { IsAutoStart = AddItemsWithAutoPlay };
            _rundown.Add(item, index);
            return item;
        }

        public LiveInputRundownItem AddLiveToQueue(InputBase input, int index)
        {
            var item = new LiveInputRundownItem(input) { IsAutoStart = AddItemsWithAutoPlay };
            _rundown.Add(item, index);
            return item;
        }

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
                if (DisableAfterUnload)
                    fileRundownItem.IsDisabled = true;
            }
            rundownItem.Pause();
            rundownItem.Unload();
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
            var media = sender as MediaFile;
            if (media is null) return;
            switch (e.PropertyName)
            {
                case nameof(MediaFile.Duration):
                    MediaDurationChanged?.Invoke(this, EventArgs.Empty);
                    break;
            }
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

        public override void Clear()
        {
            PlayingRundownItem = null;
            base.Clear();
        }

        public void DeleteDisabled() => _rundown.DeleteDisabled();

        private void PlaiyngRundownItem_Paused(object sender, EventArgs e)
        {
            Task.Run(() => // do not block incoming thread
            {
                if (sender  == _playingRundownItem) // next didn't loaded
                    Finished?.Invoke(this, EventArgs.Empty);
            });
        }

        private void Rundown_Loaded(object sender, RundownItemEventArgs e)
        {
            PlayingRundownItem = e.RundownItem;
            Loaded?.Invoke(this, e);
        }

        private void PlaiyngRundownItem_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            FramePlayed?.Invoke(this, e);
            var current = sender as FileRundownItem;
            if (current == null || current != _playingRundownItem)
                return;
            if (current.Media.Duration - e.TimeFromBegin < PreloadTime)
                return;
            var next = _rundown.NextAutoPlayItem;
            if (next == null)
                return;
            if (next.Prepare(AudioChannelCount))
            {
                PlayNext(next.Input);
                next.Play();
            }
        }

        public override void Initialize()
        {
            base.Initialize();
            PlayingRundownItem = null;
        }

        public override void Dispose()
        {
            base.Dispose();
            _rundown.Dispose();
            _rundown.Loaded -= Rundown_Loaded;
            PlayingRundownItem = null;
        }

    }


}
