using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace LucasAlias.NINA.NEK.Dockables.Converters.NikonDevicePropDesc
{
    class TryGetConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value is NEKCS.NikonDevicePropDescDS_Variant desc) {
                if (desc.TryGetInteger(out var intDesc)) return intDesc;
                else if (desc.TryGetUInteger(out var uintDesc)) return uintDesc;
                if (desc.TryGetArrayInteger(out var aintDesc)) return aintDesc;
                else if (desc.TryGetArrayUInteger(out var auintDesc)) return auintDesc;
                else if (desc.TryGetString(out var stringDesc)) return stringDesc;
            }
            else if (value.GetType().IsGenericType && value.GetType().GetGenericTypeDefinition() == typeof(NEKCS.NikonDevicePropDescDS<>)) {
                dynamic desc2 = value;
                return desc2;
            }

            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) => throw new NotImplementedException();
    }
}
