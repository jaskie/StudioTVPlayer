using StudioTVPlayer.Helpers;
using System;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public sealed class ConfigurationViewModel : ModifyableViewModelBase, IDisposable
    {
        internal static ConfigurationViewModel Instance { get; private set; }

        public ConfigurationViewModel()
        {
            Instance = this;
            WatchedFolders = new WatchedFoldersViewModel();
            WatchedFolders.Modified += Item_Modified;
            Players = new PlayersViewModel();
            Players.Modified += Item_Modified;
            PlayerControllers = new PlayerControllersViewModel();
            PlayerControllers.Modified += Item_Modified;
            SaveConfigurationCommand = new UiCommand(SaveConfiguration, _ => IsModified && IsValid());
            CancelCommand = new UiCommand(Cancel);
        }

        private async void SaveConfiguration(object obj)
        {
            try
            {
                Apply();
                MainViewModel.Instance.ShowPlayoutView();
            }
            catch (Exception e)
            {
                await MainViewModel.Instance.ShowMessageAsync("Configuration error", $"Could not initialize this configuration:\n\n{(e.InnerException ?? e).Message}");
            }
        }

        public UiCommand SaveConfigurationCommand { get; }

        public UiCommand CancelCommand { get; }

        public WatchedFoldersViewModel WatchedFolders { get; } 

        public PlayersViewModel Players { get; }

        public PlayerControllersViewModel PlayerControllers { get; }

        private void Cancel(object obj)
        {
            MainViewModel.Instance.ShowPlayoutView();
        }

        public override void Apply()
        {
            WatchedFolders.Apply();
            Players.Apply();
            PlayerControllers.Apply();
            Providers.Configuration.Current.Save();
            base.Apply();
        }

        private void Item_Modified(object sender, EventArgs e) => IsModified = true;

        public override bool IsValid()
        {
            return WatchedFolders.IsValid() && Players.IsValid() && PlayerControllers.IsValid();
        }

        public void Dispose()
        {
            PlayerControllers.Dispose();
        }
    }
}
