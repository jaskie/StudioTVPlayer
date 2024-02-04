using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using GongSolutions.Wpf.DragDrop;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.ViewModel.Main.Input;
using StudioTVPlayer.ViewModel.Main.MediaBrowser;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class PlayerViewModel : ViewModelBase, IDisposable, IDropTarget
    {
        private readonly RundownPlayer _rundownPlayer;
        private const double AudioLevelMaxValue = -6.0;
        private const double AudioLevelMinValue = -60.0;
        private bool _isFocused;
        private TimeSpan? _displayTime;
        private TimeSpan? _outTime;
        private double _sliderPosition;
        private bool _isDisposed;
        private RundownItemViewModelBase _currentRundownItem;

        //private bool _isSliderDrag;
        private float _volume;
        private ImageSource _preview;
        private bool _outTimeBlink;
        private TimeSpan _timeFromBegin;
        private TimeDisplaySource _timeDisplaySource;

        public PlayerViewModel(RundownPlayer rundownPlayer)
        {
            _rundownPlayer = rundownPlayer;
            Name = rundownPlayer.Name;
            Id = rundownPlayer.Id;
            VideoFormat = rundownPlayer.VideoFormat;
            AudioLevelBars = Enumerable.Repeat(0, rundownPlayer.AudioChannelCount).Select(_ => new AudioLevelBarViewModel(AudioLevelMinValue, AudioLevelMaxValue)).ToArray();
            IsAlpha = rundownPlayer.IsAplha;
            if (rundownPlayer.LivePreview)
                _preview = rundownPlayer.GetPreview(224, 126);
            Rundown = new ObservableCollection<RundownItemViewModelBase>(rundownPlayer.Items.Select(CreateRundownItemViewModel));
            _currentRundownItem = Rundown.FirstOrDefault(item => item.RundownItem == rundownPlayer.LoadedItem);

            rundownPlayer.ItemLoaded += MediaPlayer_Loaded;
            rundownPlayer.Cleared += MediaPlayer_Cleared;
            rundownPlayer.FramePlayed += MediaPlayer_Progress;
            rundownPlayer.PlayerStateChanged += MediaPlayer_PlayerStateChanged;
            rundownPlayer.AudioVolume += Player_AudioVolume;
            rundownPlayer.MediaDurationChanged += MediaPlayer_MediaDurationChanged;
            rundownPlayer.ItemRemoved += Rundown_ItemRemoved;
            rundownPlayer.ItemAdded += Rundown_ItemAdded;

            LoadMediaCommand = new UiCommand(LoadMedia);
            LoadSelectedMediaCommand = new UiCommand(LoadSelectedMedia, _ => SelectedRundownItem != null);
            CueCommand = new UiCommand(Cue, _ => CurrentRundownItem is FileRundownItemViewModel);
            TogglePlayCommand = new UiCommand(TogglePlay, CanTogglePlay);
            ClearCommand = new UiCommand(Clear, _ => _rundownPlayer.IsLoaded());
            LoadNextItemCommand = new UiCommand(LoadNextItem, CanLoadNextItem);
            DeleteDisabledCommand = new UiCommand(DeleteDisabled, _ => Rundown.Any(i => i.RundownItem.IsDisabled));
            DisplayTimecodeEditCommand = new UiCommand(_ => { if (DisplayTime.HasValue) Seek(DisplayTime.Value); });
            SeekFramesCommand = new UiCommand(param => SeekFrames(param));
            SaveRundownCommand = new UiCommand(SaveRundown, _ => Rundown.Any());
            LoadRundownCommand = new UiCommand(LoadRundown, _ => _rundownPlayer.LoadedItem is null); 
        }

        public string Name { get; }

        public string Id { get; }

        public TVPlayR.VideoFormat VideoFormat { get; }

        public bool IsPlaying => _rundownPlayer.IsPlaying();

        public bool IsFocused
        {
            get => _isFocused;
            set => Set(ref _isFocused, value);
        }

        public TimeSpan? DisplayTime
        {
            get => _displayTime;
            set => Set(ref _displayTime, value);
        }

        public TimeDisplaySource TimeDisplaySource
        {
            get => _timeDisplaySource;
            set
            {
                if (_timeDisplaySource == value)
                    return;
                _timeDisplaySource = value;
                switch(value)
                {
                    case TimeDisplaySource.TimeFromBegin:
                        Set(ref _displayTime, TimeFromBegin, nameof(DisplayTime));
                        break;
                    case TimeDisplaySource.Timecode:
                        Set(ref _displayTime, Timecode, nameof(DisplayTime));
                        break;
                }
            }
        }

        public TimeSpan TimeFromBegin
        {
            get => _timeFromBegin;
            set
            {
                if (_timeFromBegin == value)
                    return;
                _timeFromBegin = value;
            }
        }

        public TimeSpan Timecode { get; set; }

        public TimeSpan CurrentItemStartTime => (CurrentRundownItem?.RundownItem as FileRundownItem)?.Media.StartTime ?? TimeSpan.Zero;

        public TimeSpan CurrentItemDuration => (CurrentRundownItem?.RundownItem as FileRundownItem)?.Media.Duration ?? TimeSpan.Zero;


        public TimeSpan? OutTime
        {
            get => _outTime;
            set => Set(ref _outTime, value);
        }

        public bool OutTimeBlink { get => _outTimeBlink; set => Set(ref _outTimeBlink, value); }

        public double SliderPosition
        {
            get => _sliderPosition;
            set
            {
                if (!Set(ref _sliderPosition, value))
                    return;
                Seek(TimeSpan.FromSeconds(value));
            }
        }

        public float Volume
        {
            get => _volume;
            set
            {
                if (!Set(ref _volume, value))
                    return;
                _rundownPlayer.SetVolume(value);
            }
        }

        public AudioLevelBarViewModel[] AudioLevelBars { get; }

        public bool IsLoop
        {
            get => _rundownPlayer.IsLoop;
            set
            {
                if (_rundownPlayer.IsLoop == value)
                    return;
                _rundownPlayer.IsLoop = value;
                NotifyPropertyChanged();
            }
        }

        public bool DisableAfterUnload
        {
            get => _rundownPlayer.DisableAfterUnload;
            set
            {
                if (_rundownPlayer.DisableAfterUnload == value)
                    return;
                _rundownPlayer.DisableAfterUnload = value;
                NotifyPropertyChanged();
            }
        }

        public bool IsSeekable => _currentRundownItem?.RundownItem.CanSeek == true;

        public bool IsLive => _currentRundownItem?.RundownItem.CanSeek != true;

        public ImageSource Preview { get => _preview; private set => Set(ref _preview, value); }

        public bool IsAlpha { get; }

        public ObservableCollection<RundownItemViewModelBase> Rundown { get; }

        public RundownItemViewModelBase SelectedRundownItem { get; set; }

        public RundownItemViewModelBase CurrentRundownItem
        {
            get => _currentRundownItem;
            private set
            {
                var oldItem = _currentRundownItem;
                if (!Set(ref _currentRundownItem, value))
                    return;
                if (value is null)
                {
                    DisplayTime = null;
                    OutTime = null;
                    OutTimeBlink = false;
                }
                else
                    value.IsLoaded = true;
                if (oldItem != null)
                    oldItem.IsLoaded = false;
                _sliderPosition = CurrentItemStartTime.TotalSeconds;
                NotifyPropertyChanged(nameof(IsPlaying));
                NotifyPropertyChanged(nameof(SliderPosition));
                NotifyPropertyChanged(nameof(CurrentItemStartTime));
                NotifyPropertyChanged(nameof(CurrentItemDuration));
                NotifyPropertyChanged(nameof(IsSeekable));
                NotifyPropertyChanged(nameof(IsLive));
            }
        }

        public ICommand LoadMediaCommand { get; }
        public ICommand LoadSelectedMediaCommand { get; }
        public ICommand CueCommand { get; }
        public ICommand DisplayTimecodeEditCommand { get; }
        public ICommand DeleteDisabledCommand { get; }
        public ICommand SeekFramesCommand { get; }

        public ICommand TogglePlayCommand { get; }
        public ICommand ClearCommand { get; }
        public ICommand LoadNextItemCommand { get; }

        public ICommand SaveRundownCommand { get; }
        public ICommand LoadRundownCommand { get; }

        private async void SeekFrames(object param)
        {
            if (param == null)
                return;
            await Pause();
            var frameNumber = VideoFormat.TimeToFrameNumber(_displayTime.Value);
            switch (param.ToString())
            {
                case "-1":
                    {
                        --frameNumber;
                        break;
                    }

                case "1":
                    {
                        ++frameNumber;
                        break;
                    }

                case "-second":
                    {
                        frameNumber -= VideoFormat.FrameRate.Numerator / VideoFormat.FrameRate.Denominator;
                        break;
                    }

                case "second":
                    {
                        frameNumber += VideoFormat.FrameRate.Numerator / VideoFormat.FrameRate.Denominator;
                        break;
                    }
            }
            Seek(VideoFormat.FrameNumberToTime(frameNumber));
            await Pause();
        }

        private void Seek(TimeSpan time)
        {
            if (!_rundownPlayer.Seek(time))
                return;
            _sliderPosition = time.TotalSeconds;
            NotifyPropertyChanged(nameof(SliderPosition));
        }

        private void LoadSelectedMedia(object obj)
        {
            if (SelectedRundownItem == CurrentRundownItem)
                return;
            LoadMedia(SelectedRundownItem);
        }


        private async void Cue(object obj)
        {
            if (!await Pause())
                return;
            Seek(CurrentItemStartTime);
        }

        private void DeleteDisabled(object obj)
        {
            _rundownPlayer.DeleteDisabled();
        }

        internal void EndSliderThumbDrag()
        {
            //Seek(TimeSpan.FromMilliseconds(SliderPosition));
            //_isSliderDrag = false;
        }

        internal void BeginSliderThumbDrag()
        {
            //_isSliderDrag = true;
        }

        private void LoadMedia(object param)
        {
            LoadMedia(((param as object[])?[0] as FrameworkElement)?.DataContext as RundownItemViewModelBase ?? throw new ArgumentException(nameof(param)));
        }

        private void LoadMedia(RundownItemViewModelBase playerItem) => _rundownPlayer.Load(playerItem.RundownItem);

        private void LoadNextItem(object _) => _rundownPlayer.LoadNextItem();

        private bool CanLoadNextItem(object _) => _rundownPlayer.CanLoadNextItem();

        private void Clear(object _) => _rundownPlayer.Clear();

        private async void TogglePlay(object _)
        {
            if (IsPlaying)
                await Pause();
            else
                await Play();
        }

        public async Task<bool> Play()
        {
            try
            {
                _rundownPlayer.Play();
            }
            catch
            {
                await MainViewModel.Instance.ShowMessageAsync("Error", $"Error starting {_rundownPlayer.LoadedItem?.Name }");
                return false;
            }
            return true;
        }

        public async Task<bool> Pause()
        {
            try
            {
                _rundownPlayer.Pause();
                OutTimeBlink = false;
            }
            catch (Exception e)
            {
                Debug.WriteLine(e);
                await MainViewModel.Instance.ShowMessageAsync("Error", $"Error pausing clip {_rundownPlayer.LoadedItem?.Name}");
                return false;
            }
            return true;
        }

        private void MediaPlayer_Progress(object sender, TVPlayR.TimeEventArgs e)
        {
            OnUiThread(() =>
            {
                switch (TimeDisplaySource)
                {
                    case TimeDisplaySource.TimeFromBegin:
                        DisplayTime = e.TimeFromBegin;
                        break;
                    case TimeDisplaySource.Timecode:
                        DisplayTime = e.Timecode;
                        break;
                    default:
                        DisplayTime = null;
                        break;
                }
                OutTime = e.TimeToEnd;
                OutTimeBlink = IsPlaying && OutTime < TimeSpan.FromSeconds(10) && OutTime > _rundownPlayer.OneFrame;
                Set(ref _sliderPosition, e.TimeFromBegin.TotalSeconds, nameof(SliderPosition));
            });
        }

        private void MediaPlayer_PlayerStateChanged(object sender, EventArgs e)
        {
            Debug.WriteLine("PlayerStateChanged");
            NotifyPropertyChanged(nameof(IsPlaying));
            Refresh();
        }

        private void MediaPlayer_Loaded(object sender, Model.Args.RundownItemEventArgs e)
        {
            CurrentRundownItem = Rundown.FirstOrDefault(i => i.RundownItem == e.RundownItem);
            if (!_rundownPlayer.LivePreview)
                Preview = CurrentRundownItem?.Thumbnail;
        }

        private void MediaPlayer_Cleared(object sender, EventArgs e)
        {
            CurrentRundownItem = null;
            if (!_rundownPlayer.LivePreview)
                Preview = null;
        }

        private void Rundown_ItemRemoved(object sender, Model.Args.RundownItemIndexedEventArgs e)
        {
            Rundown.RemoveAt(e.Index);
        }

        private void Rundown_ItemAdded(object sender, Model.Args.RundownItemIndexedEventArgs e)
        {
            RundownItemViewModelBase rundownItemViewModel = null;
            switch (e.RundownItem)
            {
                case Model.FileRundownItem fileRundownItem:
                    rundownItemViewModel = new FileRundownItemViewModel(fileRundownItem);
                    break;
                case Model.LiveInputRundownItem liveInputRundownItem:
                    rundownItemViewModel = new LiveInputRundownItemViewModel(liveInputRundownItem);
                    break;
            }
            if (rundownItemViewModel is null)
                return;
            if (e.Index == -1)
                Rundown.Add(rundownItemViewModel);
            else
                Rundown.Insert(e.Index, rundownItemViewModel);
            Refresh();
        }

        private void MediaPlayer_MediaDurationChanged(object sender, EventArgs e)
        {
            NotifyPropertyChanged(nameof(CurrentItemDuration));
        }

        private void Player_AudioVolume(object sender, Model.Args.AudioVolumeEventArgs e)
        {
            if (e.AudioVolume.Length == 0)
            {
                foreach (var bar in AudioLevelBars)
                    bar.AudioLevel = bar.MinValue;
                return;
            }
            Debug.Assert(e.AudioVolume.Length == AudioLevelBars.Length);
            for (int i = 0; i < e.AudioVolume.Length; i++)
                AudioLevelBars[i].AudioLevel = Math.Max(AudioLevelBars[i].MinValue, 20 * Math.Log10(e.AudioVolume[i]));
        }

        private bool CanTogglePlay(object obj)
        {
            if (!(_rundownPlayer.LoadedItem is FileRundownItem))
                return false;
            if (!IsPlaying && _rundownPlayer.IsEof())
                return false;
            return true;
        }

        public void Dispose()
        {
            if (_isDisposed)
                return;
            _isDisposed = true;
            _rundownPlayer.ItemLoaded -= MediaPlayer_Loaded;
            _rundownPlayer.Cleared -= MediaPlayer_Cleared;
            _rundownPlayer.FramePlayed -= MediaPlayer_Progress;
            _rundownPlayer.PlayerStateChanged -= MediaPlayer_PlayerStateChanged;
            _rundownPlayer.MediaDurationChanged -= MediaPlayer_MediaDurationChanged;
            _rundownPlayer.ItemRemoved -= Rundown_ItemRemoved;
            _rundownPlayer.ItemAdded -= Rundown_ItemAdded;
        }

        #region drag&drop
        public void DragOver(IDropInfo dropInfo)
        {
            switch (dropInfo.Data)
            {
                case MediaViewModel mediaViewModel:
                    if (!mediaViewModel.IsVerified || mediaViewModel.Duration <= TimeSpan.Zero)
                        return;
                    break;
                case DecklinkInputViewModel decklink:
                    if (!decklink.Input.IsRunning)
                        return;
                    break;
                case LiveInputRundownItemViewModel _:
                case FileRundownItemViewModel _:
                    if (!CanDrop(dropInfo))
                        return;
                    break;
                case IDataObject dataObject:
                    if ((dataObject.GetData(DataFormats.FileDrop) as string[])?.FirstOrDefault(f => File.Exists(f)) is null)
                        return;
                    break;
                default:
                    return;
            }
            dropInfo.DropTargetAdorner = DropTargetAdorners.Insert;
            dropInfo.Effects = DragDropEffects.Move;
        }

        public void Drop(IDropInfo dropInfo)
        {
            switch (dropInfo.Data)
            {
                case MediaViewModel mediaViewModel:
                    var index = dropInfo.TargetCollection is null ? Rundown.Count : dropInfo.InsertIndex;
                    _rundownPlayer.AddMediaToQueue(mediaViewModel.Media, index);
                    break;

                case DecklinkInputViewModel decklink:
                    index = dropInfo.TargetCollection is null ? Rundown.Count : dropInfo.InsertIndex;
                    _rundownPlayer.AddLiveToQueue(decklink.Input, index);
                    break;
                case LiveInputRundownItemViewModel _:
                case FileRundownItemViewModel _:
                    var srcIndex = dropInfo.DragInfo.SourceIndex;
                    var destIndex = dropInfo.InsertIndex;
                    if (destIndex > srcIndex)
                        destIndex--;
                    _rundownPlayer.MoveItem(srcIndex, destIndex);
                    Rundown.Move(srcIndex, destIndex);
                    break;

                case IDataObject dataObject:
                    var fileName = (dataObject.GetData(DataFormats.FileDrop) as string[])?.FirstOrDefault(f => File.Exists(f));
                    if (fileName is null)
                        return;
                    var media = new MediaFile(fileName);
                    MediaVerifier.Current.Verify(media);
                    if (!media.IsValid)
                        return;
                    index = dropInfo.TargetCollection is null ? Rundown.Count : dropInfo.InsertIndex;
                    _rundownPlayer.AddMediaToQueue(media, index);
                    break;
            }
        }

        public bool CanDrop(IDropInfo dropInfo)
        {
            if (dropInfo.InsertIndex == dropInfo.DragInfo.SourceIndex || dropInfo.InsertIndex == dropInfo.DragInfo.SourceIndex + 1)
                return false;
            if (dropInfo.DragInfo.SourceCollection != Rundown)
                return false;
            if (dropInfo.InsertPosition == RelativeInsertPosition.None)
                return false;
            return true;
        }

        public void DragEnter(IDropInfo dropInfo) { }

        public void DragLeave(IDropInfo dropInfo) { }

        #endregion //drag&drop

        private RundownItemViewModelBase CreateRundownItemViewModel(RundownItemBase rundownItem)
        {
            switch (rundownItem)
            {
                case FileRundownItem fileRundownItem:
                    return new FileRundownItemViewModel(fileRundownItem);
                case LiveInputRundownItem liveInputRundownItem:
                    return new LiveInputRundownItemViewModel(liveInputRundownItem);
                default:
                    throw new ArgumentException(nameof(rundownItem));
            }
        }

        private void SaveRundown(object _)
        {
            var fileName = FolderHelper.SaveFileDialog("Save rundown as", "Rundowns", "rundown");
            if (string.IsNullOrEmpty(fileName))
                return;
            try
            {
                UISBusyState.SetBusyState();
                _rundownPlayer.SaveRundown(fileName);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Unable to save rundown. Error was:\n{ex.InnerException ?? ex}", "Error saving rundown", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void LoadRundown(object _)
        {
            var fileName = FolderHelper.OpenFileDialog("Load rundown", "Rundowns", "rundown");
            if (string.IsNullOrEmpty(fileName))
                return;
            try
            {
                UISBusyState.SetBusyState();
                _rundownPlayer.LoadRundown(fileName);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Unable to load rundown. Error was:\n{ex.InnerException ?? ex}", "Error loading rundown", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }


    }
}
