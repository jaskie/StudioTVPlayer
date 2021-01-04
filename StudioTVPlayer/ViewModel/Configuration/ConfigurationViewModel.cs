using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.ViewModel.Main;
using System;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ConfigurationViewModel : ModifyableViewModelBase
    {
      
        public ConfigurationViewModel()
        {
            WatchedFolders = new WatchedFoldersViewModel();
            WatchedFolders.PropertyChanged += Item_PropertyChanged;
            SaveConfigurationCommand = new UiCommand(SaveConfiguration);
            CancelCommand = new UiCommand(Cancel);
        }

        private void SaveConfiguration(object obj)
        {
            Apply();
            MainViewModel.Instance.SwitchToPlayout();
        }

        public UiCommand SaveConfigurationCommand { get; }

        public UiCommand CancelCommand { get; }

        public WatchedFoldersViewModel WatchedFolders { get; } = new WatchedFoldersViewModel();

        public ChannelsViewModel Channels { get; } = SimpleIoc.Get<ChannelsViewModel>();

        public ExtensionsViewModel Extensions { get; } = new ExtensionsViewModel();


        private void Cancel(object obj)
        {
            MainViewModel.Instance.SwitchToPlayout();
        }

        public override void Apply()
        {
            WatchedFolders.Apply();
            Channels.Apply();
            Extensions.Apply();
        }

        private void Item_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(IsModified))
                IsModified = true;
        }

    }
}
