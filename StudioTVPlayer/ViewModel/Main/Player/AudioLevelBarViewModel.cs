using System;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class AudioLevelBarViewModel(double minValue, double maxValue) : ViewModelBase
    {
        private double _audioLevel;

        public double MinValue { get; } = minValue;

        public double MaxValue { get; } = maxValue;

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