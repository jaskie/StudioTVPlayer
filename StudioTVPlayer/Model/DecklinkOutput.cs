using System.Diagnostics;
using System.Linq;

namespace StudioTVPlayer.Model
{
    public class DecklinkOutput: OutputBase
    {
        private TVPlayR.DecklinkOutput _output;
        private readonly Configuration.DecklinkOutput _configuration;

        public DecklinkOutput(Configuration.DecklinkOutput configuration): base(configuration)
        {
            _configuration = configuration;
        }

        public override void Dispose()
        {
            base.Dispose();
            _output?.Dispose();
            _output = null;
        }

        public override TVPlayR.OutputBase Output => _output;

        public override void Initialize(TVPlayR.Player player)
        {
            var info = TVPlayR.DecklinkIterator.Devices.FirstOrDefault(i => i.Index == _configuration.DeviceIndex);
            _output = info is null ? null : TVPlayR.DecklinkIterator.CreateOutput(info, _configuration.Keyer, _configuration.TimecodeSource);
            base.Initialize(player);
            Debug.WriteLine("Decklink output initialized");
        }
    }
}
