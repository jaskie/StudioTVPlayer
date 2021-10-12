using System;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class AudioLevelBarViewModel: ViewModelBase
    {
        private double _audioLevel;

        public AudioLevelBarViewModel(double minValue, double maxValue)
        {
            MinValue = minValue;
            MaxValue = maxValue;
        }

        public double MinValue { get; }

        public double MaxValue { get; }

        public double AudioLevel
        {
            get => _audioLevel;
            set
            {
                if (Math.Abs(_audioLevel - value) < double.Epsilon)
                    return;
                _audioLevel = value;
                NotifyPropertyChanged();
            }
        }
    }
}