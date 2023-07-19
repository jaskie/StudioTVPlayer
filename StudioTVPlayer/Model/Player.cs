using StudioTVPlayer.Model.Args;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Windows;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public abstract class Player : IDisposable
    {
        private TVPlayR.Player _player;
        private TVPlayR.PreviewSink _outputPreview;
        private readonly List<OutputBase> _outputs = new List<OutputBase>();
        private bool _addItemsWithAutoPlay;
        private TVPlayR.VideoFormat _videoFormat;
        private TVPlayR.PixelFormat _pixelFormat;
        private int _initialized;

        public event EventHandler<AudioVolumeEventArgs> AudioVolume;
        public event EventHandler Cleared;

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
            }
        }

        public bool LivePreview => Configuration.LivePreview;

        public bool AddItemsWithAutoPlay { get => _addItemsWithAutoPlay; set => _addItemsWithAutoPlay = value; }

        public bool IsInitialized => _initialized != default;

        public int AudioChannelCount { get; } = 2;

        public virtual void Initialize()
        {
            if (Interlocked.Exchange(ref _initialized, 1) != default)
                return;
            var newVideoFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(f => f.Name == Configuration.VideoFormat);
            VideoFormat = newVideoFormat;
            _pixelFormat = Configuration.PixelFormat;
            _addItemsWithAutoPlay = Configuration.AddItemsWithAutoPlay;
            _player = new TVPlayR.Player(Name, VideoFormat, _pixelFormat, AudioChannelCount);
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
            if (_initialized == default)
                return;
            _player.Volume = (float)Math.Pow(10, value / 20);
        }

        public ImageSource GetPreview(int width, int height)
        {
            if (_initialized == default)
                return null;
            if (_outputPreview is null)
            {
                _outputPreview = new TVPlayR.PreviewSink(Application.Current.Dispatcher, width, height);
                _player.AddOutputSink(_outputPreview);
            }
            return _outputPreview.PreviewSource;
        }

        public void Uninitialize()
        {
            if (Interlocked.Exchange(ref _initialized, default) == default)
                return;
            foreach (var o in _outputs)
            {
                _player.RemoveOutputSink(o.Output);
                o.Dispose();
            }
            _outputs.Clear(); ;
            if (!(_outputPreview is null))
            {
                _outputPreview.Dispose();
                _outputPreview = null;
            }
            _player.AudioVolume -= Player_AudioVolume;
            _player.Clear();
            _player.Dispose();
        }

        protected void Load(TVPlayR.InputBase item)
        {
            Debug.Assert(item != null);
            if (_initialized == default)
                return;
            _player.Load(item);
        }

        protected bool LoadNext(RundownItemBase rundownItem)
        {
            if (_initialized == default)
                return false;
            if (rundownItem.Prepare(AudioChannelCount))
            {
                _player.LoadNext(rundownItem.TVPlayRInput);
                rundownItem.Play();
                return true;
            }
            return false;
        }

        public bool IsAplha => _pixelFormat == TVPlayR.PixelFormat.bgra;
                
        public virtual void Clear()
        {
            if (_initialized == default)
                return;
            _player.Clear();
            Cleared?.Invoke(this, EventArgs.Empty);
        }

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
