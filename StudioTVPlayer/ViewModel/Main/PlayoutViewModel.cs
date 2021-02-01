using System;
using System.Collections.Generic;
using System.Linq;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.ViewModel.Main.MediaBrowser;
using StudioTVPlayer.ViewModel.Main.Player;

namespace StudioTVPlayer.ViewModel.Main
{
    public class PlayoutViewModel : ViewModelBase, IDisposable
    {
        public PlayoutViewModel(IEnumerable<MediaPlayer> players)
        {
            Players = players.Select(p => new MediaPlayerViewModel(p)).ToList();
            Browsers = new BrowsersViewModel();
            FocusPlayerCommand = new UiCommand(FocusPlayer);
            FocusBrowserCommand = new UiCommand(FocusBrowser);

        }

        public UiCommand FocusPlayerCommand { get; }
        public UiCommand FocusBrowserCommand { get; }

        public List<MediaPlayerViewModel> Players { get; }

        public BrowsersViewModel Browsers { get; }


        private void FocusBrowser(object obj)
        {
        }

        private void FocusPlayer(object obj)
        {
        }

        public void Dispose()
        {
            Players.ForEach(p => p.Dispose());
            Browsers.Dispose();
        }

    }
}
