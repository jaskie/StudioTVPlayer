using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using TVPlayR;
using StudioTVPlayer.Model.Interfaces;

namespace StudioTVPlayer.Model
{
    public class Media : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        protected void RaisePropertyChanged([CallerMemberName]string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }

        private string _path;
        public string Path
        {
            get { return _path; }
            set
            {
                if (_path == value)
                    return;

                _path = value;
                RaisePropertyChanged();
            }
        }

        private string _name;
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

        private TimeSpan _duration;
        public TimeSpan Duration
        {
            get { return _duration; }
            set
            {
                if (_duration == value)
                    return;

                _duration = value;
                RaisePropertyChanged();
            }
        }

        private DateTime _creationDate;
        public DateTime CreationDate
        {
            get { return _creationDate; }
            set
            {
                if (_creationDate == value)
                    return;

                _creationDate = value;
                RaisePropertyChanged();
            }
        }                  
    }
}
