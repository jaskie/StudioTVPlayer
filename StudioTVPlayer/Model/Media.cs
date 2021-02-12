using System;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public class Media : INotifyPropertyChanged
    {
        private string _directoryName;
        private string _name;
        private TimeSpan _duration;
        private DateTime _creationTime;
        private ImageSource _thumbnail;

        internal void Play()
        {
            throw new NotImplementedException();
        }

        private bool _isVerified;
        private int _height;
        private int _width;
        private ScanType _scanType;
        private string _frameRate;
        private int _audioChannelCount;
        private readonly FileInfo _fileInfo;

        public Media(string path)
        {
            _fileInfo = new FileInfo(path);
            ReadFileInfo();
        }


        public event PropertyChangedEventHandler PropertyChanged;

        private void RaisePropertyChanged([CallerMemberName] string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }

        public string DirectoryName
        {
            get => _directoryName;
            set
            {
                if (_directoryName == value)
                    return;
                _directoryName = value;
                RaisePropertyChanged();
            }
        }

        public string Name
        {
            get => _name;
            set
            {
                if (_name == value)
                    return;
                _name = value;
                RaisePropertyChanged();
            }
        }

        public TimeSpan Duration
        {
            get => _duration;
            set
            {
                if (_duration == value)
                    return;
                _duration = value;
                RaisePropertyChanged();
            }
        }

        public DateTime CreationTime
        {
            get => _creationTime;
            set
            {
                if (_creationTime == value)
                    return;
                _creationTime = value;
                RaisePropertyChanged();
            }
        }

        public int Height
        {
            get => _height;
            set
            {
                if (_height == value)
                    return;
                _height = value;
                RaisePropertyChanged();
            }
        }

        public int Width
        {
            get => _width;
            set
            {
                if (_width == value)
                    return;
                _width = value;
                RaisePropertyChanged();
            }
        }

        public ScanType ScanType
        {
            get => _scanType;
            set
            {
                if (_scanType == value)
                    return;
                _scanType = value;
                RaisePropertyChanged();
            }
        }

        public string FrameRate
        {
            get => _frameRate;
            set
            {
                if (_frameRate == value)
                    return;
                _frameRate = value;
                RaisePropertyChanged();
            }
        }

        public int AudioChannelCount
        {
            get => _audioChannelCount; 
            set
            {
                if (_audioChannelCount == value)
                    return;
                _audioChannelCount = value;
                RaisePropertyChanged();
            }
        }

        public ImageSource Thumbnail
        {
            get => _thumbnail;
            set
            {
                _thumbnail = value;
                RaisePropertyChanged();
            }
        }

        public string FullPath => Path.Combine(DirectoryName, Name);

        public bool IsVerified
        {
            get => _isVerified;
            internal set
            {
                if (_isVerified == value)
                    return;
                _isVerified = value;
                RaisePropertyChanged();
            }
        }

        public void Refresh()
        {
            _fileInfo.Refresh();
            ReadFileInfo();
        }

        private void ReadFileInfo()
        {
            Name = _fileInfo.Name;
            CreationTime = _fileInfo.CreationTime;
            DirectoryName = _fileInfo.DirectoryName;
        }
    }
}
