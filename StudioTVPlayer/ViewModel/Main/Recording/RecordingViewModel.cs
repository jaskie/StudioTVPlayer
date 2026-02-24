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
        private string _folder;
        private RecordingStep _step;
        private Model.EncoderPreset _encoderPreset;
        private bool _disposed;
        private ImageSource _thumbnail;
        private DateTime _startTime;
        private TimeSpan _duration;
        private readonly Model.InputBase _input;

        public RecordingViewModel(Model.InputBase input) : this(input, null)
        {
            _folder = Folders.LastOrDefault();
            _fileName = $"Recording_{DateTime.Now:yyyyMMdd_HHmmss}";
        }

        public RecordingViewModel(Model.Recording recording) : this(recording.Input, recording.EncoderPreset)
        {
            _recording = recording;
            _step = recording.State switch
            {
                Model.RecordingState.Pending => RecordingStep.Preparing,
                Model.RecordingState.Running => RecordingStep.Running,
                _ => RecordingStep.Finished
            };
            _folder = Path.GetDirectoryName(recording.FullPath);
            _fileName = Path.GetFileNameWithoutExtension(recording.FullPath);
            CommandToggleRecording = new UiCommand(ToggleRecording, CanToggleRecording);
        }

        private RecordingViewModel(Model.InputBase input, Model.EncoderPreset encoderPreset)
        {
            BrowseForFolderCommand = new UiCommand(BrowseForFolder);
            OpenExternalFolderCommand = new UiCommand(OpenExternalFolder);
            _input = input;
            _encoderPreset = encoderPreset ?? EncoderPresets.FirstOrDefault();
            CommandToggleRecording = new UiCommand(ToggleRecording, CanToggleRecording);
        }

        public IEnumerable<string> Folders => Providers.MostRecentUsed.Current.Folders;

        public string Folder
        {
            get => _folder;
            set
            {
                if (!Set(ref _folder, value))
                    return;
                NotifyPropertyChanged(nameof(FullPath));
            }
        }

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
                var currentFormat = _input.CurrentFormat();
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
                NotifyPropertyChanged(nameof(ShowRecordingButton));
                NotifyPropertyChanged(nameof(CanRemove));
            }
        }

        public DateTime StartTime { get => _startTime; private set => Set(ref _startTime, value); }

        public TimeSpan Duration { get => _duration; private set => Set(ref _duration, value); }

        public ImageSource Thumbnail { get => _thumbnail; set => Set(ref _thumbnail, value); }

        public ICommand CommandToggleRecording { get; }

        private bool CanToggleRecording(object _) => Step switch
        {
            RecordingStep.Preparing => EncoderPreset != null && Directory.Exists(Folder) && !string.IsNullOrEmpty(FullPath) && !File.Exists(FullPath),
            RecordingStep.Running => true,
            _ => false
        };

        private void ToggleRecording(object obj)
        {
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

        public bool ShowRecordingButton => Step is RecordingStep.Preparing or RecordingStep.Running;

        public bool CanRemove => Step != RecordingStep.Running;

        public ICommand BrowseForFolderCommand { get; }

        public ICommand OpenExternalFolderCommand { get; }

        #region IDataErrorInfo

        public string Error => string.Empty;

        public string this[string columnName] => ReadErrorInfo(columnName);

        private string ReadErrorInfo(string propertyName)
        {
            if (Step == RecordingStep.Preparing)
                switch (propertyName)
                {
                    case nameof(Folder) when Step == RecordingStep.Preparing && !Directory.Exists(Folder):
                        return "Folder does not exists";
                    case nameof(FileName) when File.Exists(FullPath):
                        return "File already exists";
                    case nameof(FileName) when string.IsNullOrWhiteSpace(FileName):
                        return "Filename can't be empty";
                    case nameof(EncoderPreset) when EncoderPreset is null:
                        return "Preset is required to start recording";
                }
            return string.Empty;
        }

        #endregion IDataErrorInfo

        public override bool IsValid()
        {
            return true;
        }

        protected override bool CanRequestRemove(object obj)
        {
            return Step != RecordingStep.Running;
        }

        public void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
        }

        public string FullPath
        {
            get
            {
                if (string.IsNullOrEmpty(Folder) || string.IsNullOrEmpty(FileName) || EncoderPreset is null)
                    return null;
                return Path.Combine(Folder, $"{FileName}.{EncoderPreset.FilenameExtension}");
            }
        }

        private void BrowseForFolder(object _)
        {
            if (FolderHelper.BrowseForFolder(ref _folder, $"Select folder to capture video"))
            {
                NotifyPropertyChanged(nameof(Folder));
                NotifyPropertyChanged(nameof(FullPath));
                Providers.MostRecentUsed.Current.AddMostRecentlyUsedFolder(_folder);
            }
        }

        private void OpenExternalFolder(object _)
        {
            Process.Start("explorer.exe", $"/select,\"{FullPath}\"");
        }

        private void StopRecording()
        {
            _recording?.Stop();
            Step = RecordingStep.Finished;
        }

        private void StartRecording()
        {
            Debug.Assert(_recording is null);
            Providers.MostRecentUsed.Current.AddMostRecentlyUsedFolder(_folder);
            _recording = new Model.Recording(_input, _encoderPreset, FullPath);
            _recording.Start();
            _recording.PropertyChanged += Recording_PropertyChanged;
            Thumbnail = _input.Thumbnail;
            Step = RecordingStep.Running;
        }

        private void Recording_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            var recording = sender as Model.Recording ?? throw new ArgumentException(nameof(sender));
            switch (e.PropertyName)
            {
                case nameof(Model.Recording.State) when recording.State is Model.RecordingState.Completed or Model.RecordingState.Failed or Model.RecordingState.Aborted:
                    recording.PropertyChanged -= Recording_PropertyChanged;
                    Duration = recording.Duration;
                    Thumbnail = recording.Thumbnail;
                    _recording = null;
                    Step = RecordingStep.Finished;
                    break;
                case nameof(Model.Recording.StartTime):
                    StartTime = recording.StartTime;
                    break;
            }
        }
    }

    public enum RecordingStep
    {
        Preparing,
        Running,
        Finished
    }
}
