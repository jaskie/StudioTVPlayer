using StudioTVPlayer.Helpers;
using System;
using System.ComponentModel;
using System.IO;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public sealed class Recording : PropertyChangedBase, IDisposable
    {
        private readonly object _stopLock = new();
        private bool _disposed;
        private TVPlayR.FFOutput _outputFile;
        private RecordingState _state;
        private DateTime _startTime;
        private TimeSpan _duration;
        private ImageSource _thumbnail;

        public Recording(InputBase input, EncoderPreset encoderPreset, string fullPath)
        {
            Input = input;
            Thumbnail = input.Thumbnail;
            EncoderPreset = encoderPreset;
            FullPath = fullPath;
            Id = Guid.NewGuid().ToString();
        }


        /// <summary>
        /// Creating the Recording instance from deserialized one
        /// </summary>
        /// <remarks>The constructor is intended to use only for finished recordings, the ongoing ones are kept in <see cref="Providers.GlobalApplicationData"/></remarks>
        public Recording(Persistence.Recording recording)
        {
            Id = recording.Id;
            FullPath = recording.File;
            Input = Providers.InputList.Current.Find(recording.InputId);
            _state = recording.State;
            _startTime = recording.GetStartTime();
            _duration = recording.GetDuration();
            switch (recording.State)
            {
                case RecordingState.Pending:
                    break;
                case RecordingState.Running:
                    break;
                case RecordingState.Completed:
                    break;
                case RecordingState.Failed:
                    break;
                case RecordingState.Aborted:
                    break;
            }
        }

        public event EventHandler VerificationCompleted;

        public string Id { get; }

        public EncoderPreset EncoderPreset { get; }

        public string FullPath { get; }

        public ImageSource Thumbnail { get => _thumbnail; private set => Set(ref _thumbnail, value); }

        public InputBase Input { get; }

        public RecordingState State { get => _state; private set => Set(ref _state, value); }

        public DateTime StartTime { get => _startTime; private set => Set(ref _startTime, value); }

        public TimeSpan Duration { get => _duration; private set => Set(ref _duration, value); }

        public MediaFile Media { get; private set; }

        public void Dispose()
        {
            if (!_disposed)
            {
                Stop();
                _disposed = true;
            }
        }

        public void Start()
        {
            if (_state is not RecordingState.Pending)
                return;
            _outputFile = new TVPlayR.FFOutput(
                address: FullPath,
                video_codec: EncoderPreset.VideoCodec,
                audio_codec: EncoderPreset.AudioCodec,
                video_bitrate: EncoderPreset.VideoBitrate,
                audio_bitrate: EncoderPreset.AudioBitrate,
                options: EncoderPreset.Options,
                video_filter: EncoderPreset.VideoFilter,
                pixel_format: EncoderPreset.PixelFormat,
                output_metadata: EncoderPreset.OutputMetadata,
                video_metadata: EncoderPreset.VideoMetadata,
                audio_metadata: EncoderPreset.AudioMetadata,
                video_stream_id: EncoderPreset.VideoStreamId,
                audio_stream_id: EncoderPreset.AudioStreamId,
                output_format: EncoderPreset.OutputFormat
                );
            Input.TVPlayRInput.FramePlayed += TVPlayRInput_FramePlayed;
            if (Input is DecklinkInput decklinkInput)
            {
                decklinkInput.FormatChanged += DecklinkInput_FormatChanged;
            }
            _outputFile.Initialize(Input.CurrentFormat(), TVPlayR.PixelFormat.yuv422, 2, 48000);
            Input.AddOutputSink(_outputFile);
            StartTime = DateTime.Now;
            State = RecordingState.Running;
            Providers.RecordingStore.Current.AddRunningRecording(this);
            Providers.RecordingStore.Current.SaveRecording(this);
        }

        public void Stop()
        {
            Stop(RecordingState.Completed);
        }

        private void Stop(RecordingState state)
        {
            lock (_stopLock)
            {
                if (_state is not RecordingState.Running)
                    return;
                if (_outputFile is null)
                    return;
                Input.RemoveOutputSink(_outputFile);
                if (Input is DecklinkInput decklinkInput)
                {
                    decklinkInput.FormatChanged -= DecklinkInput_FormatChanged;
                }
                Input.TVPlayRInput.FramePlayed -= TVPlayRInput_FramePlayed;
                _outputFile.Dispose();
                _outputFile = null;
            }
            VerifyAndSaveRecording(state);
        }

        private void TVPlayRInput_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            Duration = DateTime.Now - _startTime;
        }

        private void DecklinkInput_FormatChanged(object sender, EventArgs e)
        {
            Stop(RecordingState.Aborted); // can't continue recording with a different format
        }

        public void Delete()
        {
            Providers.RecordingStore.Current.DeleteRecording(this);
        }

        private void VerifyAndSaveRecording(RecordingState stateOnSuccess)
        {
            State = Verify() ? stateOnSuccess : RecordingState.Failed;
            Providers.RecordingStore.Current.SaveRecording(this);
        }

        public bool Verify()
        {
            if (Media is not null)
                return true;
            if (File.Exists(FullPath))
                try
                {
                    Media = new MediaFile(FullPath);
                    MediaVerifier.Current.Verify(Media);
                    Thumbnail = Media.Thumbnail;
                    Duration = Media.Duration;
                    return Media.IsValid;
                }
                catch { }
            return false;
        }

        public void QueueVerify()
        {
            if (Media is not null)
                return;
            Media = new MediaFile(FullPath);
            Media.PropertyChanged += Media_PropertyChanged;
            MediaVerifier.Current.Queue(Media);
        }

        private void Media_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            var media = sender as MediaFile ?? throw new ArgumentException($"{nameof(MediaFile)} expected, {sender?.GetType()} got.");
            switch(e.PropertyName)
            {
                case nameof(MediaFile.Duration):
                    Duration = media.Duration;
                    break;
                case nameof(MediaFile.Thumbnail):
                    Thumbnail = media.Thumbnail;
                    break;
                case nameof(MediaFile.IsVerified):
                    media.PropertyChanged -= Media_PropertyChanged;
                    VerificationCompleted?.Invoke(this, EventArgs.Empty);
                    break;
            }
        }
    }
}