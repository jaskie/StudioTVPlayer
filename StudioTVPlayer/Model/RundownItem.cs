using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class RundownItem: IDisposable
    {
        public RundownItem(Media media)
        {
            Media = media;
        }

        public event EventHandler Stopped;

        public event EventHandler<TVPlayR.TimeEventArgs> FramePlayed;

        public Media Media { get; }

        internal TVPlayR.InputFile InputFile { get; private set; }

        public bool IsAutoStart { get; set; }

        public bool Enabled { get; set; } = true;

        public void Unload()
        {
            if (InputFile is null)
                return;
            InputFile.FramePlayed -= InputFile_FramePlayed;
            InputFile.Stopped -= InputFile_Stopped;
            InputFile.Dispose();
        }

        public void Preload(int audioChannelCount)
        {
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
