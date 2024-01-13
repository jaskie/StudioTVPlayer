using System.Runtime.CompilerServices;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public abstract class ConfigurationItemBase: Helpers.PropertyChangedBase
    {
        private bool _isModified;

        [XmlIgnore]
        internal bool IsModified { get => GetIsModified(); set => SetIsModified(value); }

        protected virtual void SetIsModified(bool value)
        {
            if (value == _isModified)
                return; 
            _isModified = value;
            NotifyPropertyChanged(nameof(IsModified));
        }

        protected virtual bool GetIsModified()
        {
            return _isModified;
        }

        protected override bool Set<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (!base.Set(ref field, value, propertyName))
                return false;
            SetIsModified(true);
            return true;
        }

    }
}