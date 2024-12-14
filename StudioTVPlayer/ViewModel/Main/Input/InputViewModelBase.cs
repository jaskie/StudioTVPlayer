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
        private readonly ObservableCollection<RecordingInstantViewModel> _recordings = new ObservableCollection<RecordingInstantViewModel>();

        public InputViewModelBase(Model.InputBase input)
        {
            Input = input;
            CommandAddRecordingInstant = new UiCommand(AddRecordingInstant);
            foreach(var recording in Providers.GlobalApplicationData.Current.Recordings.Where(r => r.Input == input))
            {
                switch(recording)
                {
                    case Model.RecordingInstant recordingInstant:
                        var recordingVm = new RecordingInstantViewModel(recordingInstant);
                        recordingVm.RemoveRequested += Recording_RemoveRequested;
                        _recordings.Add(recordingVm);
                        break;
                }
            }
        }

        public Model.InputBase Input { get; }

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

        public IEnumerable<RecordingInstantViewModel> Recordings => _recordings;

        private void AddRecordingInstant(object obj)
        {
            var recording = new RecordingInstantViewModel(Input);
            recording.RemoveRequested += Recording_RemoveRequested;
            _recordings.Add(recording);
        }

        private void Recording_RemoveRequested(object sender, EventArgs e)
        {
            var recording = sender as RecordingInstantViewModel ?? throw new ArgumentException(nameof(sender));
            recording.RemoveRequested -= Recording_RemoveRequested;
            _recordings.Remove(recording);
        }
    }
}
