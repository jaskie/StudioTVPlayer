using System;
using System.Collections.ObjectModel;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Main.Recording
{
    public sealed class RecordingHistoryViewModel : ViewModelBase
    {
        public RecordingHistoryViewModel()
        {
            Recordings = new ObservableCollection<RecordingViewModel>(Providers.RecordingStore.Current
                .LoadRecordings()
                .Select(CreateRecordingViewModel));
        }

        public ObservableCollection<RecordingViewModel> Recordings { get; }

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
        }
    }
}
