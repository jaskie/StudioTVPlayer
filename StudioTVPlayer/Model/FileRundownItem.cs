﻿using System;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public sealed class FileRundownItem : RundownItemBase
    {
        private bool _isLoop;
        private TVPlayR.FileInput _input;

        public FileRundownItem(MediaFile media)
        {
            Media = media;
        }

        public event EventHandler Paused;

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
            SubscribeToEvents();
            _input.IsLoop = IsLoop;
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

        protected override void SubscribeToEvents()
        {
            base.SubscribeToEvents();
            _input.Paused += InputFile_Paused;
        }

        protected override void UnsubscribeFromEvents()
        {
            _input.Paused -= InputFile_Paused;
            base.UnsubscribeFromEvents();
        }


        private void InputFile_Paused(object sender, EventArgs e)
        {
            Paused?.Invoke(this, EventArgs.Empty);
        }
    }
}
