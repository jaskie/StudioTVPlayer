using System;
using System.Linq;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Args;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.ViewModel.Main.Browser;
using StudioTVPlayer.ViewModel.Main.Player;

namespace StudioTVPlayer.Services
{
    public class ExchangeService : IExchangeService
    {
        public event EventHandler<ConfigurationChangedEventArgs> NotifyOnConfigurationChanged;
        public event EventHandler<ModifiedEventArgs> NotifyOnConfigurationIsModifiedChanged;      
        public event EventHandler<BrowserItemEventArgs> NotifyOnSelectedMediaChanged;                

        public void RaiseConfigurationChanged(ConfigurationChangedEventArgs arg)
        {
            NotifyOnConfigurationChanged?.Invoke(this, arg);
        }

        public void RaiseConfigurationIsModifiedChanged(ModifiedEventArgs arg)
        {
            NotifyOnConfigurationIsModifiedChanged?.Invoke(this, arg);
        }
        
        public void RaiseSelectedMediaChanged(BrowserItemEventArgs arg)
        {
            NotifyOnSelectedMediaChanged?.Invoke(this, arg);
        }

        public void AddToPlayerQueueByIndex(int index, BrowserTabItemViewModel item)
        {          
            var players = SimpleIoc.GetInstances<PlayerViewModel>();
            if (index >= players.Count)
                return;

            players[index].MediaQueue.Add(new PlayerQueueItemViewModel(item));
        }

        public void AddToPlayerQueueByChannelID(int id, BrowserTabItemViewModel item)
        {           
            SimpleIoc.GetInstances<PlayerViewModel>().FirstOrDefault(param => param.Channel.Id == id).MediaQueue.Add(new PlayerQueueItemViewModel(item));
        }
    }
}
