using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace LucasAlias.NINA.NEK.Dockables.Converters.NikonDevicePropDesc
{
    public class GetTypeFormConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value == null) return null;

            if (value is NEKCS.NikonDevicePropDescDS_Variant desc) {
                if (desc.FormFlag == NEKCS.NikonMtpFormtypeCode.Empty) return desc.DataType.ToString();
                return desc.DataType.ToString() + $" ({ desc.FormFlag.ToString() })";
            } 
            else if (value.GetType().IsGenericType && value.GetType().GetGenericTypeDefinition() == typeof(NEKCS.NikonDevicePropDescDS<>)) {
                dynamic desc2 = value;
                if (desc2.FormFlag == NEKCS.NikonMtpFormtypeCode.Empty) return desc2.DataType.ToString();
                return desc2.DataType.ToString() + $" ({desc2.FormFlag.ToString()})";
            }

            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) => throw new NotImplementedException();
    }
}
