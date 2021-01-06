using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace StudioTVPlayer.Model
{
    public class Media : INotifyPropertyChanged
    {
        private string _path;
        private string _name;
        private TimeSpan _duration;
        private DateTime _creationDate;

        public event PropertyChangedEventHandler PropertyChanged;
        private void RaisePropertyChanged([CallerMemberName]string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }

        public string Path
        {
            get => _path;
            set
            {
                if (_path == value)
                    return;
                _path = value;
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

        public DateTime CreationDate
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

        public string FullPath => System.IO.Path.Combine(Path, Name);
    }
}
