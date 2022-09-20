using StudioTVPlayer.Model.Args;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public class Player : INotifyPropertyChanged, IDisposable
    {
        private TVPlayR.Player _player;
        private TVPlayR.PreviewSink _outputPreview;

        private List<OutputBase> _outputs = new List<OutputBase>();
        private bool _disablePlayedItems;
        private bool _addItemsWithAutoPlay;

        public Player(Configuration.Player configuration)
        {
            Configuration = configuration;
            VideoFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(f => f.Name == configuration.Name);
            foreach (var outputConfiguration in configuration.Outputs)
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
                _outputs.Add(output);
            }
        }

        public Configuration.Player Configuration { get; }

        public string Name => Configuration.Name;

        public TVPlayR.VideoFormat VideoFormat { get; private set; }


        public TVPlayR.PixelFormat PixelFormat => Configuration.PixelFormat;

        public OutputBase[] Outputs
        {
            get => _outputs.ToArray();
            set
            {
                foreach (var output in _outputs)
                    output.Dispose();
                _outputs.Clear();
                if (value is null)
                    return;
                _outputs.AddRange(value);
            }
        }

        public bool LivePreview => Configuration.LivePreview;

        public bool DisablePlayedItems { get => _disablePlayedItems; set => _disablePlayedItems = value; }

        public bool AddItemsWithAutoPlay { get => _addItemsWithAutoPlay; set => _addItemsWithAutoPlay = value; }

        public bool IsInitialized => _player != null;

        public int AudioChannelCount { get; } = 2;
        
        public void Initialize()
        {
            if (_player != null)
                throw new ApplicationException($"Player {Name} already initialized");
            VideoFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(f => f.Name == Configuration.VideoFormat);
            _player = new TVPlayR.Player(Name, VideoFormat, PixelFormat, AudioChannelCount);
            foreach (var output in Outputs)
            {
                output.Initialize(_player);
                if (output.IsFrameClock)
                    _player.SetFrameClockSource(output.Output);
                _player.AddOutputSink(output.Output);
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
            if (_player == null)
                return;
            foreach (var o in Outputs)
            {
                _player.RemoveOutputSink(o.Output);
                o.Dispose();
            }
            if (!(_outputPreview is null))
            {
                _outputPreview.Dispose();
                _outputPreview = null;
            }
            _player.AudioVolume -= Player_AudioVolume;
            _player.Clear();
            _player.Dispose();
            _player = null;
        }

        public void Load(TVPlayR.InputBase item)
        {
            Debug.Assert(item != null);
            _player.Load(item);
        }

        public void Preload(TVPlayR.InputBase item)
        {
            Debug.Assert(item != null);
            _player.Preload(item);
        }

        public virtual void Clear()
        {
            _player.Clear();
        }

        public event PropertyChangedEventHandler PropertyChanged;
        public event EventHandler<AudioVolumeEventArgs> AudioVolume;

        private bool Set<T>(ref T field, T value, [CallerMemberName] string propertyname = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, value))
                return false;
            field = value;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
            return true;
        }

        private void Player_AudioVolume(object sender, TVPlayR.AudioVolumeEventArgs e)
        {
            AudioVolume?.Invoke(this, new AudioVolumeEventArgs(e.AudioVolume));
        }

        public virtual void Dispose()
        {
            if (!IsInitialized)
                return;
            Uninitialize();
        }

    }
}
