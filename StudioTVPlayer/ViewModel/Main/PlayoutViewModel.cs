﻿using System;
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
        public PlayoutViewModel()
        {
            Players = GlobalApplicationData.Current.RundownPlayers.Select(p => new PlayerViewModel(p)).ToArray();
            Browsers = Providers.Configuration.Current.WatchedFolders.Select(f => new BrowserViewModel(f)).ToArray();
            Inputs = new InputsViewModel();
            FocusPlayerCommand = new UiCommand(FocusPlayer);
            FocusBrowserCommand = new UiCommand(FocusBrowser);
        }

        public UiCommand FocusPlayerCommand { get; }
        public UiCommand FocusBrowserCommand { get; }

        public IReadOnlyCollection<PlayerViewModel> Players { get; }

        public IReadOnlyCollection<BrowserViewModel> Browsers { get; }

        public InputsViewModel Inputs { get; }

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
            foreach (var browser in Browsers)
                browser.Dispose();
            Inputs.Dispose();
        }

    }
}
