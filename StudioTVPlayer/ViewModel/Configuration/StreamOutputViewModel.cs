using StudioTVPlayer.Model;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class StreamOutputViewModel : OutputViewModelBase
    {
        private string _address;
        private string _videoCodec;
        private string _audioCodec;
        private int _videoBitrate;
        private int _audioBitrate;
        private string _outputFilter;
        private string _outputMetadata;
        private string _audioMetadata;
        private string _videoMetadata;
        private string _options;
        private int _videoStreamId;
        private int _audioStreamId;

        public StreamOutputViewModel(StreamOutput output) : base(output)
        { }

        public string Address { get => _address; set => Set(ref _address, value); }
        public string VideoCodec { get => _videoCodec; set => Set(ref _videoCodec, value); }
        public string AudioCodec { get => _audioCodec; set => Set(ref _audioCodec, value); }
        public int VideoBitrate { get => _videoBitrate; set => Set(ref _videoBitrate, value); }
        public int AudioBitrate { get => _audioBitrate; set => Set(ref _audioBitrate, value); }
        public string OutputFilter { get => _outputFilter; set => Set(ref _outputFilter, value); }
        public string OutputMetadata { get => _outputMetadata; set => Set(ref _outputMetadata, value); }
        public string AudioMetadata { get => _audioMetadata; set => Set(ref _audioMetadata, value); }
        public string VideoMetadata { get => _videoMetadata; set => Set(ref _videoMetadata, value); }
        public string Options { get => _options; set => Set(ref _options, value); }
        public int VideoStreamId { get => _videoStreamId; set => Set(ref _videoStreamId, value); }
        public int AudioStreamId { get => _audioStreamId; set => Set(ref _audioStreamId, value); }

        public static string[] VideoCodecs => TVPlayR.StreamOutput.VideoCodecs;
        public static string[] AudioCodecs => TVPlayR.StreamOutput.AudioCodecs;

        protected override string ReadErrorInfo(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(Address) when string.IsNullOrWhiteSpace(Address):
                    return "Destination address can't be empty";
            }
            return base.ReadErrorInfo(propertyName);
        }

        public override bool IsValid()
        {
            return string.IsNullOrEmpty(this[nameof(Address)]);
        }
    }
}
