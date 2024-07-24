using StudioTVPlayer.Model;
using System;

namespace StudioTVPlayer.ViewModel
{
    public sealed class PlayerControllerViewModel : ViewModelBase, IDisposable
    {
        private readonly PlayerControllerBase _playerController;
        private bool _disposed;

        public PlayerControllerViewModel(Model.PlayerControllerBase playerController)
        {
            _playerController = playerController;
            _playerController.ConnectionStateChanged += _playerController_ConnectionStateChanged;
        }

        private void _playerController_ConnectionStateChanged(object sender, System.EventArgs e)
        {
            NotifyPropertyChanged(nameof(IsConnected));
        }

        public void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            _playerController.ConnectionStateChanged -= _playerController_ConnectionStateChanged;
        }

        public string Name => _playerController.Name;

        public bool IsConnected => _playerController.IsConnected;


    }
}