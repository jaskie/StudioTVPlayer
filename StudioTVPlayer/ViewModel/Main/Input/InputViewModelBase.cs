using System;
using System.ComponentModel;

namespace StudioTVPlayer.ViewModel.Main.Input
{
    public abstract class InputViewModelBase : RemovableViewModelBase, IDisposable, IDataErrorInfo
    {
        public abstract Model.InputBase Input { get; }

        public string Error => string.Empty;

        public string this[string columnName] => ReadErrorInfo(columnName);

        protected abstract string ReadErrorInfo(string propertyName);

        public abstract void Dispose();
    }
}
