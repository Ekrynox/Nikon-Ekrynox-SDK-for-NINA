using NEKCS;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace LucasAlias.NINA.NEK.Dockables.Converters {
    public class GetFieldConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value == null || parameter is not string fieldName) return null;

            var type = value.GetType();

            var prop = type.GetProperty(fieldName, BindingFlags.Public | BindingFlags.Instance);
            if (prop != null) return prop.GetValue(value);

            var field = type.GetField(fieldName, BindingFlags.Public | BindingFlags.Instance);
            if (field != null) return field.GetValue(value);

            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) => throw new NotImplementedException();
    }
}
