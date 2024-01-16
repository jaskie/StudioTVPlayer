using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public abstract class PlayerControllerViewModelBase : RemovableViewModelBase, IDataErrorInfo, ICheckErrorInfo
    {
        public readonly Model.Configuration.PlayerControllerBase PlayerControllerConfiguration;
        private readonly ObservableCollection<PlayerControllerBindingViewModelBase> _bindings = new ObservableCollection<PlayerControllerBindingViewModelBase>();

        public PlayerControllerViewModelBase(Model.Configuration.PlayerControllerBase playerControllerConfiguration)
        {
            PlayerControllerConfiguration = playerControllerConfiguration;
            AddBindingCommand = new UiCommand(AddBinging);
        }

        public IList<PlayerControllerBindingViewModelBase> Bindings => _bindings;

        public override bool IsValid()
        {
            return Bindings.All(binding => binding.IsValid());
        }

        public ICommand AddBindingCommand { get; }

        public abstract string DisplayName { get ; }

        public string this[string columnName] => ReadErrorInfo(columnName);

        public string Error => null;

        public event EventHandler<CheckErrorEventArgs> CheckErrorInfo;

        protected virtual string ReadErrorInfo(string propertyName)
        {
            if (CheckErrorInfo is null)
                return string.Empty;
            var checkErrorInfo = new CheckErrorEventArgs(this, propertyName);
            CheckErrorInfo(this, checkErrorInfo);
            return checkErrorInfo.Message;
        }

        protected abstract PlayerControllerBindingViewModelBase CreatePlayerControlerBindingViewModel(Model.Configuration.PlayerBindingBase bindingConfiguration = null);

        protected void AddBindingViewModel(PlayerControllerBindingViewModelBase vm)
        {
            vm.Modified += Binding_Modified;
            vm.RemoveRequested += Binding_RemoveRequested;
            Bindings.Add(vm);
        }

        private void AddBinging(object obj)
        {
            var vm = CreatePlayerControlerBindingViewModel();
            IsModified = true;
        }

        private void Binding_RemoveRequested(object sender, EventArgs e)
        {
            var vm = sender as PlayerControllerBindingViewModelBase ?? throw new ArgumentException(nameof(sender));
            vm.Modified -= Binding_Modified;
            vm.RemoveRequested -= Binding_RemoveRequested;
            Bindings.Remove(vm);
        }

        private void Binding_Modified(object sender, EventArgs e)
        {
            IsModified = true;
        }

    }
}