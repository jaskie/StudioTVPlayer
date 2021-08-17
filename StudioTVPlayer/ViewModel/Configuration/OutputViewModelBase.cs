using StudioTVPlayer.Model;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class OutputViewModelBase : RemovableViewModelBase
    {
        private readonly OutputBase _output;
        private bool _isFrameClock;

        public OutputViewModelBase(OutputBase output)
        {
            _output = output;
            _isFrameClock = output.IsFrameClock;
        }

        public bool IsFrameClock { get => _isFrameClock; set => Set(ref _isFrameClock, value); }

        public override void Apply()
        {
            if (!IsModified)
                return;
            _output.IsFrameClock = _isFrameClock;
        }

        public override bool IsValid()
        {
            return true;
        }
    }
}
