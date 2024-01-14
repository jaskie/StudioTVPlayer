using System;
using System.ComponentModel;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public abstract class PlayerControllerViewModelBase : RemovableViewModelBase, IDataErrorInfo, ICheckErrorInfo
    {
        public readonly Model.Configuration.PlayerControllerBase PlayerControllerConfiguration;

        public PlayerControllerViewModelBase(Model.Configuration.PlayerControllerBase playerControllerConfiguration)
        {
            PlayerControllerConfiguration = playerControllerConfiguration;
        }

        public abstract string Id { get ; }

        public abstract string DisplayName { get ; }

        public string this[string columnName] => ReadErrorInfo(columnName);

        public string Error => null;

        public event EventHandler<CheckErrorEventArgs> CheckErrorInfo;

        protected virtual string ReadErrorInfo(string propertyName)
        {
            if (CheckErrorInfo is null)
                return string.Empty;
            var checkErrorInfo = new CheckErrorEventArgs(this, propertyName);
            CheckErrorInfo(this, checkErrorInfo);
            return checkErrorInfo.Message;
        }

    }
}