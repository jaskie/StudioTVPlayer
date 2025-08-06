using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace StudioTVPlayer.Converters
{
    public class TimeSpanToDateTimeConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is TimeSpan timeSpan)
            {
                return new DateTime(timeSpan.Ticks);
            }
            return DependencyProperty.UnsetValue;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is DateTime dateTime)
            {
                return new TimeSpan(dateTime.Ticks);
            }
            return DependencyProperty.UnsetValue;
        }
    }
}
