using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.ViewModel.Main.Input;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Main
{
    public class RecordingSchedulerItemViewModel : RemovableViewModelBase
    {
        private readonly RecordingSchedulerItem _item;
        private string _name;
        private DateTime _startTime;
        private TimeSpan _duration;
        private FilenameCreationRule _filenameCreationRule;
        private bool _isRepeatDaily;
        private bool _isRepeatMonday;
        private bool _isRepeatTuesday;
        private bool _isRepeatWednesday;
        private bool _isRepeatThursday;
        private bool _isRepeatFriday;
        private bool _isRepeatSaturday;
        private bool _isRepeatSunday;
        private InputViewModelBase _selectedInput;

        public RecordingSchedulerItemViewModel(RecordingSchedulerItem item)
        {
            _item = item ?? throw new ArgumentNullException(nameof(item));
            Load(false);
            UpdateCommand = new UiCommand(Update, _ => IsModified);
            UndoCommand = new UiCommand(_ => Load(true), _ => IsModified);
        }

        internal bool IsNew { get; set; }

        public event EventHandler Saved;

        public RecordingSchedulerItem Item => _item;

        public ICommand UpdateCommand { get; }

        public ICommand UndoCommand { get; }

        public string Name { get => _name; set => Set(ref _name, value); }

        public static Array FilenameCreationRules { get; } = Enum.GetValues(typeof(FilenameCreationRule));

        public FilenameCreationRule FilenameCreationRule { get => _filenameCreationRule; set => Set(ref _filenameCreationRule, value); }

        public DateTime StartTime { get => _startTime; set => Set(ref _startTime, value); }
        public TimeSpan Duration { get => _duration; set => Set(ref _duration, value); }

        public bool IsRepeatDaily
        {
            get => _isRepeatDaily;
            set
            {
                if (!Set(ref _isRepeatDaily, value))
                    return;
            }
        }

        #region repeat days
        public bool IsRepeatMonday { get => _isRepeatMonday; set => Set(ref _isRepeatMonday, value); }
        public bool IsRepeatTuesday { get => _isRepeatTuesday; set => Set(ref _isRepeatTuesday, value); }
        public bool IsRepeatWednesday { get => _isRepeatWednesday; set => Set(ref _isRepeatWednesday, value); }
        public bool IsRepeatThursday { get => _isRepeatThursday; set => Set(ref _isRepeatThursday, value); }
        public bool IsRepeatFriday { get => _isRepeatFriday; set => Set(ref _isRepeatFriday, value); }
        public bool IsRepeatSaturday { get => _isRepeatSaturday; set => Set(ref _isRepeatSaturday, value); }
        public bool IsRepeatSunday { get => _isRepeatSunday; set => Set(ref _isRepeatSunday, value); }

        #endregion repeat days

        public IEnumerable<InputViewModelBase> Inputs { get; } = Providers.InputList.Current.Inputs.Select(input =>
        {
            switch (input)
            {
                case DecklinkInput decklinkInput:
                    return new DecklinkInputViewModel(decklinkInput);
                default:
                    throw new ApplicationException("Invalid model type provided");
            }
        }).ToArray();

        public InputViewModelBase SelectedInput { get => _selectedInput; set => Set(ref _selectedInput, value); }

        private void Update(object _)
        {
            _item.Name = Name;
            _item.StartTime = StartTime;
            _item.Duration = Duration;
            _item.RepeatType = IsRepeatDaily ? ScheduleRepeatType.Daily : ScheduleRepeatType.Single;
            var repeatDays = new List<DayOfWeek>();
            if (IsRepeatMonday) repeatDays.Add(DayOfWeek.Monday);
            if (IsRepeatTuesday) repeatDays.Add(DayOfWeek.Tuesday);
            if (IsRepeatWednesday) repeatDays.Add(DayOfWeek.Wednesday);
            if (IsRepeatThursday) repeatDays.Add(DayOfWeek.Thursday);
            if (IsRepeatFriday) repeatDays.Add(DayOfWeek.Friday);
            if (IsRepeatSaturday) repeatDays.Add(DayOfWeek.Saturday);
            if (IsRepeatSunday) repeatDays.Add(DayOfWeek.Sunday);
            _item.RepeatDays = repeatDays.ToArray();
            _item.FilenameCreationRule = FilenameCreationRule;
            _item.InputId = SelectedInput?.Input?.Id;
            IsModified = false;
            Saved?.Invoke(this, EventArgs.Empty);
        }

        private void Load(bool refreshUi)
        {
            _name = _item.Name;
            _startTime = _item.StartTime;
            _duration = _item.Duration;
            _isRepeatDaily = _item.RepeatType == ScheduleRepeatType.Daily;
            _isRepeatMonday = _item.RepeatDays?.Contains(DayOfWeek.Monday) ?? true;
            _isRepeatTuesday = _item.RepeatDays?.Contains(DayOfWeek.Tuesday) ?? true;
            _isRepeatWednesday = _item.RepeatDays?.Contains(DayOfWeek.Wednesday) ?? true;
            _isRepeatThursday = _item.RepeatDays?.Contains(DayOfWeek.Thursday) ?? true;
            _isRepeatFriday = _item.RepeatDays?.Contains(DayOfWeek.Friday) ?? true;
            _isRepeatSaturday = _item.RepeatDays?.Contains(DayOfWeek.Saturday) ?? true;
            _isRepeatSunday = _item.RepeatDays?.Contains(DayOfWeek.Sunday) ?? true;
            _filenameCreationRule = _item.FilenameCreationRule;
            _selectedInput = Inputs.FirstOrDefault(vm => vm.Input?.Id == _item.InputId);
            if (refreshUi)
                NotifyPropertyChanged(null);
            IsModified = false;
        }

        public override bool IsValid()
        {
            return _selectedInput != null;
        }

    }
}
