using LibAtem.Common;
using StudioTVPlayer.Model;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class BlackmagicDesignAtemPlayerBindingViewModel : RemovableViewModelBase, IDataErrorInfo, ICheckErrorInfo
    {
        private bool _isAtemListening;
        private MixEffectBlockId _me;
        private VideoSource _videoSource;
        private BlackmagicDesignAtemCommand _atemCommand;
        private PlayerMethodKind _playerMethod;
        private PlayerViewModel _player;
        public readonly Model.Configuration.BlackmagicDesignAtemPlayerBinding BingingConfiguration;

        public BlackmagicDesignAtemPlayerBindingViewModel(Model.Configuration.BlackmagicDesignAtemPlayerBinding bingingConfiguration = null)
        {
            BingingConfiguration = bingingConfiguration ?? new Model.Configuration.BlackmagicDesignAtemPlayerBinding();
            _playerMethod = BingingConfiguration.PlayerMethod;
            _player = Players.FirstOrDefault(player => player.Player.Id == BingingConfiguration.PlayerId);
            _me = BingingConfiguration.Me;
            _atemCommand = BingingConfiguration.Command;
            _videoSource = BingingConfiguration.VideoSource;
        }

        public string this[string columnName] => ReadErrorInfo(columnName);

        public string Error => null;

        public event EventHandler<CheckErrorEventArgs> CheckErrorInfo;

        public override bool IsValid()
        {
            return true;
        }

        public override void Apply()
        {
            BingingConfiguration.PlayerMethod = _playerMethod;
            BingingConfiguration.PlayerId = _player.Player.Id;
            BingingConfiguration.VideoSource = _videoSource;
            BingingConfiguration.Me = _me;
            BingingConfiguration.Command = _atemCommand;
            base.Apply();
        }

        public PlayerViewModel Player { get => _player; set => Set(ref _player, value); }
        public IEnumerable<PlayerViewModel> Players => ConfigurationViewModel.Instance.Players.Players;

        public PlayerMethodKind PlayerMethod { get => _playerMethod; set => Set(ref _playerMethod, value); }
        public static Array PlayerMethods { get; } = Enum.GetValues(typeof(PlayerMethodKind));

        public MixEffectBlockId Me { get => _me; set => Set(ref _me, value); }
        public Array Mes { get; } = Enum.GetValues(typeof(MixEffectBlockId));

        public VideoSource VideoSource { get => _videoSource; set => Set(ref _videoSource, value); }
        public static Array VideoSources { get; } = Enum.GetValues(typeof(VideoSource));

        public BlackmagicDesignAtemCommand AtemCommand { get => _atemCommand; set => Set(ref _atemCommand, value); }
        public static Array AtemCommands { get; } = Enum.GetValues(typeof(BlackmagicDesignAtemCommand));

        public bool IsAtemListening { get => _isAtemListening; set => Set(ref _isAtemListening, value); }

        internal void AtemCommandReceived(BlackmagicDesignAtemCommand atemCommand, MixEffectBlockId me, VideoSource videoSource)
        {
            if (!IsAtemListening)
                return;
            AtemCommand = atemCommand;
            Me = me;
            VideoSource = videoSource;
            IsAtemListening = false;
        }

        protected string ReadErrorInfo(string columnName)
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
