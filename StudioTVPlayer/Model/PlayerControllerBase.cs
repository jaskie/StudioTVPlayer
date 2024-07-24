using System;

namespace StudioTVPlayer.Model
{
    public abstract class PlayerControllerBase : IDisposable
    {
        protected PlayerControllerBase(Configuration.PlayerControllerBase playerControllerConfiguration)
        {
            Name = playerControllerConfiguration.Name;
        }

        private bool _isConnected;

        public event EventHandler ConnectionStateChanged;

        public bool IsConnected => _isConnected;

        public string Name { get; }

        public abstract void Dispose();

        protected void NotifyConnectionStateChanged(bool isConnected)
        {
            _isConnected = isConnected;
            ConnectionStateChanged?.Invoke(this, EventArgs.Empty);
        }

        public abstract void NotifyPlayerChanged(RundownPlayer player);

    }
}
