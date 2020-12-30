using System.Collections.Generic;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.ViewModel.Main.Player;

namespace StudioTVPlayer.Providers
{
    public class PilotingDataProvider : IPilotingDataProvider
    {
        public List<PlayerViewModel> GetPlayers()
        {
            List<PlayerViewModel> Players = new List<PlayerViewModel>();
            foreach (Channel c in GlobalApplicationData.Current.Configuration.Channels)
            {
                PlayerViewModel temp = SimpleIoc.Get<PlayerViewModel>();
                temp.Channel = c;
                Players.Add(temp);
            }
            return Players;
        }
    }
}
