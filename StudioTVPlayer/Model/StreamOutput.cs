namespace StudioTVPlayer.Model
{
    public class StreamOutput : OutputBase
    {
        private TVPlayR.StreamOutput _streamOutput;

        public string Address { get; set; }
        public string VideoCodec { get; set; }
        public string AudioCodec { get; set; }
        public int VideoBitrate { get; set; }
        public int AudioBitrate { get; set; }
        public string OutputFilter { get; set; }
        public string OutputMetadata { get; set; }
        public string AudioMetadata { get; set; }
        public string VideoMetadata { get; set; }
        public string Options { get; set; }
        public int VideoStreamId { get; set; }
        public int AudioStreamId { get; set; }

        public override void Dispose()
        {
            if (_streamOutput is null)
                return;
            _streamOutput.Dispose();
            _streamOutput = null;
        }

        public override TVPlayR.OutputBase GetOutput()
        {
            return _streamOutput;
        }

        public override void Initialize()
        {
            _streamOutput = new TVPlayR.StreamOutput(
                Address,
                VideoCodec,
                AudioCodec,
                VideoBitrate * 1024,
                AudioBitrate * 1024,
                OutputFilter,
                OutputMetadata,
                VideoMetadata,
                AudioMetadata,
                Options,
                VideoStreamId,
                AudioStreamId
                );
        }
    }
}
