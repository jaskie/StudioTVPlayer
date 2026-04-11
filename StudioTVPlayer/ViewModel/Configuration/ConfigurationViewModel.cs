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
            Folders = new FoldersViewModel();
            Folders.Modified += Item_Modified;
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
                await ShellViewModel.Instance.ShowPlayoutView();
            }
            catch (Exception e)
            {
                await ShellViewModel.Instance.ShowMessageAsync("Configuration error", $"Could not initialize this configuration:\n\n{(e.InnerException ?? e).Message}");
            }
        }

        public UiCommand SaveConfigurationCommand { get; }

        public UiCommand CancelCommand { get; }

        public FoldersViewModel Folders { get; } 

        public PlayersViewModel Players { get; }

        public PlayerControllersViewModel PlayerControllers { get; }

        private void Cancel(object obj)
        {
            _ = ShellViewModel.Instance.ShowPlayoutView();
        }

        public override void Apply()
        {
            Folders.Apply();
            Players.Apply();
            PlayerControllers.Apply();
            Providers.Configuration.Current.Save();
            base.Apply();
        }

        private void Item_Modified(object sender, EventArgs e) => IsModified = true;

        public override bool IsValid()
        {
            return Folders.IsValid() && Players.IsValid() && PlayerControllers.IsValid();
        }

        public void Dispose()
        {
            PlayerControllers.Dispose();
        }
    }
}
