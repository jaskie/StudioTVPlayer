using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.ViewModel.Main.Browser;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class PlayerViewModel : ViewModelBase, IDisposable
    {
        private TVPlayR.InputFile _inputFile;

        private bool _isPlaying;
        public bool IsPlaying
        {
            get => _isPlaying;
            set => Set(ref _isPlaying, value);
        }

        private bool _isFocused;
        public bool IsFocused
        {
            get => _isFocused;
            set => Set(ref _isFocused, value);
        }

        private TimeSpan _displayTime;
        public TimeSpan DisplayTime
        {
            get => _displayTime;
            set => Set(ref _displayTime, value);
        }

        private TimeSpan _outTime;
        public TimeSpan OutTime
        {
            get => _outTime;
            set => Set(ref _outTime, value);
        }

        public Channel Channel { get; set; }

        private int _selectedIndex;
        public int SelectedIndex 
        {
            get => _selectedIndex;
            set => Set(ref _selectedIndex, value);
        }

        private double _seekbar;
        public double Seekbar
        {
            get => _seekbar;
            set
            {
                Set(ref _seekbar, value);
                SeekMedia();
            }
        }

        private PlayerQueueItemViewModel _playerItem;
        public PlayerQueueItemViewModel PlayerItem
        {
            get => _playerItem;
            set => Set(ref _playerItem, value);
        }

        private void MediaChanged(object sender, PropertyChangedEventArgs e)
        {
            switch (e.PropertyName)
            {
                case nameof(Media.Duration):
                    OutTime = _playerItem.BrowserItem.Media.Duration - DisplayTime;
                    break;              
            }
        }

        private ObservableCollection<PlayerQueueItemViewModel> _mediaQueue;
        public ObservableCollection<PlayerQueueItemViewModel> MediaQueue
        {
            get => _mediaQueue;
            set => Set(ref _mediaQueue, value);
        }

        public UiCommand DropReceiveCommand { get; }
        public UiCommand LoadMediaCommand { get; }
        public UiCommand LoadSelectedMediaCommand { get; }
        public UiCommand CheckItemCommand { get; }
        public UiCommand SliderDragStartCommand { get; }
        public UiCommand DisplayTimecodeEditCommand { get; }
        public UiCommand DeleteDisabledCommand { get; }
        public UiCommand SeekFramesCommand { get; }

        public UiCommand PlayPauseCommand { get; }
        public UiCommand StopCommand { get; }
        public UiCommand NextCommand { get; }

        public UiCommand PlayerQueueItem_MoveCommand { get; }


        public PlayerViewModel()
        {
            Application.Current.MainWindow.Closing += MainWindow_Closing;
            MediaQueue = new ObservableCollection<PlayerQueueItemViewModel>();
            MediaQueue.CollectionChanged += MediaQueue_CollectionChanged;

            DropReceiveCommand = new UiCommand(param => DropReceive(param));
            LoadMediaCommand = new UiCommand(param => PreliminaryLoadMedia(param));
            LoadSelectedMediaCommand = new UiCommand(LoadSelectedMedia);
            CheckItemCommand = new UiCommand(param => CheckItem(param));
            PlayPauseCommand = new UiCommand(PlayPauseMedia);
            SliderDragStartCommand = new UiCommand(param => SldierDragStart(param));
            StopCommand = new UiCommand(StopMedia);
            NextCommand = new UiCommand(NextMedia);
            DeleteDisabledCommand = new UiCommand(DeleteDisabled, CanDeleteDisabled);
            PlayerQueueItem_MoveCommand = new UiCommand(param => PlayerQueueItem_Move(param));
            DisplayTimecodeEditCommand = new UiCommand(param => SeekMedia(false));
            SeekFramesCommand = new UiCommand(param => SeekFrames(param));
        }

        private void MediaQueue_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            if(e.Action == System.Collections.Specialized.NotifyCollectionChangedAction.Add)
            {
                List<PlayerQueueItemViewModel> items = e.NewItems.Cast<PlayerQueueItemViewModel>().ToList();
                foreach(var item in items)
                {
                    item.BrowserItem.IsQueued = true;                  
                }
            }
            else if (e.Action == System.Collections.Specialized.NotifyCollectionChangedAction.Remove)
            {
                List<PlayerQueueItemViewModel> items = e.OldItems.Cast<PlayerQueueItemViewModel>().ToList();
                foreach (var item in items)
                {
                    item.BrowserItem.IsQueued = false;
                }
            }
                
        }       

        private void MainWindow_Closing(object sender, CancelEventArgs e)
        {
            Dispose();
        }

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
            switch(param.ToString())
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
                        frameNumber -= Channel.VideoFormat.FrameRate.Numerator/Channel.VideoFormat.FrameRate.Denominator;
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
            if (SelectedIndex < _mediaQueue.Count && SelectedIndex > -1)
                LoadMedia(_mediaQueue[SelectedIndex]);
        }

        private void DeleteDisabled(object obj)
        {
            var mediaToDelete = MediaQueue.Where(param => param.IsDisabled == true);

            foreach(PlayerQueueItemViewModel playerItem in mediaToDelete.ToList())
            {
                MediaQueue.Remove(playerItem);
            }
        }

        private bool CanDeleteDisabled(object obj)
        {
            if (MediaQueue.Count == 0)
                return false;
            if (MediaQueue.Select(param => param.IsDisabled == true).Count() > 0)
                return true;
            return false;
        }

        private void SldierDragStart(object _)
        {
            if (_inputFile == null)
                return;
            Pause();
        }

        private void CheckItem(object param)
        {
            if (param == null)
                return;

            object[] parameters = param as object[];
            FrameworkElement element = (FrameworkElement)parameters[0];
            PlayerQueueItemViewModel m = (PlayerQueueItemViewModel)element.DataContext;
            m.IsDisabled = !m.IsDisabled;
        }

        private void SeekMedia(bool usingSeekbar = true)
        {
            if (_inputFile == null)
                return;
                   
            if (usingSeekbar)
            {
                DisplayTime = TimeSpan.FromMilliseconds(_seekbar);
                OutTime = _playerItem.BrowserItem.Media.Duration - DisplayTime;               
                Seek(TimeSpan.FromMilliseconds(_seekbar));
            }
            else
            {
                OutTime = _playerItem.BrowserItem.Media.Duration - DisplayTime;
                _inputFile.Seek(DisplayTime);
            }         
        }

        private void PreliminaryLoadMedia(object param)
        {
            if (param == null)
                return;

            object[] parameters = param as object[];
            FrameworkElement element = (FrameworkElement)parameters[0];
           
            LoadMedia((PlayerQueueItemViewModel)(element.DataContext));
        }

        private void LoadMedia(PlayerQueueItemViewModel playerItem)
        {                      
            if (playerItem.IsDisabled || !playerItem.BrowserItem.IsVerified)
                return;

            if (_playerItem != null)
                Stop();     

            IsPlaying = false;
            _inputFile = new TVPlayR.InputFile(playerItem.BrowserItem.Media.Path);           
            _inputFile.FramePlayed += Media_FramePlayed;
            _inputFile.Stopped += InputFileStopped;

            PlayerItem = playerItem;
            PlayerItem.BrowserItem.Media.PropertyChanged += MediaChanged;

            Channel.Load(_inputFile);
            PlayerItem.IsLoaded = true;
        }

        private void Media_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {           
            DisplayTime = e.Time;
            OutTime = _playerItem.BrowserItem.Media.Duration - DisplayTime;

            if (IsPlaying)
            {
                _seekbar = e.Time.TotalMilliseconds;
                NotifyPropertyChanged(nameof(Seekbar));
            }
                
        }

        private void NextMedia(object obj)
        {          
            var currentIndex = MediaQueue.IndexOf(_playerItem);

            if (currentIndex >= MediaQueue.Count - 1)
                return;         

            while (true)
            {              
                if (MediaQueue[++currentIndex].IsDisabled == true)
                {                
                    if (currentIndex >= MediaQueue.Count)
                        return;
                    continue;
                }
                LoadMedia(MediaQueue[currentIndex]);             
                SelectedIndex = currentIndex;
                break;
            }
        }

        private void StopMedia(object obj = null)
        {          
            if (_inputFile == null)
                return;
           
            Channel.Clear();
            Stop();                     
        }

        private void PlayPauseMedia(object obj)
        {
            if (_inputFile == null)
                return;

            if (IsPlaying)           
                Pause();          
            else           
                Play();           
        }
     

        public bool Play()
        {
            if (_inputFile == null)
                return false;
            try
            {
                _inputFile.Play();
                IsPlaying = true;
            }
            catch
            {
                MessageBox.Show("Błąd Play");
                return false;
            }
            return true;
        }

        public bool Pause()
        {
            if (_inputFile == null)
                return false;
            try
            {
                _inputFile.Pause();
                IsPlaying = false;
            }
            catch
            {
                MessageBox.Show("Błąd Pause");
                return false;
            }
            return true;
        }

        public bool Seek(TimeSpan timeSpan)
        {
            if (_inputFile == null)
                return false;
            try
            {
                _inputFile.Seek(timeSpan);
            }
            catch
            {
                MessageBox.Show("Błąd seekowania");
                return false;
            }
            return true;
        }

        public bool Stop()
        {
            if (_inputFile == null)
                return false;
            try
            {
                _inputFile.FramePlayed -= Media_FramePlayed;
                _inputFile.Stopped -= InputFileStopped;
                _inputFile.Dispose();
                _inputFile = null;

                IsPlaying = false;
                PlayerItem.IsLoaded = false;
                PlayerItem.BrowserItem.Media.PropertyChanged -= MediaChanged;
                PlayerItem = null;
                Seekbar = 0;
                DisplayTime = TimeSpan.Zero;
                OutTime = TimeSpan.Zero;
            }
            catch
            {
                MessageBox.Show("Błąd Dispose");
                return false;
            }
            return true;
        }

        public void PlayerQueueItem_Move(object param)
        {
            if (param == null)
                return;

            object[] parameters = param as object[];
            FrameworkElement sender = (FrameworkElement)parameters[0];
            MouseEventArgs e = (MouseEventArgs)parameters[1];

            if (e.LeftButton == MouseButtonState.Pressed)
                DragDrop.DoDragDrop(sender, sender.DataContext, DragDropEffects.Move);

        }

        private void DropReceive(object obj)
        {
            if (obj == null)
                return;

            object[] parameters = obj as object[];
            FrameworkElement sender = (FrameworkElement)parameters[0];
            DragEventArgs e = (DragEventArgs)parameters[1];

            if (e.Data == null)
                return;

            var browserVM = (BrowserTabItemViewModel)e.Data.GetData(typeof(BrowserTabItemViewModel));
            var newIndex = MediaQueue.IndexOf(MediaQueue.FirstOrDefault(playerItemParam => playerItemParam.DragOver == true));

            if (browserVM == null && newIndex>-1) //browserItem null czyli to jest playerItem
            {                                                
                var playerItemQueueVM = (PlayerQueueItemViewModel)e.Data.GetData(typeof(PlayerQueueItemViewModel));
                var oldIndex = MediaQueue.IndexOf(playerItemQueueVM);
                newIndex = newIndex > oldIndex ? --newIndex : newIndex;

                if (oldIndex>-1)
                    MediaQueue.Move(oldIndex, newIndex < 0 ? 0 : newIndex);   
                else
                    MediaQueue.Add(new PlayerQueueItemViewModel(browserVM));
                return;
            }

            if (browserVM == null)
                return;

            if (newIndex>-1)
                MediaQueue.Insert(newIndex, new PlayerQueueItemViewModel(browserVM));
            else
                MediaQueue.Add(new PlayerQueueItemViewModel(browserVM));
        }      

        public void Dispose()
        {
            Stop();
            Channel?.Dispose();
        }
    }
}
