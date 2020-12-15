using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Args;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.ViewModel.Main.Piloting.Browser;
using StudioTVPlayer.ViewModel.Main.Piloting.Player;

namespace StudioTVPlayer.ViewModel.Main.Piloting
{
    public class PilotingViewModel : ViewModelBase
    {
        private IPilotingDataProvider _pilotingDataProvider;
        private IExchangeService _exchangeService;
        private IUIFocusService _uiFocusService;

        public UiCommand FocusPlayerCommand { get; set; }
        public UiCommand FocusBrowserCommand { get; set; }

        public ObservableCollection<PlayerViewModel> Players { get; }

        private void LoadCommands()
        {
            FocusPlayerCommand = new UiCommand(FocusPlayer);
            FocusBrowserCommand = new UiCommand(FocusBrowser);
        }

        private void FocusBrowser(object obj)
        {
            _uiFocusService.FocusBrowser();            
        }

        private void FocusPlayer(object obj)
        {
            if (obj == null)
                return;
            var parameter = Int32.Parse(obj.ToString());

            _uiFocusService.FocusPlayer(parameter);
        }

        public PilotingViewModel(IPilotingDataProvider pilotingDataProvider, IExchangeService vMNotifyService, IUIFocusService uiFocusService)
        {
            _exchangeService = vMNotifyService;
            _pilotingDataProvider = pilotingDataProvider;
            _uiFocusService = uiFocusService;
            Players = new ObservableCollection<PlayerViewModel>(_pilotingDataProvider.GetPlayers());
            _exchangeService.NotifyOnConfigurationChanged += ConfigurationChanged;
            LoadCommands();
        }

        private void ConfigurationChanged(object sender, ConfigurationChangedEventArgs e)
        {                     
            foreach(PlayerViewModel player in Players.ToList())
            {
                var changedChannel = e.Configuration.Channels.FirstOrDefault(param => param.ID == player.Channel.ID);

                if (changedChannel == null)
                {
                    player.Dispose();
                    Players.Remove(player);
                    continue;
                }
                else
                {
                    if (!(changedChannel.Device.ID == player.Channel.Device.ID &&
                        changedChannel.VideoFormat.Id == player.Channel.VideoFormat.Id &&
                        changedChannel.PixelFormat == player.Channel.PixelFormat))
                    {
                        player.Dispose();
                        player.Channel = changedChannel;
                        player.Channel.Init(player.Channel.VideoFormat.Id, player.Channel.PixelFormat, player.Channel.AudioChannelsCount);
                        player.Channel.AddOutput(TVPlayR.DecklinkDevice.EnumerateDevices().FirstOrDefault(f => f.Index == player.Channel.Device.ID));
                    }
                   
                    player.Channel.Name = changedChannel.Name;
                    
                }
            }

            foreach(Channel c in e.Configuration.Channels)
            {
                var foundChannel = Players.FirstOrDefault(param => param.Channel.ID == c.ID);

                if (foundChannel != null)
                    continue;

                
                c.Init(c.VideoFormat.Id, c.PixelFormat, c.AudioChannelsCount);
                c.AddOutput(TVPlayR.DecklinkDevice.EnumerateDevices().FirstOrDefault(f => f.Index == c.Device.ID));
                PlayerViewModel newPlayer = SimpleIoc.GetInstance<PlayerViewModel>(true);
                newPlayer.Channel = c;
                Players.Add(newPlayer);
            }
        }
    }
}
