using OpenMacroBoard.SDK;
using StreamDeckSharp;
using StreamDeckSharp.Exceptions;
using System;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class ElgatoStreamDeckPlayerController : PlayerControllerBase
    {
        private readonly Configuration.ElgatoStreamDeckPlayerController _playerControllerConfiguration;
        private readonly ElgatoStreamDeckPlayerBinding[] _bindings;
        private IMacroBoard _streamDeck;
        private bool _disposed;
        private CancellationTokenSource _connectCancelationTokenSource;
        private Font _font;


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
                    _font = new Font("Segoe MDL2 Assets", _streamDeck.Keys.KeySize / 2);
                    NotifyConnectionStateChanged(true);
                    await ShowInitialScreen();
                }
                catch (Exception e) when (e is InvalidOperationException || e is StreamDeckNotFoundException)
                {
                    Thread.Sleep(5000);
                }
            }
        }

        private async Task ShowInitialScreen()
        {
            _connectCancelationTokenSource = new CancellationTokenSource();
            try
            {
                using (Stream stream = Assembly.GetExecutingAssembly().GetManifestResourceStream("StudioTVPlayer.Resources.Player.png"))
                {
                    var splashKeyImage = KeyBitmap.Create.FromImageSharpImage(await SixLabors.ImageSharp.Image.LoadAsync(stream, _connectCancelationTokenSource.Token));
                    _streamDeck.SetKeyBitmap(splashKeyImage);
                }
                var random = new Random();
                var showKeyTasks = new Task[_streamDeck.Keys.Count];
                for (int i = 0; i < _streamDeck.Keys.Count; i++)
                    showKeyTasks[i] = UpdateKey(i, random.Next(500), _connectCancelationTokenSource.Token);
                await Task.WhenAll(showKeyTasks);
            }
            catch (TaskCanceledException) { }
            _connectCancelationTokenSource.Dispose();
            _connectCancelationTokenSource = null;
        }

        private async Task UpdateKey(int key, int delay, CancellationToken cancellationToken)
        {
            await Task.Delay(delay, cancellationToken);
            var bitmap = _bindings.FirstOrDefault(b => b.Key == key)?.GetKeyBitmap(_font, _streamDeck.Keys.KeySize, false) ?? KeyBitmap.Black;
            _streamDeck.SetKeyBitmap(key, bitmap);
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
                _streamDeck.ShowLogo();
                _streamDeck.Dispose();
                _connectCancelationTokenSource?.Cancel();
                _streamDeck = null;
            }
            if (_font != null)
            {
                _font.Dispose();
                _font = null;
            }
        }
    }
}
