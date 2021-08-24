using StudioTVPlayer.Model;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class NdiOutputViewModel : OutputViewModelBase
    {
        private NdiOutput _ndi;
        private string _groupName;
        private string _sourceName;

        public NdiOutputViewModel(NdiOutput ndi) : base(ndi)
        {
            _ndi = ndi;
            _groupName = ndi.GroupName;
            _sourceName = ndi.SourceName;
        }

        public string GroupName { get => _groupName; set => Set(ref _groupName, value); }
        
        public string SourceName { get => _sourceName; set => Set(ref _sourceName, value); }

        public override void Apply()
        {
            if (!IsModified)
                return;
            base.Apply();
            _ndi.GroupName = GroupName;
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
                case nameof(GroupName) when GroupName?.Any(b => b >= 127) == true:
                    return "Group name can contain only alphanumeric characters";
            }
            return base.ReadErrorInfo(propertyName);
        }

        public override bool IsValid()
        {
            return string.IsNullOrEmpty(this[nameof(SourceName)]) && string.IsNullOrEmpty(this[nameof(GroupName)]);
        }
    }
}
