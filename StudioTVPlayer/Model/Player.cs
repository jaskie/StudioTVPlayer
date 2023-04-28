using StudioTVPlayer.Model.Args;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public class Player : IDisposable
    {
        private TVPlayR.Player _player;
        private TVPlayR.PreviewSink _outputPreview;
        private List<OutputBase> _outputs = new List<OutputBase>();
        private bool _disablePlayedItems;
        private bool _addItemsWithAutoPlay;
        private TVPlayR.VideoFormat _videoFormat;
        private TVPlayR.PixelFormat _pixelFormat;

        public Player(Configuration.Player configuration)
        {
            Configuration = configuration;
        }
        
        public Configuration.Player Configuration { get; }

        public string Name => Configuration.Name;

        public TVPlayR.VideoFormat VideoFormat
        {
            get => _videoFormat;
            private set
            {
                if (_videoFormat == value)
                    return;
                _videoFormat = value;
                Uninitialize();
            }
        }

        public TVPlayR.PixelFormat PixelFormat
        {
            get => _pixelFormat; 
            private set
            {
                if (_pixelFormat == value)
                    return;
                _pixelFormat = value;
                Uninitialize();
            }
        }

        public bool LivePreview => Configuration.LivePreview;

        public bool DisablePlayedItems { get => _disablePlayedItems; set => _disablePlayedItems = value; }

        public bool AddItemsWithAutoPlay { get => _addItemsWithAutoPlay; set => _addItemsWithAutoPlay = value; }

        public bool IsInitialized => _player != null;

        public int AudioChannelCount { get; } = 2;

        public virtual void Initialize()
        {
            var newVideoFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(f => f.Name == Configuration.VideoFormat);
            VideoFormat = newVideoFormat;
            PixelFormat = Configuration.PixelFormat;
            _player = new TVPlayR.Player(Name, VideoFormat, PixelFormat, AudioChannelCount);
            foreach (var outputConfiguration in Configuration.Outputs)
            {
                OutputBase output;
                switch (outputConfiguration)
                {
                    case Configuration.DecklinkOutput decklink:
                        output = new DecklinkOutput(decklink);
                        break;
                    case Configuration.NdiOutput ndi:
                        output = new NdiOutput(ndi);
                        break;
                    case Configuration.FFOutput ff:
                        output = new FFOutput(ff);
                        break;
                    default:
                        throw new ApplicationException("Invalid output configuration type");
                }
                output.Initialize(_player);
                if (output.IsFrameClock)
                    _player.SetFrameClockSource(output.Output);
                _player.AddOutputSink(output.Output);
                _outputs.Add(output);
            }
            _player.AudioVolume += Player_AudioVolume;
        }

        public void SetVolume(float value)
        {
            if (_player == null)
                throw new ApplicationException($"Player {Name} not initialized");
            _player.Volume = (float)Math.Pow(10, value / 20);
        }

        public ImageSource GetPreview(int width, int height)
        {
            if (_player == null)
                throw new ApplicationException($"Player {Name} not initialized");
            if (_outputPreview is null)
            {
                _outputPreview = new TVPlayR.PreviewSink(Application.Current.Dispatcher, width, height);
                _player.AddOutputSink(_outputPreview);
            }
            return _outputPreview.PreviewSource;
        }

        public void Uninitialize()
        {
            var player = _player;
            _player = null;
            if (player == null)
                return;
            foreach (var o in _outputs)
            {
                player.RemoveOutputSink(o.Output);
                o.Dispose();
            }
            _outputs.Clear(); ;
            if (!(_outputPreview is null))
            {
                _outputPreview.Dispose();
                _outputPreview = null;
            }
            player.AudioVolume -= Player_AudioVolume;
            player.Clear();
            player.Dispose();
        }

        protected void Load(TVPlayR.InputBase item)
        {
            Debug.Assert(item != null);
            _player.Load(item);
        }

        protected void PlayNext(TVPlayR.InputBase item)
        {
            Debug.Assert(item != null);
            _player.PlayNext(item);
        }

        public virtual void Clear()
        {
            _player.Clear();
        }

        public event EventHandler<AudioVolumeEventArgs> AudioVolume;

        private void Player_AudioVolume(object sender, TVPlayR.AudioVolumeEventArgs e)
        {
            AudioVolume?.Invoke(this, new AudioVolumeEventArgs(e.AudioVolume));
        }

        public virtual void Dispose()
        {
            Uninitialize();
        }

    }
}
