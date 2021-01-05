using System.Runtime.CompilerServices;

namespace StudioTVPlayer.ViewModel
{
    public abstract class ModifyableViewModelBase: ViewModelBase
    {
        private bool _isModified;

        public bool IsModified
        {
            get => _isModified;
            protected set
            {
                if (_isModified == value)
                    return;
                _isModified = value;
                NotifyPropertyChanged();
            }
        }

        protected override bool Set<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (base.Set(ref field, value, propertyName))
            {
                IsModified = true;
                return true;
            }
            return false;
        }

        public abstract bool IsValid();

        public abstract void Apply();
    }
}