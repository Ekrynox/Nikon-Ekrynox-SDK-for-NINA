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

            string[] parts = fieldName.Split('.');

            object current = value;
            foreach (var part in parts) {
                var type = current.GetType();

                var prop = type.GetProperty(part, BindingFlags.Public | BindingFlags.Instance);
                if (prop != null) {
                    current = prop.GetValue(current);
                    continue;
                }

                var field = type.GetField(part, BindingFlags.Public | BindingFlags.Instance);
                if (field != null) {
                    current = field.GetValue(current);
                    continue;
                }

                return null; // Not found
            }

            return current;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) => throw new NotImplementedException();
    }
}
