namespace StudioTVPlayer.Model
{
    public class NdiOutput(Configuration.NdiOutput configuration) : OutputBase(configuration)
    {
        private TVPlayR.NdiOutput _outputDevice;

        public string SourceName => configuration.SourceName;

        public string GroupNames => configuration.GroupNames;

        public override TVPlayR.OutputBase Output => _outputDevice;

        public override void Initialize(TVPlayR.Player player)
        {
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
