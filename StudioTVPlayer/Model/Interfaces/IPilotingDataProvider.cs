using System.Collections.Generic;
using StudioTVPlayer.ViewModel.Main.Player;

namespace StudioTVPlayer.Model.Interfaces
{
    public interface IPilotingDataProvider
    {
        List<PlayerViewModel> GetPlayers();
    }
}
