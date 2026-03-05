using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Main.Input
{
    public abstract class InputViewModelBase : RemovableViewModelBase, IDisposable, IDataErrorInfo
    {
        private readonly ObservableCollection<Recording.RecordingViewModel> _recordings = [];

        public InputViewModelBase(Model.InputBase input)
        {
            Input = input;
            CommandAddRecordingInstant = new UiCommand(AddRecordingInstant);
            foreach (var recording in Providers.RecordingStore.Current.RunningRecordings.Where(r => r.Input == input))
            {
                var recordingVm = new Recording.RecordingViewModel(recording);
                recordingVm.RemoveRequested += Recording_RemoveRequested;
                _recordings.Add(recordingVm);
            }
        }

        public Model.InputBase Input { get; }

        public abstract string DisplayName { get; }

        public string Error => string.Empty;

        public string this[string columnName] => ReadErrorInfo(columnName);

        public virtual void Dispose()
        {
            foreach (var recording in _recordings)
                recording.Dispose();
            _recordings.Clear();
        }

        protected abstract string ReadErrorInfo(string propertyName);

        public ICommand CommandAddRecordingInstant { get; }

        public IEnumerable<Recording.RecordingViewModel> Recordings => _recordings;

        private void AddRecordingInstant(object obj)
        {
            var recording = new Recording.RecordingViewModel(Input);
            recording.RemoveRequested += Recording_RemoveRequested;
            _recordings.Add(recording);
        }

        private void Recording_RemoveRequested(object sender, EventArgs e)
        {
            var recordingViewModel = sender as Recording.RecordingViewModel ?? throw new ArgumentException(nameof(sender));
            recordingViewModel.RemoveRequested -= Recording_RemoveRequested;
            _recordings.Remove(recordingViewModel);
        }
    }
}
