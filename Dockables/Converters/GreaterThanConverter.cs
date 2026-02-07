using Google.Protobuf.WellKnownTypes;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace LucasAlias.NINA.NEK.Dockables.Converters {
    public class GreaterThanConverter : IValueConverter, IMultiValueConverter {
        public object Convert(object value, System.Type targetType, object parameter, CultureInfo culture) {
            if (value == null || parameter == null) return false;

            if (value is IComparable left && parameter is IComparable right) {
                // If both are the same type, compare directly
                if (left.GetType() == right.GetType()) return left.CompareTo(right) > 0;

                // Try numeric comparison
                double leftNum, rightNum;
                if (TryToDouble(left, out leftNum) && TryToDouble(right, out rightNum)) return leftNum > rightNum;

                // Try string comparison
                return String.Compare(left.ToString(), right.ToString(), StringComparison.Ordinal) > 0;
            }

            return false;
        }

        private bool TryToDouble(object input, out double result) {
            try {
                result = System.Convert.ToDouble(input);
                return true;
            } catch {
                result = 0;
                return false;
            }
        }

        public object Convert(object[] values, System.Type targetType, object parameter, CultureInfo culture) {
            if (values.Length >= 2) {
                var value = values.Last();
                return Convert(values.Take(values.Length - 1).ToArray(), targetType, value, culture);
            }
            else if (values.Length == 1 && parameter != null) {
                return Convert(values.First(), targetType, parameter, culture);
            }

            return false;
        }

        public object ConvertBack(object value, System.Type targetType, object parameter, CultureInfo culture) => throw new NotImplementedException();
        public object[] ConvertBack(object value, System.Type[] targetTypes, object parameter, CultureInfo culture) => throw new NotImplementedException();
    }
}
