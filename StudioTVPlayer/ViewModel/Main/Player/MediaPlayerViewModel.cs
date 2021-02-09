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
        private RundownItem _currentRundownItem;

        public MediaPlayerViewModel(MediaPlayer player)
        {
            LoadMediaCommand = new UiCommand(LoadMedia);
            LoadSelectedMediaCommand = new UiCommand(LoadSelectedMedia);
            CheckItemCommand = new UiCommand(param => CheckItem(param));
            TogglePlayCommand = new UiCommand(TogglePlay);
            SliderDragStartCommand = new UiCommand(param => SldierDragStart(param));
            UnloadCommand = new UiCommand(Unload);
            NextCommand = new UiCommand(NextMedia);
            DeleteDisabledCommand = new UiCommand(DeleteDisabled, CanDeleteDisabled);
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

        public RundownItem CurrentRundownItem { get => _currentRundownItem; private set => Set(ref _currentRundownItem, value); }

        public UiCommand LoadMediaCommand { get; }
        public UiCommand LoadSelectedMediaCommand { get; }
        public UiCommand CheckItemCommand { get; }
        public UiCommand SliderDragStartCommand { get; }
        public UiCommand DisplayTimecodeEditCommand { get; }
        public UiCommand DeleteDisabledCommand { get; }
        public UiCommand SeekFramesCommand { get; }

        public UiCommand TogglePlayCommand { get; }
        public UiCommand UnloadCommand { get; }
        public UiCommand NextCommand { get; }


        private void InputFileStopped(object sender, EventArgs e)
        {
            IsPlaying = false;
        }


        private void SeekFrames(object param)
        {
            if (param == null)
                return;

            if (IsPlaying)
                Pause();

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
            Pause();
        }

        private void LoadSelectedMedia(object obj)
        {
        }

        private void DeleteDisabled(object obj)
        {
            //var mediaToDelete = MediaQueue.Where(param => param.IsDisabled == true);

            //foreach(PlayerQueueItemViewModel playerItem in mediaToDelete.ToList())
            //{
            //    MediaQueue.Remove(playerItem);
            //}
        }

        private bool CanDeleteDisabled(object obj)
        {
            //if (MediaQueue.Count == 0)
            //    return false;
            //if (MediaQueue.Select(param => param.IsDisabled == true).Count() > 0)
            //    return true;
            return false;
        }

        private void SldierDragStart(object _)
        {
            //if (_inputFile == null)
            //    return;
            Pause();
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
            //if (_inputFile == null)
            //    return;

            //if (usingSeekbar)
            //{
            //    DisplayTime = TimeSpan.FromMilliseconds(_seekbar);
            //    //OutTime = _playerItem.BrowserItem.Media.Duration - DisplayTime;               
            //    Seek(TimeSpan.FromMilliseconds(_seekbar));
            //}
            //else
            //{
            //    //OutTime = _playerItem.BrowserItem.Media.Duration - DisplayTime;
            //    _inputFile.Seek(DisplayTime);
            //}         
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
            //

            //if (_playerItem != null)
            //    Stop();     

            //IsPlaying = false;
            //_inputFile = new TVPlayR.InputFile(playerItem.BrowserItem.Media.DirectoryName, 2);           
            //_inputFile.FramePlayed += Media_FramePlayed;
            //_inputFile.Stopped += InputFileStopped;

            //PlayerItem = playerItem;
            //PlayerItem.BrowserItem.Media.PropertyChanged += MediaChanged;

            //Channel.Load(_inputFile);
            //PlayerItem.IsLoaded = true;
        }

        private void Media_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            DisplayTime = e.Time;
            //OutTime = _playerItem.BrowserItem.Media.Duration - DisplayTime;

            if (IsPlaying)
            {
                _seekbar = e.Time.TotalMilliseconds;
                NotifyPropertyChanged(nameof(Seekbar));
            }

        }

        private void NextMedia(object obj)
        {
            //var currentIndex = MediaQueue.IndexOf(_playerItem);

            //if (currentIndex >= MediaQueue.Count - 1)
            //    return;         

            //while (true)
            //{              
            //    if (MediaQueue[++currentIndex].IsDisabled == true)
            //    {                
            //        if (currentIndex >= MediaQueue.Count)
            //            return;
            //        continue;
            //    }
            //    LoadMedia(MediaQueue[currentIndex]);             
            //    SelectedIndex = currentIndex;
            //    break;
            //}
        }

        private void Unload(object obj = null)
        {
            //if (_inputFile == null)
            //    return;

            Channel.Clear();
            Stop();
        }

        private async void TogglePlay(object obj)
        {
            //if (_inputFile == null)
            //    return;

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
                await _dialogCoordinator.ShowMessageAsync(MainViewModel.Instance, "Error", $"Error occured when playing clip {_mediaPlayer.PlayingQueueItem?.Media.Name}", MahApps.Metro.Controls.Dialogs.MessageDialogStyle.Affirmative);
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
                await _dialogCoordinator.ShowMessageAsync(MainViewModel.Instance, "Error", $"Error occured pausing clip {_mediaPlayer.PlayingQueueItem?.Media.Name}", MahApps.Metro.Controls.Dialogs.MessageDialogStyle.Affirmative);
                return false;
            }
            IsPlaying = false;
            return true;
        }

        public bool Seek(TimeSpan timeSpan)
        {
            //if (_inputFile == null)
            //    return false;
            //try
            //{
            //    _inputFile.Seek(timeSpan);
            //}
            //catch
            //{
            //    MessageBox.Show("Błąd seekowania");
            //    return false;
            //}
            return true;
        }

        public bool Stop()
        {
            //if (_inputFile == null)
            //    return false;
            //try
            //{
            //    _inputFile.FramePlayed -= Media_FramePlayed;
            //    _inputFile.Stopped -= InputFileStopped;
            //    _inputFile.Dispose();
            //    _inputFile = null;

            //    IsPlaying = false;
            //    SelectedRundownItem.IsLoaded = false;
            //    //PlayerItem.BrowserItem.Media.PropertyChanged -= MediaChanged;
            //    SelectedRundownItem = null;
            //    Seekbar = 0;
            //    DisplayTime = TimeSpan.Zero;
            //    OutTime = TimeSpan.Zero;
            //}
            //catch
            //{
            //    MessageBox.Show("Błąd Dispose");
            //    return false;
            //}
            return true;
        }

        private void MediaPlayer_Progress(object sender, Model.Args.TimeEventArgs e)
        {
            DisplayTime = e.Time;
            OutTime = CurrentRundownItem?.Media.Duration - e.Time ?? TimeSpan.Zero;
        }

        private void MediaPlayer_Loaded(object sender, Model.Args.RundownItemEventArgs e)
        {
            CurrentRundownItem = e.RundownItem;
        }

        private void MediaPlayer_MediaSubmitted(object sender, Model.Args.RundownItemEventArgs e)
        {
            Rundown.Add(new RundownItemViewModel(e.RundownItem));
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
                dropInfo.DropTargetAdorner = DropTargetAdorners.Insert;
                dropInfo.Effects = DragDropEffects.Move;
                var rundownItem = _mediaPlayer.AddToQueue(mediaViewModel.Media, 0);
                Rundown.Add(new RundownItemViewModel(rundownItem));
            }
        }
    }
}
