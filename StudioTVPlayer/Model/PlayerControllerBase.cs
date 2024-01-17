using System;

namespace StudioTVPlayer.Model
{
    public abstract class PlayerControllerBase : IDisposable
    {
        private bool _isConnected;

        public event EventHandler ConnectionStateChanged;

        public bool IsConnected => _isConnected;

        public abstract void Dispose();

        protected void NotifyConnectionStateChanged(bool isConnected)
        {
            _isConnected = isConnected;
            ConnectionStateChanged?.Invoke(this, EventArgs.Empty);
        }
    }
}
