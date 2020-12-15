using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.ViewModel.Main.Piloting.Browser;
using StudioTVPlayer.ViewModel.Main.Piloting.Player;

namespace StudioTVPlayer.Services
{
    public class UIFocusService : IUIFocusService
    {
        public void FocusBrowser()
        {
            var browserTabsVM = SimpleIoc.GetInstances<BrowserTabViewModel>();

            foreach (var browserTab in browserTabsVM)
            {
                if (browserTab.IsFocused == true)
                    browserTab.IsFocused = false;

                browserTab.IsFocused = true;
            }
        }

        public void FocusPlayer(int index)
        {          
            List<PlayerViewModel> players = SimpleIoc.GetInstances<PlayerViewModel>();
            foreach (PlayerViewModel player in players)
            {
                player.IsFocused = false;
            }

            if (players.Count >= index)
                players[index - 1].IsFocused = true;
        }
    }
}
