using StudioTVPlayer.Helpers;
using System;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ConfigurationViewModel : ModifyableViewModelBase
    {
      
        public ConfigurationViewModel()
        {
            WatchedFolders = new WatchedFoldersViewModel();
            WatchedFolders.Modified += Item_Modified;
            Players = new PlayersViewModel();
            Players.Modified += Item_Modified;

            SaveConfigurationCommand = new UiCommand(SaveConfiguration, _ => IsModified && IsValid());
            CancelCommand = new UiCommand(Cancel);
        }

  
        private void SaveConfiguration(object obj)
        {
            Apply();
            MainViewModel.Instance.ShowPlayoutView();
        }

        public UiCommand SaveConfigurationCommand { get; }

        public UiCommand CancelCommand { get; }

        public WatchedFoldersViewModel WatchedFolders { get; } 

        public PlayersViewModel Players { get; }

        
        private void Cancel(object obj)
        {
            MainViewModel.Instance.ShowPlayoutView();
        }

        public override void Apply()
        {
            WatchedFolders.Apply();
            Players.Apply();
            Providers.Configuration.Current.Save();
            IsModified = false;
        }

        private void Item_Modified(object sender, EventArgs e) => IsModified = true;

        public override bool IsValid()
        {
            return WatchedFolders.IsValid() && Players.IsValid();
        }
    }
}
