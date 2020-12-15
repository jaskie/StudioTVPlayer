using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class StringWrapper : INotifyPropertyChanged
    {
        private string _value;
        public string Value {
            get { return _value; }
            set
            {
                if (_value == value)
                    return;
                _value = value;
                RaisePropertyChanged();
            }
        }

        public StringWrapper(string value = null)
        {
            Value = value;
        }

        public static explicit operator String (StringWrapper wrapper)
        {
            return wrapper.Value;
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected void RaisePropertyChanged([CallerMemberName]string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }
    }
}
