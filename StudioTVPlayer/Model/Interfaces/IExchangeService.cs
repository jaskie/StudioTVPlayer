using System;
using StudioTVPlayer.Model.Args;
using StudioTVPlayer.ViewModel.Main.Piloting.Browser;

namespace StudioTVPlayer.Model.Interfaces
{
    public interface IExchangeService
    {
        event EventHandler<ModifiedEventArgs> NotifyOnConfigurationIsModifiedChanged;
        void RaiseConfigurationIsModifiedChanged(ModifiedEventArgs arg);

        event EventHandler<ConfigurationChangedEventArgs> NotifyOnConfigurationChanged;
        void RaiseConfigurationChanged(ConfigurationChangedEventArgs arg);

        event EventHandler<BrowserItemEventArgs> NotifyOnSelectedMediaChanged;
        void RaiseSelectedMediaChanged(BrowserItemEventArgs arg);

        void AddToPlayerQueueByIndex(int index, BrowserTabItemViewModel item);
        void AddToPlayerQueueByChannelID(int id, BrowserTabItemViewModel item);
    }
}
