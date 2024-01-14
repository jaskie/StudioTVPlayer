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
            _blackmagicDesignAtemDiscovery.DeviceSeen += BlackmagicDesignAtemDiscovery_DeviceSeen;
            _blackmagicDesignAtemDiscovery.DeviceLost += BlackmagicDesignAtemDiscovery_DeviceLost;
            PlayerControllers = new ObservableCollection<PlayerControllerViewModelBase>(Providers.Configuration.Current.PlayerControllers.Select(playerControllerConfiguration =>
            {
                switch (playerControllerConfiguration)
                {
                    case Model.Configuration.BlackmagicDesignAtemPlayerController blackmagicDecklinkPlayerController:
                        var vm = new BlackmagicDesignAtemPlayerControllerViewModel(_blackmagicDesignAtemDiscovery, blackmagicDecklinkPlayerController);
                        vm.Modified += PlayerController_Modified;
                        vm.RemoveRequested += PlayerController_RemoveRequested;
                        vm.CheckErrorInfo += PlayerController_CheckErrorInfo;
                        return vm;
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
            base.Apply();
        }

        public override bool IsValid()
        {
            return PlayerControllers.All(x => x.IsValid());
        }

        public void Dispose()
        {
            _blackmagicDesignAtemDiscovery.DeviceSeen -= BlackmagicDesignAtemDiscovery_DeviceSeen;
            _blackmagicDesignAtemDiscovery.DeviceLost -= BlackmagicDesignAtemDiscovery_DeviceLost;
            _blackmagicDesignAtemDiscovery.Dispose();
        }

        public ObservableCollection<PlayerControllerViewModelBase> PlayerControllers { get; }

        public PlayerControllerViewModelBase SelectedPlayerController
        {
            get => _selectedPlayerController; set
            {
                if (_selectedPlayerController == value)
                    return;
                _selectedPlayerController = value;
                NotifyPropertyChanged();
            }
        }

        public ICommand AddBlackmagicDesignAtemPlayerControllerCommand { get; }

        private void BlackmagicDesignAtemDiscovery_DeviceSeen(object sender, Model.BlackmagicAtemDeviceEventArgs e)
        {
            foreach (var blackmagicDecklinkPlayerControllerViewModel in PlayerControllers.OfType<BlackmagicDesignAtemPlayerControllerViewModel>())
                blackmagicDecklinkPlayerControllerViewModel.NotifyDeviceSeen(e.Device);
        }

        private void BlackmagicDesignAtemDiscovery_DeviceLost(object sender, Model.BlackmagicAtemDeviceEventArgs e)
        {
            foreach (var blackmagicDecklinkPlayerControllerViewModel in PlayerControllers.OfType<BlackmagicDesignAtemPlayerControllerViewModel>())
                blackmagicDecklinkPlayerControllerViewModel.NotifyDeviceLost(e.Device);
        }

        private void AddPlayerController(object obj)
        {
            var vm = new BlackmagicDesignAtemPlayerControllerViewModel(_blackmagicDesignAtemDiscovery);
            vm.Modified += PlayerController_Modified;
            vm.RemoveRequested += PlayerController_RemoveRequested;
            vm.CheckErrorInfo += PlayerController_CheckErrorInfo;
            PlayerControllers.Add(vm);
            SelectedPlayerController = vm;
        }

        private void PlayerController_CheckErrorInfo(object sender, CheckErrorEventArgs e)
        {
            switch (e.Source)
            {
                case BlackmagicDesignAtemPlayerControllerViewModel blackmagicDesignAtemPlayerControllerViewModel
                    when e.PropertyName == nameof(BlackmagicDesignAtemPlayerControllerViewModel.SelectedDevice) && 
                        PlayerControllers
                            .OfType<BlackmagicDesignAtemPlayerControllerViewModel>()
                            .Any(controller => controller != blackmagicDesignAtemPlayerControllerViewModel && controller.Id == blackmagicDesignAtemPlayerControllerViewModel.Id):
                    e.Message = "This switcher is already in use for other player controller";
                    break;
            }
        }

        private void PlayerController_RemoveRequested(object sender, EventArgs e)
        {
            var vm = sender as PlayerControllerViewModelBase ?? throw new ArgumentException(nameof(sender));
            if (!PlayerControllers.Remove(vm))
                throw new ApplicationException("PlayerController was not in list");
            vm.RemoveRequested -= PlayerController_RemoveRequested;
            vm.Modified -= PlayerController_Modified;
            vm.CheckErrorInfo -= PlayerController_CheckErrorInfo;
            IsModified = true;
        }

        private void PlayerController_Modified(object sender, EventArgs e)
        {
            IsModified = true;
        }
    }
}
