using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Main
{
    public class RecordingSchedulerViewModel : ViewModelBase
    {
        private readonly ObservableCollection<RecordingSchedulerItemViewModel> _items;
        private RecordingSchedulerItemViewModel _selectedItem;

        public RecordingSchedulerViewModel(IEnumerable<RecordingSchedulerItem> items)
        {
            AddRecordingScheduledItemCommand = new UiCommand(AddRecordingScheduledItem);
            _items = [.. items.Select(item =>
            {
                var newVm = new RecordingSchedulerItemViewModel(item);
                newVm.RemoveRequested += Item_RemoveRequested;
                return newVm;
            })];
            _items.CollectionChanged += Items_CollectionChanged;
        }

        private void Item_RemoveRequested(object sender, EventArgs e)
        {
            throw new NotImplementedException();
        }

        private void AddRecordingScheduledItem(object obj)
        {
            var newItem = new RecordingSchedulerItem();
            _items.Add(new RecordingSchedulerItemViewModel(newItem));
        }

        public ICommand AddRecordingScheduledItemCommand { get; }

        public IEnumerable<RecordingSchedulerItemViewModel> Items => _items;

        public RecordingSchedulerItemViewModel SelectedItem { get => _selectedItem; set => Set(ref _selectedItem, value); }
        
        private void Items_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            switch (e.Action)
            {
                case NotifyCollectionChangedAction.Add:
                    foreach (var item in e.NewItems)
                        (item as RecordingSchedulerItemViewModel).RemoveRequested += RecordingSchedulerViewModel_RemoveRequested;
                    break;
                case NotifyCollectionChangedAction.Remove:
                    foreach (var item in e.OldItems)
                        (item as RecordingSchedulerItemViewModel).RemoveRequested -= RecordingSchedulerViewModel_RemoveRequested;
                    break;
            }
        }

        private void RecordingSchedulerViewModel_RemoveRequested(object sender, EventArgs e)
        {
            var vm = sender as RecordingSchedulerItemViewModel ?? throw new ArgumentException(nameof(sender));
            _items.Remove(vm);
        }
    }
}
