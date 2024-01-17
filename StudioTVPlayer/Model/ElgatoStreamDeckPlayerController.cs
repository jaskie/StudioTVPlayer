using OpenMacroBoard.SDK;
using StreamDeckSharp;
using StreamDeckSharp.Exceptions;
using System;
using System.Linq;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class ElgatoStreamDeckPlayerController : PlayerControllerBase
    {
        private readonly Configuration.ElgatoStreamDeckPlayerController _playerControllerConfiguration;
        private readonly ElgatoStreamDeckPlayerBinding[] _bindings;
        private IMacroBoard _streamDeck;
        private bool _disposed;

        public ElgatoStreamDeckPlayerController(Configuration.ElgatoStreamDeckPlayerController playerControllerConfiguration)
        {
            _playerControllerConfiguration = playerControllerConfiguration;
            _bindings = playerControllerConfiguration.Bindings.Select(CreateBinding).ToArray();
            TryToReconnect();
        }

        private ElgatoStreamDeckPlayerBinding CreateBinding(Configuration.PlayerBindingBase playerBindingConfiguration)
        {
            var elgatoStreamDeckPlayerBindingConfiguration = playerBindingConfiguration as Configuration.ElgatoStreamDeckPlayerBinding ?? throw new ArgumentException(nameof(playerBindingConfiguration));
            return new ElgatoStreamDeckPlayerBinding(elgatoStreamDeckPlayerBindingConfiguration);
        }

        private async void TryToReconnect()
        {
            while (!_disposed && _streamDeck is null)
            {
                try
                {
                    _streamDeck = StreamDeck.OpenDevice(_playerControllerConfiguration.Path, false).WithDisconnectReplay().WithButtonPressEffect();
                    _streamDeck.ConnectionStateChanged += OnConnectionStateChanged;
                    _streamDeck.KeyStateChanged += OnKeyStateChanged;
                    NotifyConnectionStateChanged(true);
                }
                catch (Exception e) when (e is InvalidOperationException || e is StreamDeckNotFoundException)
                {
                    await Task.Delay(5000);
                }
            }
        }

        private void OnKeyStateChanged(object sender, KeyEventArgs e)
        {
            if (e.IsDown)
            {
                foreach (var binding in _bindings)
                    binding.KeyPressed(e.Key);
            }
        }

        private void OnConnectionStateChanged(object sender, ConnectionEventArgs e)
        {
            NotifyConnectionStateChanged(e.NewConnectionState);
        }

        public override void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            if (_streamDeck != null)
            {
                _streamDeck.KeyStateChanged -= OnKeyStateChanged;
                _streamDeck.ConnectionStateChanged -= OnConnectionStateChanged;
                _streamDeck.Dispose();
                _streamDeck = null;
            }
        }
    }
}
