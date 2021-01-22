using System;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;

namespace StudioTVPlayer.Model
{
    public class Media : INotifyPropertyChanged
    {
        private string _directoryName;
        private string _name;
        private TimeSpan _duration;
        private DateTime _creationDate;
        private readonly FileInfo _fileInfo;

        public Media(string path)
        {
            _fileInfo = new FileInfo(path);
            ReadInfo();
        }


        public event PropertyChangedEventHandler PropertyChanged;
        
        private void RaisePropertyChanged([CallerMemberName]string propertyname = null)
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
            get => _creationDate;
            set
            {
                if (_creationDate == value)
                    return;
                _creationDate = value;
                RaisePropertyChanged();
            }
        }

        public string FullPath => Path.Combine(DirectoryName, Name);

        public void Refresh()
        {
            _fileInfo.Refresh();
            ReadInfo();
        }
        
        private void ReadInfo()
        {
            Name = _fileInfo.Name;
            CreationTime = _fileInfo.CreationTime;
            DirectoryName = _fileInfo.DirectoryName;
        }
    }
}
