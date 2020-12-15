using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class WatcherMeta : INotifyPropertyChanged
    {
        private string _name;
        [XmlElement("Name")]
        public string Name
        {
            get => _name;
            set
            {
                _name = value;
                RaisePropertyRaised();
            }
        }

        private string _path;
        [XmlElement("Path")]
        public string Path {
            get => _path;
            set
            {
                _path = value;
                RaisePropertyRaised();
            }
        }

        private bool _isFiltered;
        [XmlElement("IsFiltered")]
        public bool IsFiltered {
            get => _isFiltered;
            set
            {
                _isFiltered = value;
                RaisePropertyRaised();
            }
        }

        private bool _watcherAssigned;
        [XmlIgnore]
        public bool WatcherAssigned
        {
            get => _watcherAssigned;
            set
            {
                _watcherAssigned = value;
                RaisePropertyRaised();
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        private void RaisePropertyRaised([CallerMemberName]string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }
    }
}
