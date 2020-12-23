using StudioTVPlayer.ViewModel;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ExtensionViewModel : ModifyableViewModelBase
    {
        private string _value;
        public string Value
        {
            get => _value;
            set => Set(ref _value, value);
        }

        public ExtensionViewModel(string value = null)
        {
            Value = value;
        }

        public static explicit operator string(ExtensionViewModel wrapper)
        {
            return wrapper.Value;
        }

        public override void Apply()
        {
            throw new NotImplementedException();
        }
    }
}
