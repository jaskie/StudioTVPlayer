namespace StudioTVPlayer.Model
{
    public class FFOutput : OutputBase
    {
        private TVPlayR.FFOutput _ffOutput;

        public string Url { get; set; }
        public string VideoCodec { get; set; }
        public string AudioCodec { get; set; }
        public int VideoBitrate { get; set; }
        public int AudioBitrate { get; set; }
        public string VideoFilter { get; set; }
        public string OutputMetadata { get; set; }
        public string AudioMetadata { get; set; }
        public string VideoMetadata { get; set; }
        public string Options { get; set; }
        public int VideoStreamId { get; set; }
        public int AudioStreamId { get; set; }

        public override void Dispose()
        {
            if (_ffOutput is null)
                return;
            base.Dispose();
            _ffOutput.Dispose();
            _ffOutput = null;
        }

        public override TVPlayR.OutputBase Output => _ffOutput;

        public override void Initialize(TVPlayR.Channel channel)
        {
            _ffOutput = new TVPlayR.FFOutput(
                Url,
                VideoCodec, AudioCodec,
                VideoBitrate, AudioBitrate,
                Options, 
                VideoFilter,
                OutputMetadata, VideoMetadata, AudioMetadata,
                VideoStreamId, AudioStreamId
                );
            base.Initialize(channel);
        }
    }
}
