namespace StudioTVPlayer.Model
{
    public class FFOutput : OutputBase
    {
        private TVPlayR.FFOutput _ffOutput;

        public override void Dispose()
        {
            if (_ffOutput is null)
                return;
            base.Dispose();
            _ffOutput.Dispose();
            _ffOutput = null;
        }

        public string Url { get; set; }

        public EncoderSettings EncoderSettings { get; set; }

        public override TVPlayR.OutputBase Output => _ffOutput;

        public override void Initialize(TVPlayR.Player player)
        {
            _ffOutput = new TVPlayR.FFOutput(
                Url,
                EncoderSettings.VideoCodec, EncoderSettings.AudioCodec,
                EncoderSettings.VideoBitrate, EncoderSettings.AudioBitrate,
                EncoderSettings.Options,
                EncoderSettings.VideoFilter,
                EncoderSettings.OutputMetadata, EncoderSettings.VideoMetadata, EncoderSettings.AudioMetadata,
                EncoderSettings.VideoStreamId, EncoderSettings.AudioStreamId
                );
            base.Initialize(player);
        }
    }
}
