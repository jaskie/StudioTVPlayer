using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Main.Input
{
    public abstract class InputViewModelBase : RemovableViewModelBase, IDisposable, IDataErrorInfo
    {
        private readonly ObservableCollection<RecorderViewModel> _recorders = new ObservableCollection<RecorderViewModel>();

        public InputViewModelBase(Model.InputBase input)
        {
            Input = input;
            CommandAddRecorder = new UiCommand(AddRecorder);
        }

        public Model.InputBase Input { get; }

        public string Error => string.Empty;

        public string this[string columnName] => ReadErrorInfo(columnName);

        public abstract void Dispose();

        protected abstract string ReadErrorInfo(string propertyName);


        public ICommand CommandAddRecorder { get; }

        public IEnumerable<RecorderViewModel> Recorders => _recorders;

        private void AddRecorder(object obj)
        {
            var recorder = new RecorderViewModel(Input);
            recorder.RemoveRequested += Recorder_RemoveRequested;
            _recorders.Add(recorder);
        }

        private void Recorder_RemoveRequested(object sender, EventArgs e)
        {
            var recorder = sender as RecorderViewModel ?? throw new ArgumentException(nameof(sender));
            recorder.RemoveRequested -= Recorder_RemoveRequested;
            _recorders.Remove(recorder);
        }
    }
}
