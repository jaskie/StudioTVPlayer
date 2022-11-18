using System;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public class FileRundownItem : RundownItemBase
    {
        private bool _isLoop;
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


        public override ImageSource Thumbnail => Media.Thumbnail;

        public override string Name => Media.Name;

        public override bool CanSeek => true;

        public override bool Prepare(int audioChannelCount)
        {
            if (!base.Prepare(audioChannelCount))
                return false;
            _input = new TVPlayR.FileInput(Media.FullPath);
            InputAdded(_input);
            _input.IsLoop = IsLoop;
            _input.Stopped += InputFile_Stopped;
            return true;
        }

        public override void Play()
        {
            _input.Play();
        }

        public override bool Unload()
        {
            if (!base.Unload())
                return false;
            InputRemoved(_input);
            _input.Stopped -= InputFile_Stopped;
            _input.Dispose();
            _input = null;
            return true;
        }

        public override void Pause()
        {
            _input?.Pause();
        }

        public bool Seek(TimeSpan timeSpan)
        {
            return _input?.Seek(timeSpan) ?? false;
        }

        protected override void Input_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            var timeEventArgs = new TVPlayR.TimeEventArgs(e.TimeFromBegin, Media.Duration - e.TimeFromBegin, e.Timecode);
            RaiseFramePlayed(timeEventArgs);
        }

        private void InputFile_Stopped(object sender, EventArgs e)
        {
            Stopped?.Invoke(this, EventArgs.Empty);
        }
    }
}
