namespace StudioTVPlayer.ViewModel.Configuration
{
    public class FFOutputViewModel(Model.Configuration.FFOutput streamOutputConfiguration) : OutputViewModelBase(streamOutputConfiguration)
    {
        private string _url = streamOutputConfiguration.Url;
        private string _videoCodec = streamOutputConfiguration.EncoderSettings.VideoCodec;
        private string _audioCodec = streamOutputConfiguration.EncoderSettings.AudioCodec;
        private int _videoBitrate = streamOutputConfiguration.EncoderSettings.VideoBitrate;
        private int _audioBitrate = streamOutputConfiguration.EncoderSettings.AudioBitrate;
        private string _videoFilter = streamOutputConfiguration.EncoderSettings.VideoFilter;
        private string _pixelFormat = streamOutputConfiguration.EncoderSettings.PixelFormat;
        private string _outputMetadata = streamOutputConfiguration.EncoderSettings.OutputMetadata;
        private string _audioMetadata = streamOutputConfiguration.EncoderSettings.AudioMetadata;
        private string _videoMetadata = streamOutputConfiguration.EncoderSettings.VideoMetadata;
        private string _options = streamOutputConfiguration.EncoderSettings.Options;
        private int _videoStreamId = streamOutputConfiguration.EncoderSettings.VideoStreamId;
        private int _audioStreamId = streamOutputConfiguration.EncoderSettings.AudioStreamId;

        public string Url { get => _url; set => Set(ref _url, value); }
        public string VideoCodec { get => _videoCodec; set => Set(ref _videoCodec, value); }
        public string AudioCodec { get => _audioCodec; set => Set(ref _audioCodec, value); }
        public int VideoBitrate { get => _videoBitrate; set => Set(ref _videoBitrate, value); }
        public int AudioBitrate { get => _audioBitrate; set => Set(ref _audioBitrate, value); }
        public string VideoFilter { get => _videoFilter; set => Set(ref _videoFilter, value); }
        public string PixelFormat { get => _pixelFormat; set => Set(ref _pixelFormat, value); }
        public string OutputMetadata { get => _outputMetadata; set => Set(ref _outputMetadata, value); }
        public string AudioMetadata { get => _audioMetadata; set => Set(ref _audioMetadata, value); }
        public string VideoMetadata { get => _videoMetadata; set => Set(ref _videoMetadata, value); }
        public string Options { get => _options; set => Set(ref _options, value); }
        public int VideoStreamId { get => _videoStreamId; set => Set(ref _videoStreamId, value); }
        public int AudioStreamId { get => _audioStreamId; set => Set(ref _audioStreamId, value); }

        public static string[] VideoCodecs => TVPlayR.FFOutput.VideoCodecs;
        public static string[] AudioCodecs => TVPlayR.FFOutput.AudioCodecs;

        public override void Apply()
        {
            base.Apply();
            streamOutputConfiguration.Url = _url;
            streamOutputConfiguration.EncoderSettings = new Model.EncoderSettings
            {
                VideoCodec = _videoCodec,
                AudioCodec = _audioCodec,
                VideoBitrate = _videoBitrate,
                AudioBitrate = _audioBitrate,
                VideoFilter = _videoFilter,
                PixelFormat = _pixelFormat,
                OutputMetadata = _outputMetadata,
                AudioMetadata = _audioMetadata,
                VideoMetadata = _videoMetadata,
                Options = _options,
                VideoStreamId = _videoStreamId,
                AudioStreamId = _audioStreamId
            };
        }
        protected override string ReadErrorInfo(string propertyName)
        {
            return propertyName switch
            {
                nameof(Url) when string.IsNullOrWhiteSpace(Url) => "Destination address can't be empty",
                _ => base.ReadErrorInfo(propertyName),
            };
        }

        public override bool IsValid()
        {
            return string.IsNullOrEmpty(this[nameof(Url)]);
        }
    }
}
