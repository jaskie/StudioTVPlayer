using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Windows.Media;
using TVPlayR;

namespace StudioTVPlayer.Model
{
    public abstract class RundownItemBase : INotifyPropertyChanged, IDisposable
    {
        private bool _isAutoStart;
        private bool _isDisabled;
        private int _preloaded;

        public abstract bool IsPlaying { get; }

        public event EventHandler<TimeEventArgs> FramePlayed;
        public event EventHandler RemoveRequested;
        public event PropertyChangedEventHandler PropertyChanged;

        public bool Preloaded => _preloaded != default;

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

        public void Dispose()
        {
            Unload();
        }

        public abstract TVPlayR.InputBase Input { get; }

        public abstract ImageSource Thumbnail { get; }

        public abstract string Name { get; }

        public abstract bool CanSeek { get; }

        public abstract void Play();

        public abstract void Pause();

        public virtual bool Preload(int audioChannelCount)
        {
            if (Interlocked.Exchange(ref _preloaded, 1) == default)
                return false;
            return true;
        }

        public void RemoveFromRundown()
        {
            RemoveRequested?.Invoke(this, EventArgs.Empty);
        }

        public virtual bool Unload()
        {
            if (Interlocked.Exchange(ref _preloaded, default) == default)
                return false;
            return true;
        }

        private void Input_FramePlayed(object sender, TimeEventArgs e)
        {
            FramePlayed?.Invoke(this, e);
        }

        protected void InputAdded(TVPlayR.InputBase input)
        {
            input.FramePlayed += Input_FramePlayed;
        }

        protected void InputRemoved(TVPlayR.InputBase input)
        {
            input.FramePlayed -= Input_FramePlayed;
        }


        protected void RaisePropertyChanged([CallerMemberName] string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }
    }
}