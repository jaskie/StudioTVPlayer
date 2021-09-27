using System;
using System.Linq;
using System.Xml.Serialization;

namespace TimecodeDecoderService
{
    public enum Keyer
    {
        Internal,
        Passthrough
    }

    [XmlType("channel")]
    public class Channel: IDisposable
    {

        private TVPlayR.Channel _channel;
        private TVPlayR.DecklinkInput _decklinkInput;
        private TVPlayR.DecklinkOutput _decklinkOutput;
        private TVPlayR.TimecodeOverlay _timecodeOverlay;

        [XmlAttribute]
        public int Input { get; set; }

        [XmlAttribute]
        public int Output { get; set; }

        [XmlAttribute]
        public Keyer Keyer { get; set; }

        [XmlAttribute]
        public string VideoFormat { get; set; }

        [XmlAttribute]
        public TVPlayR.DecklinkTimecodeSource TcSource { get; set; }

        public void Dispose()
        {
            if (!(_decklinkOutput is null))
            {
                _channel.RemoveOutput(_decklinkOutput);
                _decklinkOutput.Dispose();
                _decklinkOutput = null;
            }
            if (!(_channel is null))
            {
                _channel.Clear();
                _channel?.Dispose();
                _channel = null;
            }
            _decklinkInput?.Dispose();
            _decklinkInput = null;
            _timecodeOverlay?.Dispose();
            _timecodeOverlay = null;
        }

        public void Start()
        {
            var format = TVPlayR.VideoFormat.Formats.FirstOrDefault(f => f.Name.Equals(VideoFormat, StringComparison.OrdinalIgnoreCase))
                ?? throw new ApplicationException("Video format not found");
            var inputDecklink = TVPlayR.DecklinkIterator.Devices.ElementAtOrDefault(Input)
                ?? throw new ApplicationException("Input Decklink not found");
            var outputDecklink = TVPlayR.DecklinkIterator.Devices.ElementAtOrDefault(Output)
                ?? throw new ApplicationException("Output Decklink not found");
            _channel = new TVPlayR.Channel($"Input {Input} Output {Output} Keyer {Keyer} Format {VideoFormat}", format, TVPlayR.PixelFormat.bgra, 2);
            _decklinkInput = TVPlayR.DecklinkIterator.CreateInput(inputDecklink, format, 2, TcSource, Keyer != Keyer.Internal);
            _decklinkOutput = TVPlayR.DecklinkIterator.CreateOutput(outputDecklink, Keyer == Keyer.Internal);
            _timecodeOverlay = new TVPlayR.TimecodeOverlay(format, _channel.PixelFormat, Keyer == Keyer.Internal);
            _channel.AddOverlay(_timecodeOverlay);
            _channel.AddOutput(_decklinkOutput, true);
            _channel.Load(_decklinkInput);
        }
    }
}