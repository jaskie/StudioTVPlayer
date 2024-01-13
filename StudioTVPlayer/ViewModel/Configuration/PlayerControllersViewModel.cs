using StudioTVPlayer.Helpers;
using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class PlayerControllersViewModel : ModifyableViewModelBase, IDisposable
    {
        private readonly Model.BlackmagicDesignAtemDiscovery _blackmagicDesignAtemDiscovery;
        private PlayerControllerViewModelBase _selectedPlayerController;

        public PlayerControllersViewModel()
        {
            _blackmagicDesignAtemDiscovery = new Model.BlackmagicDesignAtemDiscovery();
            _blackmagicDesignAtemDiscovery.DeviceSeen += BlackmagicDesignAtemDiscovery_DevicesChanged;
            _blackmagicDesignAtemDiscovery.DeviceLost += BlackmagicDesignAtemDiscovery_DevicesChanged;
            PlayerControllers = new ObservableCollection<PlayerControllerViewModelBase>(Providers.Configuration.Current.PlayerControllers.Select(playerControllerConfiguration =>
            {
                switch (playerControllerConfiguration)
                {
                    case Model.Configuration.BlackmagicDesignAtemPlayerController blackmagicDecklinkPlayerController:
                        return new BlackmagicDesignAtemPlayerControllerViewModel(_blackmagicDesignAtemDiscovery, blackmagicDecklinkPlayerController);
                    default:
                        throw new NotImplementedException();
                }
            }));
            AddBlackmagicDesignAtemPlayerControllerCommand = new UiCommand(AddPlayerController);
        }

        public override void Apply()
        {
            Providers.Configuration.Current.PlayerControllers = PlayerControllers
                .Select(vm =>
                        {
                            vm.Apply();
                            return vm.PlayerController;
                        })
                .ToList();
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

        public PlayerControllerViewModelBase SelectedPlayerController { get => _selectedPlayerController; set => Set(ref _selectedPlayerController, value); }

        public ICommand AddBlackmagicDesignAtemPlayerControllerCommand { get; }

        private void BlackmagicDesignAtemDiscovery_DevicesChanged(object sender, Model.BlackmagicAtemDeviceEventArgs e)
        {
            foreach (var blackmagicDecklinkPlayerControllerViewModel in PlayerControllers.OfType<BlackmagicDesignAtemPlayerControllerViewModel>())
                blackmagicDecklinkPlayerControllerViewModel.NotifyDevicesChanged();
        }

        private void AddPlayerController(object obj)
        {
            var vm = new BlackmagicDesignAtemPlayerControllerViewModel(_blackmagicDesignAtemDiscovery);
            vm.Modified += PlayerController_Modified;
            vm.RemoveRequested += PlayerController_RemoveRequested;
            PlayerControllers.Add(vm);
        }

        private void PlayerController_RemoveRequested(object sender, EventArgs e)
        {
            var vm = sender as PlayerControllerViewModelBase ?? throw new ArgumentException(nameof(sender));
            if (!PlayerControllers.Remove(vm))
                throw new ApplicationException("PlayerController was not in list");
            vm.RemoveRequested -= PlayerController_RemoveRequested;
            vm.Modified -= PlayerController_Modified;
            IsModified = true;
        }

        private void PlayerController_Modified(object sender, EventArgs e)
        {
            IsModified = true;
        }
    }
}
