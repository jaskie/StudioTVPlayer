using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Windows.Data;

namespace StudioTVPlayer.ViewModel.Main.Recording
{
    public sealed class RecordingHistoryViewModel : ViewModelBase, IDisposable
    {
        private readonly ICollectionView _recordingsView;
        private bool _isDisposed;

        public RecordingHistoryViewModel()
        {
            Recordings = [.. Providers.RecordingStore.Current
                .LoadRecordings()
                .Select(CreateRecordingViewModel)];
            Providers.RecordingStore.Current.RecordingAdded += RecordingStore_RecordingAdded;
            Providers.RecordingStore.Current.RecordingDeleted += RecordingStore_RecordingDeleted;
            _recordingsView = CollectionViewSource.GetDefaultView(Recordings);
            _recordingsView.SortDescriptions.Add(new SortDescription(nameof(RecordingViewModel.StartTime), ListSortDirection.Descending));
        }

        private void RecordingStore_RecordingDeleted(object sender, Providers.RecordingEventArgs e)
        {
            OnUiThread(() =>
            {
                var recordingViewModel = Recordings.FirstOrDefault(vm => vm.Recording == e.Recording);
                if (recordingViewModel is null)
                    return;
                Recordings.Remove(recordingViewModel);
                _recordingsView.Refresh();
            });
        }

        private void RecordingStore_RecordingAdded(object sender, Providers.RecordingEventArgs e)
        {
            OnUiThread(() =>
            {
                Recordings.Add(CreateRecordingViewModel(e.Recording));
                _recordingsView.Refresh();
            });
        }

        public IList<RecordingViewModel> Recordings { get; }

        private RecordingViewModel CreateRecordingViewModel(Model.Recording recording)
        {
            var recordingViewModel = new RecordingViewModel(recording);
            recordingViewModel.RemoveRequested += RecordingViewModel_RemoveRequested;
            return recordingViewModel;
        }

        private void RecordingViewModel_RemoveRequested(object sender, EventArgs e)
        {
            var recordingViewModel = sender as RecordingViewModel ?? throw new ArgumentException(nameof(sender));
            recordingViewModel.RemoveRequested -= RecordingViewModel_RemoveRequested;
            Recordings.Remove(recordingViewModel);
            _recordingsView.Refresh();
        }

        public void Dispose()
        {
            if (_isDisposed)
                return;
            _isDisposed = true;
            Providers.RecordingStore.Current.RecordingAdded -= RecordingStore_RecordingAdded;
            Providers.RecordingStore.Current.RecordingDeleted -= RecordingStore_RecordingDeleted;
            foreach (var recording in Recordings)
                recording.RemoveRequested -= RecordingViewModel_RemoveRequested;
        }
    }
}
