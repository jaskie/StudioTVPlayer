using System;
using System.Collections.Generic;
using System.Linq;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Providers;
using StudioTVPlayer.ViewModel.Main.MediaBrowser;
using StudioTVPlayer.ViewModel.Main.Player;

namespace StudioTVPlayer.ViewModel.Main
{
    public class PlayoutViewModel : ViewModelBase, IDisposable
    {
        public PlayoutViewModel()
        {
            Players = GlobalApplicationData.Current.Players.Select(p => new MediaPlayerViewModel(p)).ToArray();
            Browsers = GlobalApplicationData.Current.Configuration.WatchedFolders.Select(f => new BrowserViewModel(f)).ToArray();
            FocusPlayerCommand = new UiCommand(FocusPlayer);
            FocusBrowserCommand = new UiCommand(FocusBrowser);
        }

        public UiCommand FocusPlayerCommand { get; }
        public UiCommand FocusBrowserCommand { get; }

        public MediaPlayerViewModel[] Players { get; }

        public BrowserViewModel[] Browsers { get; }

        private void FocusBrowser(object obj)
        {
        }

        private void FocusPlayer(object obj)
        {
        }

        public void Dispose()
        {
            foreach (var player in Players)
                player.Dispose();
            foreach (var browser in Browsers)
                browser.Dispose();
        }

    }
}
