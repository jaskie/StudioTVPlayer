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

        public override bool IsValid()
        {
            return base.IsValid() 
                && !string.IsNullOrWhiteSpace(_sourceName) && _sourceName.All(b => b < 127) // is not empty and all chars are ascii
                && (string.IsNullOrEmpty(_groupName) ||  _groupName.All(b =>  b < 127)); // can be empty, otherwise all chars should be ascii
        }
    }
}
