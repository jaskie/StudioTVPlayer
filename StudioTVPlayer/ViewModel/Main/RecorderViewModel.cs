using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows.Input;
using System.Xml.Linq;

namespace StudioTVPlayer.ViewModel.Main
{
    public class RecorderViewModel : RemovableViewModelBase, IDataErrorInfo
    {
        private Model.Recording _recording;
        private string _fileName;
        private string _folder;
        private string _fullPath;
        private bool _isRecording;
        private Model.EncoderPreset _encoderPreset;
        private readonly Model.InputBase _input;

        public RecorderViewModel(Model.InputBase input)
        {
            _input = input;
            CommandBrowseForFolder = new UiCommand(BrowseForFolder);
            _folder = Folders.LastOrDefault();
        }

        public RecorderViewModel(Model.Recording recording)
        {
            _recording = recording;
            _input = recording.Input;
            _isRecording = true;
            EncoderPreset = recording.EncoderPreset;
            _folder = Path.GetDirectoryName(recording.FullPath);
            _fileName = Path.GetFileNameWithoutExtension(recording.FullPath);
        }

        public IEnumerable<string> Folders => Providers.MostRecentUsed.Current.Folders;

        public string Folder
        {
            get => _folder;
            set
            {
                if (!Set(ref _folder, value))
                    return;
                SetNewFullPath();
            }
        }

        public string FileName
        {
            get => _fileName;
            set
            {
                if (!Set(ref _fileName, value))
                    return;
                SetNewFullPath();
            }
        }

        public IEnumerable<Model.EncoderPreset> EncoderPresets => Providers.GlobalApplicationData.Current.EncoderPresets;

        public Model.EncoderPreset EncoderPreset
        {
            get => _encoderPreset;
            set
            {
                if (!Set(ref _encoderPreset, value))
                    return;
                NotifyPropertyChanged(nameof(CanChangeRecordingState));
                NotifyPropertyChanged(nameof(FileName));
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

        public bool CanChangeRecordingState => IsRecording // to stop the ongiong recording
            || (EncoderPreset != null && Directory.Exists(Folder) && !string.IsNullOrEmpty(_fullPath) && !File.Exists($"{_fullPath}.{EncoderPreset.FilenameExtension}")); // to start new one

        public ICommand CommandBrowseForFolder { get; }

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
                case nameof(FileName) when File.Exists($"{_fullPath}.{EncoderPreset?.FilenameExtension}"):
                    return "File already exists";
                case nameof(FileName) when string.IsNullOrWhiteSpace(FileName):
                    return "Filename can't be empty";
                case nameof(EncoderPreset) when EncoderPreset is null:
                    return "Preset is required to start recording";
            }
            return string.Empty;
        }

        #endregion IDataErrorInfo

        public override void Apply()
        {
            throw new NotImplementedException();
        }

        public override bool IsValid()
        {
            return true;
        }

        protected override bool CanRequestRemove(object obj)
        {
            return !IsRecording;
        }

        private void SetNewFullPath()
        {
            if (string.IsNullOrEmpty(Folder) || string.IsNullOrEmpty(FileName))
            {
                _fullPath = null;
            }
            else
                _fullPath = Path.Combine(Folder, FileName);
            NotifyPropertyChanged(nameof(CanChangeRecordingState));
        }

        private void BrowseForFolder(object _)
        {
            if (FolderHelper.Browse(ref _folder, $"Select folder to capture video"))
            {
                NotifyPropertyChanged(nameof(Folder));
                SetNewFullPath();
                Providers.MostRecentUsed.Current.AddMostRecentlyUsedFolder(_folder);
            }
        }

        private void StopRecording() => _recording?.StopRecording();

        private void StartRecording()
        {
            Debug.Assert(_recording is null);
            Providers.MostRecentUsed.Current.AddMostRecentlyUsedFolder(_folder);
            _recording = new Model.Recording(_input);
            _recording.StartRecording($"{_fullPath}.{EncoderPreset.FilenameExtension}" , EncoderPreset);
            _recording.Finished += Recording_Finished;
        }

        private void Recording_Finished(object sender, EventArgs e)
        {
            var recording = sender as Model.Recording ?? throw new ArgumentException(nameof(sender));
            recording.Finished -= Recording_Finished;
            _recording = null;
        }
    }
}
