using StudioTVPlayer.Helpers;
using System;
using System.Diagnostics;
using System.IO;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{

    [DebuggerDisplay(nameof(FullPath))]
    public class MediaFile : PropertyChangedBase
    {
        private string _directoryName;
        private string _name;
        private TimeSpan _duration;
        private DateTime _creationTime;
        private ImageSource _thumbnail;
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

        public string DirectoryName
        {
            get => _directoryName;
            private set => Set(ref _directoryName, value);
        }

        public string Name
        {
            get => _name;
            private set => Set(ref _name, value);
        }

        public TimeSpan StartTime
        {
            get => _startTime;
            internal set => Set(ref _startTime, value);
        }

        public TimeSpan Duration
        {
            get => _duration;
            internal set => Set(ref _duration, value);
        }

        public DateTime CreationTime
        {
            get => _creationTime;
            private set => Set(ref _creationTime, value);
        }

        public int Height
        {
            get => _height;
            internal set => Set(ref _height, value);
        }

        public int Width
        {
            get => _width;
            internal set => Set(ref _width, value);
        }

        public ScanType ScanType
        {
            get => _scanType;
            internal set => Set(ref _scanType, value);
        }

        public string FrameRate
        {
            get => _frameRate;
            internal set => Set(ref _frameRate, value);
        }

        public int AudioChannelCount
        {
            get => _audioChannelCount;
            internal set => Set(ref _audioChannelCount, value);
        }

        public bool HaveAlphaChannel
        {
            get => _haveAlphaChannel;
            internal set => Set(ref _haveAlphaChannel, value);
        }

        public ImageSource Thumbnail
        {
            get => _thumbnail;
            internal set => Set(ref _thumbnail, value);
        }

        public string FullPath => Path.Combine(DirectoryName, Name);

        public bool IsVerified
        {
            get => _isVerified;
            internal set => Set(ref _isVerified, value);
        }

        public bool IsValid
        {
            get => _isValid;
            internal set => Set(ref _isValid, value);
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
