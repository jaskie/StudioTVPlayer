using System;
using System.Collections.ObjectModel;
using System.Linq;
using static Microsoft.WindowsAPICodePack.Shell.PropertySystem.SystemProperties.System;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class PlayerControllersViewModel : ModifyableViewModelBase, IDisposable
    {
        private readonly Model.BlackmagicDesignAtemDiscovery _blackmagicDesignAtemDiscovery;

        public PlayerControllersViewModel()
        {
            _blackmagicDesignAtemDiscovery = new Model.BlackmagicDesignAtemDiscovery();
            _blackmagicDesignAtemDiscovery.DeviceSeen += BlackmagicDesignAtemDiscovery_DevicesChanged;
            _blackmagicDesignAtemDiscovery.DeviceLost += BlackmagicDesignAtemDiscovery_DevicesChanged;
            PlayerControllers = new ObservableCollection<PlayerControllerViewModelBase>(Providers.Configuration.Current.PlayerControllers.Select(x =>
            {
                switch (x)
                {
                    case Model.Configuration.BlackmagicDesignAtemPlayerController blackmagicDecklinkPlayerController:
                        return new BlackmagicDesignAtemPlayerControllerViewModel(blackmagicDecklinkPlayerController, _blackmagicDesignAtemDiscovery);
                    default:
                        throw new NotImplementedException();
                }
            }));
        }

        public override void Apply()
        {
            Providers.Configuration.Current.PlayerControllers = PlayerControllers.Select(f => {
                f.Apply();
                return f.PlayerController;
            }).ToList();
            IsModified = false;
        }

        public override bool IsValid()
        {
            return PlayerControllers.All(x => x.IsValid());
        }

        public void Dispose()
        {
            _blackmagicDesignAtemDiscovery.DeviceSeen -= BlackmagicDesignAtemDiscovery_DevicesChanged;
            _blackmagicDesignAtemDiscovery.DeviceLost -= BlackmagicDesignAtemDiscovery_DevicesChanged;
            _blackmagicDesignAtemDiscovery.Dispose();
        }

        public ObservableCollection<PlayerControllerViewModelBase> PlayerControllers { get; }

        private void BlackmagicDesignAtemDiscovery_DevicesChanged(object sender, Model.BlackmagicAtemDeviceEventArgs e)
        {
            foreach (var blackmagicDecklinkPlayerController in PlayerControllers.OfType<BlackmagicDesignAtemPlayerControllerViewModel>())
                blackmagicDecklinkPlayerController.NotifyDevicesChanged();
        }


    }
}
