using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace StudioTVPlayer.Converters
{
    public class EnumToBooleanConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value != null && value.GetType().IsEnum)
                return Equals(value, parameter);
            else
                return DependencyProperty.UnsetValue;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value is bool v && v ? parameter : DependencyProperty.UnsetValue;
        }
    }
}
