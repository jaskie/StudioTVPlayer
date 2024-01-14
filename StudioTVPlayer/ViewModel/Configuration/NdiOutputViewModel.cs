using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class NdiOutputViewModel : OutputViewModelBase
    {
        private Model.Configuration.NdiOutput _ndi;
        private string _groupNames;
        private string _sourceName;

        public NdiOutputViewModel(Model.Configuration.NdiOutput ndiConfiguration) : base(ndiConfiguration)
        {
            _ndi = ndiConfiguration;
            _groupNames = ndiConfiguration.GroupNames;
            _sourceName = ndiConfiguration.SourceName;
        }

        public string GroupNames { get => _groupNames; set => Set(ref _groupNames, value); }
        
        public string SourceName { get => _sourceName; set => Set(ref _sourceName, value); }

        public override void Apply()
        {
            if (!IsModified)
                return;
            base.Apply();
            _ndi.GroupNames = GroupNames;
            _ndi.SourceName = SourceName;
        }

        protected override string ReadErrorInfo(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(SourceName) when string.IsNullOrWhiteSpace(SourceName):
                    return "Source name can't be empty";
                case nameof(SourceName) when SourceName.Any(b => b >= 127):
                    return "Source name can contain only alphanumeric characters";
                case nameof(GroupNames) when GroupNames?.Any(b => b >= 127) == true:
                    return "Group names can contain only alphanumeric characters";
            }
            return base.ReadErrorInfo(propertyName);
        }

        public override bool IsValid()
        {
            return string.IsNullOrEmpty(this[nameof(SourceName)]) && string.IsNullOrEmpty(this[nameof(GroupNames)]);
        }
    }
}
