using System;
using System.ComponentModel;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public abstract class PlayerControllerViewModelBase : RemovableViewModelBase, IDataErrorInfo, ICheckErrorInfo
    {
        public readonly Model.Configuration.PlayerControllerBase PlayerController;

        public PlayerControllerViewModelBase(Model.Configuration.PlayerControllerBase playerController)
        {
            PlayerController = playerController;
        }

        public string this[string columnName] => throw new System.NotImplementedException();

        public string Error => throw new System.NotImplementedException();

        public event EventHandler<CheckErrorEventArgs> CheckErrorInfo;
    }
}