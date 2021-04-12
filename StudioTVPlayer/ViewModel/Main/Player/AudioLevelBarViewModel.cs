using System;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class AudioLevelBarViewModel: ViewModelBase
    {
        private double _audioLevel;

        public static double MinValue { get; } = -60.0;

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