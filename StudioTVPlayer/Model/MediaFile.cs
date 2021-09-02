using System;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Runtime.CompilerServices;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{

    [DebuggerDisplay(nameof(FullPath))]
    public class MediaFile : INotifyPropertyChanged
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
        private TimeSpan _startTime;
        private bool _isValid;
        private bool _haveAlphaChannel;
        private readonly FileInfo _fileInfo;

        public MediaFile(string path)
        {
            _fileInfo = new FileInfo(path);
            ReadFileInfo();
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public string DirectoryName
        {
            get => _directoryName;
            private set
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
            private set
            {
                if (_name == value)
                    return;
                _name = value;
                RaisePropertyChanged();
            }
        }

        public TimeSpan StartTime
        {
            get => _startTime;
            internal set
            {
                if (_startTime == value)
                    return;
                _startTime = value;
                RaisePropertyChanged();
            }
        }

        public TimeSpan Duration
        {
            get => _duration;
            internal set
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
            private set
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
            internal set
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
            internal set
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
            internal set
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
            internal set
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
            internal set
            {
                if (_audioChannelCount == value)
                    return;
                _audioChannelCount = value;
                RaisePropertyChanged();
            }
        }

        public bool HaveAlphaChannel
        {
            get => _haveAlphaChannel;
            internal set
            {
                if (_haveAlphaChannel == value)
                    return;
                _haveAlphaChannel = value;
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

        public bool IsValid
        {
            get => _isValid;
            internal set
            {
                if (_isValid == value)
                    return;
                _isValid = value;
                RaisePropertyChanged();
            }
        }

        public void Refresh()
        {
            _fileInfo.Refresh();
            ReadFileInfo();
        }

        private void RaisePropertyChanged([CallerMemberName] string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }

        private void ReadFileInfo()
        {
            Name = _fileInfo.Name;
            CreationTime = _fileInfo.CreationTime;
            DirectoryName = _fileInfo.DirectoryName;
        }
    }
}
