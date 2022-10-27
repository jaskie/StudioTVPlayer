using StudioTVPlayer.Providers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;

namespace StudioTVPlayer.ViewModel.Main
{
    public class RecorderViewModel : RemovableViewModelBase, IDataErrorInfo
    {
        private Model.InputBase _input;
        private string _fileName;
        private string _folder;
        private string _fullPath;

        public RecorderViewModel(Model.InputBase input)
        {
            _input = input;
        }

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

        public Model.EncoderPreset EncoderPreset { get; set; }

        public bool IsRecording { get; set; }

        public bool CanStartRecord => IsRecording || CanStartRecording();

        public static IEnumerable<Model.EncoderPreset> EncoderPresets => GlobalApplicationData.Current.EncoderPresets;

        public string Error => string.Empty;

        public string this[string columnName] => ReadErrorInfo(columnName);

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

        private string ReadErrorInfo(string propertyName)
        {
            return string.Empty;
        }

        private void SetNewFullPath()
        {
            _fullPath = Path.Combine(Folder, FileName);
            NotifyPropertyChanged(nameof(CanStartRecord));
        }

        private bool CanStartRecording()
        {
            return Directory.Exists(Folder) && !File.Exists(_fullPath);
        }



    }
}
