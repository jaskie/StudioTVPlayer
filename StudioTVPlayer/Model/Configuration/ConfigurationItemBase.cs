using System.Collections.Generic;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public class ConfigurationItemBase
    {
        private bool _isModified;

        [XmlIgnore]
        internal bool IsModified { get => _isModified; set => SetIsModified(value); }

        protected virtual void SetIsModified(bool value)
        {
            _isModified = value;
        }

        protected bool Set<T>(ref T field, T value)
        {
            if (EqualityComparer<T>.Default.Equals(field, value)) return false;
            field = value;
            IsModified = true;
            return true;
        }
    }
}