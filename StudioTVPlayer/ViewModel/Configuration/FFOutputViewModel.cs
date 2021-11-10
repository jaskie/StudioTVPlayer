﻿using StudioTVPlayer.Model;

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
            _videoCodec = streamOutput.VideoCodec;
            _audioCodec = streamOutput.AudioCodec;
            _videoBitrate = streamOutput.VideoBitrate;
            _audioBitrate = streamOutput.AudioBitrate;
            _videoFilter = streamOutput.VideoFilter;
            _outputMetadata = streamOutput.OutputMetadata;
            _audioMetadata = streamOutput.AudioMetadata;
            _videoMetadata = streamOutput.VideoMetadata;
            _options = streamOutput.Options;
            _videoStreamId = streamOutput.VideoStreamId;
            _audioStreamId = streamOutput.AudioStreamId;
        }

        public string Url { get => _url; set => Set(ref _url, value); }
        public string VideoCodec { get => _videoCodec; set => Set(ref _videoCodec, value); }
        public string AudioCodec { get => _audioCodec; set => Set(ref _audioCodec, value); }
        public int VideoBitrate { get => _videoBitrate; set => Set(ref _videoBitrate, value); }
        public int AudioBitrate { get => _audioBitrate; set => Set(ref _audioBitrate, value); }
        public string VideoFilter { get => _videoFilter; set => Set(ref _videoFilter, value); }
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
            _streamOutput.VideoCodec = _videoCodec;
            _streamOutput.AudioCodec = _audioCodec;
            _streamOutput.VideoBitrate = _videoBitrate;
            _streamOutput.AudioBitrate = _audioBitrate;
            _streamOutput.VideoFilter = _videoFilter;
            _streamOutput.OutputMetadata = _outputMetadata;
            _streamOutput.AudioMetadata = _audioMetadata;
            _streamOutput.VideoMetadata  =_videoMetadata;
            _streamOutput.Options = _options;
            _streamOutput.VideoStreamId = _videoStreamId;
            _streamOutput.AudioStreamId = _audioStreamId;
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
