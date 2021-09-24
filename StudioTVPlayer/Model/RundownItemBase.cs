using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using TVPlayR;

namespace StudioTVPlayer.Model
{
    public abstract class RundownItemBase: INotifyPropertyChanged, IDisposable
    {

        public abstract bool IsPlaying { get; }

        public event EventHandler<TimeEventArgs> FramePlayed;
        public event EventHandler RemoveRequested;
        public event PropertyChangedEventHandler PropertyChanged;

        public void Dispose()
        {
            Unload();
        }

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