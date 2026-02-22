using MahApps.Metro.Controls.Dialogs;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Main.Recording
{
    public sealed class RecordingSchedulerViewModel : ViewModelBase, ICanClose
    {
        private readonly ObservableCollection<RecordingSchedulerItemViewModel> _items;
        private RecordingSchedulerItemViewModel _selectedItem;

        public RecordingSchedulerViewModel()
        {
            AddRecordingScheduledItemCommand = new UiCommand(AddRecordingScheduledItem);
            _items = [.. RecordingScheduler.Current.Recordings.Select(item =>
            {
                var newVm = new RecordingSchedulerItemViewModel(item);
                newVm.RemoveRequested += RecordingSchedulerItemViewModel_RemoveRequested;
                newVm.Saved += RecordingSchedulerItemViewModel_Saved;
                return newVm;
            })];
            _items.CollectionChanged += Items_CollectionChanged;
        }

        private void AddRecordingScheduledItem(object obj)
        {
            var newItem = new RecordingSchedulerItem();
            _items.Add(new RecordingSchedulerItemViewModel(newItem) { IsNew = true });
        }

        public ICommand AddRecordingScheduledItemCommand { get; }

        public IEnumerable<RecordingSchedulerItemViewModel> Items => _items;

        public RecordingSchedulerItemViewModel SelectedItem { get => _selectedItem; set => Set(ref _selectedItem, value); }

        private void Items_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            switch (e.Action)
            {
                case NotifyCollectionChangedAction.Add:
                    foreach (RecordingSchedulerItemViewModel item in e.NewItems)
                    {
                        item.RemoveRequested += RecordingSchedulerItemViewModel_RemoveRequested;
                        item.Saved += RecordingSchedulerItemViewModel_Saved;
                    }
                    break;
                case NotifyCollectionChangedAction.Remove:
                    foreach (RecordingSchedulerItemViewModel item in e.OldItems)
                    {
                        item.RemoveRequested -= RecordingSchedulerItemViewModel_RemoveRequested;
                        item.Saved -= RecordingSchedulerItemViewModel_Saved;
                    }
                    break;
            }
        }

        private void RecordingSchedulerItemViewModel_Saved(object sender, EventArgs e)
        {
            var vm = sender as RecordingSchedulerItemViewModel ?? throw new ArgumentException(nameof(sender));
            if (vm.IsNew)
            {
                RecordingScheduler.Current.AddRecording(vm.ModelItem);
                vm.IsNew = false;
            }
            SaveRecordingList();
        }

        private void RecordingSchedulerItemViewModel_RemoveRequested(object sender, EventArgs e)
        {
            var vm = sender as RecordingSchedulerItemViewModel ?? throw new ArgumentException(nameof(sender));
            if (vm.IsNew || RecordingScheduler.Current.RemoveRecording(vm.ModelItem))
            {
                _items.Remove(vm);
                SaveRecordingList();
            }
        }

        private void SaveRecordingList()
        {
            RecordingScheduler.Current.Save();
        }

        public async Task<bool> ConfirmCloseAsync()
        {
            if (_items.Any(item => item.IsModified))
            {
                var settings = new MetroDialogSettings()
                {
                    AffirmativeButtonText = "Save",
                    NegativeButtonText = "Don't save",
                    FirstAuxiliaryButtonText = "Cancel",
                    DefaultButtonFocus = MessageDialogResult.Affirmative
                };
                var result = await ShellViewModel.Instance.ShowMessageAsync("Unsaved changes", "There are unsaved changes in the recording scheduler.\nDo you want to save them before closing?", MessageDialogStyle.AffirmativeAndNegativeAndSingleAuxiliary, settings);
                switch (result)
                {
                    case MessageDialogResult.Affirmative:
                        foreach (var item in _items.Where(item => item.IsModified))
                            item.Save();
                        break;
                    case MessageDialogResult.Negative:
                        break;
                    case MessageDialogResult.FirstAuxiliary:
                        return false;
                }
            }
            return true;
        }
    }
}
