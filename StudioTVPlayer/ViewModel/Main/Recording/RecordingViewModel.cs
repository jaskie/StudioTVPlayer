using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows.Input;
using System.Windows.Media;

namespace StudioTVPlayer.ViewModel.Main.Recording
{
    public sealed class RecordingViewModel : RemovableViewModelBase, IDataErrorInfo, IDisposable
    {
        private Model.Recording _recording;
        private string _fileName;
        private RecordingStep _step;
        private Model.EncoderPreset _encoderPreset;
        private bool _disposed;
        private ImageSource _thumbnail;
        private DateTime _startTime;
        private TimeSpan _duration;
        private Model.RecordingState _state;
        private bool _isQueuedToVerify;
        private readonly Model.InputBase _input;
        private Action _verifyOnceAction; 

        public RecordingViewModel(Model.InputBase input) : this(input, null, null)
        {
            _fileName = $"Recording_{DateTime.Now:yyyyMMdd_HHmmss}";
        }

        public RecordingViewModel(Model.Recording recording) : this(recording.Input, recording.EncoderPreset, Path.GetDirectoryName(recording.FullPath))
        {
            _recording = recording;
            _step = recording.State switch
            {
                Model.RecordingState.Pending => RecordingStep.Preparing,
                Model.RecordingState.Running => RecordingStep.Running,
                _ => RecordingStep.Finished
            };
            _state = recording.State;
            _fileName = Path.GetFileNameWithoutExtension(recording.FullPath);
            _startTime = recording.StartTime;
            _duration = recording.Duration;
            _thumbnail = recording.Thumbnail;
            if (recording.State == Model.RecordingState.Running)
            {
                recording.PropertyChanged += Recording_PropertyChanged;
                Thumbnail = _input?.Thumbnail;
            }
            if (recording.Media is null && _step is RecordingStep.Finished)
            {
                _verifyOnceAction = () =>
                {
                    _verifyOnceAction = null; // will not be called anymore
                    IsQueuedToVerify = true; // display waiting ring
                    recording.VerificationCompleted += Recording_VerificationCompleted;
                    recording.QueueVerify();
                };
            }
        }

        private RecordingViewModel(Model.InputBase input, Model.EncoderPreset encoderPreset, string initialDirectory)
        {
            _input = input;
            _encoderPreset = encoderPreset ?? EncoderPresets.FirstOrDefault();
            DirectorySelector = new Shared.DirectorySelectorViewModel(initialDirectory);
            DirectorySelector.PropertyChanged += DirectorySelector_DirectoryChanged;
            OpenExternalFolderCommand = new UiCommand(OpenExternalFolder);
            CommandToggleRecording = new UiCommand(ToggleRecording, CanToggleRecording);
            CommandAddToPlayer = new UiCommand(AddToPlayer, CanAddToPlayer);
            CommandDeleteMedia = new UiCommand(DeleteMedia, CanDeleteMedia);
        }

