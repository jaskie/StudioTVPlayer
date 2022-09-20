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

        public int DeviceIndex => _configuration.DeviceIndex;

        public TVPlayR.DecklinkKeyer Keyer => _configuration.Keyer;

        public override void Dispose()
        {
            if (_output is null)
                return;
            base.Dispose();
            _output.Dispose();
            _output = null;
        }

        public override TVPlayR.OutputBase Output => _output;

        public override void Initialize(TVPlayR.Player player)
        {
            var info = TVPlayR.DecklinkIterator.Devices.FirstOrDefault(i => i.Index == DeviceIndex);
            _output = info is null ? null : TVPlayR.DecklinkIterator.CreateOutput(info, Keyer);
            base.Initialize(player);
        }
    }
}
