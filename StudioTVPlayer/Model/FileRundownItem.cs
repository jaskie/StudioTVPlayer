using System;
using System.Threading;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public class FileRundownItem : RundownItemBase
    {
        private bool _isLoop;
        private int _preloaded;
        private TVPlayR.FileInput _input;

        public FileRundownItem(MediaFile media)
        {
            Media = media;
        }

        public event EventHandler Stopped;

        public MediaFile Media { get; }

        public override TVPlayR.InputBase Input => _input; 

        public override bool IsPlaying => _input?.IsPlaying == true;


        public bool IsLoop
        {
            get => _isLoop;
            set
            {
                if (_isLoop == value)
                    return;
                _isLoop = value;
                if (!(_input is null))
                    _input.IsLoop = value;
                RaisePropertyChanged();
            }
        }

        public bool Preloaded => _preloaded != default;

        public override ImageSource Thumbnail => Media.Thumbnail;

        public override string Title => $"clip {Media.Name}";

        public override bool Preload(int audioChannelCount)
        {
            if (Interlocked.Exchange(ref _preloaded, 1) != default)
                return false;
            _input = new TVPlayR.FileInput(Media.FullPath);
            _input.IsLoop = IsLoop;
            _input.FramePlayed += Input_FramePlayed;
            _input.Stopped += InputFile_Stopped;
            return true;
        }

        public override void Play()
        {
            _input.Play();
        }

        public override void Unload()
        {
            if (Interlocked.Exchange(ref _preloaded, default) == default)
                return;
            _input.FramePlayed -= Input_FramePlayed;
            _input.Stopped -= InputFile_Stopped;
            _input.Dispose();
            _input = null;
        }


        internal override void Pause()
        {
            _input.Pause();
        }

        public bool Seek(TimeSpan timeSpan)
        {
            return _input.Seek(timeSpan);
        }

        private void InputFile_Stopped(object sender, EventArgs e)
        {
            Stopped?.Invoke(this, EventArgs.Empty);
        }
    }
}
