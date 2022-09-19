using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Controls;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Providers;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class PlayersViewModel : ModifyableViewModelBase
    {
        private readonly MahApps.Metro.Controls.Dialogs.IDialogCoordinator _dialogCoordinator = MahApps.Metro.Controls.Dialogs.DialogCoordinator.Instance;

        private PlayerViewModel _selectedPlayer;

        public PlayersViewModel()
        {
            AddPlayerCommand = new UiCommand(AddPlayer);
            DeletePlayerCommand = new UiCommand(DeletePlayer, CanDeletePlayer);
            UnloadedCommand = new UiCommand((param) => { CommitChanges(param); });
            Players = new ObservableCollection<PlayerViewModel>(Providers.Configuration.Current.Players.Select(c => new PlayerViewModel(c)));
            foreach (var player in Players)
            {
                player.Modified += (o, e) => IsModified = true;
                player.RemoveRequested += Player_RemoveRequested;
                player.CheckErrorInfo += Player_CheckErrorInfo;
            }
            _selectedPlayer = Players.FirstOrDefault();
        }

        private async void Player_RemoveRequested(object sender, EventArgs e)
        {
            var player = sender as PlayerViewModel ?? throw new ArgumentException(nameof(sender));
            if (await _dialogCoordinator.ShowMessageAsync(MainViewModel.Instance, "Confirmation", $"Really remove player \"{player.Name}\"?", MahApps.Metro.Controls.Dialogs.MessageDialogStyle.AffirmativeAndNegative) != MahApps.Metro.Controls.Dialogs.MessageDialogResult.Affirmative)
                return;
            Players.Remove(player);
            IsModified = true;
        }

        public UiCommand AddPlayerCommand { get; }
        public UiCommand DeletePlayerCommand { get; }
        public UiCommand UnloadedCommand { get; }
        public PlayerViewModel SelectedPlayer
        {
            get => _selectedPlayer;
            set
            {
                if (_selectedPlayer == value)
                    return;
                _selectedPlayer = value;
                NotifyPropertyChanged();
            }
        }

        public ObservableCollection<PlayerViewModel> Players { get; }

        private void CommitChanges(object param)
        {
            if (param != null)
            {
                object[] parameters = param as object[];
                DataGrid dg = parameters[0] as DataGrid;
                dg.CommitEdit(DataGridEditingUnit.Row, true);
            }
        }
       
        private bool CanDeletePlayer(object obj)
        {
            if (_selectedPlayer != null) return true;
            return false;
        }

        private void DeletePlayer(object obj)
        {
            var player = obj as PlayerViewModel ?? throw new ArgumentException(nameof(obj));
            Players.Remove(player);
        }

        private void AddPlayer(object obj)
        {
            var player = new RundownPlayer { Name = $"Player {Players.Count + 1}" };
            var vm = new PlayerViewModel(player);
            vm.RemoveRequested += Player_RemoveRequested;
            vm.CheckErrorInfo += Player_CheckErrorInfo;
            Players.Add(vm);
            SelectedPlayer = vm;
            IsModified = true;
        }

        public override void Apply()
        {
            foreach (var player in Players)
                player.Apply();
            GlobalApplicationData.Current.UpdatePlayers(Players.Select(vm => vm.Player).ToList());
        }

        public override bool IsValid()
        {
            return Players.All(c => c.IsValid());
        }

        private void Player_CheckErrorInfo(object sender, CheckErrorEventArgs e)
        {
            switch (e.Source)
            {
                case PlayerViewModel player 
                when e.PropertyName == nameof(PlayerViewModel.Name) && Players.Any(p => p != player && p.Name == player.Name):
                    e.Message = "Player of that name already exists";
                    break;
                case DecklinkOutputViewModel decklink 
                when e.PropertyName == nameof(DecklinkOutputViewModel.SelectedDevice) &&
                Players.Any(c => c.Outputs.Any(o => o is DecklinkOutputViewModel decklinkToCompare && decklinkToCompare != decklink && decklink.SelectedDevice == decklinkToCompare.SelectedDevice)):
                    e.Message = "This device is already in use";
                    break;
                case NdiOutputViewModel ndi when 
                e.PropertyName == nameof(NdiOutputViewModel.SourceName) &&
                Players.Any(c => c.Outputs.Any(o => o is NdiOutputViewModel ndiToCompare && ndiToCompare != ndi && ndi.SourceName == ndiToCompare.SourceName)):
                    e.Message = "This source name is in use";
                    break;
            }
        }

    }
}
