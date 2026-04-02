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
        private readonly IList<RecordingViewModel> _recordings;
        private bool _isDisposed;

        public RecordingHistoryViewModel()
        {
            _recordings = [.. Providers.RecordingStore.Current
                .Recordings
                .Select(CreateRecordingViewModel)];
            Providers.RecordingStore.Current.RecordingAdded += RecordingStore_RecordingAdded;
            Providers.RecordingStore.Current.RecordingDeleted += RecordingStore_RecordingDeleted;
            _recordingsView = CollectionViewSource.GetDefaultView(_recordings);
            _recordingsView.SortDescriptions.Add(new SortDescription(nameof(RecordingViewModel.StartTime), ListSortDirection.Descending));
        }

        private void RecordingStore_RecordingDeleted(object sender, Providers.RecordingEventArgs e)
        {
            OnUiThread(() =>
            {
                var recordingViewModel = _recordings.FirstOrDefault(vm => vm.Recording == e.Recording);
                if (recordingViewModel is null)
                    return;
                _recordings.Remove(recordingViewModel);
                _recordingsView.Refresh();
            });
        }

        private void RecordingStore_RecordingAdded(object sender, Providers.RecordingEventArgs e)
        {
            OnUiThread(() =>
            {
                _recordings.Add(CreateRecordingViewModel(e.Recording));
                _recordingsView.Refresh();
            });
        }

        public ICollectionView Recordings => _recordingsView;

        private RecordingViewModel CreateRecordingViewModel(Model.Recording recording)
        {
            var recordingViewModel = new RecordingViewModel(recording);
            recordingViewModel.RemoveRequested += RecordingViewModel_RemoveRequested;
            return recordingViewModel;
        }

        private void RecordingViewModel_RemoveRequested(object sender, EventArgs e)
        {
            var recordingViewModel = sender as RecordingViewModel ?? throw new ArgumentException($"{nameof(RecordingViewModel)} expected, {sender?.GetType()} got.");
            recordingViewModel.RemoveRequested -= RecordingViewModel_RemoveRequested;
            _recordings.Remove(recordingViewModel);
            _recordingsView.Refresh();
        }

        public void Dispose()
        {
            if (_isDisposed)
                return;
            _isDisposed = true;
            Providers.RecordingStore.Current.RecordingAdded -= RecordingStore_RecordingAdded;
            Providers.RecordingStore.Current.RecordingDeleted -= RecordingStore_RecordingDeleted;
            foreach (var recording in _recordings)
                recording.RemoveRequested -= RecordingViewModel_RemoveRequested;
        }
    }
}
