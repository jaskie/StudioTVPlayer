using System;
using System.Collections.Generic;
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
        private bool _disposed;
        private ViewModelBase selectedBrowser;

        public PlayoutViewModel()
        {
            Players = GlobalApplicationData.Current.RundownPlayers.Select(p => new PlayerViewModel(p)).ToArray();
            Browsers = (InputList.Current.CanAddInput ?
                // add InputsViewModel to the list of browsers
                (new ViewModelBase[] { new InputsViewModel() }).Concat(Providers.Configuration.Current.WatchedFolders.Select(f => new BrowserViewModel(f))) :
                // otherwise, just add BrowserViewModel to the list of browsers
                Providers.Configuration.Current.WatchedFolders.Select(f => new BrowserViewModel(f)))
                .ToArray();
            SelectedBrowser = Browsers.FirstOrDefault(x => x is BrowserViewModel) ?? Browsers.FirstOrDefault();
            FocusPlayerCommand = new UiCommand(FocusPlayer);
            FocusBrowserCommand = new UiCommand(FocusBrowser);
        }

        public UiCommand FocusPlayerCommand { get; }
        public UiCommand FocusBrowserCommand { get; }

        public IReadOnlyCollection<PlayerViewModel> Players { get; }

        public IReadOnlyCollection<ViewModelBase> Browsers { get; }

        public ViewModelBase SelectedBrowser { get => selectedBrowser; set => Set(ref selectedBrowser, value); }

        private void FocusBrowser(object obj)
        {
        }

        private void FocusPlayer(object obj)
        {
        }

        public void Dispose()
        {
            if (_disposed) 
                return;
            _disposed = true;
            foreach (var player in Players)
                player.Dispose();
            foreach (var browser in Browsers.OfType<IDisposable>())
                browser.Dispose();
        }

    }
}
