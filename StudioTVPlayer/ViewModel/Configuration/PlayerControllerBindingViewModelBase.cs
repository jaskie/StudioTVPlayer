using StudioTVPlayer.Model;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public abstract class PlayerControllerBindingViewModelBase : RemovableViewModelBase, IDataErrorInfo
    {
        private PlayerMethodKind _playerMethod;
        private PlayerViewModel _player;
        private bool _isListening;

        public PlayerControllerBindingViewModelBase(Model.Configuration.PlayerBindingBase playerBindingConfiguration)
        {
            BindingConfiguration = playerBindingConfiguration;
            _player = Players.FirstOrDefault();
        }

        public string this[string columnName] => ReadErrorInfo(columnName);

        public string Error => null;

        public PlayerViewModel Player { get => _player; set => Set(ref _player, value); }
        public IEnumerable<PlayerViewModel> Players => ConfigurationViewModel.Instance.Players.Players;

        public PlayerMethodKind PlayerMethod { get => _playerMethod; set => Set(ref _playerMethod, value); }
        public static Array PlayerMethods { get; } = Enum.GetValues(typeof(PlayerMethodKind));

        public bool IsListening { get => _isListening; set => Set(ref _isListening, value); }

        public Model.Configuration.PlayerBindingBase BindingConfiguration { get; }

        public override void Apply()
        {
            BindingConfiguration.PlayerMethod = _playerMethod;
            BindingConfiguration.PlayerId = _player.Player.Id;
            base.Apply();
        }

        public override bool IsValid()
        {
            return Player != null;
        }

        protected virtual string ReadErrorInfo(string columnName)
        {
            switch (columnName)
            {
                case nameof(Player) when Player is null:
                    return "Please select player";
            }
            return string.Empty;
        }

    }
}
