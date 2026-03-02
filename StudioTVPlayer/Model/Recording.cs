using StudioTVPlayer.Helpers;
using System;
using System.Diagnostics;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public sealed class Recording : PropertyChangedBase, IDisposable
    {
        private object _stopLock = new object();
        private bool _disposed;
        private TVPlayR.FFOutput _outputFile;
        private RecordingState _recordingState;
        private DateTime _startTime;
        private TimeSpan _duration;
        private ImageSource _thumbnail;
        private Stopwatch _stopwatch;

        public Recording(InputBase input, EncoderPreset encoderPreset, string fullPath)
        {
            Input = input;
            Thumbnail = input.Thumbnail;
            EncoderPreset = encoderPreset;
            FullPath = fullPath;
        }

        public Guid Id { get; } = Guid.NewGuid();

        public EncoderPreset EncoderPreset { get; }

        public string FullPath { get; }

        public ImageSource Thumbnail { get => _thumbnail; private set => Set(ref _thumbnail, value); }

        public InputBase Input { get; }

        public RecordingState State { get => _recordingState; private set => Set(ref _recordingState, value); }

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
            if (_recordingState is not RecordingState.Pending)
                return;
            _stopwatch = new Stopwatch();
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
            Providers.GlobalApplicationData.Current.AddRecording(this);
            StartTime = DateTime.Now;
            _stopwatch.Start();
            State = RecordingState.Running;
            Providers.RecordingPersister.SaveRecording(this);
        }

        public void Stop()
        {
            Stop(RecordingState.Completed);
        }

        private void Stop(RecordingState state)
        {
            lock (_stopLock)
            {
                if (_recordingState is not RecordingState.Running)
                    return;
                if (_outputFile is null)
                    return;
                Input.RemoveOutputSink(_outputFile);
                if (Input is DecklinkInput decklinkInput)
                {
                    decklinkInput.FormatChanged -= DecklinkInput_FormatChanged;
                }
                Input.TVPlayRInput.FramePlayed -= TVPlayRInput_FramePlayed;
                _stopwatch.Stop();
                _outputFile.Dispose();
                _outputFile = null;
            }
            VerifyAndSaveRecording(state);
        }

        private void TVPlayRInput_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            Duration = _stopwatch.Elapsed;
        }

        private void DecklinkInput_FormatChanged(object sender, EventArgs e)
        {
            Stop(RecordingState.Aborted); // can't continue recording with a different format
        }

        public void Delete()
        {
            Providers.RecordingPersister.DeleteRecording(this);
        }

        private void VerifyAndSaveRecording(RecordingState stateOnSuccess)
        {
            Media = new MediaFile(FullPath);
            MediaVerifier.Current.Verify(Media);

            Duration = Media.Duration;
            Thumbnail = Media.Thumbnail;
            State = Media.IsValid ? RecordingState.Completed : RecordingState.Failed;

            Providers.RecordingPersister.SaveRecording(this);
        }
    }
}