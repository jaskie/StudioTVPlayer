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
        private bool _disposed;

        public PlayerControllersViewModel()
        {
            _blackmagicDesignAtemDiscovery = new Model.BlackmagicDesignAtemDiscovery();
            _blackmagicDesignAtemDiscovery.DeviceSeen += BlackmagicDesignAtemDiscovery_DeviceSeen;
            _blackmagicDesignAtemDiscovery.DeviceLost += BlackmagicDesignAtemDiscovery_DeviceLost;
            PlayerControllers = new ObservableCollection<PlayerControllerViewModelBase>(Providers.Configuration.Current.PlayerControllers.Select(playerControllerConfiguration =>
            {
                PlayerControllerViewModelBase vm;
                switch (playerControllerConfiguration)
                {
                    case Model.Configuration.BlackmagicDesignAtemPlayerController blackmagicDecklinkPlayerController:
                        vm = new BlackmagicDesignAtemPlayerControllerViewModel(_blackmagicDesignAtemDiscovery, blackmagicDecklinkPlayerController);
                        break;
                    case Model.Configuration.ElgatoStreamDeckPlayerController elgatoStreamDeckPlayerController:
                        vm = new ElgatoStreamDeckPlayerControllerViewModel(elgatoStreamDeckPlayerController);
                        break;
                    default:
                        throw new NotImplementedException();
                }
                SubscribePlayerControllerEvents(vm);
                return vm;
            }));
            AddBlackmagicDesignAtemPlayerControllerCommand = new UiCommand(_ => AddAndAndSelectPlayerController(new BlackmagicDesignAtemPlayerControllerViewModel(_blackmagicDesignAtemDiscovery)));
            AddElgatoStreamDeckPlayerControllerCommand = new UiCommand(_ => AddAndAndSelectPlayerController(new ElgatoStreamDeckPlayerControllerViewModel()));
        }

        public override void Apply()
        {
            Providers.Configuration.Current.PlayerControllers = PlayerControllers
                .Select(vm =>
                        {
                            vm.Apply();
                            return vm.PlayerControllerConfiguration;
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
            if (_disposed)
                return;
            _disposed = true;
            foreach (var vm in PlayerControllers)
            {
                UnsubscribePlayerControllerEvents(vm);
                (vm as IDisposable)?.Dispose();
            }
            _blackmagicDesignAtemDiscovery.DeviceSeen -= BlackmagicDesignAtemDiscovery_DeviceSeen;
            _blackmagicDesignAtemDiscovery.DeviceLost -= BlackmagicDesignAtemDiscovery_DeviceLost;
            _blackmagicDesignAtemDiscovery.Dispose();
        }

        public ObservableCollection<PlayerControllerViewModelBase> PlayerControllers { get; }

        public PlayerControllerViewModelBase SelectedPlayerController
        {
            get => _selectedPlayerController; set
            {
                var previouslySelectedController = _selectedPlayerController;
                if (_selectedPlayerController == value)
                    return;
                _selectedPlayerController = value;
                switch (previouslySelectedController)
                {
                    case BlackmagicDesignAtemPlayerControllerViewModel blackmagicDesignAtemPlayerController:
                        blackmagicDesignAtemPlayerController.Connect = false;
                        break;
                    case ElgatoStreamDeckPlayerControllerViewModel elgatoStreamDeckPlayerController:
                        elgatoStreamDeckPlayerController.Connect = false;
                        break;
                }
                NotifyPropertyChanged();
            }
        }

        public ICommand AddBlackmagicDesignAtemPlayerControllerCommand { get; }
        public ICommand AddElgatoStreamDeckPlayerControllerCommand { get; }

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

        private void AddAndAndSelectPlayerController(PlayerControllerViewModelBase vm)
        {
            SubscribePlayerControllerEvents(vm);
            PlayerControllers.Add(vm);
            SelectedPlayerController = vm;
        }

        private void SubscribePlayerControllerEvents(PlayerControllerViewModelBase vm)
        {
            vm.Modified += PlayerController_Modified;
            vm.RemoveRequested += PlayerController_RemoveRequested;
            vm.CheckErrorInfo += PlayerController_CheckErrorInfo;
        }

        private void UnsubscribePlayerControllerEvents(PlayerControllerViewModelBase vm)
        {
            vm.Modified -= PlayerController_Modified;
            vm.RemoveRequested -= PlayerController_RemoveRequested;
            vm.CheckErrorInfo -= PlayerController_CheckErrorInfo;
        }

        private void PlayerController_CheckErrorInfo(object sender, CheckErrorEventArgs e)
        {
            switch (e.Source)
            {
                case BlackmagicDesignAtemPlayerControllerViewModel blackmagicDesignAtemPlayerControllerViewModel
                    when e.PropertyName == nameof(BlackmagicDesignAtemPlayerControllerViewModel.SelectedDevice) && 
                        PlayerControllers
                            .OfType<BlackmagicDesignAtemPlayerControllerViewModel>()
                            .Any(controller => controller != blackmagicDesignAtemPlayerControllerViewModel && controller.DeviceId == blackmagicDesignAtemPlayerControllerViewModel.DeviceId) :
                    e.Message = "This switcher is already in use for other player controller";
                    break;
                case ElgatoStreamDeckPlayerControllerViewModel elgatoStreamDeckPlayerControllerViewModel
                        when e.PropertyName == nameof(ElgatoStreamDeckPlayerControllerViewModel.SelectedDevice) &&
                            PlayerControllers
                            .OfType<ElgatoStreamDeckPlayerControllerViewModel>()
                            .Any(panel => panel != elgatoStreamDeckPlayerControllerViewModel && panel.DevicePath == elgatoStreamDeckPlayerControllerViewModel.DevicePath) :
                    e.Message = "This panel is already in use for other player controller";
                    break;
            }
        }

        private void PlayerController_RemoveRequested(object sender, EventArgs e)
        {
            var vm = sender as PlayerControllerViewModelBase ?? throw new ArgumentException(nameof(sender));
            if (!PlayerControllers.Remove(vm))
                throw new ApplicationException("PlayerController was not in the list");
            UnsubscribePlayerControllerEvents(vm);
            IsModified = true;
        }

        private void PlayerController_Modified(object sender, EventArgs e)
        {
            IsModified = true;
        }
    }
}
