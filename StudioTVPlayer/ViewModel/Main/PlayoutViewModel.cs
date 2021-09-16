using System;
using System.Linq;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Providers;
using StudioTVPlayer.ViewModel.Main.Input;
using StudioTVPlayer.ViewModel.Main.MediaBrowser;
using StudioTVPlayer.ViewModel.Main.Player;

namespace StudioTVPlayer.ViewModel.Main
{
    public class PlayoutViewModel : ViewModelBase, IDisposable
    {
        public PlayoutViewModel()
        {
            Players = GlobalApplicationData.Current.Players.Select(p => new MediaPlayerViewModel(p)).ToArray();
            Browsers = Providers.Configuration.Current.WatchedFolders.Select(f => new BrowserViewModel(f)).ToArray();
            Inputs = new InputsViewModel();
            FocusPlayerCommand = new UiCommand(FocusPlayer);
            FocusBrowserCommand = new UiCommand(FocusBrowser);
        }

        public UiCommand FocusPlayerCommand { get; }
        public UiCommand FocusBrowserCommand { get; }

        public MediaPlayerViewModel[] Players { get; }

        public BrowserViewModel[] Browsers { get; }

        public InputsViewModel Inputs { get; }

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
