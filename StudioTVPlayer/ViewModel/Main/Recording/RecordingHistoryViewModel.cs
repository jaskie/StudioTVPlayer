using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Windows.Data;

namespace StudioTVPlayer.ViewModel.Main.Recording
{
    public sealed class RecordingHistoryViewModel : ViewModelBase, IDisposable
    {
        private readonly IList<RecordingViewModel> _recordings;
        private bool _isDisposed;

        public RecordingHistoryViewModel()
        {
            _recordings = [.. Providers.RecordingStore.Current
                .LoadRecordings()
                .Select(CreateRecordingViewModel)];
            Recordings = CollectionViewSource.GetDefaultView(_recordings);
            Recordings.SortDescriptions.Add(new SortDescription(nameof(RecordingViewModel.StartTime), ListSortDirection.Descending));
        }

        public ICollectionView Recordings { get; }

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
            _recordings.Remove(recordingViewModel);
            Recordings.Refresh();
        }

        public void Dispose()
        {
            if (_isDisposed)
                return;
            _isDisposed = true;
            foreach (var recording in _recordings)
                recording.RemoveRequested -= RecordingViewModel_RemoveRequested;
        }
    }
}
