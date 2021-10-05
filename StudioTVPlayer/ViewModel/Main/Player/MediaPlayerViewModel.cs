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
    public class MediaPlayerViewModel : ViewModelBase, IDisposable, IDropTarget
    {
        private readonly Model.MediaPlayer _mediaPlayer;
        private readonly MahApps.Metro.Controls.Dialogs.IDialogCoordinator _dialogCoordinator = MahApps.Metro.Controls.Dialogs.DialogCoordinator.Instance;
        private const double AudioHeadroom = 9.0;
        private bool _isFocused;
        private TimeSpan _displayTime;
        private TimeSpan _outTime;
        private double _sliderPosition;
        private bool _isDisposed;
        private RundownItemViewModelBase _currentRundownItem;

        //private bool _isSliderDrag;
        private double _volume;
        private ImageSource _preview;
        private bool _outTimeBlink;

        public MediaPlayerViewModel(Model.MediaPlayer player)
        {
            LoadMediaCommand = new UiCommand(LoadMedia);
            LoadSelectedMediaCommand = new UiCommand(LoadSelectedMedia, _ => SelectedRundownItem != null);
            CueCommand = new UiCommand(Cue, _ => CurrentRundownItem is FileRundownItemViewModel);
            TogglePlayCommand = new UiCommand(TogglePlay, CanTogglePlay);
            UnloadCommand = new UiCommand(Unload, _ => CurrentRundownItem != null);
            LoadNextItemCommand = new UiCommand(LoadNextItem, CanLoadNextItem);
            DeleteDisabledCommand = new UiCommand(DeleteDisabled, _ => Rundown.Any(i => i.RundownItem.IsDisabled));
            DisplayTimecodeEditCommand = new UiCommand(_ => Seek(DisplayTime));
            SeekFramesCommand = new UiCommand(param => SeekFrames(param));
            Name = player.Channel.Name;
            VideoFormat = player.Channel.VideoFormat;
            AudioLevelBars = Enumerable.Repeat(0, player.Channel.AudioChannelCount).Select(_ => new AudioLevelBarViewModel()).ToArray();
            player.Loaded += MediaPlayer_Loaded;
            player.FramePlayed += MediaPlayer_Progress;
            player.Stopped += MediaPlayer_Stopped;
            player.MediaSubmitted += MediaPlayer_MediaSubmitted;
            player.AudioVolume += Player_AudioVolume;
            player.Removed += MediaPlayer_Removed;
            if (player.Channel.LivePreview)
                _preview = player.GetPreview(224, 126);
            IsAlpha = player.IsAplha;
            Rundown = new ObservableCollection<RundownItemViewModelBase>(player.Rundown.Select(CreateRundownItemViewModel));
                
            _currentRundownItem = Rundown.FirstOrDefault(item => item.RundownItem == player.PlayingRundownItem);
            _mediaPlayer = player;
        }

        public string Name { get; }

        public TVPlayR.VideoFormat VideoFormat { get; }

        public bool IsPlaying => _mediaPlayer.IsPlaying;

        public bool IsFocused
        {
            get => _isFocused;
            set => Set(ref _isFocused, value);
        }

        public TimeSpan DisplayTime
        {
            get => _displayTime;
            set => Set(ref _displayTime, value);
        }

        public TimeSpan CurrentItemStartTime => (CurrentRundownItem?.RundownItem as FileRundownItem)?.Media.StartTime ?? TimeSpan.Zero;

        public TimeSpan CurrentItemDuration => (CurrentRundownItem?.RundownItem as FileRundownItem)?.Media.Duration ?? TimeSpan.Zero;


        public TimeSpan OutTime
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
                Seek(TimeSpan.FromMilliseconds(value));
            }
        }

        public double Volume
        {
            get => _volume;
            set
            {
                if (!Set(ref _volume, value))
                    return;
                _mediaPlayer.SetVolume(value);
            }
        }

        public AudioLevelBarViewModel[] AudioLevelBars { get; }

        public bool IsLoop
        {
            get => _mediaPlayer.IsLoop;
            set
            {
                if (_mediaPlayer.IsLoop == value)
                    return;
                _mediaPlayer.IsLoop = value;
                NotifyPropertyChanged();
            }
        }

        public bool DisableAfterUnload
        {
            get => _mediaPlayer.DisableAfterUnload;
            set
            {
                if (_mediaPlayer.DisableAfterUnload == value)
                    return;
                _mediaPlayer.DisableAfterUnload = value;
                NotifyPropertyChanged();
            }
        }

        public bool IsSliderActive => _currentRundownItem?.RundownItem.CanSeek == true;

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
                    DisplayTime = TimeSpan.Zero;
                    OutTime = TimeSpan.Zero;
                    OutTimeBlink = false;
                }
                else
                    value.IsLoaded = true;
                if (oldItem != null)
                    oldItem.IsLoaded = false;
                _sliderPosition = CurrentItemStartTime.TotalMilliseconds;
                NotifyPropertyChanged(nameof(IsPlaying));
                NotifyPropertyChanged(nameof(SliderPosition));
                NotifyPropertyChanged(nameof(CurrentItemStartTime));
                NotifyPropertyChanged(nameof(CurrentItemDuration));
                NotifyPropertyChanged(nameof(IsSliderActive));
            }
        }

        public ICommand LoadMediaCommand { get; }
        public ICommand LoadSelectedMediaCommand { get; }
        public ICommand CueCommand { get; }
        public ICommand DisplayTimecodeEditCommand { get; }
        public ICommand DeleteDisabledCommand { get; }
        public ICommand SeekFramesCommand { get; }

        public ICommand TogglePlayCommand { get; }
        public ICommand UnloadCommand { get; }
        public ICommand LoadNextItemCommand { get; }

        private async void SeekFrames(object param)
        {
            if (param == null)
                return;
            await Pause();
            var frameNumber = VideoFormat.TimeToFrameNumber(_displayTime);
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
            if (!_mediaPlayer.Seek(time))
                return;
            _sliderPosition = time.TotalMilliseconds;
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
            _mediaPlayer.DeleteDisabled();
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

        private void LoadMedia(RundownItemViewModelBase playerItem)
        {
            if (playerItem.RundownItem.IsDisabled)
                return;
            _mediaPlayer.Load(playerItem.RundownItem);
        }

        private void LoadNextItem(object _)
        {
            var currentIndex = Rundown.IndexOf(CurrentRundownItem);
            if (currentIndex >= Rundown.Count - 1)
                return;
            while (++currentIndex < Rundown.Count)
            {
                if (Rundown[currentIndex].RundownItem.IsDisabled)
                    continue;
                LoadMedia(Rundown[currentIndex]);
                return;
            }
        }

        private bool CanLoadNextItem(object _)
        {
            var currentIndex = Rundown.IndexOf(CurrentRundownItem);
            while (++currentIndex < Rundown.Count)
            {
                if (Rundown[currentIndex].RundownItem.IsDisabled)
                    continue;
                return true;
            }
            return false;
        }

        private void Unload(object _)
        {
            _mediaPlayer.Clear();
        }

        private async void TogglePlay(object _)
        {
            if (IsPlaying)
                await Pause();
            else
                await Play();
        }


        private async Task<bool> Play()
        {
            try
            {
                _mediaPlayer.Play();
                NotifyPropertyChanged(nameof(IsPlaying));

            }
            catch
            {
                await _dialogCoordinator.ShowMessageAsync(MainViewModel.Instance, "Error", $"Error starting {_mediaPlayer.PlayingRundownItem?.Name }", MahApps.Metro.Controls.Dialogs.MessageDialogStyle.Affirmative);
                return false;
            }
            return true;
        }

        public async Task<bool> Pause()
        {
            try
            {
                _mediaPlayer.Pause();
                NotifyPropertyChanged(nameof(IsPlaying));
                OutTimeBlink = false;
            }
            catch
            {
                await _dialogCoordinator.ShowMessageAsync(MainViewModel.Instance, "Error", $"Error pausing clip {_mediaPlayer.PlayingRundownItem?.Name}", MahApps.Metro.Controls.Dialogs.MessageDialogStyle.Affirmative);
                return false;
            }
            return true;
        }

        private void MediaPlayer_Progress(object sender, Model.Args.TimeEventArgs e)
        {
            OnUiThread(() =>
            {
                DisplayTime = e.Time;
                OutTime = CurrentItemDuration - e.Time - _mediaPlayer.OneFrame;
                OutTimeBlink = IsPlaying && OutTime < TimeSpan.FromSeconds(10) && OutTime > _mediaPlayer.OneFrame;
                if (/*!_isSliderDrag && */IsPlaying)
                {
                    _sliderPosition = e.Time.TotalMilliseconds;
                    NotifyPropertyChanged(nameof(SliderPosition));
                }
            });
        }

        private void MediaPlayer_Stopped(object sender, EventArgs e)
        {
            Debug.WriteLine("Stopped");
            NotifyPropertyChanged(nameof(IsPlaying));
            Refresh();
        }


        private void MediaPlayer_Loaded(object sender, Model.Args.RundownItemEventArgs e)
        {
            CurrentRundownItem = Rundown.FirstOrDefault(i => i.RundownItem == e.RundownItem);
            if (!_mediaPlayer.Channel.LivePreview)
                Preview = CurrentRundownItem?.Thumbnail;
        }

        private void MediaPlayer_MediaSubmitted(object sender, Model.Args.RundownItemEventArgs e)
        {
            Rundown.Add(CreateRundownItemViewModel(e.RundownItem));
            Refresh();
        }

        private void MediaPlayer_Removed(object sender, Model.Args.RundownItemEventArgs e)
        {
            var vm = Rundown.FirstOrDefault(i => i.RundownItem == e.RundownItem);
            Debug.Assert(vm != null);
            Rundown.Remove(vm);
        }

        private void Player_AudioVolume(object sender, Model.Args.AudioVolumeEventArgs e)
        {
            if (e.AudioVolume.Length == 0)
            {
                foreach (var bar in AudioLevelBars)
                    bar.AudioLevel = AudioLevelBarViewModel.MinValue;
                return;
            }
            Debug.Assert(e.AudioVolume.Length == AudioLevelBars.Length);
            for (int i = 0; i < e.AudioVolume.Length; i++)
                AudioLevelBars[i].AudioLevel = Math.Max(AudioLevelBarViewModel.MinValue, (20 * Math.Log10(e.AudioVolume[i])) + AudioHeadroom);
        }

        private bool CanTogglePlay(object obj)
        {
            var item = _mediaPlayer.PlayingRundownItem as FileRundownItem;
            if (item == null)
                return false;
            if (!IsPlaying && _mediaPlayer.IsEof)
                return false;
            return true;
        }

        public void Dispose()
        {
            if (_isDisposed)
                return;
            _isDisposed = true;
            _mediaPlayer.Loaded -= MediaPlayer_Loaded;
            _mediaPlayer.FramePlayed -= MediaPlayer_Progress;
            _mediaPlayer.Stopped -= MediaPlayer_Stopped;
            _mediaPlayer.MediaSubmitted -= MediaPlayer_MediaSubmitted;
            _mediaPlayer.Removed -= MediaPlayer_Removed;
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
                    var fileRundownItem = _mediaPlayer.AddMediaToQueue(mediaViewModel.Media, index);
                    Rundown.Insert(index, new FileRundownItemViewModel(fileRundownItem));
                    Refresh();
                    break;

                case DecklinkInputViewModel decklink:
                    index = dropInfo.TargetCollection is null ? Rundown.Count : dropInfo.InsertIndex;
                    var liveInputRundownItem = _mediaPlayer.AddLiveToQueue(decklink.Input, index);
                    Rundown.Insert(index, new LiveInputRundownItemViewModel(liveInputRundownItem));
                    Refresh();


                    break;
                case LiveInputRundownItemViewModel _:
                case FileRundownItemViewModel _:
                    var srcIndex = dropInfo.DragInfo.SourceIndex;
                    var destIndex = dropInfo.InsertIndex;
                    if (destIndex > srcIndex)
                        destIndex--;
                    _mediaPlayer.MoveItem(srcIndex, destIndex);
                    Rundown.Move(srcIndex, destIndex);
                    Refresh();
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
                    fileRundownItem = _mediaPlayer.AddMediaToQueue(media, index);
                    Rundown.Insert(index, new FileRundownItemViewModel(fileRundownItem));
                    Refresh();
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

    }
}
