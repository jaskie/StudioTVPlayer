namespace StudioTVPlayer.Model
{
    public class NdiOutput: OutputBase
    {
        private TVPlayR.NdiOutput _outputDevice;
        private readonly Configuration.NdiOutput _configuration;

        public NdiOutput(Configuration.NdiOutput configuration) : base(configuration)
        {
            _configuration = configuration;
        }

        public string SourceName => _configuration.SourceName;

        public string GroupNames => _configuration.GroupNames;

        public override TVPlayR.OutputBase Output => _outputDevice;

        public override void Initialize(TVPlayR.Player player)
        {
            _outputDevice?.Dispose();
            _outputDevice = new TVPlayR.NdiOutput(SourceName, GroupNames);
            base.Initialize(player);
        }

        public override void Dispose()
        {
            if (_outputDevice is null)
                return;
            base.Dispose();
            _outputDevice.Dispose();
            _outputDevice = null;
        }
    }
}
