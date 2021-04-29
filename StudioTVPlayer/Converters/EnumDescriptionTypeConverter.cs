using System;
using System.ComponentModel;
using System.Linq;

namespace StudioTVPlayer.Converters
{
    public class EnumDescriptionTypeConverter : EnumConverter
    {
        public EnumDescriptionTypeConverter(Type type)
        : base(type)
        { }

        public override object ConvertTo(ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value, Type destinationType)
        {
            if (destinationType == typeof(string))
                if (value.GetType().GetField(value.ToString())?.GetCustomAttributes(typeof(DescriptionAttribute), false).FirstOrDefault() is DescriptionAttribute attribute)
                    return attribute.Description;
            return base.ConvertTo(context, culture, value, destinationType);
        }
    }
}
