using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using GongSolutions.Wpf.DragDrop;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.ViewModel.Main.MediaBrowser;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class MediaPlayerViewModel : ViewModelBase, IDisposable, IDropTarget
    {
        private readonly MediaPlayer _mediaPlayer;
        private readonly MahApps.Metro.Controls.Dialogs.IDialogCoordinator _dialogCoordinator = MahApps.Metro.Controls.Dialogs.DialogCoordinator.Instance;


        private bool _isPlaying;
        private bool _isFocused;
        private TimeSpan _displayTime;
        private TimeSpan _outTime;
        private double _sliderPosition;
        private bool _isDisposed;
        private RundownItemViewModel _currentRundownItem;
        private bool _isLoaded;

        private bool _isSliderDrag;
        private double _volume;

        public MediaPlayerViewModel(MediaPlayer player)
        {
            LoadMediaCommand = new UiCommand(LoadMedia);
            LoadSelectedMediaCommand = new UiCommand(LoadSelectedMedia, _ => SelectedRundownItem != null);
            CueCommand = new UiCommand(Cue, _ => CurrentRundownItem != null);
            CheckItemCommand = new UiCommand(param => CheckItem(param));
            TogglePlayCommand = new UiCommand(TogglePlay, CanTogglePlay);
            UnloadCommand = new UiCommand(Unload, _ => CurrentRundownItem != null);
            LoadNextItemCommand = new UiCommand(LoadNextItem, CanLoadNextItem);
            DeleteDisabledCommand = new UiCommand(DeleteDisabled, _ => Rundown.Any(i => i.IsDisabled));
            DisplayTimecodeEditCommand = new UiCommand(_ => Seek(DisplayTime));
            SeekFramesCommand = new UiCommand(param => SeekFrames(param));
            Name = player.Channel.Name;
            VideoFormat = player.Channel.VideoFormat;

            _mediaPlayer = player;
            _mediaPlayer.Loaded += MediaPlayer_Loaded;
            _mediaPlayer.FramePlayed += MediaPlayer_Progress;
            _mediaPlayer.Stopped += MediaPlayer_Stopped;
            _mediaPlayer.MediaSubmitted += MediaPlayer_MediaSubmitted;
            Rundown = new ObservableCollection<RundownItemViewModel>(player.Rundown.Select(ri => new RundownItemViewModel(ri)));
        }

        private bool CanTogglePlay(object obj)
        {
            var item = _mediaPlayer.PlayingRundownItem;
            if (item == null)
                return false;
            if (!IsPlaying && _mediaPlayer.IsEof)
                return false;
            return true;
        }

        public string Name { get; }

        public TVPlayR.VideoFormat VideoFormat { get; }

        public bool IsPlaying
        {
            get => _isPlaying;
            set => Set(ref _isPlaying, value);
        }

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

        public TimeSpan CurrentItemStartTime => CurrentRundownItem?.RundownItem.Media.StartTime ?? TimeSpan.Zero;

        public TimeSpan CurrentItemDuration => CurrentRundownItem?.RundownItem.Media.Duration ?? TimeSpan.Zero;


        public TimeSpan OutTime
        {
            get => _outTime;
            set => Set(ref _outTime, value);
        }

        public double SliderPosition
        {
            get => _sliderPosition;
            set
            {
                if (!Set(ref _sliderPosition, value))
                    return;
                if (!_isSliderDrag)
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

        public bool IsLoaded { get => _isLoaded; private set => Set(ref _isLoaded, value); }

        public ObservableCollection<RundownItemViewModel> Rundown { get; }

        public RundownItemViewModel SelectedRundownItem { get; set; }

        public RundownItemViewModel CurrentRundownItem
        {
            get => _currentRundownItem;
            private set
            {
                var oldItem = _currentRundownItem;
                if (!Set(ref _currentRundownItem, value))
                    return;
                if (value != null)
                    value.IsLoaded = true;
                if (oldItem != null)
                {
                    oldItem.IsLoaded = false;
                    oldItem.IsDisabled = true;
                }
                IsLoaded = value != null;
                SliderPosition = CurrentItemStartTime.TotalMilliseconds;
                NotifyPropertyChanged(nameof(CurrentItemStartTime));
                NotifyPropertyChanged(nameof(CurrentItemDuration));
            }
        }

        public ICommand LoadMediaCommand { get; }
        public ICommand LoadSelectedMediaCommand { get; }
        public ICommand CueCommand { get; }
        public ICommand CheckItemCommand { get; }
        public ICommand DisplayTimecodeEditCommand { get; }
        public ICommand DeleteDisabledCommand { get; }
        public ICommand SeekFramesCommand { get; }

        public ICommand TogglePlayCommand { get; }
        public ICommand UnloadCommand { get; }
        public ICommand LoadNextItemCommand { get; }


        private void InputFileStopped(object sender, EventArgs e)
        {
            IsPlaying = false;
        }

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
            SliderPosition = time.TotalMilliseconds;
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
            RundownItemViewModel item = null;
            while (true)
            {
                item = Rundown.FirstOrDefault(i => i.IsDisabled);
                if (item is null)
                    break;
                if (_mediaPlayer.RemoveItem(item.RundownItem))
                    Rundown.Remove(item);
            }
        }

        private void CheckItem(object param)
        {
            if (param == null)
                return;
            object[] parameters = param as object[];
            FrameworkElement element = (FrameworkElement)parameters[0];
            RundownItemViewModel m = (RundownItemViewModel)element.DataContext;
            m.IsDisabled = !m.IsDisabled;
        }

        public void EndSliderThumbDrag()
        {
            Seek(TimeSpan.FromMilliseconds(SliderPosition));
            _isSliderDrag = false;
        }

        public void BeginSliderThumbDrag()
        {
            _isSliderDrag = true;
        }

        private void LoadMedia(object param)
        {
            LoadMedia(((param as object[])?[0] as FrameworkElement)?.DataContext as RundownItemViewModel ?? throw new ArgumentException(nameof(param)));
        }

        private void LoadMedia(RundownItemViewModel playerItem)
        {
            if (playerItem.IsDisabled || !playerItem.RundownItem.Media.IsVerified)
                return;
            _mediaPlayer.Load(playerItem.RundownItem);
        }

        private void LoadNextItem(object obj)
        {
            var currentIndex = Rundown.IndexOf(CurrentRundownItem);
            if (currentIndex >= Rundown.Count - 1)
                return;
            while (++currentIndex < Rundown.Count)
            {
                if (Rundown[currentIndex].IsDisabled)
                    continue;
                LoadMedia(Rundown[currentIndex]);
                return;
            }
        }


        private bool CanLoadNextItem(object obj)
        {
            var currentIndex = Rundown.IndexOf(CurrentRundownItem);
            while (++currentIndex < Rundown.Count)
            {
                if (Rundown[currentIndex].IsDisabled)
                    continue;
                return true;
            }
            return false;
        }

        private void Unload(object obj = null)
        {
            CurrentRundownItem = null;
            _mediaPlayer.Clear();
        }

        private async void TogglePlay(object obj)
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
                if (_mediaPlayer.Play())
                    IsPlaying = true;
            }
            catch
            {
                await _dialogCoordinator.ShowMessageAsync(MainViewModel.Instance, "Error", $"Error starting clip {_mediaPlayer.PlayingRundownItem?.Media.Name}", MahApps.Metro.Controls.Dialogs.MessageDialogStyle.Affirmative);
                return false;
            }
            return true;
        }

        public async Task<bool> Pause()
        {
            try
            {
                _mediaPlayer.Pause();
                IsPlaying = false;
            }
            catch
            {
                await _dialogCoordinator.ShowMessageAsync(MainViewModel.Instance, "Error", $"Error pausing clip {_mediaPlayer.PlayingRundownItem?.Media.Name}", MahApps.Metro.Controls.Dialogs.MessageDialogStyle.Affirmative);
                return false;
            }
            return true;
        }

        private void MediaPlayer_Progress(object sender, Model.Args.TimeEventArgs e)
        {
            DisplayTime = e.Time;
            OutTime = CurrentItemDuration - e.Time - _mediaPlayer.OneFrame;
            if (!_isSliderDrag && IsPlaying)
            {
                _sliderPosition = e.Time.TotalMilliseconds;
                NotifyPropertyChanged(nameof(SliderPosition));
            }
        }

        private void MediaPlayer_Stopped(object sender, EventArgs e)
        {
            IsPlaying = false;
            Refresh();
        }


        private void MediaPlayer_Loaded(object sender, Model.Args.RundownItemEventArgs e)
        {
            CurrentRundownItem = Rundown.FirstOrDefault(i => i.RundownItem == e.RundownItem);
        }

        private void MediaPlayer_MediaSubmitted(object sender, Model.Args.RundownItemEventArgs e)
        {
            Rundown.Add(new RundownItemViewModel(e.RundownItem));
            Refresh();
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
        }

        public void DragOver(IDropInfo dropInfo)
        {
            if (dropInfo.Data is MediaViewModel mediaViewModel)
            {
                if (!mediaViewModel.IsVerified || mediaViewModel.Duration <= TimeSpan.Zero)
                    return;
            }
            else if (dropInfo.Data is RundownItemViewModel)
            {
                if (dropInfo.InsertIndex == dropInfo.DragInfo.SourceIndex || dropInfo.InsertIndex == dropInfo.DragInfo.SourceIndex + 1)
                    return;
                if (dropInfo.DragInfo.SourceCollection != Rundown)
                    return;
                if (dropInfo.InsertPosition == RelativeInsertPosition.None)
                    return;
            }
            else if (dropInfo.Data is IDataObject dataObject)
            {
                if ((dataObject.GetData(DataFormats.FileDrop) as string[])?.FirstOrDefault(f => File.Exists(f)) is null)
                    return;
            }
            else
                return;
            dropInfo.DropTargetAdorner = DropTargetAdorners.Insert;
            dropInfo.Effects = DragDropEffects.Move;
        }

        public void Drop(IDropInfo dropInfo)
        {
            if (dropInfo.Data is MediaViewModel mediaViewModel)
            {
                var index = dropInfo.TargetCollection is null ? Rundown.Count : dropInfo.InsertIndex;
                var rundownItem = _mediaPlayer.AddToQueue(mediaViewModel.Media, index);
                Rundown.Insert(index, new RundownItemViewModel(rundownItem));
                Refresh();
            }
            else if (dropInfo.Data is RundownItemViewModel)
            {
                var srcIndex = dropInfo.DragInfo.SourceIndex;
                var destIndex = dropInfo.InsertIndex;
                if (destIndex > srcIndex)
                    destIndex--;
                _mediaPlayer.MoveItem(srcIndex, destIndex);
                Rundown.Move(srcIndex, destIndex);
                Refresh();
            }
            else if (dropInfo.Data is IDataObject dataObject)
            {
                var fileName = (dataObject.GetData(DataFormats.FileDrop) as string[])?.FirstOrDefault(f => File.Exists(f));
                if (fileName is null)
                    return;
                var media = new Media(fileName);
                if (!MediaVerifier.Current.Verify(media))
                    return;
                var index = dropInfo.TargetCollection is null ? Rundown.Count : dropInfo.InsertIndex;
                var rundownItem = _mediaPlayer.AddToQueue(media, index);
                Rundown.Insert(index, new RundownItemViewModel(rundownItem));
                Refresh();
            }
        }
    }
}
