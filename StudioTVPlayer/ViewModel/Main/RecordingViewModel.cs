using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Main
{
    public sealed class RecordingViewModel : RemovableViewModelBase, IDataErrorInfo, IDisposable
    {
        private Model.Recording _recording;
        private string _fileName;
        private string _folder;
        private bool _isRecording;
        private bool _isCompleted;
        private Model.EncoderPreset _encoderPreset;
        private bool _disposed;
        private readonly Model.InputBase _input;

        public RecordingViewModel(Model.InputBase input) : this(input, null)
        {
            _folder = Folders.LastOrDefault();
            _fileName = $"Recording_{DateTime.Now:yyyyMMdd_HHmmss}";
        }

        public RecordingViewModel(Model.Recording recording) : this(recording.Input, recording.EncoderPreset)
        {
            _recording = recording;
            _isRecording = true;
            _folder = Path.GetDirectoryName(recording.FullPath);
            _fileName = Path.GetFileNameWithoutExtension(recording.FullPath);
        }

        private RecordingViewModel(Model.InputBase input, Model.EncoderPreset encoderPreset)
        {
            BrowseForFolderCommand = new UiCommand(BrowseForFolder);
            OpenExternalFolderCommand = new UiCommand(OpenExternalFolder);
            _input = input;
            if (input is Model.DecklinkInput decklinkInput)
                decklinkInput.FormatChanged += DecklinkInput_InputFormatChanged;
            _encoderPreset = encoderPreset ?? EncoderPresets.FirstOrDefault();
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
                NotifyPropertyChanged(nameof(CanChangeRecordingState));
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
                NotifyPropertyChanged(nameof(CanChangeRecordingState));
            }
        }

        public IEnumerable<Model.EncoderPreset> EncoderPresets
        {
            get
            {
                var currentFormatName = _input.CurrentFormat().Name;
                return Model.EncoderPresets.Instance.Presets.Where(p => p.InputFormats == null || p.InputFormats.Contains(currentFormatName));
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
                NotifyPropertyChanged(nameof(CanChangeRecordingState));
            }
        }

        public bool IsRecording
        {
            get => _isRecording;
            set
            {
                if (!Set(ref _isRecording, value))
                    return;
                if (value)
                    StartRecording();
                else
                    StopRecording();
                NotifyPropertyChanged(nameof(CanChangeRecordingState));
            }
        }

        public bool IsCompleted
        {
            get => _isCompleted;
            private set => Set(ref _isCompleted, value);
        }

        public bool CanChangeRecordingState => IsRecording // to stop the ongiong recording
            || (EncoderPreset != null && Directory.Exists(Folder) && !string.IsNullOrEmpty(FullPath) && !File.Exists(FullPath)); // to start new one

        public ICommand BrowseForFolderCommand { get; }

        public ICommand OpenExternalFolderCommand { get; }

        #region IDataErrorInfo

        public string Error => string.Empty;

        public string this[string columnName] => ReadErrorInfo(columnName);

        private string ReadErrorInfo(string propertyName)
        {
            if (IsRecording)
                return string.Empty;
            switch (propertyName)
            {
                case nameof(Folder) when !IsRecording && !Directory.Exists(Folder):
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
            return !IsRecording;
        }

        public void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            if (_input is Model.DecklinkInput decklinkInput)
                decklinkInput.FormatChanged -= DecklinkInput_InputFormatChanged;
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
                NotifyPropertyChanged(nameof(CanChangeRecordingState));
                Providers.MostRecentUsed.Current.AddMostRecentlyUsedFolder(_folder);
            }
        }

        private void OpenExternalFolder(object _)
        {
            Process.Start("explorer.exe", $"/select,\"{FullPath}\"");
        }

        private void StopRecording() => _recording?.Stop();

        private void StartRecording()
        {
            Debug.Assert(_recording is null);
            Providers.MostRecentUsed.Current.AddMostRecentlyUsedFolder(_folder);
            _recording = new Model.Recording(_input, _encoderPreset, FullPath);
            _recording.Start();
            _recording.Finished += Recording_Finished;
        }

        private void Recording_Finished(object sender, EventArgs e)
        {
            var recording = sender as Model.Recording ?? throw new ArgumentException(nameof(sender));
            recording.Finished -= Recording_Finished;
            _recording = null;
            NotifyPropertyChanged(nameof(FileName));
            Set(ref _isRecording, false, nameof(IsRecording));
            NotifyPropertyChanged(nameof(CanChangeRecordingState));
            IsCompleted = true;
        }

        private void DecklinkInput_InputFormatChanged(object sender, EventArgs e)
        {
            NotifyPropertyChanged(nameof(EncoderPresets));
        }

    }
}
