using StudioTVPlayer.Model;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class PlayerQueueItemViewModel : ViewModelBase
    {

        public PlayerQueueItemViewModel(MediaPlayerQueueItem queueItem)
        {
            QueueItem = queueItem;
        }

        private bool _isLoaded;
        public bool IsLoaded
        {
            get => _isLoaded;
            set => Set(ref _isLoaded, value);
        }

        private bool _isDisabled;
        public bool IsDisabled
        {
            get => _isDisabled;
            set => Set(ref _isDisabled, value);
        }

        public MediaPlayerQueueItem QueueItem { get; }

    }
}
