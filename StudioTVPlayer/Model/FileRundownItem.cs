using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Threading;

namespace StudioTVPlayer.Model
{
    public class FileRundownItem : RundownItemBase
    {
        private bool _isAutoStart;
        private bool _isDisabled;
        private bool _isLoop;
        private int _preloaded;

        public FileRundownItem(MediaFile media)
        {
            Media = media;
        }

        public event EventHandler Stopped;

        public MediaFile Media { get; }

        internal TVPlayR.FileInput FileInput { get; private set; }

        public override bool IsPlaying => FileInput?.IsPlaying == true;
        
        public bool IsAutoStart
        {
            get => _isAutoStart;
            set
            {
                if (_isAutoStart == value)
                    return;
                _isAutoStart = value;
                RaisePropertyChanged();
            }
        }

        public bool IsDisabled
        {
            get => _isDisabled;
            set
            {
                if (_isDisabled == value)
                    return;
                _isDisabled = value;
                RaisePropertyChanged();
            }
        }

        public bool IsLoop
        {
            get => _isLoop; 
            set
            {
                if (_isLoop == value)
                    return;
                _isLoop = value;
                if (!(FileInput is null))
                    FileInput.IsLoop = value;
                RaisePropertyChanged();
            }
        }


        public bool Preloaded => _preloaded != default;

        public override bool Preload(int audioChannelCount)
        {
            if (Interlocked.Exchange(ref _preloaded, 1) != default)
                return false;
            FileInput = new TVPlayR.FileInput(Media.FullPath);
            FileInput.IsLoop = IsLoop;
            FileInput.FramePlayed += Input_FramePlayed;
            FileInput.Stopped += InputFile_Stopped;
            return true;
        }

        public override void Play()
        {
            FileInput.Play();
        }

        public override void Unload()
        {
            if (Interlocked.Exchange(ref _preloaded, default) == default)
                return;
            FileInput.FramePlayed -= Input_FramePlayed;
            FileInput.Stopped -= InputFile_Stopped;
            FileInput.Dispose();
            FileInput = null;
        }


        internal override void Pause()
        {
            FileInput.Pause();
        }

        public bool Seek(TimeSpan timeSpan)
        {
            return FileInput.Seek(timeSpan);
        }

        private void InputFile_Stopped(object sender, EventArgs e)
        {
            Stopped?.Invoke(this, EventArgs.Empty);
        }
    }
}
