using ControlzEx.Standard;
using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public abstract class RundownItemBase : INotifyPropertyChanged, IDisposable
    {
        private bool _isAutoStart;
        private bool _isDisabled;
        private int _prepared;

        public abstract bool IsPlaying { get; }

        public event EventHandler<TVPlayR.TimeEventArgs> FramePlayed;
        public event EventHandler RemoveRequested;
        public event EventHandler Loaded;
        public event PropertyChangedEventHandler PropertyChanged;

        public bool IsPrepared => _prepared != default;

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

        public abstract ImageSource Thumbnail { get; }

        internal abstract TVPlayR.InputBase TVPlayRInput { get; }

        public abstract string Name { get; }

        public abstract bool CanSeek { get; }

        public abstract void Play();

        public abstract void Pause();

        public virtual bool Prepare(int audioChannelCount)
        {
            if (Interlocked.Exchange(ref _prepared, 1) != default)
                return false;
            return true;
        }

        public void RemoveFromRundown()
        {
            RemoveRequested?.Invoke(this, EventArgs.Empty);
        }

        public virtual bool Unload()
        {
            if (Interlocked.Exchange(ref _prepared, default) == default)
                return false;
            return true;
        }

        protected abstract void Input_FramePlayed(object sender, TVPlayR.TimeEventArgs e);

        protected virtual void SubscribeToEvents(TVPlayR.InputBase input)
        {
            input.Loaded += Input_Loaded;
            input.FramePlayed += Input_FramePlayed;
        }

        protected virtual void UnsubscribeFromEvents(TVPlayR.InputBase input)
        {
            input.FramePlayed -= Input_FramePlayed;
            input.Loaded -= Input_Loaded;
        }

        protected void RaiseFramePlayed(TVPlayR.TimeEventArgs e)
        {
            FramePlayed?.Invoke(this, e);
        }

        private void Input_Loaded(object sender, EventArgs e)
        {
            Loaded?.Invoke(this, e);
        }

        protected void RaisePropertyChanged([CallerMemberName] string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }
    }
}