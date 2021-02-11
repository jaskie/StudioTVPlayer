using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
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
        private double _seekbar;
        private bool _isDisposed;
        private RundownItemViewModel _currentRundownItem;

        public MediaPlayerViewModel(MediaPlayer player)
        {
            LoadMediaCommand = new UiCommand(LoadMedia);
            LoadSelectedMediaCommand = new UiCommand(LoadSelectedMedia, _ => SelectedRundownItem != null);
            CueCommand = new UiCommand(Cue, _ => CurrentRundownItem != null);
            CheckItemCommand = new UiCommand(param => CheckItem(param));
            TogglePlayCommand = new UiCommand(TogglePlay, _ => CurrentRundownItem != null);
            SliderDragStartCommand = new UiCommand(param => SldierDragStart(param));
            UnloadCommand = new UiCommand(Unload, _ => CurrentRundownItem != null);
            LoadNextItemCommand = new UiCommand(LoadNextItem, CanLoadNextItem);
            DeleteDisabledCommand = new UiCommand(DeleteDisabled, _ => Rundown.Any(i => i.IsDisabled));
            DisplayTimecodeEditCommand = new UiCommand(param => SeekMedia(false));
            SeekFramesCommand = new UiCommand(param => SeekFrames(param));

            Channel = player.Channel;
            _mediaPlayer = player;
            _mediaPlayer.Loaded += MediaPlayer_Loaded;
            _mediaPlayer.Progress += MediaPlayer_Progress;
            _mediaPlayer.MediaSubmitted += MediaPlayer_MediaSubmitted;
            Rundown = new ObservableCollection<RundownItemViewModel>(player.Rundown.Select(ri => new RundownItemViewModel(ri)));
        }

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

        public TimeSpan OutTime
        {
            get => _outTime;
            set => Set(ref _outTime, value);
        }

        public Channel Channel { get; }

        public double Seekbar
        {
            get => _seekbar;
            set
            {
                Set(ref _seekbar, value);
                SeekMedia();
            }
        }

        public IList<RundownItemViewModel> Rundown { get; }

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
            }
        }

        public ICommand LoadMediaCommand { get; }
        public ICommand LoadSelectedMediaCommand { get; }
        public ICommand CueCommand { get; }
        public ICommand CheckItemCommand { get; }
        public ICommand SliderDragStartCommand { get; }
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
            var frameNumber = Channel.VideoFormat.TimeToFrameNumber(_displayTime);
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
                        frameNumber -= Channel.VideoFormat.FrameRate.Numerator / Channel.VideoFormat.FrameRate.Denominator;
                        break;
                    }

                case "second":
                    {
                        frameNumber += Channel.VideoFormat.FrameRate.Numerator / Channel.VideoFormat.FrameRate.Denominator;
                        break;
                    }
            }

            Seek(Channel.VideoFormat.FrameNumberToTime(frameNumber));
            _seekbar = Channel.VideoFormat.FrameNumberToTime(frameNumber).TotalMilliseconds;
            NotifyPropertyChanged(nameof(Seekbar));
            await Pause();
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
            Seek(TimeSpan.Zero);
        }

        private void DeleteDisabled(object obj)
        {
            RundownItemViewModel item = null;
            do
            {
                item = Rundown.FirstOrDefault(i => i.IsDisabled);
                Rundown.Remove(item);
            } while (item != null);
        }

        private async void SldierDragStart(object _)
        {
            //if (_inputFile == null)
            //    return;
            await Pause();
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

        private void SeekMedia(bool usingSeekbar = true)
        {            
            if (usingSeekbar)
            {
                Seek(TimeSpan.FromMilliseconds(_seekbar));
            }
            else
            {
                _mediaPlayer.Seek(DisplayTime);
            }         
        }

        private void LoadMedia(object param)
        {
            LoadMedia(((param as object[])?[0] as FrameworkElement)?.DataContext as RundownItemViewModel ?? throw new ArgumentException(nameof(param)));
        }

        private void LoadMedia(RundownItemViewModel playerItem)
        {
            if (playerItem.IsDisabled || !playerItem.RundownItem.Media.IsVerified)
                return;
            if (!_mediaPlayer.Load(playerItem.RundownItem))
                return;
            CurrentRundownItem = playerItem;
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
            Channel.Clear();
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
                await _dialogCoordinator.ShowMessageAsync(MainViewModel.Instance, "Error", $"Error starting clip {_mediaPlayer.PlayingQueueItem?.Media.Name}", MahApps.Metro.Controls.Dialogs.MessageDialogStyle.Affirmative);
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
                await _dialogCoordinator.ShowMessageAsync(MainViewModel.Instance, "Error", $"Error pausing clip {_mediaPlayer.PlayingQueueItem?.Media.Name}", MahApps.Metro.Controls.Dialogs.MessageDialogStyle.Affirmative);
                return false;
            }
            return true;
        }

        public bool Seek(TimeSpan timeSpan)
        {
            if (_mediaPlayer.PlayingQueueItem == null)
                return false;
            return _mediaPlayer.Seek(timeSpan);
        }


        private void MediaPlayer_Progress(object sender, Model.Args.TimeEventArgs e)
        {
            DisplayTime = e.Time;
            OutTime = CurrentRundownItem?.RundownItem.Media.Duration - e.Time ?? TimeSpan.Zero;
            _seekbar = e.Time.TotalMilliseconds;
            NotifyPropertyChanged(nameof(Seekbar));
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
            _mediaPlayer.Progress -= MediaPlayer_Progress;
            _mediaPlayer.MediaSubmitted -= MediaPlayer_MediaSubmitted;
        }

        public void DragOver(IDropInfo dropInfo)
        {
            if (dropInfo.Data is MediaViewModel mediaViewModel && mediaViewModel.IsVerified)
            {
                dropInfo.DropTargetAdorner = DropTargetAdorners.Insert;
                dropInfo.Effects = DragDropEffects.Move;
            }
        }

        public void Drop(IDropInfo dropInfo)
        {
            if (dropInfo.Data is MediaViewModel mediaViewModel)
            {
                var rundownItem = _mediaPlayer.AddToQueue(mediaViewModel.Media, 0);
                Rundown.Add(new RundownItemViewModel(rundownItem));
                Refresh();
            }
        }
    }
}
