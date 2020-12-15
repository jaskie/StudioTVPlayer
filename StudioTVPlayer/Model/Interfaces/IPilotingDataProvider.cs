using System.Collections.Generic;
using StudioTVPlayer.ViewModel.Main.Piloting.Player;

namespace StudioTVPlayer.Model.Interfaces
{
    public interface IPilotingDataProvider
    {
        List<PlayerViewModel> GetPlayers();
    }
}
