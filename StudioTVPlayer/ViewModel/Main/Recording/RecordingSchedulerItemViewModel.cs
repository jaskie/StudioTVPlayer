using StudioTVPlayer.Helpers;
using StudioTVPlayer.ViewModel.Main.Input;
using StudioTVPlayer.ViewModel.Shared;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Main.Recording
{
    public sealed class RecordingSchedulerItemViewModel : RemovableViewModelBase, IDataErrorInfo, IDisposable
    {
        private readonly Model.RecordingSchedulerItem _recordingSchedulerItem;
        private string _name;
        private DateTime _startTime;
        private TimeSpan _duration;
        private Model.RecordingFilenameCreationRule _filenameCreationRule;
        private bool _isRepeatMonday;
        private bool _isRepeatTuesday;
        private bool _isRepeatWednesday;
        private bool _isRepeatThursday;
        private bool _isRepeatFriday;
        private bool _isRepeatSaturday;
        private bool _isRepeatSunday;
        private InputViewModelBase _selectedInput;
        private Model.EncoderPreset _selectedEncoderPreset;
        private Model.ScheduleRepeatType _selectedRepeatType;
        private bool _isDisposed;

        public RecordingSchedulerItemViewModel(Model.RecordingSchedulerItem recordingSchedulerItem)
        {
            Debug.Assert(recordingSchedulerItem is not null);
            _recordingSchedulerItem = recordingSchedulerItem;
            recordingSchedulerItem.PropertyChanged += RecordingSchedulerItem_PropertyChanged;
            UpdateCommand = new UiCommand(_ => Save(), CanSave);
            UndoCommand = new UiCommand(_ => Load(true), _ => IsModified);
            StartNowCommand = new UiCommand(StartNow, CanStartNow);
            DirectorySelector = new DirectorySelectorViewModel(recordingSchedulerItem.Directory);
            DirectorySelector.PropertyChanged += DirectorySelector_PropertyChanged;
            Load(false);
        }

        internal bool IsNew { get; set; }

        public event EventHandler Saved;

        internal Model.RecordingSchedulerItem RecordingScheduletItem => _recordingSchedulerItem;

        public ICommand UpdateCommand { get; }

        public ICommand UndoCommand { get; }

        public ICommand StartNowCommand { get; }

        public string Name { get => _name; set => Set(ref _name, value); }

        public DirectorySelectorViewModel DirectorySelector { get; }


        public static Array FilenameCreationRules { get; } = Enum.GetValues(typeof(Model.RecordingFilenameCreationRule));

        public Model.RecordingFilenameCreationRule FilenameCreationRule
        {
            get => _filenameCreationRule;
            set
            {
                if (!Set(ref _filenameCreationRule, value))
                    return;
                NotifyPropertyChanged(nameof(Name)); // we need re-validation of the Name
            }
        }

        public DateTime StartTime { get => _startTime; set => Set(ref _startTime, value); }

        public TimeSpan Duration { get => _duration; set => Set(ref _duration, value); }

        public Model.EncoderPreset SelectedEncoderPreset { get => _selectedEncoderPreset; set => Set(ref _selectedEncoderPreset, value); }

        public Model.EncoderPreset[] EncoderPresets => Model.EncoderPresets.Instance.Presets;

        public static Array RepeatTypes { get; } = Enum.GetValues(typeof(Model.ScheduleRepeatType));
        public Model.ScheduleRepeatType SelectedRepeatType { get => _selectedRepeatType; set => Set(ref _selectedRepeatType, value); }

        #region repeat days
        public bool IsRepeatMonday { get => _isRepeatMonday; set => Set(ref _isRepeatMonday, value); }
        public bool IsRepeatTuesday { get => _isRepeatTuesday; set => Set(ref _isRepeatTuesday, value); }
        public bool IsRepeatWednesday { get => _isRepeatWednesday; set => Set(ref _isRepeatWednesday, value); }
        public bool IsRepeatThursday { get => _isRepeatThursday; set => Set(ref _isRepeatThursday, value); }
        public bool IsRepeatFriday { get => _isRepeatFriday; set => Set(ref _isRepeatFriday, value); }
        public bool IsRepeatSaturday { get => _isRepeatSaturday; set => Set(ref _isRepeatSaturday, value); }
        public bool IsRepeatSunday { get => _isRepeatSunday; set => Set(ref _isRepeatSunday, value); }

        #endregion repeat days

        public IEnumerable<InputViewModelBase> Inputs { get; } = [.. Providers.InputList.Current.Inputs.Select(input =>
        {
            switch (input)
            {
                case Model.DecklinkInput decklinkInput:
                    return new DecklinkInputViewModel(decklinkInput);
                default:
                    throw new ApplicationException("Invalid model type provided");
            }
        })];

        public InputViewModelBase SelectedInput { get => _selectedInput; set => Set(ref _selectedInput, value); }

        public string Error => throw new NotImplementedException();

        public string this[string columnName]
        {
            get
            {
                switch (columnName)
                {
                    case nameof(SelectedInput):
                        if (SelectedInput is null)
                            return "Input is not selected";
                        if (!SelectedInput.Input.IsRunning)
                            return "Selected input is not running";
                        break;
                    case nameof(SelectedEncoderPreset) when SelectedEncoderPreset is null:
                        return "Preset is required to start recording";
                    case nameof(Name) when string.IsNullOrWhiteSpace(Name):
                        return "Name can't be empty";
                    case nameof(Name) when FilenameCreationRule != Model.RecordingFilenameCreationRule.UseNameAsFormat && Path.GetInvalidFileNameChars().Intersect(Name).Any():
                        return "Name contains character(s) not allowed in filename";
                }
                return null;
            }
        }

        private bool CanSave(object obj)
        {
            return IsModified && GetType().GetProperties().All(property => this[property.Name] is null);
        }

        public void Save()
        {
            _recordingSchedulerItem.Name = Name;
            _recordingSchedulerItem.Directory = DirectorySelector.DirectoryName;
            _recordingSchedulerItem.StartTime = StartTime;
            _recordingSchedulerItem.Duration = Duration;
            _recordingSchedulerItem.RepeatType = SelectedRepeatType;
            _recordingSchedulerItem.RepeatDays = SelectedRepeatType == Model.ScheduleRepeatType.Daily ? [.. SelectedRepeatDays] : null;
            _recordingSchedulerItem.FilenameCreationRule = FilenameCreationRule;
            _recordingSchedulerItem.InputId = SelectedInput?.Input?.Id;
            _recordingSchedulerItem.EncoderPreset = SelectedEncoderPreset.PresetName;
            IsModified = false;
            Saved?.Invoke(this, EventArgs.Empty);
        }

        private void Load(bool refreshUi)
        {
            _name = _recordingSchedulerItem.Name;
            DirectorySelector.DirectoryName = _recordingSchedulerItem.Directory;
            _startTime = _recordingSchedulerItem.StartTime;
            _duration = _recordingSchedulerItem.Duration;
            _selectedRepeatType = _recordingSchedulerItem.RepeatType;
            SelectedRepeatDays = _recordingSchedulerItem.RepeatDays;
            _filenameCreationRule = _recordingSchedulerItem.FilenameCreationRule;
            _selectedInput = Inputs.FirstOrDefault(vm => vm.Input?.Id == _recordingSchedulerItem.InputId);
            _selectedEncoderPreset = EncoderPresets.FirstOrDefault(preset => preset.PresetName == _recordingSchedulerItem.EncoderPreset);
            if (refreshUi)
                NotifyPropertyChanged(null);
            IsModified = false;
        }

        public override bool IsValid()
        {
            return _selectedInput != null;
        }

        public void Dispose()
        {
            if (_isDisposed)
                return;
            _isDisposed = true;
            _recordingSchedulerItem.PropertyChanged -= RecordingSchedulerItem_PropertyChanged;
        }

        private IEnumerable<DayOfWeek> SelectedRepeatDays
        {
            get
            {
                if (IsRepeatMonday) yield return DayOfWeek.Monday;
                if (IsRepeatTuesday) yield return DayOfWeek.Tuesday;
                if (IsRepeatWednesday) yield return DayOfWeek.Wednesday;
                if (IsRepeatThursday) yield return DayOfWeek.Thursday;
                if (IsRepeatFriday) yield return DayOfWeek.Friday;
                if (IsRepeatSaturday) yield return DayOfWeek.Saturday;
                if (IsRepeatSunday) yield return DayOfWeek.Sunday;
            }
            set
            {
                _isRepeatMonday = value?.Contains(DayOfWeek.Monday) ?? true;
                _isRepeatTuesday = value?.Contains(DayOfWeek.Tuesday) ?? true;
                _isRepeatWednesday = value?.Contains(DayOfWeek.Wednesday) ?? true;
                _isRepeatThursday = value?.Contains(DayOfWeek.Thursday) ?? true;
                _isRepeatFriday = value?.Contains(DayOfWeek.Friday) ?? true;
                _isRepeatSaturday = value?.Contains(DayOfWeek.Saturday) ?? true;
                _isRepeatSunday = value?.Contains(DayOfWeek.Sunday) ?? true;
            }
        }

        private bool CanStartNow(object _)
        {
            return !IsModified && !_recordingSchedulerItem.IsActive;
        }

        private async void StartNow(object _)
        {
            Save();
            _recordingSchedulerItem.IsActive = true;
            await Model.RecordingScheduler.Current.StartRecording(_recordingSchedulerItem, false);
        }

        private void RecordingSchedulerItem_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            switch (e.PropertyName)
            {
                case nameof(Model.RecordingSchedulerItem.IsActive):
                    Refresh();
                    break;
            }
        }

        private void DirectorySelector_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            IsModified = true;
        }

    }
}
