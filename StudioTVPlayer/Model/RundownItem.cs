using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class RundownItem : IDisposable
    {
        private bool _isAutoStart;
        private bool _enabled = true;
        private int _preloaded;

        public RundownItem(Media media)
        {
            Media = media;
        }

        public event EventHandler Stopped;

        public event EventHandler<TVPlayR.TimeEventArgs> FramePlayed;

        public event EventHandler AutoStartChanged;

        public Media Media { get; }

        internal TVPlayR.InputFile InputFile { get; private set; }

        public bool IsAutoStart
        {
            get => _isAutoStart;
            set
            {
                if (_isAutoStart == value)
                    return;
                _isAutoStart = value;
                AutoStartChanged?.Invoke(this, EventArgs.Empty);
            }
        }

        public bool Enabled
        {
            get => _enabled;
            set
            {
                if (_enabled == value)
                    return;
                _enabled = value;
                AutoStartChanged?.Invoke(this, EventArgs.Empty);
            }
        }

        public bool Preloaded { get; internal set; }

        public void Unload()
        {
            if (InputFile is null)
                return;
            InputFile.FramePlayed -= InputFile_FramePlayed;
            InputFile.Stopped -= InputFile_Stopped;
            InputFile.Dispose();
            InputFile = null;
            Interlocked.Exchange(ref _preloaded, default);
        }

        public void Preload(int audioChannelCount)
        {
            if (Interlocked.Exchange(ref _preloaded, 1) != default)
                return;
            InputFile = new TVPlayR.InputFile(Media.FullPath, audioChannelCount);
            InputFile.FramePlayed += InputFile_FramePlayed;
            InputFile.Stopped += InputFile_Stopped;
        }

        public void Play()
        {
            InputFile.Play();
        }

        internal void Pause()
        {
            InputFile.Pause();
        }

        public bool Seek(TimeSpan timeSpan)
        {
            return InputFile.Seek(timeSpan);
        }

        public void Dispose()
        {
            Unload();
        }

        private void InputFile_Stopped(object sender, EventArgs e)
        {
            Stopped?.Invoke(this, EventArgs.Empty);
        }

        private void InputFile_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            FramePlayed?.Invoke(this, e);
        }

    }
}
