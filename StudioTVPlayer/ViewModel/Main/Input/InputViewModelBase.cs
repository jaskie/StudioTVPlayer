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
        private readonly ObservableCollection<RecordingViewModel> _recordings = new ObservableCollection<RecordingViewModel>();

        public InputViewModelBase(Model.InputBase input)
        {
            Input = input;
            CommandAddRecorder = new UiCommand(AddRecorder);
            foreach(var recording in Providers.GlobalApplicationData.Current.Recordings.Where(r => r.Input == input))
            {
                var recordingVm = new RecordingViewModel(recording);
                recordingVm.RemoveRequested += Recorder_RemoveRequested;
                _recordings.Add(recordingVm);
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

        public ICommand CommandAddRecorder { get; }

        public IEnumerable<RecordingViewModel> Recordings => _recordings;

        private void AddRecorder(object obj)
        {
            var recorder = new RecordingViewModel(Input);
            recorder.RemoveRequested += Recorder_RemoveRequested;
            _recordings.Add(recorder);
        }

        private void Recorder_RemoveRequested(object sender, EventArgs e)
        {
            var recorder = sender as RecordingViewModel ?? throw new ArgumentException(nameof(sender));
            recorder.RemoveRequested -= Recorder_RemoveRequested;
            _recordings.Remove(recorder);
        }
    }
}
