using StudioTVPlayer.Model.Configuration;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class FFOutputViewModel : OutputViewModelBase
    {
        private string _url;
        private string _videoCodec;
        private string _audioCodec;
        private int _videoBitrate;
        private int _audioBitrate;
        private string _videoFilter;
        private string _pixelFormat;
        private string _outputMetadata;
        private string _audioMetadata;
        private string _videoMetadata;
        private string _options;
        private int _videoStreamId;
        private int _audioStreamId;
        private readonly FFOutput _streamOutput;

        public FFOutputViewModel(FFOutput streamOutput) : base(streamOutput)
        {
            _streamOutput = streamOutput;
            _url = streamOutput.Url;
            _videoCodec = streamOutput.EncoderSettings.VideoCodec;
            _audioCodec = streamOutput.EncoderSettings.AudioCodec;
            _videoBitrate = streamOutput.EncoderSettings.VideoBitrate;
            _audioBitrate = streamOutput.EncoderSettings.AudioBitrate;
            _videoFilter = streamOutput.EncoderSettings.VideoFilter;
            _pixelFormat = streamOutput.EncoderSettings.PixelFormat;
            _outputMetadata = streamOutput.EncoderSettings.OutputMetadata;
            _audioMetadata = streamOutput.EncoderSettings.AudioMetadata;
            _videoMetadata = streamOutput.EncoderSettings.VideoMetadata;
            _options = streamOutput.EncoderSettings.Options;
            _videoStreamId = streamOutput.EncoderSettings.VideoStreamId;
            _audioStreamId = streamOutput.EncoderSettings.AudioStreamId;
        }

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
            _streamOutput.Url = _url;
            _streamOutput.EncoderSettings = new Model.EncoderSettings
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
            switch (propertyName)
            {
                case nameof(Url) when string.IsNullOrWhiteSpace(Url):
                    return "Destination address can't be empty";
            }
            return base.ReadErrorInfo(propertyName);
        }

        public override bool IsValid()
        {
            return string.IsNullOrEmpty(this[nameof(Url)]);
        }
    }
}
