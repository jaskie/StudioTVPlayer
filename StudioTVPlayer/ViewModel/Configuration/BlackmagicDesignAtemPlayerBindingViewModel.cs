using LibAtem.Common;
using StudioTVPlayer.Model;
using System;
using System.Diagnostics;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public sealed class BlackmagicDesignAtemPlayerBindingViewModel : PlayerControllerBindingViewModelBase
    {
        private MixEffectBlockId _me;
        private VideoSource _videoSource;
        private BlackmagicDesignAtemCommand _atemCommand;

        public BlackmagicDesignAtemPlayerBindingViewModel(Model.Configuration.BlackmagicDesignAtemPlayerBinding bindingConfiguration = null) : base(bindingConfiguration ?? new Model.Configuration.BlackmagicDesignAtemPlayerBinding())
        {
            if (bindingConfiguration != null)
            {
                _me = bindingConfiguration.Me;
                _atemCommand = bindingConfiguration.Command;
                _videoSource = bindingConfiguration.VideoSource;
            }
        }

        public override void Apply()
        {
            var bindingConfiguration = BindingConfiguration as Model.Configuration.BlackmagicDesignAtemPlayerBinding;
            Debug.Assert(bindingConfiguration != null);
            bindingConfiguration.VideoSource = _videoSource;
            bindingConfiguration.Me = _me;
            bindingConfiguration.Command = _atemCommand;
            base.Apply();
        }

        public MixEffectBlockId Me { get => _me; set => Set(ref _me, value); }
        public Array Mes { get; } = Enum.GetValues(typeof(MixEffectBlockId));

        public VideoSource VideoSource { get => _videoSource; set => Set(ref _videoSource, value); }
        public static Array VideoSources { get; } = Enum.GetValues(typeof(VideoSource));

        public BlackmagicDesignAtemCommand AtemCommand { get => _atemCommand; set => Set(ref _atemCommand, value); }
        public static Array AtemCommands { get; } = Enum.GetValues(typeof(BlackmagicDesignAtemCommand));

        internal void AtemCommandReceived(BlackmagicDesignAtemCommand atemCommand, MixEffectBlockId me, VideoSource videoSource)
        {
            if (!IsListening)
                return;
            AtemCommand = atemCommand;
            Me = me;
            VideoSource = videoSource;
            IsListening = false;
        }

    }
}
