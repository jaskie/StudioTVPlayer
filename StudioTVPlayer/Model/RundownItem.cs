using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Threading;

namespace StudioTVPlayer.Model
{
    public class RundownItem : INotifyPropertyChanged, IDisposable
    {
        private bool _isAutoStart;
        private bool _isDisabled;
        private int _preloaded;
        private bool _isLoop;

        public RundownItem(MediaFile media)
        {
            Media = media;
        }

        public event EventHandler Stopped;

        public event EventHandler<TVPlayR.TimeEventArgs> FramePlayed;

        public event EventHandler RemoveRequested;
        
        public event PropertyChangedEventHandler PropertyChanged;

        public MediaFile Media { get; }

        internal TVPlayR.FileInput FileInput { get; private set; }

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

        public bool IsPlaying => FileInput?.IsPlaying == true;

        public void Unload()
        {
            if (Interlocked.Exchange(ref _preloaded, default) == default)
                return;
            FileInput.FramePlayed -= InputFile_FramePlayed;
            FileInput.Stopped -= InputFile_Stopped;
            FileInput.Dispose();
            FileInput = null;
        }

        public void RemoveFromRundown()
        {
            RemoveRequested?.Invoke(this, EventArgs.Empty);
        }

        public bool Preload(int audioChannelCount)
        {
            if (Interlocked.Exchange(ref _preloaded, 1) != default)
                return false;
            FileInput = new TVPlayR.FileInput(Media.FullPath, audioChannelCount);
            FileInput.IsLoop = IsLoop;
            FileInput.FramePlayed += InputFile_FramePlayed;
            FileInput.Stopped += InputFile_Stopped;
            return true;
        }

        public void Play()
        {
            FileInput.Play();
        }

        internal void Pause()
        {
            FileInput.Pause();
        }

        public bool Seek(TimeSpan timeSpan)
        {
            return FileInput.Seek(timeSpan);
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

        private void RaisePropertyChanged([CallerMemberName] string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }
    }
}
