using StudioTVPlayer.Helpers;
using StudioTVPlayer.Providers;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ConfigurationViewModel : ModifyableViewModelBase
    {
      
        public ConfigurationViewModel()
        {
            WatchedFolders = new WatchedFoldersViewModel();
            WatchedFolders.PropertyChanged += Item_PropertyChanged;
            Channels = new ChannelsViewModel();
            Channels.PropertyChanged += Item_PropertyChanged;

            SaveConfigurationCommand = new UiCommand(SaveConfiguration, _ => IsModified && IsValid());
            CancelCommand = new UiCommand(Cancel);
        }

  
        private void SaveConfiguration(object obj)
        {
            Apply();
            GlobalApplicationData.Current.SaveConfiguration();
            MainViewModel.Instance.Initialize();
        }

        public UiCommand SaveConfigurationCommand { get; }

        public UiCommand CancelCommand { get; }

        public WatchedFoldersViewModel WatchedFolders { get; } 

        public ChannelsViewModel Channels { get; }

        
        private void Cancel(object obj)
        {
            MainViewModel.Instance.Initialize();
        }

        public override void Apply()
        {
            WatchedFolders.Apply();
            Channels.Apply();
            IsModified = false;
        }

        private void Item_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(IsModified))
                IsModified = true;
        }

        public override bool IsValid()
        {
            return WatchedFolders.IsValid() && Channels.IsValid();
        }
    }
}
