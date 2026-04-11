namespace StudioTVPlayer.Model
{
    public class FFOutput(Configuration.FFOutput configuration) : OutputBase(configuration)
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

        public string Url => configuration.Url;

        public EncoderSettings EncoderSettings => configuration.EncoderSettings;

        public override TVPlayR.OutputBase Output => _ffOutput;

        public override void Initialize(TVPlayR.Player player)
        {
            _ffOutput = new TVPlayR.FFOutput(
                Url,
                EncoderSettings.VideoCodec, EncoderSettings.AudioCodec,
                EncoderSettings.VideoBitrate, EncoderSettings.AudioBitrate,
                EncoderSettings.Options,
                EncoderSettings.VideoFilter, EncoderSettings.PixelFormat,
                EncoderSettings.OutputMetadata, EncoderSettings.VideoMetadata, EncoderSettings.AudioMetadata,
                EncoderSettings.VideoStreamId, EncoderSettings.AudioStreamId,
                string.Empty
                );
            base.Initialize(player);
        }
    }
}
