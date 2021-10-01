using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Media;
using TVPlayR;

namespace StudioTVPlayer.Model
{
    public abstract class RundownItemBase : INotifyPropertyChanged, IDisposable
    {
        private bool _isAutoStart;
        private bool _isDisabled;

        public abstract bool IsPlaying { get; }

        public event EventHandler<TimeEventArgs> FramePlayed;
        public event EventHandler RemoveRequested;
        public event PropertyChangedEventHandler PropertyChanged;

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

        public abstract void Play();

        public abstract bool Preload(int audioChannelCount);

        public void RemoveFromRundown()
        {
            RemoveRequested?.Invoke(this, EventArgs.Empty);
        }

        public abstract void Unload();

        internal abstract void Pause();

        protected void Input_FramePlayed(object sender, TimeEventArgs e)
        {
            FramePlayed?.Invoke(this, e);
        }

        protected void RaisePropertyChanged([CallerMemberName] string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }
    }
}