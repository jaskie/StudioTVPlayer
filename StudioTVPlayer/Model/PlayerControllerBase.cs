using System;

namespace StudioTVPlayer.Model
{
    public abstract class PlayerControllerBase : IDisposable
    {
        protected PlayerControllerBase(Configuration.PlayerControllerBase playerControllerConfiguration)
        {
            Name = playerControllerConfiguration.Name;
        }

        public event EventHandler ConnectionStateChanged;

        public bool IsConnected { get; protected set; }

        public string Name { get; }

        public abstract void Dispose();

        protected void NotifyConnectionStateChanged(bool isConnected)
        {
            IsConnected = isConnected;
            ConnectionStateChanged?.Invoke(this, EventArgs.Empty);
        }

        public abstract void NotifyPlayerChanged(RundownPlayer player);

    }
}