        private void DirectorySelector_DirectoryChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName is nameof(DirectorySelector.DirectoryName))
                NotifyPropertyChanged(nameof(FullPath));
        }

        public Shared.DirectorySelectorViewModel DirectorySelector { get; }

        public string FileName
        {
            get => _fileName;
            set
            {
                if (!Set(ref _fileName, value))
                    return;
                NotifyPropertyChanged(nameof(FullPath));
            }
        }

        public IEnumerable<Model.EncoderPreset> EncoderPresets
        {
            get
            {
                var currentFormat = _input?.CurrentFormat();
                if (currentFormat is null)
                    return [];
                var currentFormatName = currentFormat.Name;
                return Model.EncoderPresets.Instance.Presets.Where(p => p.InputFormats is null || p.InputFormats.Contains(currentFormatName));
            }
        }

        public Model.EncoderPreset EncoderPreset
        {
            get => _encoderPreset;
            set
            {
                if (!Set(ref _encoderPreset, value))
                    return;
                NotifyPropertyChanged(nameof(FullPath));
            }
        }

        public RecordingStep Step
        {
            get => _step;
            private set
            {
                if (!Set(ref _step, value))
                    return;
                NotifyPropertyChanged(nameof(IsFinishedStep));
                NotifyPropertyChanged(nameof(IsPrepareStep));
            }
        }

        public Model.RecordingState State { get => _state; private set => Set(ref _state, value); }

        public DateTime StartTime { get => _startTime; private set => Set(ref _startTime, value); }

        public TimeSpan Duration { get => _duration; private set => Set(ref _duration, value); }

        public ImageSource Thumbnail
        {
            get
            {
                _verifyOnceAction?.Invoke();
                return _thumbnail;
            }
            set => Set(ref _thumbnail, value);
        }

        public bool IsQueuedToVerify { get => _isQueuedToVerify; private set => Set(ref _isQueuedToVerify, value); }

        public bool IsPrepareStep => Step is RecordingStep.Preparing;


        public bool IsFinishedStep => Step is RecordingStep.Finished;
        
        public ICommand CommandToggleRecording { get; }

        public ICommand OpenExternalFolderCommand { get; }

        public ICommand CommandAddToPlayer { get; }

        public ICommand CommandDeleteMedia { get; }

        public IEnumerable<Model.RundownPlayer> RundownPlayers => Providers.GlobalApplicationData.Current.RundownPlayers;

        public Model.Recording Recording => _recording;

        #region IDataErrorInfo

        public string Error => string.Empty;

        public string this[string columnName]
        {
            get
            {
                {
                    if (Step == RecordingStep.Preparing)
                        switch (columnName)
                        {
                            case nameof(FileName) when File.Exists(FullPath):
                                return "File already exists";
                            case nameof(FileName) when string.IsNullOrWhiteSpace(FileName):
                                return "Filename can't be empty";
                            case nameof(EncoderPreset) when EncoderPreset is null:
                                return "Preset is required to start recording";
                        }
                    return string.Empty;
                }
            }
        }

        #endregion IDataErrorInfo

        public override bool IsValid() => true;

        protected override bool CanRequestRemove(object obj)
        {
            return Step != RecordingStep.Running;
        }

        private void AddToPlayer(object parameter)
        {
            var rundownPlayer = parameter as Model.RundownPlayer ?? throw new ArgumentException(nameof(parameter));
            rundownPlayer.AddMediaToQueue(_recording.Media, rundownPlayer.Items.Count);
        }

        private bool CanToggleRecording(object _) => Step switch
        {
            RecordingStep.Preparing => EncoderPreset != null && Directory.Exists(DirectorySelector.DirectoryName) && !string.IsNullOrEmpty(FullPath) && !File.Exists(FullPath),
            RecordingStep.Running => true,
            _ => false
        };

        private void ToggleRecording(object obj)
        {
            Debug.Assert(Step is RecordingStep.Preparing or RecordingStep.Running);
            switch (Step)
            {
                case RecordingStep.Preparing:
                    StartRecording();
                    break;
                case RecordingStep.Running:
                    StopRecording();
                    break;
            }
        }

        private bool CanAddToPlayer(object _) => _recording?.Media?.IsValid is true;

        private async void DeleteMedia(object _)
        {
            if (await MahApps.Metro.Controls.Dialogs.DialogCoordinator.Instance.ShowMessageAsync(ShellViewModel.Instance, "Confirmation", $"Really delete file \"{FullPath}\"?", MahApps.Metro.Controls.Dialogs.MessageDialogStyle.AffirmativeAndNegative) != MahApps.Metro.Controls.Dialogs.MessageDialogResult.Affirmative)
                return;
            File.Delete(FullPath);
            RequestRemove(_);
        }

        private bool CanDeleteMedia(object _) => File.Exists(FullPath);

        public void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            var recording = Recording;
            if (recording?.State == Model.RecordingState.Running)
                recording.PropertyChanged -= Recording_PropertyChanged;
        }

        public string FullPath
        {
            get
            {
                if (_recording is not null)
                    return _recording.FullPath;
                if (string.IsNullOrEmpty(DirectorySelector.DirectoryName) || string.IsNullOrEmpty(FileName) || EncoderPreset is null)
                    return null;
                return Path.Combine(DirectorySelector.DirectoryName, $"{FileName}.{EncoderPreset.FilenameExtension}");
            }
        }

        private void OpenExternalFolder(object _)
        {
            Process.Start("explorer.exe", $"/select,\"{FullPath}\"");
        }

        private void StopRecording()
        {
            _recording?.Stop();
        }

        private void StartRecording()
        {
            Debug.Assert(_recording is null);
            _recording = new Model.Recording(_input, _encoderPreset, FullPath);
            _recording.PropertyChanged += Recording_PropertyChanged;
            Step = RecordingStep.Running;
            State = Model.RecordingState.Running;
            _recording.Start();
            Thumbnail = _input.Thumbnail;
        }

        private void Recording_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            var recording = sender as Model.Recording ?? throw new ArgumentException($"{nameof(Model.Recording)} expected, {sender?.GetType()} got.");
            switch (e.PropertyName)
            {
                case nameof(Model.Recording.State) when recording.State is Model.RecordingState.Completed or Model.RecordingState.Failed or Model.RecordingState.Aborted:
                    recording.PropertyChanged -= Recording_PropertyChanged;
                    Duration = recording.Duration;
                    Thumbnail = recording.Thumbnail;
                    Step = RecordingStep.Finished;
                    break;
                case nameof(Model.Recording.StartTime):
                    StartTime = recording.StartTime;
                    break;
                case nameof(Model.Recording.Duration):
                    Duration = recording.Duration;
                    break;
                case nameof(Thumbnail):
                    Thumbnail = recording.Thumbnail;
                    break;
                case nameof(State):
                    State = recording.State;
                    break;
            }
        }

        private void Recording_VerificationCompleted(object sender, EventArgs e)
        {
            var recording = sender as Model.Recording ?? throw new ArgumentException($"{nameof(Model.Recording)} expected, {sender?.GetType()} got.");
            if (recording.State is Model.RecordingState.Completed or Model.RecordingState.Failed or Model.RecordingState.Aborted)
                Step = RecordingStep.Finished;
            recording.VerificationCompleted -= Recording_VerificationCompleted;
            State = recording.State;
            StartTime = recording.StartTime;
            Duration = recording.Duration;
            Thumbnail = recording.Thumbnail;
            IsQueuedToVerify = false;
        }

        protected override void RequestRemove(object obj)
        {
            base.RequestRemove(obj);
            Providers.RecordingStore.Current.DeleteRecording(Recording);
        }
    }

    public enum RecordingStep
    {
        Preparing,
        Running,
        Finished
    }
}
