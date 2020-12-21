using System.Collections.Generic;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.ViewModel.Main.Piloting.Player;

namespace StudioTVPlayer.Providers
{
    public class PilotingDataProvider : IPilotingDataProvider
    {
        public List<PlayerViewModel> GetPlayers()
        {
            List<PlayerViewModel> Players = new List<PlayerViewModel>();
            foreach (Channel c in SimpleIoc.GetInstance<IGlobalApplicationData>().Configuration.Channels)
            {
                PlayerViewModel temp = SimpleIoc.GetInstance<PlayerViewModel>(true);
                temp.Channel = c;
                Players.Add(temp);
            }
            return Players;
        }
    }
}
